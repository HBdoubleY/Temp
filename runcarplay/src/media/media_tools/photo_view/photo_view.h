#ifndef PHOTO_VIEW_H
#define PHOTO_VIEW_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PHOTO_ZOOM_50  = 50,
    PHOTO_ZOOM_75  = 75,
    PHOTO_ZOOM_100 = 100,
    PHOTO_ZOOM_150 = 150,
    PHOTO_ZOOM_200 = 200
} photo_view_zoom_t;

typedef enum {
    PHOTO_ROTATE_0   = 0,
    PHOTO_ROTATE_90  = 90,
    PHOTO_ROTATE_180 = 180,
    PHOTO_ROTATE_270 = 270
} photo_view_rotate_t;

photo_view_zoom_t photo_view_zoom_in(photo_view_zoom_t current);

photo_view_zoom_t photo_view_zoom_out(photo_view_zoom_t current);

photo_view_rotate_t photo_view_rotate_90_cw(photo_view_rotate_t current);

void photo_view_bilinear_scale_argb8888(const unsigned char *src, unsigned int src_w, unsigned int src_h,
                                       unsigned char *dst, unsigned int dst_w, unsigned int dst_h);

void photo_view_rotate_90_cw_argb8888(const unsigned char *src, unsigned int src_w, unsigned int src_h,
                                      unsigned char *dst);

#ifdef __cplusplus
}
#endif

#endif /* PHOTO_VIEW_H */
