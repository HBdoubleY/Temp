#define _GNU_SOURCE
#ifdef ENABLE_CARPLAY

#include "carplay_display.h"
#ifndef CP_USE_FFMPEG_DECODER
#define CP_USE_FFMPEG_DECODER 1
#endif
#if CP_USE_FFMPEG_DECODER
#include "carplay_ffmpeg_decoder.h"
#endif
#include "mpp_compat.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <g2d_driver.h>
#include <PIXEL_FORMAT_E_g2d_format_convert.h>

extern unsigned int zlink_client_perf_get_session_id(void);
extern int zlink_client_perf_is_enabled(void);
extern int zlink_client_perf_sample_n(void);
extern int zlink_client_perf_warn_us(void);

extern int libzlink_touch_event(int x, int y, int is_touch_down);

#include "carplay_thread_prio.h"

extern int g_g2dfd;

int carplay_touch_screen_x   = 0;
int carplay_touch_screen_y   = 0;
int carplay_touch_screen_down = 0;
int carplay_split_screen_enable = 0;
int carplay_split_line_x = 720;
#define H264_QUEUE_CAP      64
#define H264_QUEUE_EVICT_BATCH 24
#define H264_PACKET_MAX     (256 * 1024)

typedef enum {
	H264_NAL_OTHER = 0,
	H264_NAL_NON_KEY,
	H264_NAL_IDR,
	H264_NAL_PPS,
	H264_NAL_SPS,
} h264_nal_kind_t;

typedef struct {
	char *data;
	int len;
	h264_nal_kind_t kind;
} h264_packet_t;

static struct {
	h264_packet_t slot[H264_QUEUE_CAP];
	int head;
	int tail;
	int count;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	volatile int shutdown;
} g_queue;

#define FRAME_QUEUE_CAP 8
#define FFMPEG_FRAME_POOL_CAP 10
#define DECODE_DROP_FQ_BACKLOG 1
#define DECODE_DROP_H264Q_BACKLOG 24
#define DISPLAY_KEEP_FRAMES_DEFAULT 3
#define DISPLAY_KEEP_FRAMES_TOUCH 2
#define DISPLAY_DROP_THRESHOLD 5
#define CPD_PERF_PERIOD_US (3 * 1000000LL)

static struct {
	unsigned int phy_addr[2];
	void *vir_addr[2];
	int in_use;
	VIDEO_FRAME_INFO_S frame;
} g_ffmpeg_pool[FFMPEG_FRAME_POOL_CAP];

static struct {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
} g_ffmpeg_pool_sync;
static volatile int g_ffmpeg_pool_running = 0;

static struct {
	VIDEO_FRAME_INFO_S slot[FRAME_QUEUE_CAP];
	int head;
	int tail;
	int count;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	volatile int shutdown;
} g_fq;

static void fq_init(void)
{
	memset(&g_fq, 0, sizeof(g_fq));
	pthread_mutex_init(&g_fq.mutex, NULL);
	pthread_cond_init(&g_fq.cond, NULL);
}

static void fq_shutdown(void)
{
	pthread_mutex_lock(&g_fq.mutex);
	g_fq.shutdown = 1;
	pthread_cond_broadcast(&g_fq.cond);
	pthread_mutex_unlock(&g_fq.mutex);
}

static void fq_destroy(void)
{
	pthread_mutex_destroy(&g_fq.mutex);
	pthread_cond_destroy(&g_fq.cond);
}

#if CP_USE_FFMPEG_DECODER
static int ffmpeg_pool_init(int width, int height)
{
	int aligned_w = AWALIGN(width, 16);
	int aligned_h = AWALIGN(height, 16);
	int y_size = aligned_w * aligned_h;
	int uv_size = aligned_w * aligned_h / 2;

	memset(g_ffmpeg_pool, 0, sizeof(g_ffmpeg_pool));
	pthread_mutex_init(&g_ffmpeg_pool_sync.mutex, NULL);
	pthread_cond_init(&g_ffmpeg_pool_sync.cond, NULL);

	for (int i = 0; i < FFMPEG_FRAME_POOL_CAP; i++) {
		VIDEO_FRAME_INFO_S *f = &g_ffmpeg_pool[i].frame;
		memset(f, 0, sizeof(*f));
		if (AW_MPI_SYS_MmzAlloc_Cached(&g_ffmpeg_pool[i].phy_addr[0], &g_ffmpeg_pool[i].vir_addr[0], y_size) != SUCCESS)
			goto fail;
		if (AW_MPI_SYS_MmzAlloc_Cached(&g_ffmpeg_pool[i].phy_addr[1], &g_ffmpeg_pool[i].vir_addr[1], uv_size) != SUCCESS)
			goto fail;

		f->VFrame.mWidth = aligned_w;
		f->VFrame.mHeight = aligned_h;
		f->VFrame.mPixelFormat = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
		f->VFrame.mPhyAddr[0] = g_ffmpeg_pool[i].phy_addr[0];
		f->VFrame.mPhyAddr[1] = g_ffmpeg_pool[i].phy_addr[1];
		f->VFrame.mpVirAddr[0] = g_ffmpeg_pool[i].vir_addr[0];
		f->VFrame.mpVirAddr[1] = g_ffmpeg_pool[i].vir_addr[1];
		f->VFrame.mOffsetTop = 0;
		f->VFrame.mOffsetBottom = aligned_h;
		f->VFrame.mOffsetLeft = 0;
		f->VFrame.mOffsetRight = aligned_w;
	}
	return 0;
fail:
	for (int i = 0; i < FFMPEG_FRAME_POOL_CAP; i++) {
		if (g_ffmpeg_pool[i].phy_addr[0])
			AW_MPI_SYS_MmzFree(g_ffmpeg_pool[i].phy_addr[0], g_ffmpeg_pool[i].vir_addr[0]);
		if (g_ffmpeg_pool[i].phy_addr[1])
			AW_MPI_SYS_MmzFree(g_ffmpeg_pool[i].phy_addr[1], g_ffmpeg_pool[i].vir_addr[1]);
	}
	pthread_mutex_destroy(&g_ffmpeg_pool_sync.mutex);
	pthread_cond_destroy(&g_ffmpeg_pool_sync.cond);
	memset(g_ffmpeg_pool, 0, sizeof(g_ffmpeg_pool));
	return -1;
}

static void ffmpeg_pool_destroy(void)
{
	for (int i = 0; i < FFMPEG_FRAME_POOL_CAP; i++) {
		if (g_ffmpeg_pool[i].phy_addr[0])
			AW_MPI_SYS_MmzFree(g_ffmpeg_pool[i].phy_addr[0], g_ffmpeg_pool[i].vir_addr[0]);
		if (g_ffmpeg_pool[i].phy_addr[1])
			AW_MPI_SYS_MmzFree(g_ffmpeg_pool[i].phy_addr[1], g_ffmpeg_pool[i].vir_addr[1]);
		memset(&g_ffmpeg_pool[i], 0, sizeof(g_ffmpeg_pool[i]));
	}
	pthread_mutex_destroy(&g_ffmpeg_pool_sync.mutex);
	pthread_cond_destroy(&g_ffmpeg_pool_sync.cond);
}

static VIDEO_FRAME_INFO_S *ffmpeg_pool_acquire(void)
{
	VIDEO_FRAME_INFO_S *ret = NULL;

	pthread_mutex_lock(&g_ffmpeg_pool_sync.mutex);
	while (g_ffmpeg_pool_running && !ret) {
		for (int i = 0; i < FFMPEG_FRAME_POOL_CAP; i++) {
			if (!g_ffmpeg_pool[i].in_use) {
				g_ffmpeg_pool[i].in_use = 1;
				ret = &g_ffmpeg_pool[i].frame;
				break;
			}
		}
		if (!ret)
			pthread_cond_wait(&g_ffmpeg_pool_sync.cond, &g_ffmpeg_pool_sync.mutex);
	}
	pthread_mutex_unlock(&g_ffmpeg_pool_sync.mutex);
	return ret;
}

static void ffmpeg_pool_release(VIDEO_FRAME_INFO_S *frame)
{
	if (!frame)
		return;

	pthread_mutex_lock(&g_ffmpeg_pool_sync.mutex);
	for (int i = 0; i < FFMPEG_FRAME_POOL_CAP; i++) {
		if (g_ffmpeg_pool[i].frame.VFrame.mPhyAddr[0] == frame->VFrame.mPhyAddr[0] &&
		    g_ffmpeg_pool[i].frame.VFrame.mPhyAddr[1] == frame->VFrame.mPhyAddr[1]) {
			g_ffmpeg_pool[i].in_use = 0;
			pthread_cond_signal(&g_ffmpeg_pool_sync.cond);
			break;
		}
	}
	pthread_mutex_unlock(&g_ffmpeg_pool_sync.mutex);
}

static int ffmpeg_frame_to_vo_frame(const carplay_ffmpeg_frame_t *src, VIDEO_FRAME_INFO_S *dst)
{
	int h;
	int w;
	int uv_h;
	unsigned char *dst_y;
	unsigned char *dst_vu;
	const unsigned char *src_y;
	const unsigned char *src_vu;

	if (!src || !dst || !src->data[0] || !src->data[1] || src->width <= 0 || src->height <= 0)
		return -1;

	w = src->width;
	h = src->height;
	uv_h = h / 2;
	dst_y = (unsigned char *)dst->VFrame.mpVirAddr[0];
	dst_vu = (unsigned char *)dst->VFrame.mpVirAddr[1];
	src_y = (const unsigned char *)src->data[0];
	src_vu = (const unsigned char *)src->data[1];

	dst->VFrame.mWidth = w;
	dst->VFrame.mHeight = h;
	dst->VFrame.mOffsetTop = 0;
	dst->VFrame.mOffsetBottom = h;
	dst->VFrame.mOffsetLeft = 0;
	dst->VFrame.mOffsetRight = w;

	if (src->linesize[0] == w) {
		memcpy(dst_y, src_y, (size_t)w * h);
	} else {
		for (int r = 0; r < h; r++)
			memcpy(dst_y + (size_t)r * w, src_y + (size_t)r * src->linesize[0], (size_t)w);
	}
	if (src->linesize[1] == w) {
		memcpy(dst_vu, src_vu, (size_t)w * uv_h);
	} else {
		for (int r = 0; r < uv_h; r++)
			memcpy(dst_vu + (size_t)r * w, src_vu + (size_t)r * src->linesize[1], (size_t)w);
	}

	AW_MPI_SYS_MmzFlushCache(dst->VFrame.mPhyAddr[0], dst->VFrame.mpVirAddr[0], w * h);
	AW_MPI_SYS_MmzFlushCache(dst->VFrame.mPhyAddr[1], dst->VFrame.mpVirAddr[1], w * uv_h);
	return 0;
}
#endif

static struct {
	int disp_x, disp_y, disp_width, disp_height;
	int session_width, session_height;
	int vo_dev;
	int vo_layer;
	int vo_chn;
#if !CP_USE_FFMPEG_DECODER
	int vdec_chn;
	unsigned int stream_buf_phy;
	void *stream_buf_vir;
	size_t stream_buf_size;
#endif
	int clock_chn;
	pthread_t decode_tid;
	pthread_t display_tid;
	volatile int running;
#if CP_USE_FFMPEG_DECODER
	carplay_ffmpeg_decoder_t *ffmpeg_dec;
#endif
	pthread_mutex_t rect_mutex;
	VIDEO_FRAME_INFO_S g2d_dst[3];
	int g2d_dst_allocated;
	int g2d_buf_idx;
	pthread_mutex_t g2d_use_mutex;
	pthread_cond_t  g2d_use_cond;
	int g2d_dst_in_use[3];
	volatile int got_idr;
} g_ctx;
static unsigned long long g_h264_seq = 0;
static unsigned long long g_decode_seq = 0;
static unsigned long long g_frame_seq = 0;
static unsigned long long g_queue_drop = 0;
static unsigned long long g_queue_evict_total = 0;
static unsigned long long g_queue_evict_key = 0;
static unsigned long long g_queue_push_nomem = 0;
static unsigned long long g_fq_drop = 0;
static unsigned long long g_disp_fq_skip = 0;
static long long g_last_vo_release_us = 0;
static long long g_last_touch_send_us = 0;

static struct {
	long long last_print_us;
	unsigned long long dec_in;
	unsigned long long dec_out;
	unsigned long long dec_drop_bp;
	unsigned long long dec_drop_nonkey;
	unsigned long long dec_drop_touch;
	unsigned long long disp;
	unsigned long long fq_drop;
	unsigned long long h264_evict;
	unsigned long long h264_drop;
	unsigned long long send_fail;
	unsigned long long g2d_total_us;
	unsigned long long vo_total_us;
	unsigned long long disp_fq_skip;
	unsigned long long send_total_us;
	unsigned long long pool_wait_max_us;
} g_pstat;

static long long cp_now_us(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (long long)tv.tv_sec * 1000000LL + (long long)tv.tv_usec;
}

static long long cp_tid(void)
{
	return (long long)syscall(SYS_gettid);
}

static int cpd_touch_recently_active(void)
{
	long long now = cp_now_us();
	long long dt = now - g_last_touch_send_us;
	return (dt >= 0 && dt <= 180000) ? 1 : 0;
}

static void cpd_perf_maybe_print(void)
{
	long long now = cp_now_us();
	if (g_pstat.last_print_us == 0) {
		g_pstat.last_print_us = now;
		return;
	}
	long long elapsed = now - g_pstat.last_print_us;
	if (elapsed < CPD_PERF_PERIOD_US)
		return;
	unsigned long long g2d_avg = g_pstat.disp > 0 ? g_pstat.g2d_total_us / g_pstat.disp : 0;
	unsigned long long vo_avg = g_pstat.disp > 0 ? g_pstat.vo_total_us / g_pstat.disp : 0;
	unsigned long long send_avg = g_pstat.dec_in > 0 ? g_pstat.send_total_us / g_pstat.dec_in : 0;
	printf("[carplay_perf] period=%lldms dec_in=%llu dec_out=%llu dec_drop_bp=%llu dec_drop_nonkey=%llu dec_drop_touch=%llu disp=%llu fq_drop=%llu fq_skip=%llu h264_evict=%llu h264_drop=%llu send_fail=%llu send_avg_us=%llu g2d_avg_us=%llu vo_avg_us=%llu pool_wait_max_us=%llu\n",
	       elapsed / 1000LL,
	       g_pstat.dec_in, g_pstat.dec_out, g_pstat.dec_drop_bp, g_pstat.dec_drop_nonkey, g_pstat.dec_drop_touch, g_pstat.disp,
	       g_pstat.fq_drop, g_pstat.disp_fq_skip,
	       g_pstat.h264_evict, g_pstat.h264_drop, g_pstat.send_fail,
	       send_avg, g2d_avg, vo_avg, g_pstat.pool_wait_max_us);
	memset(&g_pstat, 0, sizeof(g_pstat));
	g_pstat.last_print_us = now;
}

static int cpd_env_int(const char *key, int def)
{
	const char *s = getenv(key);
	if (!s || s[0] == '\0')
		return def;
	return atoi(s);
}

static int cpd_env_opt_int(const char *key, int *out)
{
	const char *s = getenv(key);
	if (!s || s[0] == '\0')
		return 0;
	*out = atoi(s);
	return 1;
}

static int cpd_guess_session_fps(void)
{
	int fps = cpd_env_int("ZLINK_SESSION_FPS", 20);
	if (fps < 15 || fps > 30)
		fps = 20;
	return fps;
}

static const char *h264_nal_kind_str(h264_nal_kind_t kind)
{
	switch (kind) {
	case H264_NAL_SPS:
		return "sps";
	case H264_NAL_PPS:
		return "pps";
	case H264_NAL_IDR:
		return "idr";
	case H264_NAL_NON_KEY:
		return "non_key";
	default:
		return "other";
	}
}

static int h264_nal_is_key_related(h264_nal_kind_t kind)
{
	return kind == H264_NAL_SPS || kind == H264_NAL_PPS || kind == H264_NAL_IDR;
}

static int h264_nal_is_evict_preferred(h264_nal_kind_t kind)
{
	return kind == H264_NAL_NON_KEY || kind == H264_NAL_OTHER;
}

static h264_nal_kind_t h264_classify_packet(const char *data, int len)
{
	const unsigned char *d = (const unsigned char *)data;
	h264_nal_kind_t best = H264_NAL_OTHER;

	for (int i = 0; i + 4 < len; i++) {
		if (d[i] != 0 || d[i + 1] != 0)
			continue;
		if (d[i + 2] != 1 && !(d[i + 2] == 0 && d[i + 3] == 1))
			continue;

		int nal_off = (d[i + 2] == 1) ? i + 3 : i + 4;
		if (nal_off >= len)
			continue;

		switch (d[nal_off] & 0x1F) {
		case 7:
			return H264_NAL_SPS;
		case 8:
			if (best < H264_NAL_PPS)
				best = H264_NAL_PPS;
			break;
		case 5:
			if (best < H264_NAL_IDR)
				best = H264_NAL_IDR;
			break;
		case 1:
		case 2:
		case 3:
		case 4:
			if (best < H264_NAL_NON_KEY)
				best = H264_NAL_NON_KEY;
			break;
		default:
			break;
		}
	}

	return best;
}

