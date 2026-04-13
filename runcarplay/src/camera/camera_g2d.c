#include "camera_g2d.h"

#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

#include "g2d_driver.h"
#include "PIXEL_FORMAT_E_g2d_format_convert.h"

static int camera_g2d_flag_from_degree(int degree)
{
    switch (degree) {
    case 0:
        return G2D_BLT_NONE_H;
    case 90:
        return G2D_ROT_90;
    case 180:
        return G2D_ROT_180;
    case 270:
        return G2D_ROT_270;
    default:
        return -1;
    }
}

int camera_g2d_transform_frame(int g2d_fd, const VIDEO_FRAME_INFO_S *src, VIDEO_FRAME_INFO_S *dst, const camera_g2d_transform_cfg_t *cfg)
{
    g2d_blt_h blit;
    g2d_fmt_enh src_fmt;
    g2d_fmt_enh dst_fmt;
    int flag_h;

    if (g2d_fd <= 0 || src == NULL || dst == NULL || cfg == NULL) {
        return -1;
    }

    flag_h = camera_g2d_flag_from_degree(cfg->rotate_degree);
    if (flag_h < 0) {
        printf("camera_g2d: invalid rotate degree=%d\n", cfg->rotate_degree);
        return -1;
    }

    if (convert_PIXEL_FORMAT_E_to_g2d_fmt_enh(src->VFrame.mPixelFormat, &src_fmt) != SUCCESS) {
        return -1;
    }
    if (convert_PIXEL_FORMAT_E_to_g2d_fmt_enh(dst->VFrame.mPixelFormat, &dst_fmt) != SUCCESS) {
        return -1;
    }

    memset(&blit, 0, sizeof(blit));
    blit.flag_h = flag_h;

    blit.src_image_h.format = src_fmt;
    blit.src_image_h.laddr[0] = src->VFrame.mPhyAddr[0];
    blit.src_image_h.laddr[1] = src->VFrame.mPhyAddr[1];
    blit.src_image_h.laddr[2] = src->VFrame.mPhyAddr[2];
    blit.src_image_h.width = src->VFrame.mWidth;
    blit.src_image_h.height = src->VFrame.mHeight;
    blit.src_image_h.clip_rect.x = src->VFrame.mOffsetLeft;
    blit.src_image_h.clip_rect.y = src->VFrame.mOffsetTop;
    blit.src_image_h.clip_rect.w =
        (src->VFrame.mOffsetRight > src->VFrame.mOffsetLeft) ?
        (src->VFrame.mOffsetRight - src->VFrame.mOffsetLeft) : src->VFrame.mWidth;
    blit.src_image_h.clip_rect.h =
        (src->VFrame.mOffsetBottom > src->VFrame.mOffsetTop) ?
        (src->VFrame.mOffsetBottom - src->VFrame.mOffsetTop) : src->VFrame.mHeight;
    blit.src_image_h.gamut = G2D_BT709;
    blit.src_image_h.mode = G2D_PIXEL_ALPHA;
    blit.src_image_h.fd = -1;
    blit.src_image_h.use_phy_addr = 1;

    blit.dst_image_h.format = dst_fmt;
    blit.dst_image_h.laddr[0] = dst->VFrame.mPhyAddr[0];
    blit.dst_image_h.laddr[1] = dst->VFrame.mPhyAddr[1];
    blit.dst_image_h.laddr[2] = dst->VFrame.mPhyAddr[2];
    blit.dst_image_h.width = dst->VFrame.mWidth;
    blit.dst_image_h.height = dst->VFrame.mHeight;
    blit.dst_image_h.clip_rect.x = 0;
    blit.dst_image_h.clip_rect.y = 0;
    blit.dst_image_h.clip_rect.w = cfg->enable_scale ? dst->VFrame.mWidth : blit.src_image_h.clip_rect.w;
    blit.dst_image_h.clip_rect.h = cfg->enable_scale ? dst->VFrame.mHeight : blit.src_image_h.clip_rect.h;
    blit.dst_image_h.gamut = G2D_BT709;
    blit.dst_image_h.mode = G2D_PIXEL_ALPHA;
    blit.dst_image_h.fd = -1;
    blit.dst_image_h.use_phy_addr = 1;

    if (ioctl(g2d_fd, G2D_CMD_BITBLT_H, (unsigned long)&blit) < 0) {
        printf("camera_g2d: ioctl G2D_CMD_BITBLT_H failed\n");
        return -1;
    }

    dst->VFrame.mOffsetLeft = 0;
    dst->VFrame.mOffsetTop = 0;
    dst->VFrame.mOffsetRight = dst->VFrame.mWidth;
    dst->VFrame.mOffsetBottom = dst->VFrame.mHeight;
    return 0;
}
