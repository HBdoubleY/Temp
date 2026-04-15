#ifndef LVGL_MAIN_H_H____
#define LVGL_MAIN_H_H____


#if defined(__cplusplus)||defined(c_plusplus)
extern "C"{
#endif

#include <stdbool.h>

extern int LvglService(int w, int h);
extern int the_tick_to_extern(void);
void recorder_request_start_async(void);
void recorder_request_stop_async(void);
void tire_ui_refresh_now(void);
void tire_alarm_init_if_needed(void);
void tire_alarm_refresh_threshold_ui(void);
void tire_alarm_adjust_pressure_min(int direction);
void tire_alarm_adjust_pressure_max(int direction);
void tire_alarm_adjust_temp_max(int direction);
void tire_alarm_reset_defaults(void);
void tire_alarm_set_pressure_unit(bool use_psi);
void tire_alarm_set_temp_unit(bool use_fahrenheit);

#ifdef ENABLE_CARPLAY
void link_ui_on_projection_entered(void);
void link_ui_on_projection_exited(void);
#endif

#if defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif
