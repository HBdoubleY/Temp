#ifdef ENABLE_CARPLAY

#include "zlink_client.h"
#include "carplay_display.h"
#include "link_touch_evdev.h"
#include "libzlink.h"
#include "ComStruct.h"
#include "Rtc2C.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <sys/syscall.h>

#define LINK_TYPE_CARPLAY       2
#define LINK_TYPE_ANDROIDAUTO   3

/* Minimal view of g_sys_Data (defined in lvgl) - first member is linktype */
extern struct { int linktype; } g_sys_Data;

static LIBZLINK_HANDLE g_handle;
/* Lower default fps for softer CPU load and better touch responsiveness. */
static int g_session_fps = 15;
static int g_session_width = 960;
static int g_session_height = 480;
static int g_session_fallback_w[3] = {960, 1440, 0};
static int g_session_fallback_h[3] = {480, 720, 0};
static int g_session_fallback_count = 1;
static int g_session_fallback_idx = 0;
static int g_session_init_attempts = 0;

#define PREBUF_PACKET_CAP  24
#define PREBUF_PACKET_MAX  (256 * 1024)

typedef struct {
	char *data;
	int len;
} prebuf_packet_t;

static struct {
	prebuf_packet_t slot[PREBUF_PACKET_CAP];
	int count;
	int active;
	int pending_home_link_type;
	int session_started;
	pthread_mutex_t mutex;
} g_video_state = {
	.mutex = PTHREAD_MUTEX_INITIALIZER,
};

#define ZLINK_VIDEO_DUMP_DIR  "/tmp/zlink/video"

static struct {
	int enabled;
	FILE *fp;
	pthread_mutex_t mutex;
} g_video_dump = {
	.mutex = PTHREAD_MUTEX_INITIALIZER,
};

static int g_last_focus_req = -1;
static long long g_last_focus_req_ms = 0;
static unsigned int g_perf_session_id = 0;
static int g_perf_enable = -1;
static int g_perf_deep = -1;
static int g_perf_sample_n = -1;
static int g_perf_warn_us = -1;

static long long zlink_now_us(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (long long)tv.tv_sec * 1000000LL + (long long)tv.tv_usec;
}

static long long zlink_tid(void)
{
	return (long long)syscall(SYS_gettid);
}

static int zlink_env_int(const char *key, int def)
{
	const char *s = getenv(key);
	if (!s || s[0] == '\0')
		return def;
	return atoi(s);
}

static void zlink_perf_init_once(void)
{
	if (g_perf_enable >= 0)
		return;
	g_perf_enable = zlink_env_int("CP_PERF_ENABLE", 0) ? 1 : 0;
	g_perf_deep = zlink_env_int("CP_PERF_DEEP", 0) ? 1 : 0;
	g_perf_sample_n = zlink_env_int("CP_PERF_SAMPLE_N", 1);
	if (g_perf_sample_n <= 0)
		g_perf_sample_n = 1;
	g_perf_warn_us = zlink_env_int("CP_PERF_WARN_US", 20000);
	if (g_perf_warn_us <= 0)
		g_perf_warn_us = 20000;
	printf("[cp_perf] cfg enable=%d deep=%d sample_n=%d warn_us=%d\n",
	       g_perf_enable, g_perf_deep, g_perf_sample_n, g_perf_warn_us);
}

