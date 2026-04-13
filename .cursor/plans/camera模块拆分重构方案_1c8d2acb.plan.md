---
name: camera模块拆分重构方案
overview: 该方案仅用于将 `mpp_camera` 能力拆分为可移植模块，供后续移植到其他项目使用；当前项目代码（含 `mpp_camera` 与调用链）不做任何改动。共享部分仅保留 VI 采集模块，其余功能模块均自包含（含日志、状态机、线程控制）。
todos:
  - id: create-camera-vi-core
    content: 创建 `runcarplay/src/camera` 目录与仅共享的 `camera_vi.h/.c` 采集模块
    status: completed
  - id: split-preview-module
    content: 抽离预览与前后/双路切换到 `camera_preview.h/.c`（仅新模块实现，不接入现有工程调用链）
    status: completed
  - id: split-capture-module
    content: 抽离拍照流程到 `camera_capture.h/.c`（仅新模块实现，不接入现有工程调用链）
    status: completed
  - id: split-overlay-module
    content: 抽离水印叠加到 `camera_overlay.h/.c`（bitmap 输入与字符串转 bitmap，且不改现有调用）
    status: completed
  - id: split-record-module
    content: 抽离双路录像与分段录像到 `camera_record.h/.c`（内置帧队列，不依赖 queue_mpp，不改现有调用）
    status: completed
  - id: keep-current-project-untouched
    content: 保持当前项目代码完全不变，仅输出新模块与迁移指引文档
    status: completed
  - id: docs-and-validation
    content: 补充注释与集成文档，完成功能等价验证用例
    status: completed
isProject: false
---

# camera 模块拆分方案（按补充要求修订）

## 目标与边界（强约束）

- 当前项目代码完全不改：不修改 `mpp_camera`、不修改现有调用方、不做桥接接入。
- 新增目录 `runcarplay/src/camera`，将功能拆成独立 `.h/.c` 工具模块，统一提供“初始化 / 启动(执行) / 停止 / 退出”。
- 共享部分仅保留 VI 采集模块；除 VI 外，每个功能模块均可独立集成，不依赖外部通用模块。
- 首阶段保持贴近现有 MPP 实现（不先做跨平台 HAL）。

## 现状拆分依据

- 主入口与流程目前集中在 `runcarplay/src/app/video/mpp_camera.c`：`recording()`、`stopRecording()`、内部 `createViChn/createVoChn/createVencChn/createMuxChn/prepare/startVideoRecording`。
- 录像链路当前依赖 `runcarplay/src/app/video/queue_mpp.c` 解决取帧与编码解耦；新录像模块将内置私有帧队列实现，不再依赖该外部文件。
- 水印能力当前集中在 `AW_MPI_RGN_*` 与时间戳线程逻辑；重构后改为“输入 bitmap 叠加”，并提供字符串转 bitmap 的工具接口。

## 目录与文件规划

- 新目录：`runcarplay/src/camera`
- 共享 VI 模块（唯一公共模块）：
  - `camera_vi.h/.c`
  - 能力：VI 设备/通道初始化、启动、停止、反初始化；提供前摄/后摄通道抽象；通过宏控制是否启用第二摄像头。
- 预览模块：
  - `camera_preview.h/.c`
  - 能力：前预览、后预览、双路预览、动态切换；依赖 `camera_vi` 提供输入，不依赖其他公共文件。
- 拍照模块：
  - `camera_capture.h/.c`
  - 能力：拍照通道创建、抓拍、按入参文件名存储、资源释放。
- 录像模块：
  - `camera_record.h/.c`
  - 能力：双路录像、分段策略、mux/file 切换；内部自带帧队列、线程状态机、`printf` 日志。
- 水印模块：
  - `camera_overlay.h/.c`
  - 能力：外部传入 bitmap 叠加；提供字符串转 bitmap 接口；生命周期管理（create/attach/update/detach/destroy）。
- 不新增兼容适配层到当前项目；仅保留“接口映射文档”（旧能力 -> 新模块 API）用于后续移植项目接入。

