# CarPlay + DVR Performance Analysis

## 1. Background

Observed behavior:

- CarPlay is smooth when recording is off.
- Enabling `MPP_DBG_SINGLE_REC_CH=1` improves stutter but does not eliminate it.
- This strongly suggests recording-path load is a major contributor.

Relevant files:

- `lvgl-gui/lvgl_main.c`
- `runcarplay/src/app/video/mpp_camera.c`
- `runcarplay/src/carplay_display/carplay_display.c`

---

## 2. Recording Flow (Current Implementation)

### 2.1 Trigger layer (UI / app logic)

In `lvgl_main.c`, timer callback `bt_status_check_timer()` checks TF/camera status and auto-starts recording:

1. `TFFreeMemDetection()`
2. `deletFileInRecorderPath(REC_PATH)`
3. `recording(&g_sys_Data.vipp0_config)`
4. `recording(&g_sys_Data.vipp8_config)`
5. `dashTimeMark(...)`
6. `SoundRecording(...)`
7. start `DVRstaTimer`

The same start/stop logic also exists in `screen_DVR_events_init.c` (manual record button path).

### 2.2 Camera pipeline initialization

`initVi(pContext, videv, vichn)` in `mpp_camera.c`:

1. `InitMppCameraData()`
2. `setConfigPara()`
3. `createViChn()`
4. `CreateMsgQueueThread()`
5. `pthread_create(Vi2VencFrameThread)`
6. `pthread_create(FsyncFrameThread)` (unless debug switch disables it)

### 2.3 Recording start pipeline

`recording(pContext)` calls:

1. `createAIChn()`  
2. `createAencChn()`  
3. `createVencChn()`  
4. `createMuxChn()`  
5. `prepare()`  
6. `startVideoRecording()`  

### 2.4 Data path during recording

- Capture thread `GetCSIFrameThread` pulls VI frames (`AW_MPI_VI_GetFrame`).
- If `mRecorderFlag==1`, frames are pushed into queue (`queue_mpp_push`).
- `Vi2VencFrameThread` pops queue and sends to encoder (`AW_MPI_VENC_SendFrame`).
- MUX channel writes TS file.
- `FsyncFrameThread` does periodic `open + fsync + close` every second.

### 2.5 Recording stop pipeline

`stopRecording()` -> `stopVideoRecording()`:

1. clear recorder flag
2. stop MUX / VENC / AENC
3. unbind VE<->MUX and AI/AENC<->MUX
4. destroy MUX/VENC/AENC/AI channels

---

## 3. Current Recording Attributes / Parameters

From `setConfigPara()`:

- VI:
  - `1920x1080`
  - `25fps`
  - pixel format `MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420`
  - buffer num `5`
- VENC:
  - `1920x1080`
  - `25fps`
  - bitrate `8 Mbps` (`1048576 * 8`)
  - codec `H.264`
  - RC mode `0` (CBR path in current config function)
- Audio:
  - AAC, mono, 16k sample rate, 32kbps
- MUX:
  - TS output
  - direct write mode (`FSWRITEMODE_DIRECT`)

In current app logic, both `vipp0` and `vipp8` are started for recording.

---

## 4. Why single-channel debug switch helps

When `MPP_DBG_SINGLE_REC_CH=1`, one recording channel is skipped.

Impact:

- less VI->VENC workload
- less queue pressure
- less MUX write bandwidth
- less storage synchronization pressure

Because stutter still exists, bottleneck is likely not only "double-channel count", but also write/scheduling behavior.

---

## 5. Code-level review findings (application layer)

### 5.1 Synchronous heavy start in UI timer callback

Record start is executed directly in `bt_status_check_timer()` (500ms timer), including channel creation and bind/start.  
This can cause UI/control-path jitter during transition.

### 5.2 Missing return-code checks

`recording()` currently ignores failures from intermediate setup calls and still continues.  
Recommended: fail fast with rollback.

### 5.3 Stop path is not fully defensive

`stopVideoRecording()` does many stop/destroy operations without early guard on invalid channel state.  
Recommended: channel-state checks before each operation.

### 5.4 Duplicate start logic in multiple files

Auto-start and manual-start paths each perform similar sequences.  
Recommended: one shared API for `start_recording_all()` / `stop_recording_all()`.

### 5.5 Periodic fsync strategy is expensive

`open + fsync + close` per second per channel creates IO jitter and CPU wakeups.

---

## 6. Proposed Optimization Plan

### Phase A (quick, low risk, app-level)

1. Move record start/stop to dedicated worker thread (non-UI callback context).
2. Keep "single channel" feature as runtime config (not debug-only) for low-end SKU.
3. Gate second channel by camera connectivity (`front`/`rear`) and mode.
4. Add strict error handling and rollback for `recording()` chain.

Expected: reduce startup hitch and control-path stalls.

### Phase B (IO smoothing)

1. Replace per-second `open+fsync+close` with:
   - persistent fd, and
   - lower-frequency flush or conditional flush.
2. Align MUX rotation/segment strategy to avoid aggressive sync.

Expected: reduce periodic stutter and storage-induced latency spikes.

### Phase C (encoding budget tuning)

1. Use adaptive profile:
   - when projection active: reduce recorder fps/bitrate (e.g. 20fps + lower bitrate).
2. Optional mixed mode:
   - main channel full quality, secondary channel lower quality.

Expected: improve sustained smoothness under dual workload.

### Phase D (scheduler/priority and observability)

1. Pin critical threads and tune priorities carefully.
2. Add periodic stats:
   - queue depth
   - encode latency
   - write latency
   - dropped frames

Expected: make bottlenecks measurable and optimization repeatable.

---

## 7. Suggested validation matrix

Run on same scene for 2-3 minutes each:

1. baseline (dual record + projection)
2. + disable fsync
3. + single channel
4. + single channel + disable fsync
5. + reduced fps/bitrate profile when projection active

Collect:

- subjective smoothness
- frame drop count
- CPU usage
- write bandwidth / latency (if available)

This matrix can quickly separate compute bottleneck vs IO bottleneck vs control-path jitter.