#define CP_PERF_LOG(stage, fmt, ...) \
	do { \
		zlink_perf_init_once(); \
		if (g_perf_enable && g_perf_deep) { \
			printf("[cp_perf] ts_us=%lld tid=%lld stage=%s sid=%u " fmt "\n", \
			       zlink_now_us(), zlink_tid(), stage, g_perf_session_id, ##__VA_ARGS__); \
		} \
	} while (0)

static long long zlink_now_ms(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (long long)tv.tv_sec * 1000LL + (long long)tv.tv_usec / 1000LL;
}

static int packet_has_sps(const char *data, int len)
{
	const unsigned char *d = (const unsigned char *)data;
	for (int i = 0; i + 4 < len; i++) {
		if (d[i] == 0 && d[i + 1] == 0 &&
		    (d[i + 2] == 1 || (d[i + 2] == 0 && d[i + 3] == 1))) {
			int off = (d[i + 2] == 1) ? i + 3 : i + 4;
			if (off < len && (d[off] & 0x1F) == 7)
				return 1;
		}
	}
	return 0;
}

static void prebuf_clear_locked(void)
{
	for (int i = 0; i < g_video_state.count; i++) {
		free(g_video_state.slot[i].data);
		g_video_state.slot[i].data = NULL;
		g_video_state.slot[i].len = 0;
	}
	g_video_state.count = 0;
}

static void prebuf_clear(void)
{
	pthread_mutex_lock(&g_video_state.mutex);
	prebuf_clear_locked();
	pthread_mutex_unlock(&g_video_state.mutex);
}

static void prebuf_push_locked(const char *data, int len)
{
	if (len <= 0 || len > PREBUF_PACKET_MAX)
		return;

	if (packet_has_sps(data, len))
		prebuf_clear_locked();

	if (g_video_state.count >= PREBUF_PACKET_CAP) {
		free(g_video_state.slot[0].data);
		memmove(&g_video_state.slot[0], &g_video_state.slot[1],
		        sizeof(g_video_state.slot[0]) * (PREBUF_PACKET_CAP - 1));
		g_video_state.count = PREBUF_PACKET_CAP - 1;
	}

	char *copy = (char *)malloc((size_t)len);
	if (!copy)
		return;
	memcpy(copy, data, (size_t)len);
	g_video_state.slot[g_video_state.count].data = copy;
	g_video_state.slot[g_video_state.count].len = len;
	g_video_state.count++;
}

static int link_type_from_phone_type(enum PHONE_TYPE phone_type)
{
	switch (phone_type) {
	case CARPLAY_WIRED:
	case CARPLAY_WIRELESS:
		return LINK_TYPE_CARPLAY;
	case AA_WIRED:
	case AA_WIRELESS:
		return LINK_TYPE_ANDROIDAUTO;
	default:
		return 0;
	}
}

static void zlink_load_session_tuning(void)
{
	const char *fps = getenv("ZLINK_SESSION_FPS");
	const char *res = getenv("ZLINK_SESSION_RES");
	if (fps && fps[0] != '\0') {
		int v = atoi(fps);
		if (v >= 12 && v <= 30)
			g_session_fps = v;
	}
	if (res && res[0] != '\0') {
		int w = 0;
		int h = 0;
		if (sscanf(res, "%dx%d", &w, &h) == 2) {
			if ((w == 720 && h == 360) || (w == 960 && h == 480) || (w == 1440 && h == 720)) {
				g_session_width = w;
				g_session_height = h;
			}
		}
	}

	if (g_session_width == 720 && g_session_height == 360) {
		g_session_fallback_w[0] = 720;  g_session_fallback_h[0] = 360;
		g_session_fallback_w[1] = 960;  g_session_fallback_h[1] = 480;
		g_session_fallback_w[2] = 1440; g_session_fallback_h[2] = 720;
		g_session_fallback_count = 3;
	} else if (g_session_width == 960 && g_session_height == 480) {
		g_session_fallback_w[0] = 960;  g_session_fallback_h[0] = 480;
		g_session_fallback_w[1] = 1440; g_session_fallback_h[1] = 720;
		g_session_fallback_w[2] = 0;    g_session_fallback_h[2] = 0;
		g_session_fallback_count = 2;
	} else {
		g_session_fallback_w[0] = 960;  g_session_fallback_h[0] = 480;
		g_session_fallback_w[1] = 1440; g_session_fallback_h[1] = 720;
		g_session_fallback_w[2] = 0;    g_session_fallback_h[2] = 0;
		g_session_fallback_count = 2;
	}
	g_session_fallback_idx = 0;
	printf("zlink session tuning: fps=%d req_res=%dx%d fallback_cnt=%d\n",
	       g_session_fps, g_session_width, g_session_height, g_session_fallback_count);
}

static void session_init(void)
{
	static struct SESSION_DATA session_data;
	memset(&session_data, 0, sizeof(session_data));
	session_data.width = g_session_fallback_w[g_session_fallback_idx];
	session_data.height = g_session_fallback_h[g_session_fallback_idx];
	session_data.width_margin = 0;
	session_data.height_margin = 0;
	session_data.fps = g_session_fps;
	session_data.density = 230;
	session_data.is_right_hand = 0;
	session_data.is_night_mode = 0;
	session_data.apple_wired = APPLE_WIRED_LINK_NONE;
	session_data.apple_wireless = CARPLAY_WIRELESS_MODE;
	session_data.android_wired = ANDROID_WIRED_LINK_NONE;
	session_data.android_wireless = AA_WIRELESS_MODE;
	session_data.cp_icon_path = "/opt/work/app/carplay/icon/icon_104_104.png";
	// session_data.is_use_phone_audio = 0;
	session_data.is_use_phone_audio = 1;
	session_data.platform_id = "zlink";
	session_data.vendor_name = "zlink-test";
	session_data.is_force_usb_host = 0;
	// session_data.mfi_bus_num = -1;
	session_data.mfi_bus_num = 3;
	session_data.otg_bus_num = -1;
	printf("zlink session init: attempt=%d use_res=%dx%d fps=%d\n",
	       g_session_init_attempts + 1, session_data.width, session_data.height, session_data.fps);

	libzlink_init_session_2(&session_data);
}

static int video_data_cb(char *data, int len, struct VIDEO_SCREEN_INFO *info, void *user_data)
{
	static unsigned long long pkt_seq = 0;
	static unsigned long long window_pkt = 0;
	static unsigned long long window_bytes = 0;
	static unsigned long long window_feed_fail = 0;
	static long long window_start_us = 0;
	long long cb_start_us = zlink_now_us();
	unsigned long long my_seq;
	(void)info;
	(void)user_data;
	if (data && len > 0) {
		int prebuf_count = 0;
		int feed_ret = 0;
		int active = 0;
		my_seq = ++pkt_seq;
		pthread_mutex_lock(&g_video_state.mutex);
		prebuf_push_locked(data, len);
		prebuf_count = g_video_state.count;
		active = g_video_state.active;
		pthread_mutex_unlock(&g_video_state.mutex);
		if (active) {
			feed_ret = carplay_display_feed_h264(data, len);
			if (feed_ret != 0)
				window_feed_fail++;
		}

		pthread_mutex_lock(&g_video_dump.mutex);
		if (g_video_dump.enabled && g_video_dump.fp) {
			size_t n = fwrite(data, 1, (size_t)len, g_video_dump.fp);
			(void)n;
			fflush(g_video_dump.fp);
		}
		pthread_mutex_unlock(&g_video_dump.mutex);

		window_pkt++;
		window_bytes += (unsigned long long)len;
		if (window_start_us == 0)
			window_start_us = cb_start_us;
		if ((my_seq % (unsigned long long)zlink_client_perf_sample_n()) == 0ULL) {
			long long cb_cost_us = zlink_now_us() - cb_start_us;
			CP_PERF_LOG("video_data_cb",
			            "pkt_seq=%llu len=%d active=%d prebuf_count=%d feed_ret=%d cb_cost_us=%lld",
			            my_seq, len, active, prebuf_count, feed_ret, cb_cost_us);
		}
		if (window_pkt >= 100) {
			long long now_us = zlink_now_us();
			long long dur_us = now_us - window_start_us;
			unsigned long long pps = (dur_us > 0) ? (window_pkt * 1000000ULL) / (unsigned long long)dur_us : 0ULL;
			unsigned long long avg_len = (window_pkt > 0) ? (window_bytes / window_pkt) : 0ULL;
			CP_PERF_LOG("video_data_cb_window",
			            "pkts=%llu pps=%llu avg_len=%llu feed_fail=%llu window_us=%lld",
			            window_pkt, pps, avg_len, window_feed_fail, dur_us);
			window_pkt = 0;
			window_bytes = 0;
			window_feed_fail = 0;
			window_start_us = now_us;
		}
	}
	return 0;
}

static int session_state_cb(enum LIBZLINK_SESSION_STATE session_state, enum PHONE_TYPE phone_type, void *user_data)
{
	printf("\n\n\nsession_state_cb: session_state = %d, phone_type = %d\n\n\n", session_state, phone_type);
	(void)user_data;
	if (session_state == SESSION_WAIT_INIT) {
		pthread_mutex_lock(&g_video_state.mutex);
		int started = g_video_state.session_started;
		pthread_mutex_unlock(&g_video_state.mutex);
		if (!started && g_session_init_attempts > 0 && g_session_fallback_idx + 1 < g_session_fallback_count) {
			g_session_fallback_idx++;
			printf("zlink session fallback: switch to %dx%d (idx=%d/%d)\n",
			       g_session_fallback_w[g_session_fallback_idx],
			       g_session_fallback_h[g_session_fallback_idx],
			       g_session_fallback_idx + 1, g_session_fallback_count);
		}
		session_init();
		g_session_init_attempts++;
	} else if (session_state == SESSION_STARTED) {
		int link_type = link_type_from_phone_type(phone_type);
		int active;
		if (link_type)
			g_sys_Data.linktype = link_type;
		pthread_mutex_lock(&g_video_state.mutex);
		g_video_state.session_started = 1;
		active = g_video_state.active;
		pthread_mutex_unlock(&g_video_state.mutex);
		g_session_init_attempts = 0;
		if (active)
			libzlink_video_focus(0);
		else
			libzlink_video_focus(1);
	} else {
		/* session ended or not started: clear session flag and linktype; if currently projecting, do same cleanup as video_focus_cb(1) */
		int active;
		int link_to_pending;

		pthread_mutex_lock(&g_video_state.mutex);
		g_video_state.session_started = 0;
		active = g_video_state.active;
		link_to_pending = g_sys_Data.linktype;
		if (active) {
			g_video_state.active = 0;
			g_video_state.pending_home_link_type = link_to_pending;
		}
		pthread_mutex_unlock(&g_video_state.mutex);

		g_sys_Data.linktype = 0;
		if (active) {
			carplay_display_destroy();
			carplay_link_touch_set_active(0);
		}
	}
	return 0;
}

static int request_wifi_info_cb(void *user_data)
{
	(void)user_data;
	libzlink_wifi_info2("hycarplaytest", "88888888", "192.168.1.2", 36, "wlan0");
	return 0;
}

static int main_audio_start_cb(enum ZLINK_MEDIA_TYPE media_type, void *user_data) { (void)media_type; (void)user_data; return 0; }
static int main_audio_data_cb(char *data, int len, enum ZLINK_MEDIA_TYPE media_type, int sample, int channels, int bits, void *user_data) { (void)data; (void)len; (void)media_type; (void)sample; (void)channels; (void)bits; (void)user_data; return 0; }
static int main_audio_stop_cb(enum ZLINK_MEDIA_TYPE media_type, void *user_data) { (void)media_type; (void)user_data; return 0; }
// static int video_focus_cb(int is_hu_focus_on, void *user_data) { (void)is_hu_focus_on; (void)user_data; return 0; }
static int audio_focus_cb(int is_hu_focus_on, void *user_data) { (void)is_hu_focus_on; (void)user_data; return 0; }
static int request_p2p_cb(void *user_data) { (void)user_data; return 0; }
static int mic_start_cb(enum ZLINK_MEDIA_TYPE media_type, int sample, int channels, int bits, void *user_data) { (void)media_type; (void)sample; (void)channels; (void)bits; (void)user_data; return 0; }
static int mic_stop_cb(enum ZLINK_MEDIA_TYPE media_type, void *user_data) { (void)media_type; (void)user_data; return 0; }

static int video_focus_cb(int is_hu_focus_on, void *user_data)
{
	(void)user_data;
	long long now = zlink_now_ms();
	if (g_last_focus_req == is_hu_focus_on && (now - g_last_focus_req_ms) < 1200) {
		printf("video_focus_request: debounced (focus=%d)\n", is_hu_focus_on);
		return 0;
	}
	g_last_focus_req = is_hu_focus_on;
	g_last_focus_req_ms = now;
	CP_PERF_LOG("video_focus_cb", "focus=%d", is_hu_focus_on);

	if (is_hu_focus_on) {
		int was_active = 0;
		printf("video_focus_request: request back to HU HMI\n");
		pthread_mutex_lock(&g_video_state.mutex);
		was_active = g_video_state.active;
		g_video_state.active = 0;
		g_video_state.pending_home_link_type = g_sys_Data.linktype;
		pthread_mutex_unlock(&g_video_state.mutex);
		if (was_active) {
			carplay_display_destroy();
			carplay_link_touch_set_active(0);
		}
	} else {
		printf("video_focus_request: request back to phone HMI\n");
	}

	return 0;
}

static int phone_time_cb(uint64_t now_time_second, int time_zone_minute, void *user_data)
{
	(void)user_data;

	int64_t local_epoch = (int64_t)now_time_second + (int64_t)time_zone_minute * 60;
	if (local_epoch < 0)
		local_epoch = 0;

	time_t tt = (time_t)local_epoch;
	struct tm tm_buf;
	if (gmtime_r(&tt, &tm_buf) == NULL) {
		printf("phone_time_cb: gmtime_r failed (now=%llu tz_min=%d)\n",
		       (unsigned long long)now_time_second, time_zone_minute);
		return 0;
	}

	TTime t;
	memset(&t, 0, sizeof(t));
	t.nYear = (WORD)(tm_buf.tm_year + 1900);
	t.nMonth = (BYTE)(tm_buf.tm_mon + 1);
	t.nDay = (BYTE)tm_buf.tm_mday;
	t.nHour = (BYTE)tm_buf.tm_hour;
	t.nMinute = (BYTE)tm_buf.tm_min;
	t.nSecond = (BYTE)tm_buf.tm_sec;
	t.nWeek = (BYTE)tm_buf.tm_wday;

	if (RtcSetTime2C(&t)) {
		printf("phone_time_cb: RTC synced from phone %04u-%02u-%02u %02u:%02u:%02u (tz_off_min=%d)\n",
		       (unsigned)t.nYear, (unsigned)t.nMonth, (unsigned)t.nDay,
		       (unsigned)t.nHour, (unsigned)t.nMinute, (unsigned)t.nSecond,
		       time_zone_minute);
	} else {
		printf("phone_time_cb: RtcSetTime2C failed (now=%llu tz_min=%d)\n",
		       (unsigned long long)now_time_second, time_zone_minute);
	}

	return 0;
}

static void register_callbacks(void)
{
	libzlink_session_state_cb_init(session_state_cb);
	libzlink_video_data_cb_init(video_data_cb);
//	libzlink_main_audio_start_cb_init(main_audio_start_cb);
//	libzlink_main_data_cb_init(main_audio_data_cb);
//	libzlink_main_audio_stop_cb_init(main_audio_stop_cb);
	libzlink_video_focus_request_cb_init(video_focus_cb);
//	libzlink_audio_focus_request_cb_init(audio_focus_cb);
//	libzlink_request_p2p_start_cb_init(request_p2p_cb);
	libzlink_request_wifi_info_cb_init(request_wifi_info_cb);
//	libzlink_mic_start_cb_init(mic_start_cb);
//	libzlink_mic_stop_cb_init(mic_stop_cb);
	// printf("\n\n\nregister_callbacks: phone_time_cb\n\n\n");
	libzlink_phone_time_init(phone_time_cb);
}

void zlink_client_run(void)
{
	zlink_load_session_tuning();
	register_callbacks();
	g_handle = libzlink_init(NULL);
	if (!g_handle) {
		return;
	}
	while (1) {
		// if (libzlink_check_ready(g_handle) == 0)
		// 	break;
		// sleep(1);
		if (libzlink_check_ready(g_handle))
			break;
		sleep(1);
	}
	libzlink_request_state();
	while (1)
		sleep(60);
}

void zlink_client_set_video_active(int active)
{
	long long t0 = zlink_now_us();
	int replayed = 0;
	pthread_mutex_lock(&g_video_state.mutex);
	if (active) {
		g_video_state.active = 1;
		for (int i = 0; i < g_video_state.count; i++) {
			carplay_display_feed_h264(g_video_state.slot[i].data, g_video_state.slot[i].len);
			replayed++;
		}
	} else {
		g_video_state.active = 0;
		prebuf_clear_locked();
	}
	pthread_mutex_unlock(&g_video_state.mutex);
	CP_PERF_LOG("set_video_active", "active=%d replayed=%d cost_us=%lld",
	            active, replayed, zlink_now_us() - t0);
}

int zlink_client_request_video_focus(int is_hu_focus_on)
{
	long long t0 = zlink_now_us();
	int ret = libzlink_video_focus(is_hu_focus_on ? 1 : 0);
	long long cost = zlink_now_us() - t0;
	if (cost > (long long)zlink_client_perf_warn_us()) {
		CP_PERF_LOG("request_video_focus_slow", "focus=%d ret=%d cost_us=%lld",
		            is_hu_focus_on, ret, cost);
	} else {
		CP_PERF_LOG("request_video_focus", "focus=%d ret=%d cost_us=%lld",
		            is_hu_focus_on, ret, cost);
	}
	return ret;
}

int zlink_client_take_pending_home_request(void)
{
	int link_type;

	pthread_mutex_lock(&g_video_state.mutex);
	link_type = g_video_state.pending_home_link_type;
	g_video_state.pending_home_link_type = 0;
	pthread_mutex_unlock(&g_video_state.mutex);

	return link_type;
}

int zlink_client_is_session_started(void)
{
	int started;

	pthread_mutex_lock(&g_video_state.mutex);
	started = g_video_state.session_started;
	pthread_mutex_unlock(&g_video_state.mutex);

	return started ? 1 : 0;
}

void zlink_client_set_video_dump(int enable)
{
	pthread_mutex_lock(&g_video_dump.mutex);
	if (enable) {
		if (g_video_dump.fp) {
			fclose(g_video_dump.fp);
			g_video_dump.fp = NULL;
		}
		mkdir("/tmp/zlink", 0755);
		mkdir(ZLINK_VIDEO_DUMP_DIR, 0755);
		time_t t = time(NULL);
		struct tm *tm = localtime(&t);
		char path[256];
		if (tm && snprintf(path, sizeof(path), ZLINK_VIDEO_DUMP_DIR "/dump_%04d%02d%02d_%02d%02d%02d.264",
			tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec) < (int)sizeof(path)) {
			g_video_dump.fp = fopen(path, "wb");
			if (g_video_dump.fp) {
				g_video_dump.enabled = 1;
				printf("[zlink_client] H264 dump enabled: %s\n", path);
			}
		}
	} else {
		g_video_dump.enabled = 0;
		if (g_video_dump.fp) {
			fclose(g_video_dump.fp);
			g_video_dump.fp = NULL;
			printf("[zlink_client] H264 dump disabled\n");
		}
	}
	pthread_mutex_unlock(&g_video_dump.mutex);
}

void zlink_client_reset_video_prebuffer(void)
{
	prebuf_clear();
}

void carplay_is_running2(void)
{
	libzlink_request_state();
}

void zlink_client_perf_set_session_id(unsigned int session_id)
{
	g_perf_session_id = session_id;
}

unsigned int zlink_client_perf_get_session_id(void)
{
	return g_perf_session_id;
}

int zlink_client_perf_is_enabled(void)
{
	zlink_perf_init_once();
	return g_perf_enable;
}

int zlink_client_perf_sample_n(void)
{
	zlink_perf_init_once();
	return g_perf_sample_n;
}

int zlink_client_perf_warn_us(void)
{
	zlink_perf_init_once();
	return g_perf_warn_us;
}

#endif /* ENABLE_CARPLAY */
