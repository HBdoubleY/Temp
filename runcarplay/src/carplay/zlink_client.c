#ifdef ENABLE_CARPLAY

#include "zlink_client.h"
#include "carplay_display.h"
#include "libzlink.h"
#include "ComStruct.h"
#include "Rtc2C.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sched.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>

#define LINK_TYPE_CARPLAY       2
#define LINK_TYPE_ANDROIDAUTO   3

/* Minimal view of g_sys_Data (defined in lvgl) - first member is linktype */
extern struct { int linktype; } g_sys_Data;

static LIBZLINK_HANDLE g_handle;

#define PREBUF_PACKET_CAP  24
#define PREBUF_PACKET_MAX  (256 * 1024)
#define ZLINK_METRICS_LOG_INTERVAL_US (2ULL * 1000 * 1000)
#define ZLINK_VIDEO_CB_PRIO          8

typedef struct {
	char *data;
	int len;
} prebuf_packet_t;

typedef struct {
	uint64_t cb_total;
	uint64_t active_forward_total;
	uint64_t active_forward_fail;
	uint64_t prebuf_drop_total;
	uint64_t max_gap_us;
	uint64_t last_cb_us;
	uint64_t last_log_us;
	int priority_attempted;
} zlink_video_metrics_t;

static struct {
	prebuf_packet_t slot[PREBUF_PACKET_CAP];
	int head;
	int tail;
	int count;
	int active;
	int pending_home_link_type;
	int session_started;
	zlink_video_metrics_t metrics;
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

static uint64_t monotonic_time_us(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)ts.tv_sec * 1000 * 1000 + (uint64_t)ts.tv_nsec / 1000;
}

static void zlink_video_try_raise_priority_locked(void)
{
	struct sched_param param;
	int ret;

	if (g_video_state.metrics.priority_attempted)
		return;
	g_video_state.metrics.priority_attempted = 1;

	memset(&param, 0, sizeof(param));
	param.sched_priority = ZLINK_VIDEO_CB_PRIO;
	ret = pthread_setschedparam(pthread_self(), SCHED_RR, &param);
	if (ret == 0) {
		printf("[zlink_client] video callback thread priority raised policy=%d prio=%d\n",
		       SCHED_RR, param.sched_priority);
	} else {
		printf("[zlink_client] video callback priority raise failed ret=%d (%s)\n",
		       ret, strerror(ret));
	}
}

static void zlink_video_metrics_note_locked(int active, int feed_ret, uint64_t now_us)
{
	if (g_video_state.metrics.last_cb_us != 0) {
		uint64_t gap_us = now_us - g_video_state.metrics.last_cb_us;
		if (gap_us > g_video_state.metrics.max_gap_us)
			g_video_state.metrics.max_gap_us = gap_us;
	}
	g_video_state.metrics.last_cb_us = now_us;
	g_video_state.metrics.cb_total++;
	if (active) {
		g_video_state.metrics.active_forward_total++;
		if (feed_ret != 0)
			g_video_state.metrics.active_forward_fail++;
	}
	if (g_video_state.metrics.last_log_us == 0)
		g_video_state.metrics.last_log_us = now_us;
	if (now_us - g_video_state.metrics.last_log_us >= ZLINK_METRICS_LOG_INTERVAL_US) {
		printf("[zlink_client] stats cb=%llu active=%llu feed_fail=%llu prebuf_drop=%llu prebuf_depth=%d max_gap_us=%llu\n",
		       (unsigned long long)g_video_state.metrics.cb_total,
		       (unsigned long long)g_video_state.metrics.active_forward_total,
		       (unsigned long long)g_video_state.metrics.active_forward_fail,
		       (unsigned long long)g_video_state.metrics.prebuf_drop_total,
		       g_video_state.count,
		       (unsigned long long)g_video_state.metrics.max_gap_us);
		g_video_state.metrics.last_log_us = now_us;
		g_video_state.metrics.max_gap_us = 0;
	}
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
	while (g_video_state.count > 0) {
		prebuf_packet_t *slot = &g_video_state.slot[g_video_state.head];
		free(slot->data);
		slot->data = NULL;
		slot->len = 0;
		g_video_state.head = (g_video_state.head + 1) % PREBUF_PACKET_CAP;
		g_video_state.count--;
	}
	g_video_state.head = 0;
	g_video_state.tail = 0;
}

static void prebuf_clear(void)
{
	pthread_mutex_lock(&g_video_state.mutex);
	prebuf_clear_locked();
	pthread_mutex_unlock(&g_video_state.mutex);
}

