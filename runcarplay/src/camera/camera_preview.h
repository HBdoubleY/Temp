#ifndef CAMERA_PREVIEW_H
#define CAMERA_PREVIEW_H

/*
 * camera_preview 模块
 * -------------------
 * 职责:
 * - 负责 VO 显示层/通道管理。
 * - 支持前摄预览、后摄预览、双路预览，以及运行时切换。
 *
 * 依赖:
 * - 仅依赖 camera_vi 提供输入通道，不依赖其他公共工具模块。
 */

#include "camera_vi.h"
#include "mpi_vo.h"
#include "mm_comm_vo.h"
#include "mpi_sys.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    /* 仅显示前摄 */
    CAMERA_PREVIEW_FRONT = 0,
    /* 仅显示后摄 */
    CAMERA_PREVIEW_REAR = 1,
    /* 同时显示前后摄 */
    CAMERA_PREVIEW_DUAL = 2,
} camera_preview_mode_e;

typedef struct {
    int x;       /* 显示起始 X 坐标 */
    int y;       /* 显示起始 Y 坐标 */
    int width;   /* 显示宽度 */
    int height;  /* 显示高度 */
    int zorder;  /* 图层优先级，数值越大层级越高 */
} camera_preview_rect_t;

typedef struct {
    VO_DEV vo_dev;                  /* VO 设备号 */
    VO_INTF_TYPE_E disp_type;       /* 显示输出类型（LCD/HDMI等） */
    VO_INTF_SYNC_E disp_sync;       /* 显示时序 */
    camera_preview_rect_t front_rect; /* 前摄显示区域 */
    camera_preview_rect_t rear_rect;  /* 后摄显示区域 */
} camera_preview_cfg_t;

typedef struct {
    camera_vi_ctx_t *vi_ctx;        /* VI 上下文 */
    camera_preview_cfg_t cfg;       /* 当前预览配置 */
    VO_LAYER front_layer;           /* 前摄 VO Layer */
    VO_LAYER rear_layer;            /* 后摄 VO Layer */
    VO_CHN front_chn;               /* 前摄 VO Channel */
    VO_CHN rear_chn;                /* 后摄 VO Channel */
    camera_preview_mode_e mode;     /* 当前模式 */
    int inited;                     /* 是否已 init */
    int started;                    /* 是否已 start */
} camera_preview_ctx_t;

/*
 * 初始化预览模块，创建 VO 层与通道。
 * @param ctx     [out] 预览上下文
 * @param vi_ctx  [in]  VI 上下文
 * @param cfg     [in]  预览配置
 * @return 0 成功，<0 失败
 */
int camera_preview_init(camera_preview_ctx_t *ctx, camera_vi_ctx_t *vi_ctx, const camera_preview_cfg_t *cfg);
/*
 * 启动预览并进入指定模式。
 * @param ctx   [in/out] 预览上下文
 * @param mode  [in]     目标模式
 * @return 0 成功，<0 失败
 */
int camera_preview_start(camera_preview_ctx_t *ctx, camera_preview_mode_e mode);
/*
 * 动态切换预览模式（front/rear/dual）。
 * @param ctx   [in/out] 预览上下文
 * @param mode  [in]     目标模式
 * @return 0 成功，<0 失败
 */
int camera_preview_switch(camera_preview_ctx_t *ctx, camera_preview_mode_e mode);
/*
 * 运行时设置显示区域并可选择立即生效。
 * @param ctx          [in/out] 预览上下文
 * @param sensor       [in]     目标摄像头
 * @param rect         [in]     显示区域参数
 * @param apply_now    [in]     1=立即更新到 VO，0=仅更新配置缓存
 * @return 0 成功，<0 失败
 */
int camera_preview_set_display_rect(camera_preview_ctx_t *ctx, camera_sensor_id_e sensor, const camera_preview_rect_t *rect, int apply_now);
/*
 * 停止预览显示。
 * @param ctx [in/out] 预览上下文
 * @return 0 成功，<0 失败
 */
int camera_preview_stop(camera_preview_ctx_t *ctx);
/*
 * 销毁预览资源。
 * @param ctx [in/out] 预览上下文
 * @return 0 成功，<0 失败
 */
int camera_preview_deinit(camera_preview_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif
