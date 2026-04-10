#include "carplay_ffmpeg_decoder.h"

#include <libavcodec/avcodec.h>
#include <libavutil/error.h>
#include <libavutil/frame.h>
#include <libavutil/pixfmt.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct carplay_ffmpeg_decoder {
	AVCodecContext *codec_ctx;
	AVPacket *packet;
	AVFrame *src_frame;
	uint8_t *nv21;
	int nv21_linesize;
	int nv21_w;
	int nv21_h;
	size_t nv21_size;
	int last_pix_fmt;
	int fmt_log_init;
};

static void cp_ffmpeg_log_error(const char *tag, int err)
{
	char errbuf[128];
	av_strerror(err, errbuf, sizeof(errbuf));
	printf("[carplay_ffmpeg] %s failed: %s (%d)\n", tag, errbuf, err);
}

int carplay_ffmpeg_decoder_create(carplay_ffmpeg_decoder_t **out, int width_hint, int height_hint)
{
	const AVCodec *codec;
	carplay_ffmpeg_decoder_t *dec;
	int ret;

	if (!out)
		return -1;
	*out = NULL;

	codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!codec) {
		printf("[carplay_ffmpeg] H264 decoder not found\n");
		return -1;
	}

	dec = (carplay_ffmpeg_decoder_t *)calloc(1, sizeof(*dec));
	if (!dec)
		return -1;

	dec->codec_ctx = avcodec_alloc_context3(codec);
	dec->packet = av_packet_alloc();
	dec->src_frame = av_frame_alloc();
	if (!dec->codec_ctx || !dec->packet || !dec->src_frame) {
		carplay_ffmpeg_decoder_destroy(dec);
		return -1;
	}

	if (width_hint > 0)
		dec->codec_ctx->width = width_hint;
	if (height_hint > 0)
		dec->codec_ctx->height = height_hint;
	/* V853 class CPU prefers lower thread sync overhead. */
	dec->codec_ctx->thread_count = 1;
	dec->codec_ctx->thread_type = FF_THREAD_SLICE;
	dec->codec_ctx->pkt_timebase.num = 1;
	dec->codec_ctx->pkt_timebase.den = 1000000;

	ret = avcodec_open2(dec->codec_ctx, codec, NULL);
	if (ret < 0) {
		cp_ffmpeg_log_error("avcodec_open2", ret);
		carplay_ffmpeg_decoder_destroy(dec);
		return -1;
	}
	dec->last_pix_fmt = -1;
	dec->fmt_log_init = 0;
	printf("[carplay_ffmpeg] decoder_open ok, hint=%dx%d threads=%d\n",
	       width_hint, height_hint, dec->codec_ctx->thread_count);

	*out = dec;
	return 0;
}

int carplay_ffmpeg_decoder_send_packet(carplay_ffmpeg_decoder_t *dec, const uint8_t *data, int len)
{
	int ret;
	int retry = 0;

	if (!dec || !data || len <= 0)
		return -1;

	av_packet_unref(dec->packet);
	ret = av_new_packet(dec->packet, len);
	if (ret < 0) {
		cp_ffmpeg_log_error("av_new_packet", ret);
		return -1;
	}
	memcpy(dec->packet->data, data, (size_t)len);

retry_send:
	ret = avcodec_send_packet(dec->codec_ctx, dec->packet);
	if (ret == AVERROR(EAGAIN) && retry == 0) {
		retry = 1;
		ret = avcodec_receive_frame(dec->codec_ctx, dec->src_frame);
		if (ret >= 0)
			av_frame_unref(dec->src_frame);
		goto retry_send;
	}
	if (ret < 0) {
		cp_ffmpeg_log_error("avcodec_send_packet", ret);
		return -1;
	}
	return 0;
}

int carplay_ffmpeg_decoder_receive_frame(carplay_ffmpeg_decoder_t *dec, carplay_ffmpeg_frame_t *out)
{
	int ret;
	int y_w;
	int y_h;

	if (!dec || !out)
		return -1;

	ret = avcodec_receive_frame(dec->codec_ctx, dec->src_frame);
	if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
		return 1;
	if (ret < 0) {
		cp_ffmpeg_log_error("avcodec_receive_frame", ret);
		return -1;
	}

	out->width = dec->src_frame->width;
	out->height = dec->src_frame->height;
	out->pts = dec->src_frame->pts;
	if (!dec->fmt_log_init || dec->last_pix_fmt != dec->src_frame->format) {
		dec->fmt_log_init = 1;
		dec->last_pix_fmt = dec->src_frame->format;
		printf("[carplay_ffmpeg] frame_fmt_change fmt=%d w=%d h=%d ls0=%d ls1=%d\n",
		       dec->src_frame->format, dec->src_frame->width, dec->src_frame->height,
		       dec->src_frame->linesize[0], dec->src_frame->linesize[1]);
	}
	if (dec->src_frame->format == AV_PIX_FMT_NV21) {
		out->data[0] = dec->src_frame->data[0];
		out->data[1] = dec->src_frame->data[1];
		out->data[2] = NULL;
		out->linesize[0] = dec->src_frame->linesize[0];
		out->linesize[1] = dec->src_frame->linesize[1];
		out->linesize[2] = 0;
		av_frame_unref(dec->src_frame);
		return 0;
	}

	y_w = dec->src_frame->width;
	y_h = dec->src_frame->height;
	if ((size_t)(y_w * y_h * 3 / 2) > dec->nv21_size || dec->nv21_w != y_w || dec->nv21_h != y_h) {
		free(dec->nv21);
		dec->nv21_size = (size_t)(y_w * y_h * 3 / 2);
		dec->nv21 = (uint8_t *)malloc(dec->nv21_size);
		if (!dec->nv21) {
			av_frame_unref(dec->src_frame);
			return -1;
		}
		dec->nv21_w = y_w;
		dec->nv21_h = y_h;
		dec->nv21_linesize = y_w;
	}

	if (dec->src_frame->format == AV_PIX_FMT_YUV420P || dec->src_frame->format == AV_PIX_FMT_YUVJ420P) {
		uint8_t *dst_y = dec->nv21;
		uint8_t *dst_vu = dec->nv21 + y_w * y_h;
		for (int r = 0; r < y_h; r++) {
			memcpy(dst_y + (size_t)r * y_w,
			       dec->src_frame->data[0] + (size_t)r * dec->src_frame->linesize[0],
			       (size_t)y_w);
		}
		for (int r = 0; r < y_h / 2; r++) {
			for (int c = 0; c < y_w / 2; c++) {
				uint8_t u = dec->src_frame->data[1][r * dec->src_frame->linesize[1] + c];
				uint8_t v = dec->src_frame->data[2][r * dec->src_frame->linesize[2] + c];
				dst_vu[r * y_w + 2 * c] = v;
				dst_vu[r * y_w + 2 * c + 1] = u;
			}
		}
	} else if (dec->src_frame->format == AV_PIX_FMT_NV12) {
		uint8_t *dst_y = dec->nv21;
		uint8_t *dst_vu = dec->nv21 + y_w * y_h;
		const uint8_t *src_uv = dec->src_frame->data[1];
		for (int r = 0; r < y_h; r++) {
			memcpy(dst_y + (size_t)r * y_w,
			       dec->src_frame->data[0] + (size_t)r * dec->src_frame->linesize[0],
			       (size_t)y_w);
		}
		for (int r = 0; r < y_h / 2; r++) {
			for (int c = 0; c < y_w / 2; c++) {
				uint8_t u = src_uv[r * dec->src_frame->linesize[1] + 2 * c];
				uint8_t v = src_uv[r * dec->src_frame->linesize[1] + 2 * c + 1];
				dst_vu[r * y_w + 2 * c] = v;
				dst_vu[r * y_w + 2 * c + 1] = u;
			}
		}
	} else {
		printf("[carplay_ffmpeg] unsupported format: %d\n", dec->src_frame->format);
		av_frame_unref(dec->src_frame);
		return -1;
	}

	out->data[0] = dec->nv21;
	out->data[1] = dec->nv21 + y_w * y_h;
	out->data[2] = NULL;
	out->linesize[0] = dec->nv21_linesize;
	out->linesize[1] = dec->nv21_linesize;
	out->linesize[2] = 0;

	av_frame_unref(dec->src_frame);
	return 0;
}

int carplay_ffmpeg_decoder_flush(carplay_ffmpeg_decoder_t *dec)
{
	int ret;

	if (!dec)
		return -1;
	ret = avcodec_send_packet(dec->codec_ctx, NULL);
	if (ret < 0 && ret != AVERROR_EOF && ret != AVERROR(EAGAIN)) {
		cp_ffmpeg_log_error("avcodec_send_packet(flush)", ret);
		return -1;
	}
	return 0;
}

void carplay_ffmpeg_decoder_destroy(carplay_ffmpeg_decoder_t *dec)
{
	if (!dec)
		return;
	free(dec->nv21);
	av_frame_free(&dec->src_frame);
	av_packet_free(&dec->packet);
	avcodec_free_context(&dec->codec_ctx);
	free(dec);
}
