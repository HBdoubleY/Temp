#ifndef CAMERA_CAPTURE_H
#define CAMERA_CAPTURE_H

/*
 * camera_capture 模块
 * -------------------
 * 职责:
 * - 通过 VI->VENC JPEG 链路完成抓拍。
 * - 对外暴露“自定义文件名”拍照接口，便于业务自定义命名策略。
 *
 * 调用顺序:
 *   init -> start -> take_with_filename(可多次) -> stop -> deinit
 */

#include "camera_vi.h"
#include "mpi_venc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int width;        /* 拍照输出宽度 */
    int height;       /* 拍照输出高度 */
    int jpeg_qfactor; /* JPEG 质量因子 */
    int frame_rate;   /* 编码帧率 */
} camera_capture_cfg_t;

typedef struct {
    camera_vi_ctx_t *vi_ctx;     /* VI 上下文 */
    camera_sensor_id_e sensor;   /* 目标摄像头 */
    camera_capture_cfg_t cfg;    /* 拍照参数 */
    VENC_CHN venc_chn;           /* JPEG 编码通道 */
    int inited;                  /* 初始化标记 */
    int started;                 /* 启动标记 */
} camera_capture_ctx_t;

/*
 * 初始化拍照链路（绑定 VI 与 JPEG VENC）。
 * @param ctx    [out] 拍照上下文
 * @param vi_ctx [in]  VI 上下文
 * @param sensor [in]  目标摄像头
 * @param cfg    [in]  拍照配置
 * @return 0 成功，<0 失败
 */
int camera_capture_init(camera_capture_ctx_t *ctx, camera_vi_ctx_t *vi_ctx, camera_sensor_id_e sensor, const camera_capture_cfg_t *cfg);
/*
 * 启动拍照接收。
 * @param ctx [in/out] 拍照上下文
 * @return 0 成功，<0 失败
 */
int camera_capture_start(camera_capture_ctx_t *ctx);
/*
 * 拍照并保存到指定文件。file_name 由调用方提供。
 * @param ctx       [in/out] 拍照上下文
 * @param file_name [in]     输出文件路径
 * @return 0 成功，<0 失败
 */
int camera_capture_take_with_filename(camera_capture_ctx_t *ctx, const char *file_name);
/*
 * 停止拍照接收。
 * @param ctx [in/out] 拍照上下文
 * @return 0 成功，<0 失败
 */
int camera_capture_stop(camera_capture_ctx_t *ctx);
/*
 * 释放拍照资源。
 * @param ctx [in/out] 拍照上下文
 * @return 0 成功，<0 失败
 */
int camera_capture_deinit(camera_capture_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif
