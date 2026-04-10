# CarPlay + 录像并发性能分析模板

## 1. 测试信息
- 固件版本：
- 编译时间：
- 设备型号：
- 解码后端：`aw_vdec` / `ffmpeg`
- 会话分辨率：`ZLINK_SESSION_RES=...`
- 会话帧率：`ZLINK_SESSION_FPS=...`
- 是否开启录像：是/否

## 2. 场景矩阵
- 基线：默认参数
- A：低分辨率输入（720x360）
- A+B：低分辨率 + 队列策略
- A+B+C：低分辨率 + 队列策略 + 触控低延迟

每个场景建议运行 2~3 分钟，并分别记录“静止画面/地图拖动/频繁触控”。

## 3. 关键日志标签

### CarPlay 解码显示侧
- `carplay_perf`
- `aw_send_stream_slow`
- `aw_send_stream_fail`
- `aw_decode_no_frame`
- `aw_get_image_slow`
- `touch_send_xy`
- `touch_map_send`
- `display_frame`

### 录像并发侧
- `rec_step_ok`
- `rec_start_venc`
- `rec_start_mux`
- `rec_start_done`
- `rec_venc_bad_fps`
- `RecRender ... rec_in_pts_v_invalid`
- `FsWriter ... Bytes too long`

### 触控链路侧
- `touch_batch`
- `touch_emit`
- `touch_emit_interval`
- `touch_poll_gap_warn`

## 4. 指标口径
- 解码送流稳定性：`send_fail`、`send_avg_us`
- 解码输出稳定性：`dec_in/dec_out` 差值趋势
- 显示丢帧强度：`fq_skip`
- 显示渲染负载：`g2d_avg_us`、`vo_avg_us`
- 触控链路延迟：`touch_emit.pipeline_us`、`touch_emit_interval`
- 录像写盘风险：`RecRender/FsWriter` 异常次数与持续时长

## 5. 结论模板（接口级）
- 结论一句话：
  - 例：`CarPlay 主要阻塞发生在 AW_MPI_VDEC_SendStream，根因来自录像写盘链路长时阻塞。`
- 证据链：
  1) 录像侧异常标签及时间点  
  2) 同时间窗 `send_fail/send_avg_us/fq_skip` 的变化  
  3) `g2d_avg_us/vo_avg_us` 是否同步恶化（用于排除显示端主因）
- 归因边界：
  - 主因接口：
  - 连带影响接口：
  - 已排除接口：

## 6. 回归验收阈值（建议）
- `send_fail`：持续窗口内趋近 0
- `send_avg_us`：稳定在百微秒级
- `fq_skip`：较基线下降，且触控时不出现长时间高位
- `FsWriter`：无 >500ms 的长写阻塞日志

