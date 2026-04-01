#define _GNU_SOURCE
#ifdef ENABLE_CARPLAY

#include "carplay_display.h"
#include "mpp_compat.h"
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

extern int libzlink_touch_event(int x, int y, int is_touch_down);

int carplay_touch_screen_x = 0;
int carplay_touch_screen_y = 0;
int carplay_touch_screen_down = 0;
int carplay_split_screen_enable = 0;
int carplay_split_line_x = 720;

#define LINK_TYPE_CARPLAY       2
#define LINK_TYPE_ANDROIDAUTO   3
#define H264_QUEUE_CAP          128
#define H264_PACKET_MAX         (256 * 1024)
#define H264_QUEUE_HIGH_WATERMARK ((H264_QUEUE_CAP * 3) / 4)
#define LVGL_LOGICAL_W          1440
#define LVGL_LOGICAL_H          720
#define VO_LAYER_DEFAULT        0
#define VO_CHN_DEFAULT          0
#define VDEC_CHN_DEFAULT        0
#define CLOCK_CHN_DEFAULT       0
#define STREAM_STAGE_BUF_NUM    2
#define STREAM_STAGE_BUF_SIZE   H264_PACKET_MAX
#define SENDSTREAM_TIMEOUT_MS   20
#define SENDSTREAM_RETRY_MAX    4
#define SENDSTREAM_RETRY_US     2000
#define DECODE_THREAD_PRIO      10
#define METRICS_LOG_INTERVAL_US (2ULL * 1000 * 1000)
#define ROTATE_USE_G2D_DEFAULT  0
#define VO_STRIDE_ALIGN         16

#define H264_PACKET_FLAG_IDR    (1U << 0)
#define H264_PACKET_FLAG_CONFIG (1U << 1)

typedef struct {
	int block_idx;
	int len;
	unsigned int flags;
	uint64_t enqueue_us;
} h264_packet_t;

typedef struct {
	unsigned char *buf;
} h264_block_t;

static struct {
	h264_packet_t slot[H264_QUEUE_CAP];
	h264_block_t block[H264_QUEUE_CAP];
	int free_stack[H264_QUEUE_CAP];
	int free_count;
	int head;
	int tail;
	int count;
	uint64_t enqueue_total;
	uint64_t drop_total;
	uint64_t proactive_drop_total;
	uint64_t keyframe_enqueue_total;
	uint64_t max_depth;
	uint64_t max_queue_wait_us;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	volatile int shutdown;
} g_queue;

static struct {
	carplay_display_policy_t policy;
	int rotated_width;
	int rotated_height;
	int vo_dev;
	int vo_layer;
	int vo_chn;
	int vdec_chn;
	int clock_chn;
	int bind_vdec_to_vo;
	int bind_clock_to_vo;
	pthread_t decode_tid;
	volatile int running;
	volatile int got_idr;
	unsigned int stream_buf_phy[STREAM_STAGE_BUF_NUM];
	void *stream_buf_vir[STREAM_STAGE_BUF_NUM];
	size_t vdec_stream_buf_size;
	size_t stream_stage_size;
	int stream_buf_count;
	int stream_buf_next;
	uint64_t send_retry_total;
	uint64_t send_fail_total;
	uint64_t send_busy_total;
	uint64_t send_ok_total;
	uint64_t max_send_cost_us;
	uint64_t last_metrics_log_us;
	int priority_attempted;
	int use_g2d_rotate;
	pthread_mutex_t rect_mutex;
} g_ctx;

static pthread_mutex_t g_display_lifecycle_mutex = PTHREAD_MUTEX_INITIALIZER;
static int g_display_state;

enum {
	DISPLAY_STATE_STOPPED = 0,
	DISPLAY_STATE_STARTING,
	DISPLAY_STATE_RUNNING,
	DISPLAY_STATE_STOPPING,
};

static uint64_t monotonic_time_us(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)ts.tv_sec * 1000 * 1000 + (uint64_t)ts.tv_nsec / 1000;
}

static int rotation_swaps_wh(int rotation)
{
	return rotation == ROTATE_90 || rotation == ROTATE_270;
}

static int align_up_to(int value, int align)
{
	if (align <= 1 || value <= 0)
		return value;
	return ((value + align - 1) / align) * align;
}

static void carplay_display_try_raise_priority(void)
{
	struct sched_param param;
	int ret;

	if (g_ctx.priority_attempted)
		return;
	g_ctx.priority_attempted = 1;

	memset(&param, 0, sizeof(param));
	param.sched_priority = DECODE_THREAD_PRIO;
	ret = pthread_setschedparam(pthread_self(), SCHED_RR, &param);
	if (ret == 0) {
		printf("[carplay_display] decode thread priority raised policy=%d prio=%d\n",
		       SCHED_RR, param.sched_priority);
	} else {
		printf("[carplay_display] decode thread priority raise failed ret=%d (%s)\n",
		       ret, strerror(ret));
	}
}

