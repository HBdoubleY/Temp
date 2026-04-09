---
name: CarPlay解码显示深度优化
overview: 针对 CarPlay 投屏在导航界面快速滑动时的卡顿和花屏问题，围绕"解码线程取帧逻辑"、"显示线程丢帧策略"、"丢帧后解码器状态恢复"、"VE 频率"以及"日志精简与定位能力"五个维度做一轮集中优化，只修改 carplay_display.c。
todos:
  - id: getimage-fix
    content: 修改 decode_thread_fn 中 GetImage 逻辑：删除第二段 30ms 阻塞，改为 frame_cnt==0 时兜底短阻塞 10ms
    status: completed
  - id: display-smooth
    content: FRAME_QUEUE_CAP 改为 8，VO DispBufNum 改为 3，显示线程丢帧阈值改为 count > 2
    status: completed
  - id: evict-got-idr
    content: 批量淘汰关键包后重置 g_ctx.got_idr=0，去掉无条件 printf 改为仅关键包淘汰时告警
    status: completed
  - id: ve-freq-default
    content: VE 频率从不设置改为默认 480 MHz
    status: completed
  - id: perf-summary-log
    content: 新增每 3 秒一行的周期汇总日志，涵盖 dec_in/dec_out/disp/fq_drop/h264_evict/h264_drop/send_fail/g2d_avg/vo_avg，同时精简现有逐包 printf
    status: completed
isProject: false
---

# CarPlay 解码显示深度优化方案

只修改 [runcarplay/src/carplay_display/carplay_display.c](runcarplay/src/carplay_display/carplay_display.c)，不涉及其他文件。

---

## 问题 1：H264 队列淘汰关键帧后花屏，是否需要 ResetChn？

### 结论

不建议用 `AW_MPI_VDEC_ResetChn()`。

ResetChn 是一个重量级操作：它清空解码器全部内部缓存、重置状态机。在流模式下执行 ResetChn 后，你必须重新等到 SPS+PPS+IDR 才能恢复解码，期间画面会完全黑掉或冻结数百毫秒甚至更久。这比短时花屏体验更差。

### 替代方案：在批量淘汰中保护 SPS/PPS 并智能重置 got_idr

当批量淘汰触发且淘汰了关键相关包（SPS/PPS/IDR）时，将 `g_ctx.got_idr` 重置为 `0`。这样解码线程会自动跳过后续非关键帧，直到下一组 SPS 到达再恢复。这比 ResetChn 轻量得多，恢复也更快。

### 关于你提到的 NAL 类型判断

你说的 `video_data[4] & 0x0F == 0x07` 并不完全正确。H264 Annex-B 格式中 NAL type 的判定方法是：

- 先找到起始码 `00 00 01` 或 `00 00 00 01`
- 起始码之后第一个字节的低 5 位（`& 0x1F`，不是 `& 0x0F`）才是 NAL type
- type 7 = SPS, type 8 = PPS, type 5 = IDR（I 帧）, type 1 = non-IDR slice（P/B 帧）

现有代码里 `h264_classify_packet()` 已经正确实现了这个逻辑（第 191-230 行），可以完全复用。

---

## 问题 2：是否需要主动设置更高的 VE 频率？

### 结论

值得试，但建议默认设一个合理值而不是留空。

当前代码里 `CP_VDEC_VE_FREQ` 默认不设置。但从仓库中看：

- `lvgl_main.c` 启动时调用 `AW_MPI_VDEC_SetVEFreq(MM_INVALID_CHN, 0)` 把 VE 频率设为默认/最低
- `videoLibrary.c` 设为 324 MHz
- `sample_Player.cpp` 从配置读取

在双路录像和 CarPlay 同时跑时，VE 要同时服务 VDEC + 双路 VENC，默认频率很可能不够。

### 方案

- 将 `CP_VDEC_VE_FREQ` 的默认值从"不设置"改为环境变量默认 `480`
- 保留环境变量覆盖能力
- 在 `carplay_display_create()` 中，如果环境变量没设就用默认 480 MHz

---

## 问题 3：为什么 GetImage 调用了两次？

### 原因分析

```604:657:runcarplay/src/carplay_display/carplay_display.c
// 第一段：非阻塞循环取帧（timeout=0）
for (;;) {
    ERRORTYPE ret = AW_MPI_VDEC_GetImage(g_ctx.vdec_chn, &frame, 0);
    if (ret != SUCCESS) break;
    // ... 入帧队列
}

// 第二段：阻塞等待 30ms 再取一帧（timeout=30）
if (AW_MPI_VDEC_GetImage(g_ctx.vdec_chn, &frame, 30) == SUCCESS) {
    // ... 入帧队列
}
```

设计意图是：

- 先用非阻塞循环把解码器内部已经解好的帧全部取出
- 然后再阻塞等 30ms，给解码器一次"再出一帧"的机会

### 问题

这个 30ms 阻塞等待会直接卡住解码线程。在快速滑动场景下，手机连续推包，解码线程被卡 30ms 等一帧，而新包在 H264 队列里堆积，最终触发满队列淘汰。

