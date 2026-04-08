#ifdef ENABLE_CARPLAY

#include "link_touch_evdev.h"
#include "carplay_display.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <linux/input.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <unistd.h>

extern unsigned int zlink_client_perf_get_session_id(void);
extern int zlink_client_perf_is_enabled(void);
extern int zlink_client_perf_sample_n(void);
extern int zlink_client_perf_warn_us(void);

/* Match lv_drv_conf / evdev.c defaults (no lv_drv_conf include in runcarplay). */
#include "carplay_thread_prio.h"

#ifndef LINK_TOUCH_EVDEV_FALLBACK
#define LINK_TOUCH_EVDEV_FALLBACK "/dev/input/touchscreen"
#endif
#ifndef LINK_TOUCH_EVDEV_DEFAULT
#define LINK_TOUCH_EVDEV_DEFAULT "/dev/input/event0"
#endif

#define LINK_TOUCH_SWAP_AXES 0

#define LINK_TOUCH_LOGICAL_W 1440
#define LINK_TOUCH_LOGICAL_H 720

/* Default until carplay_link_touch_configure(); must match lv_disp_drv hor/ver + rotated */
static int g_disp_hor = 1440;
static int g_disp_ver = 720;
static int g_disp_rot; /* 0=LV_DISP_ROT_NONE, 1=90, 2=180, 3=270 — same as lv_disp_rot_t */

#define TOUCH_MOVE_THRESHOLD_DEFAULT 3
#define EVDEV_NOISE_FILTER_TIME_MS_DEFAULT 25

#define ST_REL 0
#define ST_PR  1

static pthread_mutex_t g_touch_mutex = PTHREAD_MUTEX_INITIALIZER;
static int g_touch_active;
static int g_tracking;
static int g_seq_x;
static int g_seq_y;

static int g_evdev_fd = -1;
static int g_evdev_root_x;
static int g_evdev_root_y;
static int g_evdev_button = ST_REL;
static long long g_last_event_time_ms;
static int g_last_valid_x;
static int g_last_valid_y;
static int g_touch_move_threshold = TOUCH_MOVE_THRESHOLD_DEFAULT;
static int g_touch_move_threshold_sq = TOUCH_MOVE_THRESHOLD_DEFAULT * TOUCH_MOVE_THRESHOLD_DEFAULT;
static int g_noise_filter_time_ms = EVDEV_NOISE_FILTER_TIME_MS_DEFAULT;

static pthread_t g_thread;
static int g_thread_started;
static unsigned long long g_touch_seq;
static unsigned long long g_noise_filter_release_cnt;
static unsigned long long g_move_suppress_cnt;
static long long g_batch_first_event_us;
static long long g_last_poll_wakeup_us;

static long long link_touch_now_us(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (long long)tv.tv_sec * 1000000LL + (long long)tv.tv_usec;
}

static long long link_touch_tid(void)
{
	return (long long)syscall(SYS_gettid);
}