static unsigned int packet_get_flags(const char *data, int len)
{
	const unsigned char *d = (const unsigned char *)data;
	unsigned int flags = 0;
	int i;

	for (i = 0; i + 4 < len; i++) {
		int off;
		unsigned int nal_type;

		if (!(d[i] == 0 && d[i + 1] == 0 &&
		      (d[i + 2] == 1 || (d[i + 2] == 0 && d[i + 3] == 1))))
			continue;
		off = (d[i + 2] == 1) ? i + 3 : i + 4;
		if (off >= len)
			break;
		nal_type = d[off] & 0x1F;
		if (nal_type == 5)
			flags |= H264_PACKET_FLAG_IDR;
		else if (nal_type == 7 || nal_type == 8)
			flags |= H264_PACKET_FLAG_CONFIG;
	}

	return flags;
}

static int packet_is_key_flags(unsigned int flags)
{
	return (flags & (H264_PACKET_FLAG_IDR | H264_PACKET_FLAG_CONFIG)) != 0;
}

static int packet_has_sps(const char *data, int len)
{
	return (packet_get_flags(data, len) & H264_PACKET_FLAG_CONFIG) != 0;
}

static int carplay_display_use_g2d_rotate(void)
{
	const char *env = getenv("CARPLAY_ROTATE_USE_G2D");

	if (env)
		return atoi(env) != 0;
	return ROTATE_USE_G2D_DEFAULT;
}

static void queue_init(void)
{
	int i;

	memset(&g_queue, 0, sizeof(g_queue));
	pthread_mutex_init(&g_queue.mutex, NULL);
	pthread_cond_init(&g_queue.cond, NULL);
	for (i = 0; i < H264_QUEUE_CAP; i++) {
		g_queue.slot[i].block_idx = -1;
		g_queue.free_stack[g_queue.free_count++] = H264_QUEUE_CAP - 1 - i;
	}
}

static void queue_fini(void)
{
	pthread_mutex_lock(&g_queue.mutex);
	g_queue.shutdown = 1;
	pthread_cond_broadcast(&g_queue.cond);
	pthread_mutex_unlock(&g_queue.mutex);
}

static void queue_destroy(void)
{
	int i;

	for (i = 0; i < H264_QUEUE_CAP; i++) {
		free(g_queue.block[i].buf);
		g_queue.block[i].buf = NULL;
	}
	pthread_mutex_destroy(&g_queue.mutex);
	pthread_cond_destroy(&g_queue.cond);
}

static int queue_acquire_block_locked(void)
{
	int block_idx;

	if (g_queue.free_count <= 0)
		return -1;
	block_idx = g_queue.free_stack[--g_queue.free_count];
	if (!g_queue.block[block_idx].buf) {
		g_queue.block[block_idx].buf = (unsigned char *)malloc(H264_PACKET_MAX);
		if (!g_queue.block[block_idx].buf) {
			g_queue.free_stack[g_queue.free_count++] = block_idx;
			return -1;
		}
	}
	return block_idx;
}

static void queue_release_block_locked(int block_idx)
{
	if (block_idx < 0 || block_idx >= H264_QUEUE_CAP)
		return;
	g_queue.free_stack[g_queue.free_count++] = block_idx;
}

static void queue_remove_at_locked(int idx)
{
	int next;
	int tail_prev;

	if (g_queue.count <= 0)
		return;

	queue_release_block_locked(g_queue.slot[idx].block_idx);
	next = (idx + 1) % H264_QUEUE_CAP;
	while (next != g_queue.tail) {
		g_queue.slot[idx] = g_queue.slot[next];
		idx = next;
		next = (next + 1) % H264_QUEUE_CAP;
	}

	tail_prev = (g_queue.tail - 1 + H264_QUEUE_CAP) % H264_QUEUE_CAP;
	g_queue.slot[tail_prev].block_idx = -1;
	g_queue.slot[tail_prev].len = 0;
	g_queue.slot[tail_prev].flags = 0;
	g_queue.slot[tail_prev].enqueue_us = 0;
	g_queue.tail = tail_prev;
	g_queue.count--;
	g_queue.drop_total++;
}

static int queue_drop_one_nonkey_locked(void)
{
	int idx;
	int remain;

	idx = g_queue.head;
	for (remain = g_queue.count; remain > 0; remain--) {
		if (!packet_is_key_flags(g_queue.slot[idx].flags)) {
			queue_remove_at_locked(idx);
			g_queue.proactive_drop_total++;
			return 0;
		}
		idx = (idx + 1) % H264_QUEUE_CAP;
	}
	return -1;
}

static int queue_push(const char *data, int len)
{
	unsigned int flags;
	uint64_t now_us;
	int block_idx;

	if (len <= 0 || len > H264_PACKET_MAX)
		return -1;

	flags = packet_get_flags(data, len);
	now_us = monotonic_time_us();

	pthread_mutex_lock(&g_queue.mutex);

	if (g_queue.count >= H264_QUEUE_HIGH_WATERMARK && !packet_is_key_flags(flags)) {
		g_queue.drop_total++;
		g_queue.proactive_drop_total++;
		pthread_mutex_unlock(&g_queue.mutex);
		return -1;
	}

	if (g_queue.count >= H264_QUEUE_CAP) {
		if (packet_is_key_flags(flags) && queue_drop_one_nonkey_locked() == 0) {
			/* preserve recovery points when queue is congested */
		} else {
			g_queue.drop_total++;
			pthread_mutex_unlock(&g_queue.mutex);
			return -1;
		}
	}

	block_idx = queue_acquire_block_locked();
	if (block_idx < 0) {
		g_queue.drop_total++;
		pthread_mutex_unlock(&g_queue.mutex);
		return -1;
	}

	memcpy(g_queue.block[block_idx].buf, data, (size_t)len);
	g_queue.slot[g_queue.tail].block_idx = block_idx;
	g_queue.slot[g_queue.tail].len = len;
	g_queue.slot[g_queue.tail].flags = flags;
	g_queue.slot[g_queue.tail].enqueue_us = now_us;
	g_queue.tail = (g_queue.tail + 1) % H264_QUEUE_CAP;
	g_queue.count++;
	g_queue.enqueue_total++;
	if (packet_is_key_flags(flags))
		g_queue.keyframe_enqueue_total++;
	if ((uint64_t)g_queue.count > g_queue.max_depth)
		g_queue.max_depth = (uint64_t)g_queue.count;

	pthread_cond_signal(&g_queue.cond);
	pthread_mutex_unlock(&g_queue.mutex);
	return 0;
}

static int queue_pop(h264_packet_t *out)
{
	pthread_mutex_lock(&g_queue.mutex);
	while (g_queue.count == 0 && !g_queue.shutdown)
		pthread_cond_wait(&g_queue.cond, &g_queue.mutex);
	if (g_queue.shutdown && g_queue.count == 0) {
		pthread_mutex_unlock(&g_queue.mutex);
		return -1;
	}

	*out = g_queue.slot[g_queue.head];
	if (out->enqueue_us != 0) {
		uint64_t wait_us = monotonic_time_us() - out->enqueue_us;
		if (wait_us > g_queue.max_queue_wait_us)
			g_queue.max_queue_wait_us = wait_us;
	}
	g_queue.slot[g_queue.head].block_idx = -1;
	g_queue.slot[g_queue.head].len = 0;
	g_queue.slot[g_queue.head].flags = 0;
	g_queue.slot[g_queue.head].enqueue_us = 0;
	g_queue.head = (g_queue.head + 1) % H264_QUEUE_CAP;
	g_queue.count--;
	pthread_mutex_unlock(&g_queue.mutex);
	return 0;
}

static void queue_release_packet(const h264_packet_t *pkt)
{
	pthread_mutex_lock(&g_queue.mutex);
	queue_release_block_locked(pkt->block_idx);
	pthread_mutex_unlock(&g_queue.mutex);
}

static void carplay_display_log_metrics_if_needed(uint64_t now_us)
{
	uint64_t enqueue_total;
	uint64_t drop_total;
	uint64_t proactive_drop_total;
	uint64_t max_depth;
	uint64_t max_queue_wait_us;
	int queue_depth;

	if (g_ctx.last_metrics_log_us != 0 &&
	    now_us - g_ctx.last_metrics_log_us < METRICS_LOG_INTERVAL_US)
		return;

	pthread_mutex_lock(&g_queue.mutex);
	enqueue_total = g_queue.enqueue_total;
	drop_total = g_queue.drop_total;
	proactive_drop_total = g_queue.proactive_drop_total;
	max_depth = g_queue.max_depth;
	max_queue_wait_us = g_queue.max_queue_wait_us;
	queue_depth = g_queue.count;
	g_queue.max_queue_wait_us = 0;
	pthread_mutex_unlock(&g_queue.mutex);

	printf("[carplay_display] stats enqueue=%llu drop=%llu proactive_drop=%llu queue_depth=%d max_depth=%llu send_ok=%llu retry=%llu busy=%llu fail=%llu max_send_us=%llu max_queue_wait_us=%llu\n",
	       (unsigned long long)enqueue_total,
	       (unsigned long long)drop_total,
	       (unsigned long long)proactive_drop_total,
	       queue_depth,
	       (unsigned long long)max_depth,
	       (unsigned long long)g_ctx.send_ok_total,
	       (unsigned long long)g_ctx.send_retry_total,
	       (unsigned long long)g_ctx.send_busy_total,
	       (unsigned long long)g_ctx.send_fail_total,
	       (unsigned long long)g_ctx.max_send_cost_us,
	       (unsigned long long)max_queue_wait_us);
	g_ctx.last_metrics_log_us = now_us;
	g_ctx.max_send_cost_us = 0;
}