#define CPD_LOG(stage, fmt, ...) \
	do { \
		if (zlink_client_perf_is_enabled()) { \
			printf("[cp_perf] ts_us=%lld tid=%lld stage=%s sid=%u " fmt "\n", \
			       cp_now_us(), cp_tid(), stage, zlink_client_perf_get_session_id(), ##__VA_ARGS__); \
		} \
	} while (0)

#define VO_LAYER_DEFAULT  0
#define VO_CHN_DEFAULT     0
#define VDEC_CHN_DEFAULT   0
#define CLOCK_CHN_DEFAULT  0

#ifndef AWALIGN
#define AWALIGN(x, a)  ((a) * (((x) + (a) - 1) / (a)))
#endif

static void queue_init(void)
{
	pthread_mutex_init(&g_queue.mutex, NULL);
	pthread_cond_init(&g_queue.cond, NULL);
	g_queue.head = g_queue.tail = g_queue.count = 0;
	g_queue.shutdown = 0;
}

static void queue_fini(void)
{
	pthread_mutex_lock(&g_queue.mutex);
	for (int i = 0; i < H264_QUEUE_CAP; i++) {
		if (g_queue.slot[i].data) {
			free(g_queue.slot[i].data);
			g_queue.slot[i].data = NULL;
		}
	}
	g_queue.count = 0;
	g_queue.shutdown = 1;
	pthread_cond_broadcast(&g_queue.cond);
	pthread_mutex_unlock(&g_queue.mutex);
}

static void queue_destroy(void)
{
	pthread_mutex_destroy(&g_queue.mutex);
	pthread_cond_destroy(&g_queue.cond);
}

static int queue_evict_old_packets_locked(int target_free, h264_nal_kind_t incoming_kind, int incoming_len)
{
	h264_packet_t kept[H264_QUEUE_CAP];
	unsigned char evict[H264_QUEUE_CAP];
	int logical_count = g_queue.count;
	int evicted = 0;
	int evicted_key = 0;
	int kept_count = 0;

	if (logical_count <= 0 || target_free <= 0)
		return 0;

	memset(evict, 0, sizeof(evict));

	for (int i = 0; i < logical_count && evicted < target_free; i++) {
		int idx = (g_queue.head + i) % H264_QUEUE_CAP;
		if (h264_nal_is_evict_preferred(g_queue.slot[idx].kind)) {
			evict[i] = 1;
			evicted++;
		}
	}

	for (int i = 0; i < logical_count && evicted < target_free; i++) {
		if (evict[i])
			continue;
		evict[i] = 1;
		evicted++;
	}

	for (int i = 0; i < logical_count; i++) {
		int idx = (g_queue.head + i) % H264_QUEUE_CAP;
		h264_packet_t pkt = g_queue.slot[idx];

		if (evict[i]) {
			if (h264_nal_is_key_related(pkt.kind))
				evicted_key++;
			free(pkt.data);
		} else {
			kept[kept_count++] = pkt;
		}
	}

	memset(g_queue.slot, 0, sizeof(g_queue.slot));
	for (int i = 0; i < kept_count; i++)
		g_queue.slot[i] = kept[i];

	g_queue.head = 0;
	g_queue.count = kept_count;
	g_queue.tail = kept_count % H264_QUEUE_CAP;
	g_queue_evict_total += (unsigned long long)evicted;
	g_queue_evict_key += (unsigned long long)evicted_key;
	g_pstat.h264_evict += (unsigned long long)evicted;

	if (evicted_key > 0) {
		g_ctx.got_idr = 0;
		printf("[carplay_display] h264_evict_key_warn evicted=%d key=%d depth=%d incoming=%s total_key=%llu\n",
		       evicted, evicted_key, g_queue.count, h264_nal_kind_str(incoming_kind), g_queue_evict_key);
	}
	CPD_LOG("h264_queue_evict", "evicted=%d key=%d depth=%d incoming_kind=%s total_evicted=%llu total_key=%llu",
	        evicted, evicted_key, g_queue.count, h264_nal_kind_str(incoming_kind),
	        g_queue_evict_total, g_queue_evict_key);
	return evicted;
}

static int queue_push(const char *data, int len)
{
	long long t0 = cp_now_us();
	h264_nal_kind_t kind;

	if (len <= 0 || len > H264_PACKET_MAX)
		return -1;
	kind = h264_classify_packet(data, len);
	pthread_mutex_lock(&g_queue.mutex);
	if (g_queue.count >= H264_QUEUE_CAP) {
		queue_evict_old_packets_locked(H264_QUEUE_EVICT_BATCH, kind, len);
		if (g_queue.count >= H264_QUEUE_CAP) {
			g_queue_drop++;
			g_pstat.h264_drop++;
			CPD_LOG("h264_queue_drop", "seq=%llu q_count=%d len=%d kind=%s total_drop=%llu",
			        g_h264_seq, g_queue.count, len, h264_nal_kind_str(kind), g_queue_drop);
			pthread_mutex_unlock(&g_queue.mutex);
			return -1;
		}
	}
	char *copy = (char *)malloc((size_t)len);
	if (!copy) {
		g_queue_push_nomem++;
		g_queue_drop++;
		g_pstat.h264_drop++;
		pthread_mutex_unlock(&g_queue.mutex);
		return -1;
	}
	memcpy(copy, data, (size_t)len);
	g_queue.slot[g_queue.tail].data = copy;
	g_queue.slot[g_queue.tail].len  = len;
	g_queue.slot[g_queue.tail].kind = kind;
	g_queue.tail = (g_queue.tail + 1) % H264_QUEUE_CAP;
	g_queue.count++;
	g_h264_seq++;
	if ((g_h264_seq % (unsigned long long)zlink_client_perf_sample_n()) == 0ULL) {
		CPD_LOG("h264_queue_push", "seq=%llu q_count=%d len=%d kind=%s cost_us=%lld",
		        g_h264_seq, g_queue.count, len, h264_nal_kind_str(kind), cp_now_us() - t0);
	}
	pthread_cond_signal(&g_queue.cond);
	pthread_mutex_unlock(&g_queue.mutex);
	return 0;
}

static int queue_pop(h264_packet_t *out)
{
	long long wait_start = cp_now_us();
	pthread_mutex_lock(&g_queue.mutex);
	while (g_queue.count == 0 && !g_queue.shutdown)
		pthread_cond_wait(&g_queue.cond, &g_queue.mutex);
	if (g_queue.shutdown && g_queue.count == 0) {
		pthread_mutex_unlock(&g_queue.mutex);
		return -1;
	}
	*out = g_queue.slot[g_queue.head];
	g_queue.slot[g_queue.head].data = NULL;
	g_queue.head = (g_queue.head + 1) % H264_QUEUE_CAP;
	g_queue.count--;
	if ((g_decode_seq % (unsigned long long)zlink_client_perf_sample_n()) == 0ULL) {
		CPD_LOG("h264_queue_pop", "q_count=%d wait_us=%lld",
		        g_queue.count, cp_now_us() - wait_start);
	}
	pthread_mutex_unlock(&g_queue.mutex);
	return 0;
}

static int queue_count_get(void)
{
	int count;
	pthread_mutex_lock(&g_queue.mutex);
	count = g_queue.count;
	pthread_mutex_unlock(&g_queue.mutex);
	return count;
}

static void decoder_release_frame(VIDEO_FRAME_INFO_S *frame)
{
#if CP_USE_FFMPEG_DECODER
	ffmpeg_pool_release(frame);
#else
	AW_MPI_VDEC_ReleaseImage(g_ctx.vdec_chn, frame);
#endif
}

static int g2d_alloc_one(VIDEO_FRAME_INFO_S *dst, int aligned_w, int aligned_h, PIXEL_FORMAT_E pix_fmt)
{
	int y_size = aligned_w * aligned_h;
	int uv_size = aligned_w * aligned_h / 2;
	ERRORTYPE ret;

	memset(dst, 0, sizeof(*dst));
	dst->VFrame.mWidth = aligned_w;
	dst->VFrame.mHeight = aligned_h;
	dst->VFrame.mPixelFormat = pix_fmt;

	ret = AW_MPI_SYS_MmzAlloc_Cached(&dst->VFrame.mPhyAddr[0], &dst->VFrame.mpVirAddr[0], y_size);
	if (ret != SUCCESS)
		return -1;
	ret = AW_MPI_SYS_MmzAlloc_Cached(&dst->VFrame.mPhyAddr[1], &dst->VFrame.mpVirAddr[1], uv_size);
	if (ret != SUCCESS) {
		AW_MPI_SYS_MmzFree(dst->VFrame.mPhyAddr[0], dst->VFrame.mpVirAddr[0]);
		return -1;
	}
	dst->VFrame.mOffsetTop = 0;
	dst->VFrame.mOffsetBottom = aligned_h;
	dst->VFrame.mOffsetLeft = 0;
	dst->VFrame.mOffsetRight = aligned_w;
	return 0;
}

static void g2d_free_one(VIDEO_FRAME_INFO_S *dst);

static int g2d_alloc_dst(int dst_w, int dst_h, PIXEL_FORMAT_E pix_fmt)
{
	if (g_ctx.g2d_dst_allocated)
		return 0;

	int aligned_w = AWALIGN(dst_w, 16);
	int aligned_h = AWALIGN(dst_h, 16);

	for (int i = 0; i < 3; i++) {
		if (g2d_alloc_one(&g_ctx.g2d_dst[i], aligned_w, aligned_h, pix_fmt) != 0) {
			printf("[carplay_display] G2D dst buf%d alloc failed\n", i);
			for (int j = 0; j < i; j++)
				g2d_free_one(&g_ctx.g2d_dst[j]);
			return -1;
		}
	}

	g_ctx.g2d_buf_idx = 0;
	g_ctx.g2d_dst_allocated = 1;
	printf("[carplay_display] G2D triple-buffer allocated %dx%d\n", aligned_w, aligned_h);
	return 0;
}

static void g2d_free_one(VIDEO_FRAME_INFO_S *dst)
{
	if (dst->VFrame.mPhyAddr[0])
		AW_MPI_SYS_MmzFree(dst->VFrame.mPhyAddr[0], dst->VFrame.mpVirAddr[0]);
	if (dst->VFrame.mPhyAddr[1])
		AW_MPI_SYS_MmzFree(dst->VFrame.mPhyAddr[1], dst->VFrame.mpVirAddr[1]);
	memset(dst, 0, sizeof(*dst));
}

static void g2d_free_dst(void)
{
	if (!g_ctx.g2d_dst_allocated)
		return;
	for (int i = 0; i < 3; i++)
		g2d_free_one(&g_ctx.g2d_dst[i]);
	g_ctx.g2d_dst_allocated = 0;
}

static int g2d_rotate_frame(VIDEO_FRAME_INFO_S *src, VIDEO_FRAME_INFO_S *dst)
{
	if (g_g2dfd <= 0)
		return -1;

	int src_w = src->VFrame.mWidth;
	int src_h = src->VFrame.mHeight;
	int src_left = src->VFrame.mOffsetLeft;
	int src_top = src->VFrame.mOffsetTop;
	int src_right = src->VFrame.mOffsetRight;
	int src_bottom = src->VFrame.mOffsetBottom;
	if (src_left < 0 || src_left >= src_w) src_left = 0;
	if (src_top < 0 || src_top >= src_h) src_top = 0;
	if (src_right <= src_left || src_right > src_w) src_right = src_w;
	if (src_bottom <= src_top || src_bottom > src_h) src_bottom = src_h;
	int vis_w = src_right - src_left;
	int vis_h = src_bottom - src_top;
	if (vis_w <= 0 || vis_h <= 0) {
		src_left = 0;
		src_top = 0;
		vis_w = src_w;
		vis_h = src_h;
	}

	g2d_fmt_enh src_fmt, dst_fmt;
	if (convert_PIXEL_FORMAT_E_to_g2d_fmt_enh(src->VFrame.mPixelFormat, &src_fmt) != SUCCESS)
		return -1;
	if (convert_PIXEL_FORMAT_E_to_g2d_fmt_enh(dst->VFrame.mPixelFormat, &dst_fmt) != SUCCESS)
		return -1;

	g2d_blt_h blit;
	memset(&blit, 0, sizeof(g2d_blt_h));
	blit.flag_h = G2D_ROT_270;

	blit.src_image_h.format = src_fmt;
	blit.src_image_h.laddr[0] = src->VFrame.mPhyAddr[0];
	blit.src_image_h.laddr[1] = src->VFrame.mPhyAddr[1];
	blit.src_image_h.laddr[2] = src->VFrame.mPhyAddr[2];
	blit.src_image_h.width = src_w;
	blit.src_image_h.height = src_h;
	blit.src_image_h.align[0] = 0;
	blit.src_image_h.align[1] = 0;
	blit.src_image_h.align[2] = 0;
	blit.src_image_h.clip_rect.x = src_left;
	blit.src_image_h.clip_rect.y = src_top;
	blit.src_image_h.clip_rect.w = vis_w;
	blit.src_image_h.clip_rect.h = vis_h;
	blit.src_image_h.gamut = G2D_BT709;
	blit.src_image_h.bpremul = 0;
	blit.src_image_h.mode = G2D_PIXEL_ALPHA;
	blit.src_image_h.alpha = 255;
	blit.src_image_h.fd = -1;
	blit.src_image_h.use_phy_addr = 1;

	blit.dst_image_h.format = dst_fmt;
	blit.dst_image_h.laddr[0] = dst->VFrame.mPhyAddr[0];
	blit.dst_image_h.laddr[1] = dst->VFrame.mPhyAddr[1];
	blit.dst_image_h.laddr[2] = dst->VFrame.mPhyAddr[2];
	blit.dst_image_h.width = dst->VFrame.mWidth;
	blit.dst_image_h.height = dst->VFrame.mHeight;
	blit.dst_image_h.align[0] = 0;
	blit.dst_image_h.align[1] = 0;
	blit.dst_image_h.align[2] = 0;
	blit.dst_image_h.clip_rect.x = 0;
	blit.dst_image_h.clip_rect.y = 0;
	blit.dst_image_h.clip_rect.w = dst->VFrame.mWidth;
	blit.dst_image_h.clip_rect.h = dst->VFrame.mHeight;
	blit.dst_image_h.gamut = G2D_BT709;
	blit.dst_image_h.bpremul = 0;
	blit.dst_image_h.mode = G2D_PIXEL_ALPHA;
	blit.dst_image_h.alpha = 255;
	blit.dst_image_h.fd = -1;
	blit.dst_image_h.use_phy_addr = 1;

	int ret = ioctl(g_g2dfd, G2D_CMD_BITBLT_H, (unsigned long)&blit);
	if (ret < 0) {
		printf("[carplay_display] G2D ioctl failed ret=%d\n", ret);
		return -1;
	}

	dst->VFrame.mOffsetTop = 0;
	dst->VFrame.mOffsetBottom = dst->VFrame.mHeight;
	dst->VFrame.mOffsetLeft = 0;
	dst->VFrame.mOffsetRight = dst->VFrame.mWidth;
	return 0;
}

static void *decode_thread_fn(void *arg)
{
	(void)arg;
	carplay_set_self_sched_fifo_max("carplay_decode");
#if CP_USE_FFMPEG_DECODER
	carplay_ffmpeg_frame_t ff_frame;

	while (g_ctx.running) {
		h264_packet_t pkt;
		int pkt_len;
		int h264_q_backlog;
		if (queue_pop(&pkt) != 0)
			break;
		pkt_len = pkt.len;
		g_decode_seq++;
		h264_q_backlog = queue_count_get();
		if (h264_q_backlog >= DECODE_DROP_H264Q_BACKLOG &&
		    (pkt.kind == H264_NAL_NON_KEY || pkt.kind == H264_NAL_OTHER)) {
			g_pstat.dec_drop_nonkey++;
			free(pkt.data);
			continue;
		}

		if (!g_ctx.got_idr) {
			int found_sps = 0;
			unsigned char *d = (unsigned char *)pkt.data;
			int dlen = pkt.len;
			for (int i = 0; i + 4 < dlen; i++) {
				if (d[i] == 0 && d[i+1] == 0 && (d[i+2] == 1 || (d[i+2] == 0 && d[i+3] == 1))) {
					int nal_off = (d[i+2] == 1) ? i + 3 : i + 4;
					if (nal_off < dlen && (d[nal_off] & 0x1F) == 7) {
						found_sps = 1;
						break;
					}
				}
			}
			if (!found_sps) {
				free(pkt.data);
				continue;
			}
			g_ctx.got_idr = 1;
		}

		g_pstat.dec_in++;
		long long send_t0 = cp_now_us();
		int send_ret = carplay_ffmpeg_decoder_send_packet(g_ctx.ffmpeg_dec, (const uint8_t *)pkt.data, pkt_len);
		long long send_cost = cp_now_us() - send_t0;
		g_pstat.send_total_us += (unsigned long long)(send_cost > 0 ? send_cost : 0);
		free(pkt.data);
		if (send_ret != SUCCESS) {
			g_pstat.send_fail++;
			CPD_LOG("decode_send_fail", "seq=%llu len=%d ret=%d cost_us=%lld",
			        g_decode_seq, pkt_len, send_ret, send_cost);
			continue;
		}
		if ((g_decode_seq % (unsigned long long)zlink_client_perf_sample_n()) == 0ULL) {
			CPD_LOG("decode_send", "seq=%llu len=%d cost_us=%lld",
			        g_decode_seq, pkt_len, send_cost);
		}

		int frame_cnt = 0;
		for (;;) {
			VIDEO_FRAME_INFO_S *vo_frame;
			int ret = carplay_ffmpeg_decoder_receive_frame(g_ctx.ffmpeg_dec, &ff_frame);
			if (ret == 1)
				break;
			if (ret != 0)
				break;
			frame_cnt++;

			pthread_mutex_lock(&g_fq.mutex);
			int fq_backlog = g_fq.count;
			pthread_mutex_unlock(&g_fq.mutex);
			if ((carplay_touch_screen_down || cpd_touch_recently_active()) && fq_backlog >= 1) {
				/* Touch interaction: favor latest frame for better perceived follow. */
				g_pstat.dec_drop_touch++;
				continue;
			}
			if (fq_backlog >= DECODE_DROP_FQ_BACKLOG) {
				/* Keep real-time smoothness under CPU pressure. */
				g_pstat.dec_drop_bp++;
				if ((g_pstat.dec_drop_bp % 60ULL) == 0ULL) {
					CPD_LOG("decode_drop_bp", "fq_backlog=%d drop_bp=%llu",
					        fq_backlog, g_pstat.dec_drop_bp);
				}
				continue;
			}
			long long pool_wait_t0 = cp_now_us();
			vo_frame = ffmpeg_pool_acquire();
			if (!vo_frame)
				break;
			long long pool_wait_us = cp_now_us() - pool_wait_t0;
			if ((unsigned long long)pool_wait_us > g_pstat.pool_wait_max_us)
				g_pstat.pool_wait_max_us = (unsigned long long)pool_wait_us;
			if (pool_wait_us > (long long)zlink_client_perf_warn_us()) {
				CPD_LOG("decode_pool_wait_slow", "wait_us=%lld", pool_wait_us);
			}
			if (ffmpeg_frame_to_vo_frame(&ff_frame, vo_frame) != 0) {
				decoder_release_frame(vo_frame);
				continue;
			}

			pthread_mutex_lock(&g_fq.mutex);
			if (g_fq.count >= FRAME_QUEUE_CAP) {
				VIDEO_FRAME_INFO_S old = g_fq.slot[g_fq.head];
				g_fq.head = (g_fq.head + 1) % FRAME_QUEUE_CAP;
				g_fq.count--;
				decoder_release_frame(&old);
				g_fq_drop++;
				g_pstat.fq_drop++;
			}
			g_fq.slot[g_fq.tail] = *vo_frame;
			g_fq.tail = (g_fq.tail + 1) % FRAME_QUEUE_CAP;
			g_fq.count++;
			g_frame_seq++;
			pthread_cond_signal(&g_fq.cond);
			pthread_mutex_unlock(&g_fq.mutex);
		}

		g_pstat.dec_out += (unsigned long long)frame_cnt;
		if ((g_decode_seq % (unsigned long long)zlink_client_perf_sample_n()) == 0ULL) {
			CPD_LOG("decode_get_image", "seq=%llu frames=%d fq_drop=%llu",
			        g_decode_seq, frame_cnt, g_fq_drop);
		}
	}

	fq_shutdown();
	return NULL;
#else
	VDEC_STREAM_S stream;
	VIDEO_FRAME_INFO_S frame;
	memset(&stream, 0, sizeof(stream));
	stream.pAddr = (unsigned char *)g_ctx.stream_buf_vir;

	while (g_ctx.running) {
		h264_packet_t pkt;
		if (queue_pop(&pkt) != 0)
			break;
		g_decode_seq++;
		if (pkt.len > (int)g_ctx.stream_buf_size) {
			free(pkt.data);
			continue;
		}
		memcpy(g_ctx.stream_buf_vir, pkt.data, (size_t)pkt.len);
		AW_MPI_SYS_MmzFlushCache(g_ctx.stream_buf_phy, g_ctx.stream_buf_vir, (int)pkt.len);
		free(pkt.data);

		stream.mLen = (unsigned int)pkt.len;
		stream.mbEndOfFrame = TRUE;
		g_pstat.dec_in++;

		long long send_t0 = cp_now_us();
		ERRORTYPE send_ret = AW_MPI_VDEC_SendStream(g_ctx.vdec_chn, &stream, 20);
		long long send_cost = cp_now_us() - send_t0;
		g_pstat.send_total_us += (unsigned long long)(send_cost > 0 ? send_cost : 0);
		if (send_ret != SUCCESS) {
			g_pstat.send_fail++;
			CPD_LOG("aw_send_stream_fail", "seq=%llu len=%u ret=0x%x send_us=%lld",
			        g_decode_seq, stream.mLen, (unsigned int)send_ret, send_cost);
			continue;
		}
		if (send_cost > (long long)zlink_client_perf_warn_us()) {
			CPD_LOG("aw_send_stream_slow", "seq=%llu len=%u send_us=%lld",
			        g_decode_seq, stream.mLen, send_cost);
		}

		int frame_cnt = 0;
		long long get_loop_t0 = cp_now_us();
		for (;;) {
			ERRORTYPE ret = AW_MPI_VDEC_GetImage(g_ctx.vdec_chn, &frame, 0);
			if (ret != SUCCESS)
				break;
			frame_cnt++;
			pthread_mutex_lock(&g_fq.mutex);
			if (g_fq.count >= FRAME_QUEUE_CAP) {
				VIDEO_FRAME_INFO_S old = g_fq.slot[g_fq.head];
				g_fq.head = (g_fq.head + 1) % FRAME_QUEUE_CAP;
				g_fq.count--;
				decoder_release_frame(&old);
				g_fq_drop++;
				g_pstat.fq_drop++;
			}
			g_fq.slot[g_fq.tail] = frame;
			g_fq.tail = (g_fq.tail + 1) % FRAME_QUEUE_CAP;
			g_fq.count++;
			g_frame_seq++;
			pthread_cond_signal(&g_fq.cond);
			pthread_mutex_unlock(&g_fq.mutex);
		}
		long long get_loop_us = cp_now_us() - get_loop_t0;
		g_pstat.dec_out += (unsigned long long)frame_cnt;
		if (frame_cnt == 0) {
			int q_count = 0;
			pthread_mutex_lock(&g_queue.mutex);
			q_count = g_queue.count;
			pthread_mutex_unlock(&g_queue.mutex);
			CPD_LOG("aw_decode_no_frame", "seq=%llu len=%u q_depth=%d send_us=%lld",
			        g_decode_seq, stream.mLen, q_count, send_cost);
		} else if (get_loop_us > (long long)zlink_client_perf_warn_us()) {
			CPD_LOG("aw_get_image_slow", "seq=%llu frames=%d get_us=%lld",
			        g_decode_seq, frame_cnt, get_loop_us);
		}
	}
	fq_shutdown();
	return NULL;
#endif
}

static void *display_thread_fn(void *arg)
{
	(void)arg;
	carplay_set_self_sched_fifo_max("carplay_display");

	while (g_ctx.running) {
		VIDEO_FRAME_INFO_S frame;
		int got = 0;

		pthread_mutex_lock(&g_fq.mutex);
		long long fq_wait_start = cp_now_us();
		while (g_fq.count == 0 && !g_fq.shutdown)
			pthread_cond_wait(&g_fq.cond, &g_fq.mutex);

		if (g_fq.shutdown && g_fq.count == 0) {
			pthread_mutex_unlock(&g_fq.mutex);
			break;
		}

		int keep_frames = cpd_touch_recently_active() ? DISPLAY_KEEP_FRAMES_TOUCH : DISPLAY_KEEP_FRAMES_DEFAULT;
		if (keep_frames < 1)
			keep_frames = 1;
		while (g_fq.count > DISPLAY_DROP_THRESHOLD) {
			VIDEO_FRAME_INFO_S old = g_fq.slot[g_fq.head];
			g_fq.head = (g_fq.head + 1) % FRAME_QUEUE_CAP;
			g_fq.count--;
			decoder_release_frame(&old);
			g_disp_fq_skip++;
			g_pstat.disp_fq_skip++;
			if (g_fq.count <= keep_frames)
				break;
		}
		frame = g_fq.slot[g_fq.head];
		g_fq.head = (g_fq.head + 1) % FRAME_QUEUE_CAP;
		g_fq.count--;
		got = 1;
		long long fq_wait_us = cp_now_us() - fq_wait_start;
		pthread_mutex_unlock(&g_fq.mutex);

		if (!got)
			continue;

#define TARGET_WIDTH  960
#define TARGET_HEIGHT 480
		
		int src_w = frame.VFrame.mWidth;
		int src_h = frame.VFrame.mHeight;
		
		// 如果输入大于目标尺寸，自动居中裁剪
		if (src_w > TARGET_WIDTH || src_h > TARGET_HEIGHT) {
			int crop_x = (src_w - TARGET_WIDTH) / 2;
			int crop_y = (src_h - TARGET_HEIGHT) / 2 - 9;
			
			frame.VFrame.mOffsetLeft   = crop_x;
			frame.VFrame.mOffsetRight  = crop_x + TARGET_WIDTH;
			frame.VFrame.mOffsetTop    = crop_y;
			frame.VFrame.mOffsetBottom = crop_y + TARGET_HEIGHT;
			
			src_w = TARGET_WIDTH;
			src_h = TARGET_HEIGHT;
		} else {
			// 小于或等于目标尺寸，不裁剪（或按需全图显示）
			frame.VFrame.mOffsetLeft   = 0;
			frame.VFrame.mOffsetRight  = src_w;
			frame.VFrame.mOffsetTop    = 0;
			frame.VFrame.mOffsetBottom = src_h;
		}
		
		{
			/* Rotate 270: output geometry becomes src_h x src_w. */
			// int dst_w = frame.VFrame.mHeight;
			// int dst_h = frame.VFrame.mWidth;
			int dst_w = src_h;
			int dst_h = src_w;
			int did_alloc = 0;
			if (dst_w <= 0 || dst_h <= 0) {
				dst_w = g_ctx.disp_width;
				dst_h = g_ctx.disp_height;
			}
			if (g_ctx.g2d_dst_allocated &&
			    (g_ctx.g2d_dst[0].VFrame.mWidth != dst_w ||
			     g_ctx.g2d_dst[0].VFrame.mHeight != dst_h ||
			     g_ctx.g2d_dst[0].VFrame.mPixelFormat != frame.VFrame.mPixelFormat)) {
				/* Resolution changed (e.g. fallback mode), rebuild G2D destination buffers. */
				g2d_free_dst();
			}
			if (!g_ctx.g2d_dst_allocated) {
				if (g2d_alloc_dst(dst_w, dst_h, frame.VFrame.mPixelFormat) != 0) {
					AW_MPI_VO_SendFrame(g_ctx.vo_layer, g_ctx.vo_chn, &frame, 0);
					decoder_release_frame(&frame);
					continue;
				}
				did_alloc = 1;
			}
			if (did_alloc) {
				pthread_mutex_lock(&g_ctx.g2d_use_mutex);
				for (int i = 0; i < 3; i++)
					g_ctx.g2d_dst_in_use[i] = 0;
				pthread_mutex_unlock(&g_ctx.g2d_use_mutex);
			}
		}

		int cur_buf = g_ctx.g2d_buf_idx;

		pthread_mutex_lock(&g_ctx.g2d_use_mutex);
		long long g2d_wait_start = cp_now_us();
		while (g_ctx.running && g_ctx.g2d_dst_in_use[cur_buf]) {
			pthread_cond_wait(&g_ctx.g2d_use_cond, &g_ctx.g2d_use_mutex);
		}
		if (!g_ctx.running) {
			pthread_mutex_unlock(&g_ctx.g2d_use_mutex);
			break;
		}
		g_ctx.g2d_dst_in_use[cur_buf] = 1;
		pthread_mutex_unlock(&g_ctx.g2d_use_mutex);
		long long g2d_wait_us = cp_now_us() - g2d_wait_start;

		VIDEO_FRAME_INFO_S *dst = &g_ctx.g2d_dst[cur_buf];
		long long g2d_t0 = cp_now_us();
		int g2d_ret = g2d_rotate_frame(&frame, dst);
		long long g2d_cost = cp_now_us() - g2d_t0;
		long long vo_t0 = cp_now_us();
		if (g2d_ret == 0) {
			AW_MPI_VO_SendFrame(g_ctx.vo_layer, g_ctx.vo_chn, dst, 0);
			g_ctx.g2d_buf_idx = (cur_buf + 1) % 3;
		} else {

			pthread_mutex_lock(&g_ctx.g2d_use_mutex);
			g_ctx.g2d_dst_in_use[cur_buf] = 0;
			pthread_cond_signal(&g_ctx.g2d_use_cond);
			pthread_mutex_unlock(&g_ctx.g2d_use_mutex);

			AW_MPI_VO_SendFrame(g_ctx.vo_layer, g_ctx.vo_chn, &frame, 0);
		}
		long long vo_cost = cp_now_us() - vo_t0;
		long long touch_to_display_us = 0;
		if (g_last_touch_send_us > 0) {
			long long now_us = cp_now_us();
			if (now_us >= g_last_touch_send_us)
				touch_to_display_us = now_us - g_last_touch_send_us;
		}
		if (vo_cost > (long long)zlink_client_perf_warn_us()) {
			CPD_LOG("display_vo_send_slow", "vo_cost_us=%lld", vo_cost);
		}
		g_pstat.disp++;
		g_pstat.g2d_total_us += (unsigned long long)(g2d_cost > 0 ? g2d_cost : 0);
		g_pstat.vo_total_us += (unsigned long long)(vo_cost > 0 ? vo_cost : 0);
		cpd_perf_maybe_print();
		if ((g_frame_seq % (unsigned long long)zlink_client_perf_sample_n()) == 0ULL) {
			CPD_LOG("display_frame", "fq_wait_us=%lld g2d_wait_us=%lld g2d_cost_us=%lld vo_cost_us=%lld g2d_ret=%d",
			        fq_wait_us, g2d_wait_us, g2d_cost, vo_cost, g2d_ret);
			if (touch_to_display_us > 0 && cpd_touch_recently_active()) {
				CPD_LOG("touch_to_display", "delta_us=%lld", touch_to_display_us);
			}
		}
		decoder_release_frame(&frame);
	}
	return NULL;
}

#define LVGL_LOGICAL_W  1440
#define LVGL_LOGICAL_H  720

static void touch_apply_and_send(int screen_x, int screen_y, int is_touch_down)
{
	int session_width, session_height;
	long long t0 = cp_now_us();

	pthread_mutex_lock(&g_ctx.rect_mutex);
	session_width  = g_ctx.session_width;
	session_height = g_ctx.session_height;
	pthread_mutex_unlock(&g_ctx.rect_mutex);

	if (session_width <= 0 || session_height <= 0)
		return;
	if (screen_x < 0 || screen_x >= LVGL_LOGICAL_W ||
	    screen_y < 0 || screen_y >= LVGL_LOGICAL_H)
		return;

	int session_x = (int)((long)screen_x * session_width  / LVGL_LOGICAL_W);
	int session_y = (int)((long)screen_y * session_height / LVGL_LOGICAL_H);
	if (session_x < 0) session_x = 0;
	if (session_x >= session_width)  session_x = session_width - 1;
	if (session_y < 0) session_y = 0;
	if (session_y >= session_height) session_y = session_height - 1;

	libzlink_touch_event(session_x, session_y, is_touch_down);
	g_last_touch_send_us = cp_now_us();
	if ((g_frame_seq % (unsigned long long)zlink_client_perf_sample_n()) == 0ULL) {
		CPD_LOG("touch_map_send", "sx=%d sy=%d tx=%d ty=%d down=%d cost_us=%lld",
		        screen_x, screen_y, session_x, session_y, is_touch_down, cp_now_us() - t0);
	}
}

static ERRORTYPE carplay_vo_callback(void *cookie, MPP_CHN_S *pChn, MPP_EVENT_TYPE event, void *pEventData)
{
	(void)cookie; (void)pChn;

	if (event == MPP_EVENT_RELEASE_VIDEO_BUFFER) {
		long long now_us = cp_now_us();
		if (g_last_vo_release_us > 0) {
			long long delta = now_us - g_last_vo_release_us;
			if (delta > (long long)zlink_client_perf_warn_us()) {
				CPD_LOG("vo_release_slow", "interval_us=%lld", delta);
			}
		}
		g_last_vo_release_us = now_us;
		VIDEO_FRAME_INFO_S *released = (VIDEO_FRAME_INFO_S *)pEventData;
		if (released) {
			pthread_mutex_lock(&g_ctx.g2d_use_mutex);
			for (int i = 0; i < 3; i++) {
				VIDEO_FRAME_INFO_S *dst = &g_ctx.g2d_dst[i];
				if (released == dst ||
				    (released->VFrame.mPhyAddr[0] == dst->VFrame.mPhyAddr[0] &&
				     released->VFrame.mPhyAddr[1] == dst->VFrame.mPhyAddr[1])) {
					g_ctx.g2d_dst_in_use[i] = 0;
					pthread_cond_signal(&g_ctx.g2d_use_cond);
					break;
				}
			}
			pthread_mutex_unlock(&g_ctx.g2d_use_mutex);
		}
	}

	return SUCCESS;
}

int carplay_display_create(int disp_x, int disp_y, int disp_width, int disp_height,
                           int session_width, int session_height)
{
	long long create_t0 = cp_now_us();
	VO_PUB_ATTR_S vo_pub;
	VO_VIDEO_LAYER_ATTR_S layer_attr;
	CLOCK_CHN_ATTR_S clock_attr;
	MPP_CHN_S clock_chn, vo_chn;
	int queue_inited = 0;
	int fq_inited = 0;
	int ffmpeg_pool_inited = 0;
	int decode_thread_started = 0;
	int display_thread_started = 0;

	if (g_ctx.running)
		return -1;
	if (session_width <= 0 || session_height <= 0)
		return -1;

#if CP_USE_FFMPEG_DECODER
	printf("[carplay_display] decoder_backend=ffmpeg\n");
#else
	printf("[carplay_display] decoder_backend=aw_vdec\n");
#endif

	memset(&g_ctx, 0, sizeof(g_ctx));
	g_ctx.disp_x = disp_x;
	g_ctx.disp_y = disp_y;
	g_ctx.disp_width  = disp_width  > 0 ? disp_width  : session_width;
	g_ctx.disp_height = disp_height > 0 ? disp_height : session_height;
	g_ctx.session_width  = session_width;
	g_ctx.session_height = session_height;
	g_ctx.vo_dev   = 0;
	g_ctx.vo_layer = VO_LAYER_DEFAULT;
	g_ctx.vo_chn   = VO_CHN_DEFAULT;
#if !CP_USE_FFMPEG_DECODER
	g_ctx.vdec_chn = VDEC_CHN_DEFAULT;
	g_ctx.stream_buf_size = 4 * 1024 * 1024;
#endif
	g_ctx.clock_chn = CLOCK_CHN_DEFAULT;

	pthread_mutex_init(&g_ctx.rect_mutex, NULL);
	pthread_mutex_init(&g_ctx.g2d_use_mutex, NULL);
	pthread_cond_init(&g_ctx.g2d_use_cond, NULL);
	for (int i = 0; i < 3; i++)
		g_ctx.g2d_dst_in_use[i] = 0;

	queue_init();
	queue_inited = 1;
	fq_init();
	fq_inited = 1;
#if CP_USE_FFMPEG_DECODER
	if (ffmpeg_pool_init(session_width, session_height) != 0)
		goto err_cleanup;
	ffmpeg_pool_inited = 1;
	g_ffmpeg_pool_running = 1;
	if (carplay_ffmpeg_decoder_create(&g_ctx.ffmpeg_dec, session_width, session_height) != 0)
		goto err_cleanup;
#else
	if (AW_MPI_SYS_MmzAlloc_Cached(&g_ctx.stream_buf_phy, &g_ctx.stream_buf_vir,
	                               (int)g_ctx.stream_buf_size) != SUCCESS)
		goto err_cleanup;
	VDEC_CHN_ATTR_S vdec_attr;
	memset(&vdec_attr, 0, sizeof(vdec_attr));
	vdec_attr.mType = PT_H264;
	vdec_attr.mOutputPixelFormat = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
	vdec_attr.mBufSize = (unsigned int)(4 * 1024 * 1024);
	vdec_attr.mPicWidth  = (unsigned int)session_width;
	vdec_attr.mPicHeight = (unsigned int)session_height;
	vdec_attr.mVdecVideoAttr.mMode = VIDEO_MODE_STREAM;
	vdec_attr.bEnableExtraFrameNum = TRUE;
	vdec_attr.mExtraFrameNum = 4;
	ERRORTYPE vret = AW_MPI_VDEC_CreateChn(g_ctx.vdec_chn, &vdec_attr);
	if (vret != SUCCESS && vret != ERR_VDEC_EXIST)
		goto err_cleanup;

	AW_MPI_VDEC_SetVEFreq(g_ctx.vdec_chn, 500);
	AW_MPI_VDEC_StartRecvStream(g_ctx.vdec_chn);
#endif

	MPPCallbackInfo cb_empty = { 0 };

	AW_MPI_VO_Enable(g_ctx.vo_dev);
	AW_MPI_VO_AddOutsideVideoLayer(1);
	AW_MPI_VO_CloseVideoLayer(1);
	AW_MPI_VO_EnableVideoLayer(g_ctx.vo_layer);

	memset(&vo_pub, 0, sizeof(vo_pub));
	AW_MPI_VO_GetPubAttr(g_ctx.vo_dev, &vo_pub);
	AW_MPI_VO_SetPubAttr(g_ctx.vo_dev, &vo_pub);

	memset(&layer_attr, 0, sizeof(layer_attr));
	AW_MPI_VO_GetVideoLayerAttr(g_ctx.vo_layer, &layer_attr);
	layer_attr.stDispRect.X = g_ctx.disp_x;
	layer_attr.stDispRect.Y = g_ctx.disp_y;
	layer_attr.stDispRect.Width  = g_ctx.disp_width;
	layer_attr.stDispRect.Height = g_ctx.disp_height;
	AW_MPI_VO_SetVideoLayerAttr(g_ctx.vo_layer, &layer_attr);

	{
		BOOL vo_chn_created = FALSE;
		while (g_ctx.vo_chn < VO_MAX_CHN_NUM) {
			ERRORTYPE vo_ret = AW_MPI_VO_CreateChn(g_ctx.vo_layer, g_ctx.vo_chn);
			if (vo_ret == SUCCESS) {
				vo_chn_created = TRUE;
				break;
			}
			if (vo_ret == ERR_VO_CHN_NOT_DISABLE) {
				g_ctx.vo_chn++;
				continue;
			}
			goto err_cleanup;
		}
		if (!vo_chn_created)
			goto err_cleanup;
	}

	AW_MPI_VO_SetChnFrameRate(g_ctx.vo_layer, g_ctx.vo_chn, 20);

	{
		MPPCallbackInfo vo_cb;
		vo_cb.cookie = NULL;
		vo_cb.callback = (MPPCallbackFuncType)&carplay_vo_callback;
		AW_MPI_VO_RegisterCallback(g_ctx.vo_layer, g_ctx.vo_chn, &vo_cb);
	}
	AW_MPI_VO_SetChnDispBufNum(g_ctx.vo_layer, g_ctx.vo_chn, 2);

	memset(&clock_attr, 0, sizeof(clock_attr));
	clock_attr.nWaitMask = 1 << CLOCK_PORT_INDEX_VIDEO;
	AW_MPI_CLOCK_CreateChn(g_ctx.clock_chn, &clock_attr);
	AW_MPI_CLOCK_RegisterCallback(g_ctx.clock_chn, &cb_empty);
	clock_chn.mModId = MOD_ID_CLOCK;
	clock_chn.mDevId = 0;
	clock_chn.mChnId = g_ctx.clock_chn;
	vo_chn.mModId   = MOD_ID_VOU;
	vo_chn.mDevId   = g_ctx.vo_layer;
	vo_chn.mChnId   = g_ctx.vo_chn;
	AW_MPI_SYS_Bind(&clock_chn, &vo_chn);
	AW_MPI_CLOCK_Start(g_ctx.clock_chn);
	AW_MPI_VO_StartChn(g_ctx.vo_layer, g_ctx.vo_chn);

	g_ctx.running = 1;
	if (pthread_create(&g_ctx.decode_tid, NULL, decode_thread_fn, NULL) != 0) {
		g_ctx.running = 0;
		queue_fini();
		fq_shutdown();
		goto err_cleanup;
	}
	decode_thread_started = 1;
	if (pthread_create(&g_ctx.display_tid, NULL, display_thread_fn, NULL) != 0) {
		g_ctx.running = 0;
		queue_fini();
		fq_shutdown();
		goto err_cleanup;
	}
	display_thread_started = 1;
	memset(&g_pstat, 0, sizeof(g_pstat));
	g_pstat.last_print_us = cp_now_us();
	CPD_LOG("display_create_done", "disp=%dx%d session=%dx%d cost_us=%lld",
	        g_ctx.disp_width, g_ctx.disp_height, g_ctx.session_width, g_ctx.session_height,
	        cp_now_us() - create_t0);
	return 0;

err_cleanup:
	if (decode_thread_started)
		pthread_join(g_ctx.decode_tid, NULL);
	if (display_thread_started)
		pthread_join(g_ctx.display_tid, NULL);
	if (fq_inited)
		fq_destroy();
	if (queue_inited)
		queue_destroy();

	AW_MPI_VO_StopChn(g_ctx.vo_layer, g_ctx.vo_chn);
	AW_MPI_CLOCK_Stop(g_ctx.clock_chn);
	AW_MPI_CLOCK_DestroyChn(g_ctx.clock_chn);
	AW_MPI_VO_DestroyChn(g_ctx.vo_layer, g_ctx.vo_chn);
	AW_MPI_VO_DisableVideoLayer(g_ctx.vo_layer);
	AW_MPI_VO_RemoveOutsideVideoLayer(1);
	AW_MPI_VO_Disable(g_ctx.vo_dev);
#if CP_USE_FFMPEG_DECODER
	g_ffmpeg_pool_running = 0;
	pthread_mutex_lock(&g_ffmpeg_pool_sync.mutex);
	pthread_cond_broadcast(&g_ffmpeg_pool_sync.cond);
	pthread_mutex_unlock(&g_ffmpeg_pool_sync.mutex);
	carplay_ffmpeg_decoder_destroy(g_ctx.ffmpeg_dec);
	g_ctx.ffmpeg_dec = NULL;
	if (ffmpeg_pool_inited)
		ffmpeg_pool_destroy();
#else
	AW_MPI_VDEC_StopRecvStream(g_ctx.vdec_chn);
	AW_MPI_VDEC_DestroyChn(g_ctx.vdec_chn);
	if (g_ctx.stream_buf_phy)
		AW_MPI_SYS_MmzFree(g_ctx.stream_buf_phy, g_ctx.stream_buf_vir);
#endif
	return -1;
}

int carplay_display_feed_h264(const char *data, int len)
{
	if (!data || len <= 0 || !g_ctx.running)
		return -1;
	return queue_push(data, len);
}

void carplay_display_set_rect(int x, int y, int width, int height)
{
	VO_VIDEO_LAYER_ATTR_S layer_attr;
	if (!g_ctx.running)
		return;
	pthread_mutex_lock(&g_ctx.rect_mutex);
	g_ctx.disp_x = x;
	g_ctx.disp_y = y;
	if (width > 0)  g_ctx.disp_width  = width;
	if (height > 0) g_ctx.disp_height = height;
	pthread_mutex_unlock(&g_ctx.rect_mutex);

	memset(&layer_attr, 0, sizeof(layer_attr));
	AW_MPI_VO_GetVideoLayerAttr(g_ctx.vo_layer, &layer_attr);
	layer_attr.stDispRect.X = x;
	layer_attr.stDispRect.Y = y;
	layer_attr.stDispRect.Width  = width  > 0 ? width  : g_ctx.disp_width;
	layer_attr.stDispRect.Height = height > 0 ? height : g_ctx.disp_height;
	AW_MPI_VO_SetVideoLayerAttr(g_ctx.vo_layer, &layer_attr);
}

void carplay_display_destroy(void)
{
	if (!g_ctx.running)
		return;
	g_ctx.running = 0;
#if CP_USE_FFMPEG_DECODER
	g_ffmpeg_pool_running = 0;
	pthread_mutex_lock(&g_ffmpeg_pool_sync.mutex);
	pthread_cond_broadcast(&g_ffmpeg_pool_sync.cond);
	pthread_mutex_unlock(&g_ffmpeg_pool_sync.mutex);
#endif

	/* Wake display thread if it's waiting for VO buffer release. */
	pthread_mutex_lock(&g_ctx.g2d_use_mutex);
	for (int i = 0; i < 3; i++)
		g_ctx.g2d_dst_in_use[i] = 0;
	pthread_cond_broadcast(&g_ctx.g2d_use_cond);
	pthread_mutex_unlock(&g_ctx.g2d_use_mutex);

	queue_fini();
	fq_shutdown();
	pthread_join(g_ctx.decode_tid, NULL);
	pthread_join(g_ctx.display_tid, NULL);
	queue_destroy();

	/* Drain any remaining frames in frame_queue */
	pthread_mutex_lock(&g_fq.mutex);
	while (g_fq.count > 0) {
		VIDEO_FRAME_INFO_S old = g_fq.slot[g_fq.head];
		g_fq.head = (g_fq.head + 1) % FRAME_QUEUE_CAP;
		g_fq.count--;
		decoder_release_frame(&old);
	}
	pthread_mutex_unlock(&g_fq.mutex);
	fq_destroy();

	AW_MPI_VO_StopChn(g_ctx.vo_layer, g_ctx.vo_chn);
	AW_MPI_CLOCK_Stop(g_ctx.clock_chn);
	AW_MPI_CLOCK_DestroyChn(g_ctx.clock_chn);
	AW_MPI_VO_DestroyChn(g_ctx.vo_layer, g_ctx.vo_chn);
	AW_MPI_VO_DisableVideoLayer(g_ctx.vo_layer);
	AW_MPI_VO_RemoveOutsideVideoLayer(1);
	AW_MPI_VO_Disable(g_ctx.vo_dev);
#if CP_USE_FFMPEG_DECODER
	carplay_ffmpeg_decoder_flush(g_ctx.ffmpeg_dec);
	carplay_ffmpeg_decoder_destroy(g_ctx.ffmpeg_dec);
	g_ctx.ffmpeg_dec = NULL;
#else
	AW_MPI_VDEC_StopRecvStream(g_ctx.vdec_chn);
	AW_MPI_VDEC_DestroyChn(g_ctx.vdec_chn);
	if (g_ctx.stream_buf_phy)
		AW_MPI_SYS_MmzFree(g_ctx.stream_buf_phy, g_ctx.stream_buf_vir);
#endif

	g2d_free_dst();
#if CP_USE_FFMPEG_DECODER
	ffmpeg_pool_destroy();
#endif

	g_ctx.session_width = g_ctx.session_height = 0;
	pthread_mutex_destroy(&g_ctx.rect_mutex);
	pthread_mutex_destroy(&g_ctx.g2d_use_mutex);
	pthread_cond_destroy(&g_ctx.g2d_use_cond);
}

void carplay_touch_send(void)
{
	touch_apply_and_send(carplay_touch_screen_x, carplay_touch_screen_y, carplay_touch_screen_down);
}

void carplay_touch_send_xy(int screen_x, int screen_y, int is_touch_down)
{
	long long t0 = cp_now_us();
	touch_apply_and_send(screen_x, screen_y, is_touch_down);
	if (is_touch_down || cpd_touch_recently_active()) {
		pthread_mutex_lock(&g_fq.mutex);
		pthread_cond_signal(&g_fq.cond);
		pthread_mutex_unlock(&g_fq.mutex);
	}
	if ((g_frame_seq % (unsigned long long)zlink_client_perf_sample_n()) == 0ULL) {
		CPD_LOG("touch_send_xy", "x=%d y=%d down=%d cost_us=%lld",
		        screen_x, screen_y, is_touch_down, cp_now_us() - t0);
	}
}

#endif /* ENABLE_CARPLAY */
