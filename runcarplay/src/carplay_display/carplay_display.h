#ifndef CARPLAY_DISPLAY_H
#define CARPLAY_DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ENABLE_CARPLAY

typedef struct {
	int link_type;
	int disp_x;
	int disp_y;
	int disp_width;
	int disp_height;
	int session_width;
	int session_height;
	int decode_width;
	int decode_height;
	int rotation;
	int scaling_mode;
	int frame_region_x;
	int frame_region_y;
	int frame_region_width;
	int frame_region_height;
} carplay_display_policy_t;

int carplay_display_build_policy(int link_type, int disp_x, int disp_y,
                                 int disp_width, int disp_height,
                                 carplay_display_policy_t *policy);

int carplay_display_create(const carplay_display_policy_t *policy);

int carplay_display_feed_h264(const char *data, int len);

void carplay_display_set_rect(int x, int y, int width, int height);

void carplay_display_destroy(void);

extern int carplay_split_screen_enable;

extern int carplay_split_line_x;
extern int carplay_touch_screen_x;
extern int carplay_touch_screen_y;
extern int carplay_touch_screen_down;

void carplay_touch_send(void);

void carplay_touch_send_xy(int screen_x, int screen_y, int is_touch_down);

#endif /* ENABLE_CARPLAY */

#ifdef __cplusplus
}
#endif

#endif /* CARPLAY_DISPLAY_H */
