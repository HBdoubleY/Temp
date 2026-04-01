#define _GNU_SOURCE
#ifdef ENABLE_CARPLAY

#include "carplay_display.h"
#include "mpp_compat.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <g2d_driver.h>
#include <PIXEL_FORMAT_E_g2d_format_convert.h>

extern int libzlink_touch_event(int x, int y, int is_touch_down);

extern int g_g2dfd;

int carplay_touch_screen_x   = 0;
int carplay_touch_screen_y   = 0;
int carplay_touch_screen_down = 0;
int carplay_split_screen_enable = 0;
int carplay_split_line_x = 720;
#define H264_QUEUE_CAP      128
#define H264_PACKET_MAX     (256 * 1024)

typedef struct {
	char *data;
	int len;
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

#define FRAME_QUEUE_CAP 4

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

static struct {
	int disp_x, disp_y, disp_width, disp_height;
	int session_width, session_height;
	int vo_dev;
	int vo_layer;
	int vo_chn;
	int vdec_chn;
	int clock_chn;
	pthread_t decode_tid;
	pthread_t display_tid;
	volatile int running;
	unsigned int stream_buf_phy;
	void *stream_buf_vir;
	size_t stream_buf_size;
	pthread_mutex_t rect_mutex;
	VIDEO_FRAME_INFO_S g2d_dst[3];
	int g2d_dst_allocated;
	int g2d_buf_idx;
	pthread_mutex_t g2d_use_mutex;
	pthread_cond_t  g2d_use_cond;
	int g2d_dst_in_use[3];
	volatile int got_idr;
} g_ctx;

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

static int queue_push(const char *data, int len)
{
	if (len <= 0 || len > H264_PACKET_MAX)
		return -1;
	pthread_mutex_lock(&g_queue.mutex);
	if (g_queue.count >= H264_QUEUE_CAP) {
		pthread_mutex_unlock(&g_queue.mutex);
		return -1;
	}
	char *copy = (char *)malloc((size_t)len);
	if (!copy) {
		pthread_mutex_unlock(&g_queue.mutex);
		return -1;
	}
	memcpy(copy, data, (size_t)len);
	g_queue.slot[g_queue.tail].data = copy;
	g_queue.slot[g_queue.tail].len  = len;
	g_queue.tail = (g_queue.tail + 1) % H264_QUEUE_CAP;
	g_queue.count++;
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
	g_queue.slot[g_queue.head].data = NULL;
	g_queue.head = (g_queue.head + 1) % H264_QUEUE_CAP;
	g_queue.count--;
	pthread_mutex_unlock(&g_queue.mutex);
	return 0;
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
	int crop_x = 0;
	int crop_y = 0;
	int crop_w = src_w;
	int crop_h = src_h;

	if (src_w >= 1440 && src_h >= 720) {
		crop_w = 1440;
		crop_h = 720;
		crop_x = (src_w - crop_w) / 2;
		crop_y = (src_h - crop_h) / 2;
	}
	blit.src_image_h.clip_rect.x = crop_x;
	blit.src_image_h.clip_rect.y = crop_y;
	blit.src_image_h.clip_rect.w = crop_w;
	blit.src_image_h.clip_rect.h = crop_h;
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
	VDEC_STREAM_S stream;
	VIDEO_FRAME_INFO_S frame;
	memset(&stream, 0, sizeof(stream));
	stream.pAddr = (unsigned char *)g_ctx.stream_buf_vir;

	while (g_ctx.running) {
		h264_packet_t pkt;
		if (queue_pop(&pkt) != 0)
			break;
		if (pkt.len > (int)g_ctx.stream_buf_size) {
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

		memcpy(g_ctx.stream_buf_vir, pkt.data, (size_t)pkt.len);
		AW_MPI_SYS_MmzFlushCache(g_ctx.stream_buf_phy, g_ctx.stream_buf_vir, (int)pkt.len);
		free(pkt.data);

		stream.mLen = (unsigned int)pkt.len;
		stream.mbEndOfFrame = TRUE;

		if (AW_MPI_VDEC_SendStream(g_ctx.vdec_chn, &stream, 200) != SUCCESS)
			continue;


		for (;;) {
			ERRORTYPE ret = AW_MPI_VDEC_GetImage(g_ctx.vdec_chn, &frame, 0);
			if (ret != SUCCESS)
				break;

			pthread_mutex_lock(&g_fq.mutex);
			if (g_fq.count >= FRAME_QUEUE_CAP) {
				VIDEO_FRAME_INFO_S old = g_fq.slot[g_fq.head];
				g_fq.head = (g_fq.head + 1) % FRAME_QUEUE_CAP;
				g_fq.count--;
				AW_MPI_VDEC_ReleaseImage(g_ctx.vdec_chn, &old);
			}
			g_fq.slot[g_fq.tail] = frame;
			g_fq.tail = (g_fq.tail + 1) % FRAME_QUEUE_CAP;
			g_fq.count++;
			pthread_cond_signal(&g_fq.cond);
			pthread_mutex_unlock(&g_fq.mutex);
		}

		if (AW_MPI_VDEC_GetImage(g_ctx.vdec_chn, &frame, 30) == SUCCESS) {
			pthread_mutex_lock(&g_fq.mutex);
			if (g_fq.count >= FRAME_QUEUE_CAP) {
				VIDEO_FRAME_INFO_S old = g_fq.slot[g_fq.head];
				g_fq.head = (g_fq.head + 1) % FRAME_QUEUE_CAP;
				g_fq.count--;
				AW_MPI_VDEC_ReleaseImage(g_ctx.vdec_chn, &old);
			}
			g_fq.slot[g_fq.tail] = frame;
			g_fq.tail = (g_fq.tail + 1) % FRAME_QUEUE_CAP;
			g_fq.count++;
			pthread_cond_signal(&g_fq.cond);
			pthread_mutex_unlock(&g_fq.mutex);
		}
	}

	fq_shutdown();
	return NULL;
}

static void *display_thread_fn(void *arg)
{
	(void)arg;

	while (g_ctx.running) {
		VIDEO_FRAME_INFO_S frame;
		int got = 0;

		pthread_mutex_lock(&g_fq.mutex);
		while (g_fq.count == 0 && !g_fq.shutdown)
			pthread_cond_wait(&g_fq.cond, &g_fq.mutex);

		if (g_fq.shutdown && g_fq.count == 0) {
			pthread_mutex_unlock(&g_fq.mutex);
			break;
		}

		while (g_fq.count > 1) {
			VIDEO_FRAME_INFO_S old = g_fq.slot[g_fq.head];
			g_fq.head = (g_fq.head + 1) % FRAME_QUEUE_CAP;
			g_fq.count--;
			AW_MPI_VDEC_ReleaseImage(g_ctx.vdec_chn, &old);
		}
		frame = g_fq.slot[g_fq.head];
		g_fq.head = (g_fq.head + 1) % FRAME_QUEUE_CAP;
		g_fq.count--;
		got = 1;
		pthread_mutex_unlock(&g_fq.mutex);

		if (!got)
			continue;

		if (!g_ctx.g2d_dst_allocated) {
			int dst_w = g_ctx.disp_width;
			int dst_h = g_ctx.disp_height;
			if (g2d_alloc_dst(dst_w, dst_h, frame.VFrame.mPixelFormat) != 0) {
				AW_MPI_VO_SendFrame(g_ctx.vo_layer, g_ctx.vo_chn, &frame, 0);
				AW_MPI_VDEC_ReleaseImage(g_ctx.vdec_chn, &frame);
				continue;
			}

			pthread_mutex_lock(&g_ctx.g2d_use_mutex);
			for (int i = 0; i < 3; i++)
				g_ctx.g2d_dst_in_use[i] = 0;
			pthread_mutex_unlock(&g_ctx.g2d_use_mutex);
		}

		int cur_buf = g_ctx.g2d_buf_idx;

		pthread_mutex_lock(&g_ctx.g2d_use_mutex);
		while (g_ctx.running && g_ctx.g2d_dst_in_use[cur_buf]) {
			pthread_cond_wait(&g_ctx.g2d_use_cond, &g_ctx.g2d_use_mutex);
		}
		if (!g_ctx.running) {
			pthread_mutex_unlock(&g_ctx.g2d_use_mutex);
			break;
		}
		g_ctx.g2d_dst_in_use[cur_buf] = 1;
		pthread_mutex_unlock(&g_ctx.g2d_use_mutex);

		VIDEO_FRAME_INFO_S *dst = &g_ctx.g2d_dst[cur_buf];
		int g2d_ret = g2d_rotate_frame(&frame, dst);
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
		AW_MPI_VDEC_ReleaseImage(g_ctx.vdec_chn, &frame);
	}
	return NULL;
}

#define LVGL_LOGICAL_W  1440
#define LVGL_LOGICAL_H  720

static void touch_apply_and_send(int screen_x, int screen_y, int is_touch_down)
{
	int session_width, session_height;

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
}

static ERRORTYPE carplay_vo_callback(void *cookie, MPP_CHN_S *pChn, MPP_EVENT_TYPE event, void *pEventData)
{
	(void)cookie; (void)pChn;

	if (event == MPP_EVENT_RELEASE_VIDEO_BUFFER) {
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
	ERRORTYPE ret;
	VDEC_CHN_ATTR_S vdec_attr;
	VO_PUB_ATTR_S vo_pub;
	VO_VIDEO_LAYER_ATTR_S layer_attr;
	CLOCK_CHN_ATTR_S clock_attr;
	MPP_CHN_S clock_chn, vo_chn;
	int queue_inited = 0;
	int fq_inited = 0;
	int decode_thread_started = 0;
	int display_thread_started = 0;

	if (g_ctx.running)
		return -1;
	if (session_width <= 0 || session_height <= 0)
		return -1;

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
	g_ctx.vdec_chn = VDEC_CHN_DEFAULT;
	g_ctx.clock_chn = CLOCK_CHN_DEFAULT;
	g_ctx.stream_buf_size = 2 * 1024 * 1024;
	ret = AW_MPI_SYS_MmzAlloc_Cached(&g_ctx.stream_buf_phy, &g_ctx.stream_buf_vir,
	                                  (int)g_ctx.stream_buf_size);
	if (ret != SUCCESS) {
		printf("[carplay_display] MmzAlloc stream_buf failed, fallback to malloc\n");
		g_ctx.stream_buf_vir = malloc(g_ctx.stream_buf_size);
		g_ctx.stream_buf_phy = 0;
		if (!g_ctx.stream_buf_vir)
			return -1;
	}

	pthread_mutex_init(&g_ctx.rect_mutex, NULL);
	pthread_mutex_init(&g_ctx.g2d_use_mutex, NULL);
	pthread_cond_init(&g_ctx.g2d_use_cond, NULL);
	for (int i = 0; i < 3; i++)
		g_ctx.g2d_dst_in_use[i] = 0;

	queue_init();
	queue_inited = 1;
	fq_init();
	fq_inited = 1;

	MPPCallbackInfo cb_empty = { 0 };

	memset(&vdec_attr, 0, sizeof(vdec_attr));
	vdec_attr.mType = PT_H264;
	vdec_attr.mOutputPixelFormat = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
	vdec_attr.mBufSize = (unsigned int)(2 * 1024 * 1024);
	vdec_attr.mPicWidth  = (unsigned int)session_width;
	vdec_attr.mPicHeight = (unsigned int)session_height;
	vdec_attr.mVdecVideoAttr.mMode = VIDEO_MODE_STREAM;
	ret = AW_MPI_VDEC_CreateChn(g_ctx.vdec_chn, &vdec_attr);
	if (ret != SUCCESS && ret != ERR_VDEC_EXIST) {
		if (g_ctx.stream_buf_phy)
			AW_MPI_SYS_MmzFree(g_ctx.stream_buf_phy, g_ctx.stream_buf_vir);
		else
			free(g_ctx.stream_buf_vir);
		if (fq_inited)
			fq_destroy();
		if (queue_inited)
			queue_destroy();
		return -1;
	}
	AW_MPI_VDEC_RegisterCallback(g_ctx.vdec_chn, &cb_empty);
	AW_MPI_VDEC_StartRecvStream(g_ctx.vdec_chn);

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

	AW_MPI_VO_CreateChn(g_ctx.vo_layer, g_ctx.vo_chn);
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
	AW_MPI_VDEC_StopRecvStream(g_ctx.vdec_chn);
	AW_MPI_VDEC_DestroyChn(g_ctx.vdec_chn);
	if (g_ctx.stream_buf_phy)
		AW_MPI_SYS_MmzFree(g_ctx.stream_buf_phy, g_ctx.stream_buf_vir);
	else
		free(g_ctx.stream_buf_vir);
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
		AW_MPI_VDEC_ReleaseImage(g_ctx.vdec_chn, &old);
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
	AW_MPI_VDEC_StopRecvStream(g_ctx.vdec_chn);
	AW_MPI_VDEC_DestroyChn(g_ctx.vdec_chn);

	g2d_free_dst();

	if (g_ctx.stream_buf_phy)
		AW_MPI_SYS_MmzFree(g_ctx.stream_buf_phy, g_ctx.stream_buf_vir);
	else
		free(g_ctx.stream_buf_vir);
	g_ctx.stream_buf_vir = NULL;
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
	touch_apply_and_send(screen_x, screen_y, is_touch_down);
}

#endif /* ENABLE_CARPLAY */
