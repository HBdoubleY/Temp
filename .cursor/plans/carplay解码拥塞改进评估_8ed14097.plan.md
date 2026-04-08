---
name: CarPlay解码拥塞改进评估
overview: 最终方案收敛为小队列+保新丢包策略，并继续使用MPI接口完成可控调参与A/B验证，优先解决CarPlay实时性卡顿。
todos:
  - id: answer-env-impact
    content: 确认未设置CP_PERF环境变量时的性能影响结论
    status: pending
  - id: evaluate-queue-policy
    content: 评估H264队列改为16并满队列丢最旧10包的收益与风险
    status: pending
  - id: assess-mpi-vs-vdecoder
    content: 评估是否需要从MPI_VDEC切换到vdecoder低层接口
    status: pending
  - id: define-staged-improvements
    content: 制定低风险优先的分阶段改进与验证方案
    status: pending
isProject: false
---

# CarPlay最终改进方案（待确认后实施）

## 方案定稿（按你的确认）

- 关于环境变量：如果启动前**不设置** `CP_PERF_ENABLE/CP_PERF_DEEP/CP_PERF_SAMPLE_N/CP_PERF_WARN_US`，当前这套 `cp_perf` 深度日志基本不会输出，性能影响可忽略（仅剩极少量分支判断开销）。
- 队列策略定为：
  - `H264_QUEUE_CAP: 128 -> 16`
  - `queue_push` 满队列时先丢弃最旧 `10` 包，再写入新包（保新）
  - 丢弃优先级：优先丢非关键帧片段（P/B），尽量保留 SPS/PPS/IDR 所在包
- 解码接口策略定为：继续使用 `AW_MPI_VDEC_`*（不切到 `vdecoder.h` 直连）。

## 关键依据（对应当前代码）

- 当前解码路径已使用 `AW_MPI_VDEC_SendStream/GetImage/ReleaseImage`，属于 MPP 通道化管理路径，稳定性与系统其余模块（VO/CLOCK）一致性更好：
  - [/home/hyby/Desktop/myShare/Temp/Temp/runcarplay/src/carplay_display/carplay_display.c](/home/hyby/Desktop/myShare/Temp/Temp/runcarplay/src/carplay_display/carplay_display.c)
- `mpi_vdec.h` 中可继续挖掘的优化接口已具备（无需切换到 `vdecoder.h`）：
  - `AW_MPI_VDEC_SetVEFreq`
  - `AW_MPI_VDEC_SetVideoStreamInfo`
  - `AW_MPI_VDEC_ForceFramePackage`
  - 文件：[/home/hyby/Desktop/myShare/Temp/Temp/external/aw_pack_src/lib_aw/include/eyesee-mpp/middleware/include/media/mpi_vdec.h](/home/hyby/Desktop/myShare/Temp/Temp/external/aw_pack_src/lib_aw/include/eyesee-mpp/middleware/include/media/mpi_vdec.h)

## 实施方案（分阶段，可直接执行）

### 阶段1：队列与丢包策略改造（核心）

- 修改位置：[carplay_display.c](/home/hyby/Desktop/myShare/Temp/Temp/runcarplay/src/carplay_display/carplay_display.c)
- 具体动作：
  - 改 `#define H264_QUEUE_CAP 16`
  - 在 `queue_push` 满队列分支中执行“批量淘汰旧包”，目标一次释放 `10` 个槽位
  - 增加包类型判定（起码识别 SPS/PPS/IDR/非关键片），优先淘汰非关键片
  - 若当前窗口内非关键片不足，才回退到淘汰最旧关键相关包（保证系统可持续前进）
- 保护规则：
  - 永不打乱剩余包相对顺序
  - 每次淘汰后打印一次汇总日志（淘汰总数、关键包淘汰数、当前队列深度）

### 阶段2：MPI参数调优（不改架构）

- 继续使用接口：[/external/aw_pack_src/lib_aw/include/eyesee-mpp/middleware/include/media/mpi_vdec.h](/home/hyby/Desktop/myShare/Temp/Temp/external/aw_pack_src/lib_aw/include/eyesee-mpp/middleware/include/media/mpi_vdec.h)
- 参数化策略（按开关启用）：
  - `AW_MPI_VDEC_SetVideoStreamInfo`：明确设置 `VideoStreamInfo`（codec/width/height/frameRate/bIsFramePackage）
  - `AW_MPI_VDEC_ForceFramePackage(TRUE/FALSE)`：仅在确认输入是整帧包时开启
  - `AW_MPI_VDEC_SetVEFreq`：加环境变量开关（例如 `CP_VDEC_VE_FREQ`），按平台允许范围试档
- 推荐引入开关：
  - `CP_VDEC_SET_STREAM_INFO=1`
  - `CP_VDEC_FORCE_FRAME_PACKAGE=0|1`
  - `CP_VDEC_VE_FREQ=<MHz>`（不设置则保持默认）

### 阶段3：A/B验证与回滚条件

- A方案（基线）：现网逻辑
- B方案（新逻辑）：`cap=16 + 淘汰10旧包 + NAL优先淘汰 + MPI调参`
- 观察窗口：每组连续 10 分钟，至少包含“未录像/开启录像/录像切换”三段
- 立即回滚条件：
  - 黑屏或持续花屏 > 2 秒
  - 关键包连续被淘汰导致无法快速恢复

## 验证指标（必须记录）

- 固定测试场景：
  - CarPlay持续滑动+点击 60s
  - 分别在“未录像 / 开录像”两组执行
- 关注指标：
  - `h264_queue_drop` 总数、每分钟增长速率
  - `queue淘汰统计`（总淘汰、关键包淘汰、非关键包淘汰）
  - `h264_queue_pop wait_us` P95/P99
  - `display_frame fq_wait_us` P95/P99
  - `touch_poll_gap_warn` 次数
  - `video_data_cb feed_ret=-1` 次数
- 成功判据：
  - `touch_poll_gap_warn` 明显减少
  - `q_count` 不再长期贴近上限
  - 主观触控跟手性改善且无明显持续花屏

## 这版不做的事

- 直接切到 `vdecoder.h` 低层重构：改动大、回归面广、短期收益不确定。
- 在未做A/B前一次性叠加多项改动：难以定位真正收益来源。

