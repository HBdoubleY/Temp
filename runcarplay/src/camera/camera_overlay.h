#ifndef CAMERA_OVERLAY_H
#define CAMERA_OVERLAY_H

/*
 * camera_overlay 模块
 * -------------------
 * 职责:
 * - 负责 VENC 通道上的 RGN 叠加生命周期管理。
 * - 支持直接传入 bitmap 叠加。
 * - 提供字符串转 bitmap 的基础工具接口。
 *
 * 说明:
 * - 文本转 bitmap 接口默认使用轻量实现，便于跨项目快速移植。
 */

#include <stdint.h>

#include "mpi_region.h"
#include "mm_comm_region.h"
#include "mpi_venc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t *data;             /* 像素数据指针 */
    int width;                 /* 位图宽度 */
    int height;                /* 位图高度 */
    int stride;                /* 行跨度（字节） */
    PIXEL_FORMAT_E pixel_format; /* 像素格式 */
} camera_bitmap_t;

typedef struct {
    int x;         /* 叠加左上角 X */
    int y;         /* 叠加左上角 Y */
    int width;     /* region 宽度 */
    int height;    /* region 高度 */
    int layer;     /* region 层级 */
    int fg_alpha;  /* 前景透明度 */
    int bg_alpha;  /* 背景透明度 */
} camera_overlay_cfg_t;

typedef struct {
    VENC_CHN venc_chn;          /* 目标编码通道 */
    RGN_HANDLE handle;          /* RGN handle */
    camera_overlay_cfg_t cfg;   /* 叠加配置 */
    int inited;                 /* init 标记 */
    int started;                /* start 标记 */
} camera_overlay_ctx_t;

/*
 * 初始化 overlay region。
 * @param ctx      [out] 上下文
 * @param venc_chn [in]  目标编码通道
 * @param handle   [in]  region handle
 * @param cfg      [in]  叠加配置（位置+尺寸+alpha）
 * @return 0 成功，<0 失败
 */
int camera_overlay_init(camera_overlay_ctx_t *ctx, VENC_CHN venc_chn, RGN_HANDLE handle, const camera_overlay_cfg_t *cfg);
/*
 * 将 region 挂载到 VENC 通道。
 * @param ctx [in/out] 上下文
 * @return 0 成功，<0 失败
 */
int camera_overlay_start(camera_overlay_ctx_t *ctx);
/*
 * 更新叠加 bitmap。
 * @param ctx    [in/out] 上下文
 * @param bitmap [in]     位图数据
 * @return 0 成功，<0 失败
 */
int camera_overlay_set_bitmap(camera_overlay_ctx_t *ctx, const camera_bitmap_t *bitmap);
/*
 * 字符串转换为 bitmap。调用方使用后需调用 camera_overlay_free_bitmap() 释放。
 * @param text     [in]  文本
 * @param font_w   [in]  字符宽
 * @param font_h   [in]  字符高
 * @param fg_argb  [in]  前景色
 * @param bg_argb  [in]  背景色
 * @param out      [out] 输出 bitmap
 * @return 0 成功，<0 失败
 */
int camera_overlay_text_to_bitmap(const char *text, int font_w, int font_h, uint32_t fg_argb, uint32_t bg_argb, camera_bitmap_t *out);
/*
 * 从 VENC 通道卸载 region。
 * @param ctx [in/out] 上下文
 * @return 0 成功，<0 失败
 */
int camera_overlay_stop(camera_overlay_ctx_t *ctx);
/*
 * 销毁 region 资源。
 * @param ctx [in/out] 上下文
 * @return 0 成功，<0 失败
 */
int camera_overlay_deinit(camera_overlay_ctx_t *ctx);
/*
 * 动态设置叠加位置（不改变尺寸）。
 * @param ctx [in/out] 上下文
 * @param x   [in]     新位置 X
 * @param y   [in]     新位置 Y
 * @return 0 成功，<0 失败
 */
int camera_overlay_set_position(camera_overlay_ctx_t *ctx, int x, int y);
/*
 * 动态设置叠加尺寸。
 * 注意: 为保证一致性，内部会重建 region。
 * @param ctx    [in/out] 上下文
 * @param width  [in]     宽度
 * @param height [in]     高度
 * @return 0 成功，<0 失败
 */
int camera_overlay_set_size(camera_overlay_ctx_t *ctx, int width, int height);
/*
 * 释放 text_to_bitmap 动态申请的像素内存。
 * @param bitmap [in/out] 位图对象
 */
void camera_overlay_free_bitmap(camera_bitmap_t *bitmap);

#ifdef __cplusplus
}
#endif

#endif