#define CPT_LOG(stage, fmt, ...) \
	do { \
		if (zlink_client_perf_is_enabled()) { \
			printf("[cp_perf] ts_us=%lld tid=%lld stage=%s sid=%u " fmt "\n", \
			       link_touch_now_us(), link_touch_tid(), stage, zlink_client_perf_get_session_id(), ##__VA_ARGS__); \
		} \
	} while (0)

static long long link_touch_now_ms(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (long long)tv.tv_sec * 1000LL + tv.tv_usec / 1000LL;
}

static void link_touch_load_tuning(void)
{
	const char *mv = getenv("LINK_TOUCH_MOVE_THRESHOLD");
	const char *nf = getenv("LINK_TOUCH_NOISE_FILTER_MS");

	if (mv && mv[0] != '\0') {
		int v = atoi(mv);
		if (v >= 1 && v <= 20) {
			g_touch_move_threshold = v;
			g_touch_move_threshold_sq = v * v;
		}
	}
	if (nf && nf[0] != '\0') {
		int v = atoi(nf);
		if (v >= 0 && v <= 200) {
			g_noise_filter_time_ms = v;
		}
	}
	printf("link_touch tuning: move_threshold=%dpx noise_filter=%dms\n",
	       g_touch_move_threshold, g_noise_filter_time_ms);
}

static int open_link_evdev_fd(void)
{
	int fd;

	if (access(LINK_TOUCH_EVDEV_FALLBACK, F_OK) == 0)
		fd = open(LINK_TOUCH_EVDEV_FALLBACK, O_RDWR | O_NOCTTY);
	else
		fd = open(LINK_TOUCH_EVDEV_DEFAULT, O_RDWR | O_NOCTTY);
	if (fd < 0) {
		perror("link_touch: open evdev");
		return -1;
	}
	{
		int fl = fcntl(fd, F_GETFL, 0);
		if (fl >= 0)
			fcntl(fd, F_SETFL, fl | O_NONBLOCK);
	}
	return fd;
}

/* Same bounds as evdev_read() before LVGL rotation: drv->disp->driver->hor/ver_res */
static void clamp_physical(int *x, int *y)
{
	if (g_disp_hor <= 0 || g_disp_ver <= 0)
		return;
	if (*x < 0)
		*x = 0;
	if (*y < 0)
		*y = 0;
	if (*x >= g_disp_hor)
		*x = g_disp_hor - 1;
	if (*y >= g_disp_ver)
		*y = g_disp_ver - 1;
}

/* lvgl/src/core/lv_indev.c indev_pointer_proc — must stay in sync */
static void apply_lvgl_pointer_rotation(int *x, int *y)
{
	int rot = g_disp_rot;

	if (rot == 2 || rot == 3) {
		*x = g_disp_hor - *x - 1;
		*y = g_disp_ver - *y - 1;
	}
	if (rot == 1 || rot == 3) {
		int tmp = *y;
		*y = *x;
		*x = g_disp_ver - tmp - 1;
	}
}

static void clamp_logical_xy(int *x, int *y)
{
	if (*x < 0)
		*x = 0;
	if (*y < 0)
		*y = 0;
	if (*x >= LINK_TOUCH_LOGICAL_W)
		*x = LINK_TOUCH_LOGICAL_W - 1;
	if (*y >= LINK_TOUCH_LOGICAL_H)
		*y = LINK_TOUCH_LOGICAL_H - 1;
}

static void emit_link_touch_locked(int x, int y, int cur_pr)
{
	if (!g_touch_active)
		return;

	if (cur_pr) {
		if (!g_tracking) {
			clamp_logical_xy(&x, &y);
			carplay_touch_send_xy(x, y, 1);
			g_seq_x = x;
			g_seq_y = y;
			g_tracking = 1;
		} else {
			int dx = x - g_seq_x;
			int dy = y - g_seq_y;
			int dist_sq = dx * dx + dy * dy;

			if (dist_sq > g_touch_move_threshold_sq) {
				clamp_logical_xy(&x, &y);
				carplay_touch_send_xy(x, y, 1);
				g_seq_x = x;
				g_seq_y = y;
			} else {
				g_move_suppress_cnt++;
			}
		}
	} else {
		if (g_tracking) {
			carplay_touch_send_xy(g_seq_x, g_seq_y, 0);
			g_tracking = 0;
		}
	}
}

static void process_input_event(const struct input_event *in)
{
	if (g_batch_first_event_us == 0)
		g_batch_first_event_us = link_touch_now_us();
	g_last_event_time_ms = link_touch_now_ms();

	if (in->type == EV_REL) {
		if (in->code == REL_X) {
#if LINK_TOUCH_SWAP_AXES
			g_evdev_root_y += in->value;
#else
			g_evdev_root_x += in->value;
#endif
		} else if (in->code == REL_Y) {
#if LINK_TOUCH_SWAP_AXES
			g_evdev_root_x += in->value;
#else
			g_evdev_root_y += in->value;
#endif
		}
		if (in->value != 0)
			g_evdev_button = ST_PR;
	} else if (in->type == EV_ABS) {
		if (in->code == ABS_X) {
#if LINK_TOUCH_SWAP_AXES
			g_evdev_root_y = in->value;
#else
			g_evdev_root_x = in->value;
#endif
		} else if (in->code == ABS_Y) {
#if LINK_TOUCH_SWAP_AXES
			g_evdev_root_x = in->value;
#else
			g_evdev_root_y = in->value;
#endif
		} else if (in->code == ABS_MT_POSITION_X) {
#if LINK_TOUCH_SWAP_AXES
			g_evdev_root_y = in->value;
#else
			g_evdev_root_x = in->value;
#endif
		} else if (in->code == ABS_MT_POSITION_Y) {
#if LINK_TOUCH_SWAP_AXES
			g_evdev_root_x = in->value;
#else
			g_evdev_root_y = in->value;
#endif
		} else if (in->code == ABS_MT_TRACKING_ID) {
			if (in->value == -1)
				g_evdev_button = ST_REL;
			else if (in->value >= 0)
				g_evdev_button = ST_PR;
		} else if (in->code == ABS_MT_TOUCH_MAJOR) {
			if (in->value == 0)
				g_evdev_button = ST_REL;
			else
				g_evdev_button = ST_PR;
		} else if (in->code == ABS_PRESSURE) {
			if (in->value == 0)
				g_evdev_button = ST_REL;
			else if (in->value > 0)
				g_evdev_button = ST_PR;
		}
	} else if (in->type == EV_KEY) {
		if (in->code == BTN_MOUSE || in->code == BTN_TOUCH || in->code == BTN_TOOL_FINGER) {
			if (in->value == 0)
				g_evdev_button = ST_REL;
			else if (in->value == 1)
				g_evdev_button = ST_PR;
		}
	}

	if (g_evdev_button == ST_PR || g_evdev_button == ST_REL) {
		if (g_evdev_button == ST_PR) {
			g_last_valid_x = g_evdev_root_x;
			g_last_valid_y = g_evdev_root_y;
		}
	}
}

static void run_emit_after_batch(void)
{
	long long now = link_touch_now_ms();
	int current_state = g_evdev_button;
	int out_x, out_y;

	if (g_evdev_button == ST_PR && (now - g_last_event_time_ms) > g_noise_filter_time_ms) {
		current_state = ST_REL;
		g_evdev_button = ST_REL;
		g_noise_filter_release_cnt++;
	}

	if (current_state == ST_PR) {
		out_x = g_evdev_root_x;
		out_y = g_evdev_root_y;
	} else {
		out_x = g_last_valid_x;
		out_y = g_last_valid_y;
	}

	clamp_physical(&out_x, &out_y);
	apply_lvgl_pointer_rotation(&out_x, &out_y);
	clamp_logical_xy(&out_x, &out_y);

	pthread_mutex_lock(&g_touch_mutex);
	g_touch_seq++;
	emit_link_touch_locked(out_x, out_y, current_state);
	pthread_mutex_unlock(&g_touch_mutex);
	if ((g_touch_seq % (unsigned long long)zlink_client_perf_sample_n()) == 0ULL && g_batch_first_event_us > 0) {
		long long now_us = link_touch_now_us();
		long long pipeline_us = now_us - g_batch_first_event_us;
		CPT_LOG("touch_emit", "seq=%llu state=%d x=%d y=%d pipeline_us=%lld noise_rel=%llu move_suppress=%llu",
		        g_touch_seq, current_state, out_x, out_y, pipeline_us,
		        g_noise_filter_release_cnt, g_move_suppress_cnt);
	}
	g_batch_first_event_us = 0;
}

static void *link_touch_thread_fn(void *arg)
{
	(void)arg;
	carplay_set_self_sched_fifo_max("carplay_touch");
	struct pollfd pfd;

	if (g_evdev_fd < 0)
		return NULL;

	pfd.fd = g_evdev_fd;
	pfd.events = POLLIN;

	for (;;) {
		long long poll_start_us = link_touch_now_us();
		int pr = poll(&pfd, 1, -1);
		long long poll_wakeup_us = link_touch_now_us();
		if (g_last_poll_wakeup_us > 0 && zlink_client_perf_is_enabled()) {
			long long interval = poll_wakeup_us - g_last_poll_wakeup_us;
			if (interval > (long long)zlink_client_perf_warn_us()) {
				CPT_LOG("touch_poll_gap_warn", "interval_us=%lld", interval);
			}
		}
		g_last_poll_wakeup_us = poll_wakeup_us;

		if (pr < 0) {
			if (errno == EINTR)
				continue;
			perror("link_touch: poll");
			break;
		}
		if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL))
			break;

		if (!(pfd.revents & POLLIN))
			continue;

		int batch_events = 0;
		for (;;) {
			struct input_event in;
			ssize_t n = read(g_evdev_fd, &in, sizeof(in));

			if (n == (ssize_t)sizeof(in)) {
				process_input_event(&in);
				batch_events++;
				continue;
			}
			if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
				break;
			if (n < 0)
				perror("link_touch: read");
			break;
		}
		if (zlink_client_perf_is_enabled() && batch_events > 0) {
			long long loop_cost = link_touch_now_us() - poll_start_us;
			CPT_LOG("touch_batch", "events=%d read_loop_cost_us=%lld", batch_events, loop_cost);
		}
		run_emit_after_batch();
	}

	if (g_evdev_fd >= 0) {
		close(g_evdev_fd);
		g_evdev_fd = -1;
	}
	return NULL;
}

