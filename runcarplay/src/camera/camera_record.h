#ifndef CAMERA_RECORD_H
#define CAMERA_RECORD_H

/*
 * camera_record 模块
 * ------------------
 * 职责:
 * - 封装录像流程（VI -> VENC -> MUX）。
 * - 支持单路/双路录像与分段切换。
 * - 内置帧队列和生产/消费线程，不依赖外部 queue_mpp。
 *
 * 线程模型:
 * - 每路录像包含 producer(取帧) + consumer(送编码/切段) 两个线程。
 *
 * 非目标:
 * - 不包含存储巡检、空间回收、删除旧文件逻辑。
 */

#include <pthread.h>
#include <stdint.h>

#include "camera_vi.h"
#include "mpi_venc.h"
#include "mpi_mux.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CAMERA_RECORD_IDLE = 0,
    CAMERA_RECORD_INITED,
    CAMERA_RECORD_RUNNING,
    CAMERA_RECORD_STOPPING,
} camera_record_state_e;

typedef struct {
    int width;              /* 编码宽度 */
    int height;             /* 编码高度 */
    int frame_rate;         /* 编码帧率 */
    int bit_rate;           /* 码率（bps） */
    PAYLOAD_TYPE_E payload_type; /* 编码类型 PT_H264/PT_H265... */
    int segment_duration_s; /* 分段时长（秒），>0 生效 */
} camera_record_stream_cfg_t;

typedef struct {
    camera_record_stream_cfg_t front; /* 前摄录像参数 */
#if CAMERA_ENABLE_SECOND_SENSOR
    camera_record_stream_cfg_t rear;  /* 后摄录像参数 */
#endif
    int enable_dual;                /* 1=双路录像，0=仅前路 */
    int enable_segment;             /* 1=开启分段，0=关闭分段切换 */
    const char *output_dir_front;   /* 前路输出目录 */
    const char *output_dir_rear;    /* 后路输出目录 */
} camera_record_cfg_t;

typedef struct camera_frame_node_s {
    VIDEO_FRAME_INFO_S frame;       /* 缓存帧 */
    struct camera_frame_node_s *next; /* 下一个节点 */
} camera_frame_node_t;

typedef struct {
    camera_frame_node_t *head;      /* 队头 */
    camera_frame_node_t *tail;      /* 队尾 */
    int size;                       /* 队列长度 */
    int stop_flag;                  /* 停止标记 */
    pthread_mutex_t mutex;          /* 队列锁 */
    pthread_cond_t cond;            /* 队列条件变量 */
} camera_frame_queue_t;

typedef struct {
    camera_vi_ctx_t *vi_ctx;        /* VI 上下文 */
    camera_sensor_id_e sensor;      /* 前/后摄标识 */
    const char *output_dir;         /* 输出目录 */
    camera_record_stream_cfg_t cfg; /* 本路参数 */
    VENC_CHN venc_chn;              /* 编码通道号 */
    MUX_CHN mux_chn;                /* 封装通道号 */
    int mux_fd;                     /* 当前分段文件 fd */
    int segment_index;              /* 分段索引 */
    uint64_t segment_start_ms;      /* 当前分段起始时间 */
    camera_frame_queue_t frame_queue; /* 内置帧队列 */
    pthread_t producer_tid;         /* 采集线程 */
    pthread_t consumer_tid;         /* 编码线程 */
    int producer_running;           /* 采集线程运行标记 */
    int consumer_running;           /* 编码线程运行标记 */
} camera_record_channel_t;

typedef struct {
    camera_record_cfg_t cfg;        /* 总体配置 */
    camera_record_state_e state;    /* 运行状态机 */
    int inited;                     /* 是否已初始化 */
    camera_record_channel_t front;  /* 前路上下文 */
#if CAMERA_ENABLE_SECOND_SENSOR
    camera_record_channel_t rear;   /* 后路上下文 */
#endif
} camera_record_ctx_t;

/*
 * 初始化录像模块（仅配置，不启动线程）。
 * @param ctx    [out] 录像上下文
 * @param vi_ctx [in]  VI 上下文
 * @param cfg    [in]  录像配置
 * @return 0 成功，<0 失败
 */
int camera_record_init(camera_record_ctx_t *ctx, camera_vi_ctx_t *vi_ctx, const camera_record_cfg_t *cfg);
/*
 * 启动录像（创建编码与封装通道，启动内部线程）。
 * @param ctx [in/out] 录像上下文
 * @return 0 成功，<0 失败
 */
int camera_record_start(camera_record_ctx_t *ctx);
/*
 * 停止录像并回收线程和通道资源。
 * @param ctx [in/out] 录像上下文
 * @return 0 成功，<0 失败
 */
int camera_record_stop(camera_record_ctx_t *ctx);
/*
 * 反初始化录像模块。
 * @param ctx [in/out] 录像上下文
 * @return 0 成功，<0 失败
 */
int camera_record_deinit(camera_record_ctx_t *ctx);
/*
 * 动态设置前后路统一分段时长（秒）。
 * @param ctx                [in/out] 录像上下文
 * @param segment_duration_s [in]     分段时长，>0
 * @return 0 成功，<0 失败
 */
int camera_record_set_segment(camera_record_ctx_t *ctx, int segment_duration_s);
/*
 * 设置分段参数。
 * @param ctx                    [in/out] 录像上下文
 * @param enable_segment         [in]     1=开启分段 0=关闭
 * @param front_segment_seconds  [in]     前路分段时长（秒，<=0 表示不修改）
 * @param rear_segment_seconds   [in]     后路分段时长（秒，<=0 表示不修改）
 * @return 0 成功，<0 失败
 */
int camera_record_set_segment_params(camera_record_ctx_t *ctx, int enable_segment, int front_segment_seconds, int rear_segment_seconds);

#ifdef __cplusplus
}
#endif

#endif