### 方案

删掉第二段 30ms 阻塞 GetImage。改为：在非阻塞循环取帧后，如果一帧都没取到（`frame_cnt == 0`），才做一次短阻塞 `GetImage(timeout=10)`，仅作为"这次 SendStream 确实一帧都没解出来"的兜底。

---

## 问题 4：显示线程丢帧策略优化

### 当前行为

```687:692:runcarplay/src/carplay_display/carplay_display.c
while (g_fq.count > 1) {
    // 丢弃所有旧帧，只保留最新的一帧
}
```

这是一种"永远跳到最新帧"的极端策略。优点是低延迟；缺点是如果解码速度稍快于显示速度，中间帧全部浪费，画面看起来跳跃。

### 你提到的"平滑丢帧 + 增大队列 + 增大 VO buffer"

逐一分析：

- 增大 `FRAME_QUEUE_CAP` 到 8：可以，但不要太大，会增加整体延迟
- 增大 VO buffer 到 4：可以试，但当前 VO buffer=2 加上 G2D triple buffer 已经有 5 个 buffer 在流转，增大到 4 主要缓解 VO 侧抖动
- 平滑丢帧：对 CarPlay 投屏场景，关键目标是"低延迟"而不是"流畅过渡"。完全平滑意味着可能显示过时画面。但可以做一个折中：允许队列里保留 2 帧而不是 1 帧

### 方案

- `FRAME_QUEUE_CAP` 从 4 改为 8
- VO display buffer 从 2 改为 3（`AW_MPI_VO_SetChnDispBufNum`）
- 显示线程丢帧逻辑改为 `while (g_fq.count > 2)` 而不是 `> 1`，允许保留 2 帧的小缓冲吸收抖动，但仍然在积压超过 2 帧时快进追实时
- 给显示侧每次丢帧操作增加计数和日志

---

## 问题 5：日志精简与"解码慢还是显示慢"定位能力

### 当前日志问题

1. `CPD_LOG` 受 `CP_PERF_ENABLE` 和 `CP_PERF_DEEP` 双重门控，默认关闭
2. `h264_queue_evict` 的 printf 无条件输出但信息量大
3. 缺少关键指标：解码线程每秒处理包数 vs 显示线程每秒输出帧数
4. 大量 `%` 采样日志在不需要时仍消耗取模运算

### 方案

引入一组轻量"周期汇总日志"，每 3 秒打印一次，无条件输出，格式紧凑一行：

```
[carplay_perf] period=3s dec_in=60 dec_out=58 disp=19 fq_drop=39 h264_evict=0 h264_drop=0 send_fail=0 g2d_avg_us=XXX vo_avg_us=XXX
```

字段含义：

- `dec_in`：解码线程从 H264 队列 pop 的包数
- `dec_out`：解码线程从 VDEC GetImage 取到的帧数
- `disp`：显示线程实际送到 VO 的帧数
- `fq_drop`：帧队列里被丢弃的帧数
- `h264_evict`：H264 队列批量淘汰的包数
- `h264_drop`：H264 队列入队失败的包数
- `send_fail`：SendStream 失败次数
- `g2d_avg_us` / `vo_avg_us`：G2D 旋转和 VO 送帧的平均耗时

同时：

- 去掉逐包 `h264_queue_evict` 的无条件 printf，改为计数汇总
- 保留 `CPD_LOG` 体系不动，它是深度调试用的
- `h264_queue_evict` 仅在淘汰了关键相关包时才 printf 告警

这样在正常运行时只有每 3 秒一行日志，但一看就能判断"解码慢"（dec_in 高但 dec_out 低）还是"显示慢"（dec_out 高但 disp 低）。

---

## 补充优化：批量淘汰后的 got_idr 重置

当 `queue_evict_old_packets_locked()` 淘汰了 SPS/PPS/IDR 包时，将 `g_ctx.got_idr` 置 0。这样解码线程会自动跳过后续非关键帧直到新 SPS 到来，避免把"缺少参考帧"的数据喂给解码器引发花屏。

---

## 改动汇总

所有改动均在 `carplay_display.c` 内：

- `FRAME_QUEUE_CAP`：4 -> 8
- `AW_MPI_VO_SetChnDispBufNum`：2 -> 3
- `decode_thread_fn` 中 GetImage 逻辑：删除第二段 30ms 阻塞，改为 `frame_cnt==0` 时兜底短阻塞 10ms
- `display_thread_fn` 丢帧阈值：`count > 1` -> `count > 2`
- `queue_evict_old_packets_locked`：淘汰关键包时重置 `g_ctx.got_idr = 0`；去掉无条件 printf，改为仅在淘汰关键包时告警
- VE 频率默认值：不设置 -> 默认 480 MHz
- 新增周期汇总日志：每 3 秒一行，涵盖解码/显示/丢帧全链路指标
- 删除现有逐包级别的无条件 printf（evict/drop/nomem），统一收编到周期汇总

