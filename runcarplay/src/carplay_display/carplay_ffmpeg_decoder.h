#ifndef CARPLAY_FFMPEG_DECODER_H
#define CARPLAY_FFMPEG_DECODER_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct carplay_ffmpeg_decoder carplay_ffmpeg_decoder_t;

typedef struct {
	int width;
	int height;
	int linesize[3];
	uint8_t *data[3];
	int64_t pts;
} carplay_ffmpeg_frame_t;

int carplay_ffmpeg_decoder_create(carplay_ffmpeg_decoder_t **out, int width_hint, int height_hint);
int carplay_ffmpeg_decoder_send_packet(carplay_ffmpeg_decoder_t *dec, const uint8_t *data, int len);
int carplay_ffmpeg_decoder_receive_frame(carplay_ffmpeg_decoder_t *dec, carplay_ffmpeg_frame_t *out);
int carplay_ffmpeg_decoder_flush(carplay_ffmpeg_decoder_t *dec);
void carplay_ffmpeg_decoder_destroy(carplay_ffmpeg_decoder_t *dec);

#ifdef __cplusplus
}
#endif

#endif /* CARPLAY_FFMPEG_DECODER_H */
