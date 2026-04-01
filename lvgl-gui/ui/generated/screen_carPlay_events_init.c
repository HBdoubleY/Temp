#include "events_init.h"
#include <stdio.h>
#include "lvgl.h"
#if LV_USE_GUIDER_SIMULATOR && LV_USE_FREEMASTER
#include "freemaster_client.h"
#endif
#include "hwdisplay.h"
#include "lv_obj_draw.h"
#ifdef ENABLE_CARPLAY
#include "carplay_display.h"
#include "zlink_client.h"
#endif

void screen_carPlay_btn_return_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
// #ifdef ENABLE_CARPLAY
// 		if (carplay_split_screen_enable) {
// 			stopPreview(&g_sys_Data.vipp0_config);
// 			stopPreview(&g_sys_Data.vipp8_config);
// 		}
// 		zlink_client_set_video_active(0);
// 		zlink_client_request_video_focus(1);
// 		if (g_sys_Data.linktype == LINK_TYPE_CARPLAY)
// 			request_link_action(LINK_TYPE_CARPLAY, LINK_ACTION_VIDEO_CTRL, 0, NULL);
// 		carplay_display_destroy();
// #endif
		ui_load_scr_animation(&guider_ui, &guider_ui.screen, guider_ui.screen_del, &guider_ui.screen_carPlay_del, setup_scr_screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, true, true);
		break;
	default:
		break;
	}
}

void events_init_screen_carPlay(lv_ui *ui){
	lv_obj_add_event_cb(ui->screen_carPlay_btn_return,screen_carPlay_btn_return_event_handler,LV_EVENT_ALL,ui);
}