void carplay_link_touch_set_active(int active)
{
	pthread_mutex_lock(&g_touch_mutex);
	g_touch_active = active ? 1 : 0;
	if (!g_touch_active && g_tracking) {
		carplay_touch_send_xy(g_seq_x, g_seq_y, 0);
		g_tracking = 0;
	}
	pthread_mutex_unlock(&g_touch_mutex);
}

void carplay_link_touch_configure(int hor_res, int ver_res, int disp_rot)
{
	g_disp_hor = hor_res;
	g_disp_ver = ver_res;
	g_disp_rot = disp_rot;
}

void carplay_link_touch_init(void)
{
	if (g_thread_started)
		return;

	g_evdev_fd = open_link_evdev_fd();
	if (g_evdev_fd < 0)
		return;

	g_evdev_root_x = 0;
	g_evdev_root_y = 0;
	g_evdev_button = ST_REL;
	link_touch_load_tuning();
	g_last_event_time_ms = link_touch_now_ms();
	g_last_valid_x = 0;
	g_last_valid_y = 0;

	if (pthread_create(&g_thread, NULL, link_touch_thread_fn, NULL) != 0) {
		perror("link_touch: pthread_create");
		close(g_evdev_fd);
		g_evdev_fd = -1;
		return;
	}
	pthread_detach(g_thread);
	g_thread_started = 1;
}

#endif /* ENABLE_CARPLAY */
