# Camera Modules Validation Checklist

本清单用于验证 `src/camera` 模块能力完整性，不要求接入当前项目主链路。

## 1. 编译级检查

- 头文件可被独立包含：
  - `camera_vi.h`
  - `camera_preview.h`
  - `camera_capture.h`
  - `camera_record.h`
  - `camera_overlay.h`
- `CAMERA_ENABLE_SECOND_SENSOR=0/1` 两种宏配置下均可通过编译。

## 2. 生命周期检查

- 每个模块都满足：
  - `init` 成功后可 `start`
  - `start` 后可 `stop`
  - `stop` 后可重复 `start`
  - `deinit` 可幂等调用（不崩溃）

## 3. 预览功能

- `camera_preview_start(..., CAMERA_PREVIEW_FRONT)` 正常显示前摄。
- `camera_preview_switch(..., CAMERA_PREVIEW_REAR)` 正常切后摄（双摄宏开启时）。
- `camera_preview_switch(..., CAMERA_PREVIEW_DUAL)` 正常双路显示（双摄宏开启时）。

## 4. 拍照功能

- `camera_capture_take_with_filename("xxx.jpg")` 正常输出文件。
- 连续拍照不崩溃，文件可被标准图片工具识别。

## 5. 录像功能

- 单路录像可输出 ts 文件。
- 双路模式（宏开启）可同时输出前后路文件。
- 分段时长到期后触发切段，生成多个连续文件。
- 停止录像后线程退出、句柄释放、再次启动成功。

## 6. 水印功能

- `camera_overlay_set_bitmap()` 能把外部 bitmap 叠加到编码通道。
- `camera_overlay_text_to_bitmap()` 输出 bitmap 可被 `set_bitmap` 直接使用。
- `camera_overlay_stop/deinit` 后 region 资源释放正常。

## 7. 不影响现有项目

- `runcarplay/src/app/video/mpp_camera.c/.h` 无改动。
- 现有业务调用链无改动。