static void prebuf_push_locked(const char *data, int len)
{
	prebuf_packet_t *slot;
	char *copy;

	if (len <= 0 || len > PREBUF_PACKET_MAX)
		return;

	if (packet_has_sps(data, len))
		prebuf_clear_locked();

	if (g_video_state.count >= PREBUF_PACKET_CAP) {
		slot = &g_video_state.slot[g_video_state.head];
		free(slot->data);
		slot->data = NULL;
		slot->len = 0;
		g_video_state.head = (g_video_state.head + 1) % PREBUF_PACKET_CAP;
		g_video_state.count--;
		g_video_state.metrics.prebuf_drop_total++;
	}

	copy = (char *)malloc((size_t)len);
	if (!copy)
		return;
	memcpy(copy, data, (size_t)len);
	slot = &g_video_state.slot[g_video_state.tail];
	slot->data = copy;
	slot->len = len;
	g_video_state.tail = (g_video_state.tail + 1) % PREBUF_PACKET_CAP;
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

static void session_init(void)
{
	static struct SESSION_DATA session_data;
	memset(&session_data, 0, sizeof(session_data));
	session_data.width = 1440;		// （要和后面解码显示的 session 宽高一致）。
	session_data.height = 720;
	session_data.width_margin = 0;
	session_data.height_margin = 0;
	session_data.fps = 30;
	session_data.density = 230;
	session_data.is_right_hand = 0;
	session_data.is_night_mode = 0;
	session_data.apple_wired = APPLE_WIRED_LINK_NONE;
	session_data.apple_wireless = CARPLAY_WIRELESS_MODE;
	session_data.android_wired = ANDROID_WIRED_LINK_NONE;
	session_data.android_wireless = AA_WIRELESS_MODE;
	session_data.cp_icon_path = "/opt/work/app/carplay/icon/icon_104_104.png";
	session_data.is_use_phone_audio = 0;
	session_data.platform_id = "zlink";
	session_data.vendor_name = "zlink-test";
	session_data.is_force_usb_host = 0;
	session_data.mfi_bus_num = -1;
	session_data.otg_bus_num = -1;

	libzlink_init_session_2(&session_data);
}

static int video_data_cb(char *data, int len, struct VIDEO_SCREEN_INFO *info, void *user_data)
{
	uint64_t now_us;
	int active;
	int feed_ret = 0;

	(void)info;
	(void)user_data;
	if (data && len > 0) {
		now_us = monotonic_time_us();
		pthread_mutex_lock(&g_video_state.mutex);
		zlink_video_try_raise_priority_locked();
		active = g_video_state.active;
		if (!active)
			prebuf_push_locked(data, len);
		pthread_mutex_unlock(&g_video_state.mutex);
		if (active)
			feed_ret = carplay_display_feed_h264(data, len);

		pthread_mutex_lock(&g_video_state.mutex);
		zlink_video_metrics_note_locked(active, feed_ret, now_us);
		pthread_mutex_unlock(&g_video_state.mutex);

		pthread_mutex_lock(&g_video_dump.mutex);
		if (g_video_dump.enabled && g_video_dump.fp) {
			size_t n = fwrite(data, 1, (size_t)len, g_video_dump.fp);
			(void)n;
			fflush(g_video_dump.fp);
		}
		pthread_mutex_unlock(&g_video_dump.mutex);
	}
	return 0;
}

static int zlink_client_stop_projection_if_active(int clear_link_type)
{
	int active;
	int link_to_pending;

	pthread_mutex_lock(&g_video_state.mutex);
	active = g_video_state.active;
	link_to_pending = g_sys_Data.linktype;
	if (active) {
		g_video_state.active = 0;
		g_video_state.pending_home_link_type = link_to_pending;
	}
	pthread_mutex_unlock(&g_video_state.mutex);

	if (clear_link_type)
		g_sys_Data.linktype = 0;
	if (active)
		carplay_display_destroy();
	return active;
}

static int session_state_cb(enum LIBZLINK_SESSION_STATE session_state, enum PHONE_TYPE phone_type, void *user_data)
{
	printf("\n\n\nsession_state_cb: session_state = %d, phone_type = %d\n\n\n", session_state, phone_type);
	(void)user_data;
	if (session_state == SESSION_WAIT_INIT) {
		session_init();
	} else if (session_state == SESSION_STARTED) {
		int link_type = link_type_from_phone_type(phone_type);
		int active;
		if (link_type)
			g_sys_Data.linktype = link_type;
		pthread_mutex_lock(&g_video_state.mutex);
		g_video_state.session_started = 1;
		active = g_video_state.active;
		pthread_mutex_unlock(&g_video_state.mutex);
		if (active)
			libzlink_video_focus(0);
		else
			libzlink_video_focus(1);
	} else {
		pthread_mutex_lock(&g_video_state.mutex);
		g_video_state.session_started = 0;
		pthread_mutex_unlock(&g_video_state.mutex);
		zlink_client_stop_projection_if_active(1);
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

	if (is_hu_focus_on) {
		printf("video_focus_request: request back to HU HMI\n");
		zlink_client_stop_projection_if_active(0);
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
	pthread_mutex_lock(&g_video_state.mutex);
	if (active) {
		g_video_state.active = 1;
		while (g_video_state.count > 0) {
			prebuf_packet_t pkt = g_video_state.slot[g_video_state.head];
			g_video_state.slot[g_video_state.head].data = NULL;
			g_video_state.slot[g_video_state.head].len = 0;
			g_video_state.head = (g_video_state.head + 1) % PREBUF_PACKET_CAP;
			g_video_state.count--;
			pthread_mutex_unlock(&g_video_state.mutex);
			carplay_display_feed_h264(pkt.data, pkt.len);
			free(pkt.data);
			pthread_mutex_lock(&g_video_state.mutex);
		}
		g_video_state.head = 0;
		g_video_state.tail = 0;
	} else {
		g_video_state.active = 0;
		prebuf_clear_locked();
	}
	pthread_mutex_unlock(&g_video_state.mutex);
}

int zlink_client_request_video_focus(int is_hu_focus_on)
{
	return libzlink_video_focus(is_hu_focus_on ? 1 : 0);
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

#endif /* ENABLE_CARPLAY */
