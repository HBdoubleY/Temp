#ifndef LVGL_MAIN_H_H____
#define LVGL_MAIN_H_H____


#if defined(__cplusplus)||defined(c_plusplus)
extern "C"{
#endif

extern int LvglService(int w, int h);
extern int the_tick_to_extern(void);
void recorder_request_start_async(void);
void recorder_request_stop_async(void);

#ifdef ENABLE_CARPLAY
void link_ui_on_projection_entered(void);
void link_ui_on_projection_exited(void);
#endif

#if defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif
