#ifndef ZLINK_CLIENT_H
#define ZLINK_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/*
 * Compile-time switch for CarPlay performance instrumentation.
 * Keep enabled by default to preserve current behavior.
 */
#ifndef CP_PERF_COMPILE
#define CP_PERF_COMPILE 0
#endif

#ifdef ENABLE_CARPLAY

/** Run zlink client loop (blocking). Call from a dedicated thread. Returns when init fails. */
void zlink_client_run(void);

/* Enable/disable forwarding buffered H264 packets to carplay_display. */
void zlink_client_set_video_active(int active);

/* Drop cached H264 prebuffer so next enter uses a fresh stream. */
void zlink_client_reset_video_prebuffer(void);

/* Request zlink video focus: 0 = phone projection, 1 = HU UI. */
int zlink_client_request_video_focus(int is_hu_focus_on);

/* Fetch and clear a pending "return to home" UI request. Returns link type or 0. */
int zlink_client_take_pending_home_request(void);

/* Return 1 if session_state == SESSION_STARTED, 0 otherwise. */
int zlink_client_is_session_started(void);

/**
 * Enable(1) or disable(0) dumping H264 from video_data_cb to /tmp/zlink/video/.
 * When enabled, creates the directory if needed and opens a new file named
 * dump_YYYYMMDD_HHMMSS.264. Call with 0 to stop and close the file.
 */
void zlink_client_set_video_dump(int enable);

void carplay_is_running2(void);

/* Perf tracing helpers shared across CarPlay modules. */
void zlink_client_perf_set_session_id(unsigned int session_id);
unsigned int zlink_client_perf_get_session_id(void);
int zlink_client_perf_is_enabled(void);
int zlink_client_perf_sample_n(void);
int zlink_client_perf_warn_us(void);

#endif /* ENABLE_CARPLAY */

#ifdef __cplusplus
}
#endif

#endif /* ZLINK_CLIENT_H */
