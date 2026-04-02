#ifndef LINK_TOUCH_EVDEV_H
#define LINK_TOUCH_EVDEV_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ENABLE_CARPLAY

/**
 * Start dedicated evdev reader thread (second open of touch device).
 * Safe to call once; subsequent calls are no-ops.
 */
void carplay_link_touch_init(void);

/**
 * Must be called from the LVGL thread after lv_disp_drv_register, with the same
 * hor_res, ver_res and rotated as the display driver (e.g. sunxifb_get_sizes +
 * LV_DISP_ROT_*). Applies the same touch transform as LVGL indev_pointer_proc
 * so coordinates match lv_indev_get_point() / feedback_cb behavior.
 */
void carplay_link_touch_configure(int hor_res, int ver_res, int disp_rot);

/**
 * When active is non-zero, touch events are forwarded to carplay_touch_send_xy.
 * When cleared, forwarding stops; if a finger was down, an UP is synthesized.
 * Call only from the LVGL/UI thread at projection enter/exit boundaries.
 */
void carplay_link_touch_set_active(int active);

#endif /* ENABLE_CARPLAY */

#ifdef __cplusplus
}
#endif

#endif /* LINK_TOUCH_EVDEV_H */