static void carplay_display_get_frame_region(RECT_S *rect)
{
	int width;

	if (g_ctx.policy.frame_region_width > 0 && g_ctx.policy.frame_region_height > 0) {
		rect->X = g_ctx.policy.frame_region_x;
		rect->Y = g_ctx.policy.frame_region_y;
		width = align_up_to(g_ctx.policy.frame_region_width, VO_STRIDE_ALIGN);
		rect->Width = width;
		rect->Height = g_ctx.policy.frame_region_height;
	} else {
		rect->X = 0;
		rect->Y = 0;
		rect->Width = align_up_to(g_ctx.rotated_width, VO_STRIDE_ALIGN);
		rect->Height = g_ctx.rotated_height;
	}
}

static ERRORTYPE carplay_display_apply_vo_layout(void)
{
	ERRORTYPE ret;
	VO_VIDEO_LAYER_ATTR_S layer_attr;
	RECT_S frame_region;
	int disp_width;

	memset(&layer_attr, 0, sizeof(layer_attr));
	ret = AW_MPI_VO_GetVideoLayerAttr(g_ctx.vo_layer, &layer_attr);
	if (ret != SUCCESS)
		return ret;

	pthread_mutex_lock(&g_ctx.rect_mutex);
	disp_width = align_up_to(g_ctx.policy.disp_width, VO_STRIDE_ALIGN);
	layer_attr.stDispRect.X = g_ctx.policy.disp_x;
	layer_attr.stDispRect.Y = g_ctx.policy.disp_y;
	layer_attr.stDispRect.Width = disp_width;
	layer_attr.stDispRect.Height = g_ctx.policy.disp_height;
	pthread_mutex_unlock(&g_ctx.rect_mutex);

	ret = AW_MPI_VO_SetVideoLayerAttr(g_ctx.vo_layer, &layer_attr);
	if (ret != SUCCESS)
		return ret;

	carplay_display_get_frame_region(&frame_region);
	ret = AW_MPI_VO_SetFrameDisplayRegion(g_ctx.vo_layer, g_ctx.vo_chn, &frame_region);
	if (ret != SUCCESS)
		return ret;

	ret = AW_MPI_VO_SetVideoScalingMode(g_ctx.vo_layer, g_ctx.vo_chn,
	                                    g_ctx.policy.scaling_mode);
	if (ret != SUCCESS) {
		printf("[carplay_display] VO scaling mode %d unsupported, fallback to default path ret=0x%x\n",
		       g_ctx.policy.scaling_mode, ret);
		return SUCCESS;
	}

	return SUCCESS;
}

static void carplay_display_unbind_if_needed(MPP_CHN_S *src, MPP_CHN_S *dst,
					     int *bound_flag, const char *tag)
{
	ERRORTYPE ret;

	if (!bound_flag || !*bound_flag)
		return;
	ret = AW_MPI_SYS_UnBind(src, dst);
	if (ret != SUCCESS) {
		printf("[carplay_display] unbind %s failed ret=0x%x, continue destroy\n",
		       tag, ret);
	}
	*bound_flag = 0;
}

static ERRORTYPE carplay_display_mpp_callback(void *cookie, MPP_CHN_S *pChn,
					      MPP_EVENT_TYPE event, void *pEventData)
{
	(void)cookie;

	if (!pChn)
		return FAILURE;

	if (pChn->mModId == MOD_ID_VOU) {
		switch (event) {
		case MPP_EVENT_RELEASE_VIDEO_BUFFER:
			return SUCCESS;
		case MPP_EVENT_SET_VIDEO_SIZE:
			if (pEventData) {
				SIZE_S *size = (SIZE_S *)pEventData;
				printf("[carplay_display] vo size %dx%d\n",
				       size->Width, size->Height);
			}
			return SUCCESS;
		case MPP_EVENT_RENDERING_START:
			printf("[carplay_display] vo rendering start\n");
			return SUCCESS;
		default:
			return SUCCESS;
		}
	}

	if (pChn->mModId == MOD_ID_VDEC) {
		switch (event) {
		case MPP_EVENT_VDEC_NOTIFY_NO_FRAME_BUFFER:
			printf("[carplay_display] vdec no frame buffer available\n");
			return SUCCESS;
		default:
			return SUCCESS;
		}
	}

	if (pChn->mModId == MOD_ID_CLOCK)
		return SUCCESS;

	return SUCCESS;
}

