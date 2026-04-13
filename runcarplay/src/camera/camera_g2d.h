#ifndef CAMERA_G2D_H
#define CAMERA_G2D_H

/*
 * camera_g2d 模块
 * ---------------
 * 职责:
 * - 封装 G2D 的旋转 + 缩放能力。
 * - 通过一个接口完成变换控制，避免业务层直接拼装 g2d_blt_h。
 *
 * 说明:
 * - 输入输出均使用 VIDEO_FRAME_INFO_S（物理地址帧）。
 * - 旋转角度支持 0/90/180/270（顺时针）。
 */

#include "mm_comm_video.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int enable_scale;    /* 1=按 dst 尺寸缩放，0=按 src 可视区域直拷 */
    int rotate_degree;   /* 0/90/180/270 */
} camera_g2d_transform_cfg_t;

/*
 * 执行 G2D 旋转+缩放。
 * @param g2d_fd [in]     g2d 设备 fd
 * @param src    [in]     源帧
 * @param dst    [in/out] 目标帧（需由调用方预分配物理内存）
 * @param cfg    [in]     变换配置
 * @return 0 成功，<0 失败
 */
int camera_g2d_transform_frame(int g2d_fd, const VIDEO_FRAME_INFO_S *src, VIDEO_FRAME_INFO_S *dst, const camera_g2d_transform_cfg_t *cfg);

#ifdef __cplusplus
}
#endif

#endif
