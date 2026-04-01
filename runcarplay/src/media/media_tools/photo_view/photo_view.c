#include "photo_view.h"
#include <string.h>

static const photo_view_zoom_t zoom_levels[] = {
    PHOTO_ZOOM_50, PHOTO_ZOOM_75, PHOTO_ZOOM_100, PHOTO_ZOOM_150, PHOTO_ZOOM_200
};
static const int zoom_level_count = sizeof(zoom_levels) / sizeof(zoom_levels[0]);

photo_view_zoom_t photo_view_zoom_in(photo_view_zoom_t current)
{
    for (int i = 0; i < zoom_level_count - 1; i++) {
        if (zoom_levels[i] == current)
            return zoom_levels[i + 1];
    }
    return current;
}

photo_view_zoom_t photo_view_zoom_out(photo_view_zoom_t current)
{
    for (int i = 1; i < zoom_level_count; i++) {
        if (zoom_levels[i] == current)
            return zoom_levels[i - 1];
    }
    return current;
}

photo_view_rotate_t photo_view_rotate_90_cw(photo_view_rotate_t current)
{
    switch (current) {
        case PHOTO_ROTATE_0:   return PHOTO_ROTATE_90;
        case PHOTO_ROTATE_90:  return PHOTO_ROTATE_180;
        case PHOTO_ROTATE_180: return PHOTO_ROTATE_270;
        case PHOTO_ROTATE_270: return PHOTO_ROTATE_0;
        default:               return PHOTO_ROTATE_0;
    }
}

void photo_view_bilinear_scale_argb8888(const unsigned char *src, unsigned int src_w, unsigned int src_h,
                                       unsigned char *dst, unsigned int dst_w, unsigned int dst_h)
{
    if (!src || !dst || src_w == 0 || src_h == 0 || dst_w == 0 || dst_h == 0) return;
    float x_ratio = (float)(src_w - 1) / (float)dst_w;
    float y_ratio = (float)(src_h - 1) / (float)dst_h;
    for (unsigned int y = 0; y < dst_h; y++) {
        for (unsigned int x = 0; x < dst_w; x++) {
            float x_src = x * x_ratio;
            float y_src = y * y_ratio;
            unsigned int x_int = (unsigned int)x_src;
            unsigned int y_int = (unsigned int)y_src;
            float x_frac = x_src - x_int;
            float y_frac = y_src - y_int;
            unsigned int x2 = (x_int + 1 < src_w) ? x_int + 1 : x_int;
            unsigned int y2 = (y_int + 1 < src_h) ? y_int + 1 : y_int;
            unsigned int idx_tl = (y_int * src_w + x_int) * 4;
            unsigned int idx_tr = (y_int * src_w + x2) * 4;
            unsigned int idx_bl = (y2 * src_w + x_int) * 4;
            unsigned int idx_br = (y2 * src_w + x2) * 4;
            unsigned int dst_idx = (y * dst_w + x) * 4;
            for (int c = 0; c < 4; c++) {
                float top = src[idx_tl + c] * (1.f - x_frac) + src[idx_tr + c] * x_frac;
                float bottom = src[idx_bl + c] * (1.f - x_frac) + src[idx_br + c] * x_frac;
                float value = top * (1.f - y_frac) + bottom * y_frac;
                if (value > 255.f) value = 255.f;
                if (value < 0.f) value = 0.f;
                dst[dst_idx + c] = (unsigned char)value;
            }
        }
    }
}

void photo_view_rotate_90_cw_argb8888(const unsigned char *src, unsigned int src_w, unsigned int src_h,
                                      unsigned char *dst)
{
    if (!src || !dst) return;
    const unsigned int PIXEL_SIZE = 4;
    for (unsigned int y = 0; y < src_h; y++) {
        const unsigned char *src_row = src + y * src_w * PIXEL_SIZE;
        for (unsigned int x = 0; x < src_w; x++) {
            const unsigned char *src_pixel = src_row + x * PIXEL_SIZE;
            unsigned int dst_x = y;
            unsigned int dst_y = src_w - 1 - x;
            unsigned char *dst_pixel = dst + (dst_y * src_h + dst_x) * PIXEL_SIZE;
            dst_pixel[0] = src_pixel[0];
            dst_pixel[1] = src_pixel[1];
            dst_pixel[2] = src_pixel[2];
            dst_pixel[3] = src_pixel[3];
        }
    }
}
