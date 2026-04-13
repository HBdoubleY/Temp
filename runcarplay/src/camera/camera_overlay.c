#include "camera_overlay.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * camera_overlay.c
 * ----------------
 * 实现 VENC 叠加层管理与 bitmap 更新。
 * 文本转 bitmap 采用可移植简化实现，方便在无字体库环境下快速落地。
 */

/* 简单绘制块，用于构造轻量字符图。 */
static void camera_overlay_draw_block(uint8_t *argb, int stride, int x0, int y0, int w, int h, uint32_t color)
{
    int x, y;
    for (y = y0; y < y0 + h; ++y) {
        uint32_t *row = (uint32_t *)(argb + y * stride);
        for (x = x0; x < x0 + w; ++x) {
            row[x] = color;
        }
    }
}

int camera_overlay_text_to_bitmap(const char *text, int font_w, int font_h, uint32_t fg_argb, uint32_t bg_argb, camera_bitmap_t *out)
{
    int i;
    int len;
    int width;
    int height;
    uint8_t *buf;

    if (text == NULL || out == NULL || font_w <= 0 || font_h <= 0) {
        return -1;
    }

    len = (int)strlen(text);
    if (len <= 0) {
        return -1;
    }

    width = len * font_w;
    height = font_h;
    buf = (uint8_t *)malloc(width * height * 4);
    if (buf == NULL) {
        return -1;
    }

    memset(buf, 0, width * height * 4);
    for (i = 0; i < width * height; ++i) {
        ((uint32_t *)buf)[i] = bg_argb;
    }

    /*
     * 这里用简化字模：每个字符画一个内部实心块，保证通用可移植。
     * 若目标项目需要高质量字体，可替换为 FreeType 渲染实现。
     */
    for (i = 0; i < len; ++i) {
        int x = i * font_w;
        camera_overlay_draw_block(buf, width * 4, x + 1, 1, font_w - 2, font_h - 2, fg_argb);
    }

    out->data = buf;
    out->width = width;
    out->height = height;
    out->stride = width * 4;
    out->pixel_format = MM_PIXEL_FORMAT_RGB_8888;
    return 0;
}

void camera_overlay_free_bitmap(camera_bitmap_t *bitmap)
{
    if (bitmap == NULL) {
        return;
    }
    if (bitmap->data) {
        free(bitmap->data);
    }
    memset(bitmap, 0, sizeof(*bitmap));
}

int camera_overlay_init(camera_overlay_ctx_t *ctx, VENC_CHN venc_chn, RGN_HANDLE handle, const camera_overlay_cfg_t *cfg)
{
    RGN_ATTR_S attr;
    if (ctx == NULL || cfg == NULL) {
        return -1;
    }
    memset(ctx, 0, sizeof(*ctx));
    ctx->venc_chn = venc_chn;
    ctx->handle = handle;
    ctx->cfg = *cfg;
    if (ctx->cfg.width <= 0) {
        ctx->cfg.width = 16;
    }
    if (ctx->cfg.height <= 0) {
        ctx->cfg.height = 16;
    }

    memset(&attr, 0, sizeof(attr));
    attr.enType = OVERLAY_RGN;
    attr.unAttr.stOverlay.mPixelFmt = MM_PIXEL_FORMAT_RGB_8888;
    attr.unAttr.stOverlay.mSize.Width = ctx->cfg.width;
    attr.unAttr.stOverlay.mSize.Height = ctx->cfg.height;

    if (AW_MPI_RGN_Create(ctx->handle, &attr) != SUCCESS) {
        printf("camera_overlay: create region failed handle=%d\n", ctx->handle);
        return -1;
    }

    ctx->inited = 1;
    return 0;
}

int camera_overlay_start(camera_overlay_ctx_t *ctx)
{
    MPP_CHN_S chn;
    RGN_CHN_ATTR_S chn_attr;
    if (ctx == NULL || !ctx->inited) {
        return -1;
    }

    memset(&chn, 0, sizeof(chn));
    memset(&chn_attr, 0, sizeof(chn_attr));
    chn.mModId = MOD_ID_VENC;
    chn.mDevId = 0;
    chn.mChnId = ctx->venc_chn;

    chn_attr.bShow = TRUE;
    chn_attr.enType = OVERLAY_RGN;
    chn_attr.unChnAttr.stOverlayChn.stPoint.X = ctx->cfg.x;
    chn_attr.unChnAttr.stOverlayChn.stPoint.Y = ctx->cfg.y;
    chn_attr.unChnAttr.stOverlayChn.mLayer = ctx->cfg.layer;
    chn_attr.unChnAttr.stOverlayChn.mFgAlpha = ctx->cfg.fg_alpha;
    chn_attr.unChnAttr.stOverlayChn.mBgAlpha = ctx->cfg.bg_alpha;

    if (AW_MPI_RGN_AttachToChn(ctx->handle, &chn, &chn_attr) != SUCCESS) {
        printf("camera_overlay: attach failed handle=%d venc=%d\n", ctx->handle, ctx->venc_chn);
        return -1;
    }

    ctx->started = 1;
    return 0;
}

