#ifndef CAMERA_VI_H
#define CAMERA_VI_H

/*
 * camera_vi 模块
 * --------------
 * 职责:
 * - 封装 VI/ISP 的生命周期（创建、启动、停止、销毁）。
 * - 提供统一的前后摄节点访问接口。
 * - 提供取帧/还帧能力，供预览、录像、拍照等上层模块复用。
 *
 * 线程模型:
 * - 本模块本身不创建业务线程，仅提供线程安全可重入的底层调用。
 *
 * 依赖关系:
 * - 作为唯一共享公共模块，可被 camera_preview/camera_capture/camera_record 依赖。
 */

#include <stdbool.h>
#include <string.h>

#include "mm_comm_vi.h"
#include "mpi_vi.h"
#include "mpi_isp.h"
#include "mm_comm_video.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CAMERA_ENABLE_SECOND_SENSOR
#define CAMERA_ENABLE_SECOND_SENSOR 1
#endif

typedef enum {
    CAMERA_SENSOR_FRONT = 0,
    CAMERA_SENSOR_REAR = 1,
    CAMERA_SENSOR_MAX
} camera_sensor_id_e;

typedef struct {
    int width;                     /* 采集宽度 */
    int height;                    /* 采集高度 */
    PIXEL_FORMAT_E pixel_format;   /* 采集像素格式 */
    int frame_rate;                /* 采集帧率 */
    int vi_buf_num;                /* VI 缓冲数量 */
    int enable_wdr;                /* 是否开启 WDR */
    int vi_drop_frame_num;         /* 丢帧参数 */
    enum v4l2_colorspace color_space; /* 色彩空间 */
    int mirror;                    /* 水平镜像 0/1 */
    int flip;                      /* 垂直翻转 0/1 */
} camera_vi_attr_t;

typedef struct {
    VI_DEV vi_dev;                 /* 物理 VI 设备号 */
    VI_CHN vi_chn;                 /* 虚拟 VI 通道号 */
    ISP_DEV isp_dev;               /* ISP 设备号 */
    VI_ATTR_S vi_attr;             /* 底层 VI 属性 */
    camera_vi_attr_t cfg;          /* 用户配置缓存 */
    bool created;                  /* 是否已创建 VI 资源 */
    bool started;                  /* 是否已启动采集 */
} camera_vi_node_t;

typedef struct {
    camera_vi_node_t front;        /* 前摄节点 */
#if CAMERA_ENABLE_SECOND_SENSOR
    camera_vi_node_t rear;         /* 后摄节点 */
#endif
    bool inited;                   /* 模块初始化标记 */
} camera_vi_ctx_t;

/*
 * 初始化 VI 资源。rear_cfg 仅在 CAMERA_ENABLE_SECOND_SENSOR=1 时有效。
 * @param ctx       [out] VI 上下文
 * @param front_cfg [in]  前摄配置
 * @param rear_cfg  [in]  后摄配置（可空）
 * @return 0 成功，<0 失败
 */
int camera_vi_init(camera_vi_ctx_t *ctx, const camera_vi_attr_t *front_cfg
#if CAMERA_ENABLE_SECOND_SENSOR
    , const camera_vi_attr_t *rear_cfg
#endif
);
/*
 * 启动已初始化的 VI/ISP。
 * @param ctx [in/out] VI 上下文
 * @return 0 成功，<0 失败
 */
int camera_vi_start(camera_vi_ctx_t *ctx);
/*
 * 停止 VI/ISP。可重复调用，要求幂等。
 * @param ctx [in/out] VI 上下文
 * @return 0 成功，<0 失败
 */
int camera_vi_stop(camera_vi_ctx_t *ctx);
/*
 * 销毁 VI 相关资源。
 * @param ctx [in/out] VI 上下文
 * @return 0 成功，<0 失败
 */
int camera_vi_deinit(camera_vi_ctx_t *ctx);
/*
 * 从指定传感器取一帧。timeout_ms<0 表示阻塞。
 * @param ctx        [in]  VI 上下文
 * @param sensor     [in]  前/后摄
 * @param frame      [out] 输出帧
 * @param timeout_ms [in]  超时毫秒
 * @return 0 成功，<0 失败
 */
int camera_vi_get_frame(camera_vi_ctx_t *ctx, camera_sensor_id_e sensor, VIDEO_FRAME_INFO_S *frame, int timeout_ms);
/*
 * 归还已取出的帧。
 * @param ctx    [in] VI 上下文
 * @param sensor [in] 前/后摄
 * @param frame  [in] 待归还帧
 * @return 0 成功，<0 失败
 */
int camera_vi_release_frame(camera_vi_ctx_t *ctx, camera_sensor_id_e sensor, VIDEO_FRAME_INFO_S *frame);
/*
 * 获取指定传感器节点。主要给需要底层通道号的模块使用。
 * @param ctx    [in] VI 上下文
 * @param sensor [in] 前/后摄
 * @return 非空=成功，NULL=失败
 */
camera_vi_node_t *camera_vi_get_node(camera_vi_ctx_t *ctx, camera_sensor_id_e sensor);

#ifdef __cplusplus
}
#endif

#endif
