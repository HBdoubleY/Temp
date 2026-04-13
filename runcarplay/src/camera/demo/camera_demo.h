#ifndef CAMERA_DEMO_H
#define CAMERA_DEMO_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * camera_demo_run_single()
 * ------------------------
 * 单路示例（前摄）：
 * 1) 初始化 VI
 * 2) 启动前摄预览
 * 3) 启动单路录像（前摄）
 * 4) 对录像编码通道叠加水印（字符串转 bitmap）
 * 5) 拍照（自定义文件名）
 * 6) 停止并释放全部模块
 *
 * 返回:
 *   0  成功
 *  <0  失败
 */
int camera_demo_run_single(void);

/*
 * camera_demo_run_dual()
 * ----------------------
 * 双路示例（前后摄）：
 * 1) 初始化前后双 VI
 * 2) 启动双路预览
 * 3) 启动双路录像（前后各一路）
 * 4) 前后路分别叠加水印
 * 5) 前后路分别拍照（自定义文件名）
 * 6) 停止并释放全部模块
 *
 * 注意:
 * - 受 CAMERA_ENABLE_SECOND_SENSOR 宏控制，关闭双摄时会直接返回失败。
 */
int camera_demo_run_dual(void);

#ifdef __cplusplus
}
#endif

#endif