int camera_overlay_set_bitmap(camera_overlay_ctx_t *ctx, const camera_bitmap_t *bitmap)
{
    BITMAP_S bmp;
    if (ctx == NULL || !ctx->started || bitmap == NULL || bitmap->data == NULL) {
        return -1;
    }
    if (bitmap->width > ctx->cfg.width || bitmap->height > ctx->cfg.height) {
        printf("camera_overlay: bitmap(%dx%d) larger than region(%dx%d)\n",
               bitmap->width, bitmap->height, ctx->cfg.width, ctx->cfg.height);
        return -1;
    }

    memset(&bmp, 0, sizeof(bmp));
    bmp.mPixelFormat = bitmap->pixel_format;
    bmp.mWidth = bitmap->width;
    bmp.mHeight = bitmap->height;
    bmp.mpData = bitmap->data;

    if (AW_MPI_RGN_SetBitMap(ctx->handle, &bmp) != SUCCESS) {
        printf("camera_overlay: set bitmap failed handle=%d\n", ctx->handle);
        return -1;
    }
    return 0;
}

int camera_overlay_stop(camera_overlay_ctx_t *ctx)
{
    MPP_CHN_S chn;
    if (ctx == NULL || !ctx->inited) {
        return -1;
    }
    if (!ctx->started) {
        return 0;
    }

    memset(&chn, 0, sizeof(chn));
    chn.mModId = MOD_ID_VENC;
    chn.mDevId = 0;
    chn.mChnId = ctx->venc_chn;
    AW_MPI_RGN_DetachFromChn(ctx->handle, &chn);
    ctx->started = 0;
    return 0;
}

int camera_overlay_deinit(camera_overlay_ctx_t *ctx)
{
    if (ctx == NULL || !ctx->inited) {
        return -1;
    }
    camera_overlay_stop(ctx);
    AW_MPI_RGN_Destroy(ctx->handle);
    ctx->inited = 0;
    return 0;
}

int camera_overlay_set_position(camera_overlay_ctx_t *ctx, int x, int y)
{
    MPP_CHN_S chn;
    RGN_CHN_ATTR_S chn_attr;
    if (ctx == NULL || !ctx->started) {
        return -1;
    }

    memset(&chn, 0, sizeof(chn));
    chn.mModId = MOD_ID_VENC;
    chn.mDevId = 0;
    chn.mChnId = ctx->venc_chn;

    if (AW_MPI_RGN_GetDisplayAttr(ctx->handle, &chn, &chn_attr) != SUCCESS) {
        return -1;
    }
    chn_attr.unChnAttr.stOverlayChn.stPoint.X = x;
    chn_attr.unChnAttr.stOverlayChn.stPoint.Y = y;
    if (AW_MPI_RGN_SetDisplayAttr(ctx->handle, &chn, &chn_attr) != SUCCESS) {
        return -1;
    }
    ctx->cfg.x = x;
    ctx->cfg.y = y;
    return 0;
}

int camera_overlay_set_size(camera_overlay_ctx_t *ctx, int width, int height)
{
    int restart = 0;
    MPP_CHN_S chn;
    RGN_CHN_ATTR_S chn_attr;
    RGN_ATTR_S attr;

    if (ctx == NULL || !ctx->inited || width <= 0 || height <= 0) {
        return -1;
    }
    restart = ctx->started;
    if (restart) {
        camera_overlay_stop(ctx);
    }

    AW_MPI_RGN_Destroy(ctx->handle);
    memset(&attr, 0, sizeof(attr));
    attr.enType = OVERLAY_RGN;
    attr.unAttr.stOverlay.mPixelFmt = MM_PIXEL_FORMAT_RGB_8888;
    attr.unAttr.stOverlay.mSize.Width = width;
    attr.unAttr.stOverlay.mSize.Height = height;
    if (AW_MPI_RGN_Create(ctx->handle, &attr) != SUCCESS) {
        return -1;
    }

    ctx->cfg.width = width;
    ctx->cfg.height = height;

    if (restart) {
        memset(&chn, 0, sizeof(chn));
        memset(&chn_attr, 0, sizeof(chn_attr));
        chn.mModId = MOD_ID_VENC;
        chn.mDevId = 0;
        chn.mChnId = ctx->venc_chn;
        chn_attr.bShow = TRUE;
        chn_attr.enType = OVERLAY_RGN;
        chn_attr.unChnAttr.stOverlayChn.stPoint.X = ctx->cfg.x;
        chn_attr.unChnAttr.stOverlayChn.stPoint.Y = ctx->cfg.y;
        chn_attr.unChnAttr.stOverlayChn.mLayer = ctx->cfg.layer;
        chn_attr.unChnAttr.stOverlayChn.mFgAlpha = ctx->cfg.fg_alpha;
        chn_attr.unChnAttr.stOverlayChn.mBgAlpha = ctx->cfg.bg_alpha;
        if (AW_MPI_RGN_AttachToChn(ctx->handle, &chn, &chn_attr) != SUCCESS) {
            return -1;
        }
        ctx->started = 1;
    }
    return 0;
}