static void touch_apply_and_send(int screen_x, int screen_y, int is_touch_down)
{
	int session_width;
	int session_height;

	pthread_mutex_lock(&g_ctx.rect_mutex);
	session_width = g_ctx.policy.session_width;
	session_height = g_ctx.policy.session_height;
	pthread_mutex_unlock(&g_ctx.rect_mutex);

	if (session_width <= 0 || session_height <= 0)
		return;
	if (screen_x < 0 || screen_x >= LVGL_LOGICAL_W ||
	    screen_y < 0 || screen_y >= LVGL_LOGICAL_H)
		return;

	screen_x = (int)((long)screen_x * session_width / LVGL_LOGICAL_W);
	screen_y = (int)((long)screen_y * session_height / LVGL_LOGICAL_H);
	if (screen_x < 0)
		screen_x = 0;
	if (screen_x >= session_width)
		screen_x = session_width - 1;
	if (screen_y < 0)
		screen_y = 0;
	if (screen_y >= session_height)
		screen_y = session_height - 1;

	libzlink_touch_event(screen_x, screen_y, is_touch_down);
}

static void *decode_thread_fn(void *arg)
{
	(void)arg;
	carplay_display_try_raise_priority();

	while (g_ctx.running) {
		h264_packet_t pkt;
		VDEC_STREAM_S stream;
		unsigned char *packet_data;
		void *stream_buf_vir;
		unsigned int stream_buf_phy;
		ERRORTYPE send_ret;
		int attempt;
		int buf_idx;

		if (queue_pop(&pkt) != 0)
			break;

		packet_data = g_queue.block[pkt.block_idx].buf;
		if (pkt.len > (int)g_ctx.stream_stage_size) {
			queue_release_packet(&pkt);
			continue;
		}

		if (!g_ctx.got_idr && !packet_has_sps((const char *)packet_data, pkt.len)) {
			queue_release_packet(&pkt);
			continue;
		}
		g_ctx.got_idr = 1;

		buf_idx = g_ctx.stream_buf_next;
		stream_buf_vir = g_ctx.stream_buf_vir[buf_idx];
		stream_buf_phy = g_ctx.stream_buf_phy[buf_idx];
		g_ctx.stream_buf_next = (g_ctx.stream_buf_next + 1) % g_ctx.stream_buf_count;

		memcpy(stream_buf_vir, packet_data, (size_t)pkt.len);
		if (stream_buf_phy)
			AW_MPI_SYS_MmzFlushCache(stream_buf_phy, stream_buf_vir, pkt.len);

		memset(&stream, 0, sizeof(stream));
		stream.pAddr = (unsigned char *)stream_buf_vir;
		stream.mLen = (unsigned int)pkt.len;
		stream.mbEndOfFrame = TRUE;
		stream.mbEndOfStream = FALSE;

		send_ret = FAILURE;
		for (attempt = 0; attempt < SENDSTREAM_RETRY_MAX; attempt++) {
			uint64_t start_us = monotonic_time_us();
			uint64_t end_us;

			send_ret = AW_MPI_VDEC_SendStream(g_ctx.vdec_chn, &stream, SENDSTREAM_TIMEOUT_MS);
			end_us = monotonic_time_us();
			if (end_us - start_us > g_ctx.max_send_cost_us)
				g_ctx.max_send_cost_us = end_us - start_us;
			if (send_ret == SUCCESS) {
				g_ctx.send_ok_total++;
				break;
			}
			g_ctx.send_busy_total++;
			g_ctx.send_retry_total++;
			if (attempt + 1 < SENDSTREAM_RETRY_MAX)
				usleep(SENDSTREAM_RETRY_US);
		}
		if (send_ret != SUCCESS)
			g_ctx.send_fail_total++;

		queue_release_packet(&pkt);
		carplay_display_log_metrics_if_needed(monotonic_time_us());
	}
	return NULL;
}

int carplay_display_build_policy(int link_type, int disp_x, int disp_y,
                                 int disp_width, int disp_height,
                                 carplay_display_policy_t *policy)
{
	if (!policy || disp_width <= 0 || disp_height <= 0)
		return -1;

	memset(policy, 0, sizeof(*policy));
	policy->link_type = link_type;
	policy->disp_x = disp_x;
	policy->disp_y = disp_y;
	policy->disp_width = disp_width;
	policy->disp_height = disp_height;
	policy->session_width = 1440;
	policy->session_height = 720;
	policy->rotation = ROTATE_270;
	policy->scaling_mode = NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW;

	if (link_type == LINK_TYPE_ANDROIDAUTO) {
		policy->decode_width = 1920;
		policy->decode_height = 1080;
		policy->frame_region_x = 180;
		policy->frame_region_y = 240;
		policy->frame_region_width = 720;
		policy->frame_region_height = 1440;
		policy->scaling_mode = NATIVE_WINDOW_SCALING_MODE_NO_SCALE_CROP;
	} else {
		policy->decode_width = 1440;
		policy->decode_height = 720;
	}

	return 0;
}

int carplay_display_create(const carplay_display_policy_t *policy)
{
	ERRORTYPE ret;
	VDEC_CHN_ATTR_S vdec_attr;
	VO_PUB_ATTR_S vo_pub;
	CLOCK_CHN_ATTR_S clock_attr;
	MPP_CHN_S vdec_chn;
	MPP_CHN_S clock_chn;
	MPP_CHN_S vo_chn;
	MPPCallbackInfo cb_info = {0};
	int queue_inited = 0;
	int decode_thread_started = 0;
	int i;

	if (!policy)
		return -1;
	if (policy->session_width <= 0 || policy->session_height <= 0 ||
	    policy->decode_width <= 0 || policy->decode_height <= 0 ||
	    policy->disp_width <= 0 || policy->disp_height <= 0) {
		return -1;
	}

	pthread_mutex_lock(&g_display_lifecycle_mutex);
	if (g_display_state != DISPLAY_STATE_STOPPED) {
		pthread_mutex_unlock(&g_display_lifecycle_mutex);
		printf("[carplay_display] create ignored, state=%d\n", g_display_state);
		return -1;
	}
	g_display_state = DISPLAY_STATE_STARTING;
	pthread_mutex_unlock(&g_display_lifecycle_mutex);

	memset(&g_ctx, 0, sizeof(g_ctx));
	g_ctx.policy = *policy;
	g_ctx.rotated_width = rotation_swaps_wh(policy->rotation) ? policy->decode_height : policy->decode_width;
	g_ctx.rotated_height = rotation_swaps_wh(policy->rotation) ? policy->decode_width : policy->decode_height;
	g_ctx.vo_dev = 0;
	g_ctx.vo_layer = VO_LAYER_DEFAULT;
	g_ctx.vo_chn = VO_CHN_DEFAULT;
	g_ctx.vdec_chn = VDEC_CHN_DEFAULT;
	g_ctx.clock_chn = CLOCK_CHN_DEFAULT;
	g_ctx.vdec_stream_buf_size = 2 * 1024 * 1024;
	g_ctx.stream_stage_size = STREAM_STAGE_BUF_SIZE;
	g_ctx.stream_buf_count = STREAM_STAGE_BUF_NUM;
	g_ctx.use_g2d_rotate = carplay_display_use_g2d_rotate();

	for (i = 0; i < g_ctx.stream_buf_count; i++) {
		ret = AW_MPI_SYS_MmzAlloc_Cached(&g_ctx.stream_buf_phy[i], &g_ctx.stream_buf_vir[i],
		                                 (int)g_ctx.stream_stage_size);
		if (ret != SUCCESS) {
			printf("[carplay_display] MmzAlloc stream_buf[%d] failed, fallback to malloc\n", i);
			g_ctx.stream_buf_vir[i] = malloc(g_ctx.stream_stage_size);
			g_ctx.stream_buf_phy[i] = 0;
			if (!g_ctx.stream_buf_vir[i])
				goto err_cleanup;
		}
	}

	pthread_mutex_init(&g_ctx.rect_mutex, NULL);
	queue_init();
	queue_inited = 1;

	cb_info.cookie = NULL;
	cb_info.callback = (MPPCallbackFuncType)&carplay_display_mpp_callback;

	memset(&vdec_attr, 0, sizeof(vdec_attr));
	vdec_attr.mType = PT_H264;
	vdec_attr.mBufSize = (unsigned int)g_ctx.vdec_stream_buf_size;
	vdec_attr.mPicWidth = (unsigned int)policy->decode_width;
	vdec_attr.mPicHeight = (unsigned int)policy->decode_height;
	vdec_attr.mInitRotation = (ROTATE_E)policy->rotation;
	vdec_attr.bRotateUseG2d = g_ctx.use_g2d_rotate ? TRUE : FALSE;
	vdec_attr.mOutputPixelFormat = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
	vdec_attr.mVdecVideoAttr.mMode = VIDEO_MODE_STREAM;
	printf("[carplay_display] rotate backend=%s\n",
	       g_ctx.use_g2d_rotate ? "g2d" : "ve");

	ret = AW_MPI_VDEC_CreateChn(g_ctx.vdec_chn, &vdec_attr);
	if (ret != SUCCESS && ret != ERR_VDEC_EXIST)
		goto err_cleanup;
	AW_MPI_VDEC_RegisterCallback(g_ctx.vdec_chn, &cb_info);
	AW_MPI_VDEC_SetRotate(g_ctx.vdec_chn, (ROTATE_E)policy->rotation);

	ret = AW_MPI_VO_Enable(g_ctx.vo_dev);
	if (ret != SUCCESS)
		goto err_cleanup;
	AW_MPI_VO_AddOutsideVideoLayer(1);
	AW_MPI_VO_CloseVideoLayer(1);
	ret = AW_MPI_VO_EnableVideoLayer(g_ctx.vo_layer);
	if (ret != SUCCESS)
		goto err_cleanup;

	memset(&vo_pub, 0, sizeof(vo_pub));
	AW_MPI_VO_GetPubAttr(g_ctx.vo_dev, &vo_pub);
	AW_MPI_VO_SetPubAttr(g_ctx.vo_dev, &vo_pub);

	ret = AW_MPI_VO_CreateChn(g_ctx.vo_layer, g_ctx.vo_chn);
	if (ret != SUCCESS)
		goto err_cleanup;
	AW_MPI_VO_RegisterCallback(g_ctx.vo_layer, g_ctx.vo_chn, &cb_info);
	ret = AW_MPI_VO_SetChnDispBufNum(g_ctx.vo_layer, g_ctx.vo_chn, 2);
	if (ret != SUCCESS)
		goto err_cleanup;
	ret = carplay_display_apply_vo_layout();
	if (ret != SUCCESS)
		goto err_cleanup;

	memset(&clock_attr, 0, sizeof(clock_attr));
	clock_attr.nWaitMask = 1 << CLOCK_PORT_INDEX_VIDEO;
	ret = AW_MPI_CLOCK_CreateChn(g_ctx.clock_chn, &clock_attr);
	if (ret != SUCCESS)
		goto err_cleanup;
	AW_MPI_CLOCK_RegisterCallback(g_ctx.clock_chn, &cb_info);

	vdec_chn.mModId = MOD_ID_VDEC;
	vdec_chn.mDevId = 0;
	vdec_chn.mChnId = g_ctx.vdec_chn;
	clock_chn.mModId = MOD_ID_CLOCK;
	clock_chn.mDevId = 0;
	clock_chn.mChnId = g_ctx.clock_chn;
	vo_chn.mModId = MOD_ID_VOU;
	vo_chn.mDevId = g_ctx.vo_layer;
	vo_chn.mChnId = g_ctx.vo_chn;
	ret = AW_MPI_SYS_Bind(&vdec_chn, &vo_chn);
	if (ret != SUCCESS)
		goto err_cleanup;
	g_ctx.bind_vdec_to_vo = 1;
	ret = AW_MPI_SYS_Bind(&clock_chn, &vo_chn);
	if (ret != SUCCESS)
		goto err_cleanup;
	g_ctx.bind_clock_to_vo = 1;
	ret = AW_MPI_VDEC_StartRecvStream(g_ctx.vdec_chn);
	if (ret != SUCCESS)
		goto err_cleanup;
	ret = AW_MPI_CLOCK_Start(g_ctx.clock_chn);
	if (ret != SUCCESS)
		goto err_cleanup;
	ret = AW_MPI_VO_StartChn(g_ctx.vo_layer, g_ctx.vo_chn);
	if (ret != SUCCESS)
		goto err_cleanup;

	g_ctx.running = 1;
	if (pthread_create(&g_ctx.decode_tid, NULL, decode_thread_fn, NULL) != 0) {
		g_ctx.running = 0;
		queue_fini();
		goto err_cleanup;
	}
	decode_thread_started = 1;
	pthread_mutex_lock(&g_display_lifecycle_mutex);
	g_display_state = DISPLAY_STATE_RUNNING;
	pthread_mutex_unlock(&g_display_lifecycle_mutex);
	return 0;

err_cleanup:
	if (decode_thread_started) {
		g_ctx.running = 0;
		queue_fini();
		pthread_join(g_ctx.decode_tid, NULL);
	}
	if (queue_inited)
		queue_destroy();
	AW_MPI_VO_StopChn(g_ctx.vo_layer, g_ctx.vo_chn);
	AW_MPI_CLOCK_Stop(g_ctx.clock_chn);
	AW_MPI_VDEC_StopRecvStream(g_ctx.vdec_chn);
	carplay_display_unbind_if_needed(&vdec_chn, &vo_chn, &g_ctx.bind_vdec_to_vo, "vdec->vo");
	carplay_display_unbind_if_needed(&clock_chn, &vo_chn, &g_ctx.bind_clock_to_vo, "clock->vo");
	AW_MPI_CLOCK_DestroyChn(g_ctx.clock_chn);
	AW_MPI_VO_DestroyChn(g_ctx.vo_layer, g_ctx.vo_chn);
	AW_MPI_VO_DisableVideoLayer(g_ctx.vo_layer);
	AW_MPI_VO_RemoveOutsideVideoLayer(1);
	AW_MPI_VO_Disable(g_ctx.vo_dev);
	AW_MPI_VDEC_DestroyChn(g_ctx.vdec_chn);
	for (i = 0; i < g_ctx.stream_buf_count; i++) {
		if (g_ctx.stream_buf_phy[i])
			AW_MPI_SYS_MmzFree(g_ctx.stream_buf_phy[i], g_ctx.stream_buf_vir[i]);
		else if (g_ctx.stream_buf_vir[i])
			free(g_ctx.stream_buf_vir[i]);
	}
	if (queue_inited)
		pthread_mutex_destroy(&g_ctx.rect_mutex);
	memset(&g_ctx, 0, sizeof(g_ctx));
	pthread_mutex_lock(&g_display_lifecycle_mutex);
	g_display_state = DISPLAY_STATE_STOPPED;
	pthread_mutex_unlock(&g_display_lifecycle_mutex);
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
	if (!g_ctx.running)
		return;

	pthread_mutex_lock(&g_ctx.rect_mutex);
	g_ctx.policy.disp_x = x;
	g_ctx.policy.disp_y = y;
	if (width > 0)
		g_ctx.policy.disp_width = width;
	if (height > 0)
		g_ctx.policy.disp_height = height;
	pthread_mutex_unlock(&g_ctx.rect_mutex);

	carplay_display_apply_vo_layout();
}

