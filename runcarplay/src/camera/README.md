# Camera Portable Modules

该目录是面向“后续移植项目”的独立相机能力模块集合，不替换当前项目的 `mpp_camera` 调用链。

## 设计原则

- 仅共享 `camera_vi.h/.c`，其余功能模块均自包含。
- 每个模块统一生命周期：`init -> start -> stop -> deinit`。
- 模块日志统一 `printf`。
- 线程控制和状态机在模块内部实现，不依赖外部工具文件。
- 支持 `CAMERA_ENABLE_SECOND_SENSOR` 宏控制后摄/双路相关代码。

## 模块清单

- `camera_vi.h/.c`
  - 负责 VI/ISP 初始化、启动、停止、反初始化。
  - 对外提供前摄/后摄节点访问和帧读写接口。
- `camera_preview.h/.c`
  - 负责前预览、后预览、双路预览和动态切换。
  - 依赖 `camera_vi`，不依赖其他公共模块。
  - 仅负责 VO 显示，不在预览链路做 G2D 旋转。
- `camera_capture.h/.c`
  - 负责抓拍 JPEG。
  - 核心接口 `camera_capture_take_with_filename()` 支持调用方传入文件名。
- `camera_record.h/.c`
  - 负责单路/双路录像和分段录像。
  - 内置帧队列与采集/编码线程，不依赖 `queue_mpp`。
  - 不包含存储巡检、空间回收、删除旧文件逻辑。
- `camera_overlay.h/.c`
  - 负责水印叠加（输入 bitmap）。
  - 提供 `camera_overlay_text_to_bitmap()` 作为字符串转 bitmap 工具。
- `camera_g2d.h/.c`
  - 独立封装 G2D 旋转 + 缩放能力。
  - 提供单接口控制 `rotate + scale`。

- `demo/camera_demo.h/.c`
  - 提供可直接参考的单路/双路示例：
    - `camera_demo_run_single()`
    - `camera_demo_run_dual()`
- `demo/camera_demo_main.c`
  - 最小可执行入口，命令行参数选择 `single|dual`。

## 与现有 mpp_camera 的关系

- 本目录代码不改动 `runcarplay/src/app/video/mpp_camera.c/.h`。
- 当前项目仍按旧逻辑运行。
- 后续移植项目可直接按本目录 API 集成。

## 迁移映射（旧能力 -> 新 API）

- VI 初始化/启停：
  - `initVi/deinitVi` -> `camera_vi_init/start/stop/deinit`
- 预览切换：
  - `startPreview/stopPreview` + 模式控制 -> `camera_preview_start/switch/stop`
- 拍照：
  - `startTakePic/savePic` -> `camera_capture_start + camera_capture_take_with_filename`
- 录像：
  - `recording/stopRecording/changeVideoRecordingMode` -> `camera_record_init/start/stop/set_segment`
- 水印：
  - `dashTimeMark`（固定时间戳）-> `camera_overlay_set_bitmap` / `camera_overlay_text_to_bitmap`

## Demo 用法

### 1) 单路：预览 + 录像 + 拍照 + 水印

可直接调用 `demo/camera_demo.h` 中接口：

- `camera_demo_run_single()`

内部流程：

1. `camera_vi_init/start`
2. `camera_preview_init/start(CAMERA_PREVIEW_FRONT)`
3. `camera_record_init/start`（单路）
4. `camera_overlay_text_to_bitmap + camera_overlay_set_bitmap`
5. `camera_capture_take_with_filename`
6. `camera_record_stop/deinit -> camera_preview_stop/deinit -> camera_vi_stop/deinit`

### 2) 双路：前后预览 + 双路录像 + 双路拍照 + 双路水印

可直接调用 `demo/camera_demo.h` 中接口：

- `camera_demo_run_dual()`

注意：

- 需要 `CAMERA_ENABLE_SECOND_SENSOR=1`。
- 双路示例会对 front/rear 分别设置录像和水印。
- 直接运行入口参考 `demo/camera_demo_main.c`。