## 模块 API 规范（统一形态）

- 每个模块至少包含：
  - `*_init(ctx, cfg)`：初始化资源与参数。
  - `*_start(ctx, mode)`：启动主功能。
  - `*_stop(ctx)`：停止功能（幂等）。
  - `*_deinit(ctx)`：释放资源（幂等）。
- 预览模块额外：`camera_preview_switch(mode)`，`mode` 包含 `FRONT/REAR/DUAL`。
- 第二摄像头控制：统一编译宏（例如 `CAMERA_ENABLE_SECOND_SENSOR`）控制 rear/dual 代码路径启用与裁剪。
- 拍照模块额外：`camera_capture_take_with_filename(const char *file_name, ...)`，由调用方传入目标文件名。
- 水印模块额外：
  - `camera_overlay_set_bitmap(...)`：直接传入 bitmap 叠加。
  - `camera_overlay_text_to_bitmap(const char *text, ..., CameraBitmap *out)`：字符串转 bitmap。
- 日志规范：统一使用 `printf`，不依赖外部日志库。
- 线程与状态控制：各模块内部实现私有状态机与线程控制，不依赖外部通用状态机文件。

## 状态与上下文重构

- 从 `mpp_camera_para_conf` 提炼“VI 公共上下文 + 子模块私有上下文”：
  - 公共上下文仅保留 VI 采集相关句柄与配置。
  - 预览/拍照/录像/水印状态均在各自 `.c` 内部维护，头文件仅暴露必要 API 与轻量配置。
- 明确生命周期顺序：
  - 预览：`init -> start/switch -> stop -> deinit`
  - 录像：`init -> start -> segment switch(loop) -> stop -> deinit`
  - 水印：`init -> start(update thread) -> stop -> deinit`

## 与现有 mpp_camera 的关系（修订）

- 当前项目中 `mpp_camera` 保持原状，不进行任何源码改造。
- 新模块仅作为“独立能力库”产出，服务于后续其他项目移植。
- 本次仅做能力对齐与 API 对照，不做线上替换。
- 明确排除：不迁移 `mpp_camera` 内的存储检查与删除文件策略到新模块。

## 分阶段落地

- 阶段1（骨架阶段）
  - 建立 `src/camera` 目录，创建 `camera_vi` 与四个功能模块头文件及最小实现（空实现+参数校验+printf 日志）。
  - 不修改任何现有业务调用链。
- 阶段2（预览先行）
  - 迁移前/后/双路预览创建与切换流程到 `camera_preview`，并接入第二摄像头宏开关。
  - 仅保证模块内能力完整，不要求当前 UI 路径接入。
- 阶段3（拍照与水印）
  - 迁移抓拍与图片编码流程到 `camera_capture`，新增“自定义文件名”拍照接口。
  - 迁移 RGN 水印到 `camera_overlay`，实现 bitmap 传入与字符串转 bitmap。
- 阶段4（录像）
  - 迁移双路录像、分段、mux 切换到 `camera_record`，内置帧队列替代 `queue_mpp` 外部依赖。
  - 对分段切换与异常回滚做边界测试。
- 阶段5（收口）
  - 不触碰 `mpp_camera` 代码；仅补齐注释、接口文档和“移植接入说明”。

## 代码规范与可移植性约束

- 命名统一：`camera_<feature>_*`。
- 每个模块文件顶部写明：职责、依赖、线程模型、可移植边界。
- 头文件不暴露不必要的 MPP 细节；尽量将 MPP 结构体留在 `.c` 内。
- 模块日志全部使用 `printf`；停止/退出接口必须幂等。

## 验收标准

- 功能等价：前预览/后预览/双预览切换、拍照（支持自定义文件名）、双路录像、分段录像、水印 bitmap 叠加均可用。
- 当前项目不受影响：现有调用方与现有行为保持原样。
- 模块独立性：`camera_vi` + 任一功能模块可独立编译集成；录像模块无需依赖外部 `queue_mpp`。
- 可移植性：新模块可在其他项目独立接入，按文档完成能力映射。