void carplay_display_destroy(void)
{
	MPP_CHN_S vdec_chn;
	MPP_CHN_S clock_chn;
	MPP_CHN_S vo_chn;
	int i;

	pthread_mutex_lock(&g_display_lifecycle_mutex);
	if (g_display_state != DISPLAY_STATE_RUNNING) {
		pthread_mutex_unlock(&g_display_lifecycle_mutex);
		return;
	}
	g_display_state = DISPLAY_STATE_STOPPING;
	pthread_mutex_unlock(&g_display_lifecycle_mutex);

	g_ctx.running = 0;
	queue_fini();
	pthread_join(g_ctx.decode_tid, NULL);
	queue_destroy();

	vdec_chn.mModId = MOD_ID_VDEC;
	vdec_chn.mDevId = 0;
	vdec_chn.mChnId = g_ctx.vdec_chn;
	clock_chn.mModId = MOD_ID_CLOCK;
	clock_chn.mDevId = 0;
	clock_chn.mChnId = g_ctx.clock_chn;
	vo_chn.mModId = MOD_ID_VOU;
	vo_chn.mDevId = g_ctx.vo_layer;
	vo_chn.mChnId = g_ctx.vo_chn;

	AW_MPI_VO_StopChn(g_ctx.vo_layer, g_ctx.vo_chn);
	AW_MPI_CLOCK_Stop(g_ctx.clock_chn);
	AW_MPI_VDEC_StopRecvStream(g_ctx.vdec_chn);

	carplay_display_unbind_if_needed(&vdec_chn, &vo_chn, &g_ctx.bind_vdec_to_vo, "vdec->vo");
	carplay_display_unbind_if_needed(&clock_chn, &vo_chn, &g_ctx.bind_clock_to_vo, "clock->vo");

	AW_MPI_CLOCK_DestroyChn(g_ctx.clock_chn);
	AW_MPI_VO_DestroyChn(g_ctx.vo_layer, g_ctx.vo_chn);
	AW_MPI_VO_DisableVideoLayer(g_ctx.vo_layer);
	AW_MPI_VO_RemoveOutsideVideoLayer(1);
	AW_MPI_VO_Disable(g_ctx.vo_dev);
	AW_MPI_VDEC_DestroyChn(g_ctx.vdec_chn);

	for (i = 0; i < g_ctx.stream_buf_count; i++) {
		if (g_ctx.stream_buf_phy[i])
			AW_MPI_SYS_MmzFree(g_ctx.stream_buf_phy[i], g_ctx.stream_buf_vir[i]);
		else if (g_ctx.stream_buf_vir[i])
			free(g_ctx.stream_buf_vir[i]);
	}
	pthread_mutex_destroy(&g_ctx.rect_mutex);
	memset(&g_ctx, 0, sizeof(g_ctx));
	pthread_mutex_lock(&g_display_lifecycle_mutex);
	g_display_state = DISPLAY_STATE_STOPPED;
	pthread_mutex_unlock(&g_display_lifecycle_mutex);
}

void carplay_touch_send(void)
{
	touch_apply_and_send(carplay_touch_screen_x, carplay_touch_screen_y,
	                     carplay_touch_screen_down);
}

void carplay_touch_send_xy(int screen_x, int screen_y, int is_touch_down)
{
	touch_apply_and_send(screen_x, screen_y, is_touch_down);
}

#endif /* ENABLE_CARPLAY */
