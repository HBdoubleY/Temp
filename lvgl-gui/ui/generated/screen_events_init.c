#include "events_init.h"
#include <stdio.h>
#include "lvgl.h"
#include "pthread.h"
#if LV_USE_GUIDER_SIMULATOR && LV_USE_FREEMASTER
#include "freemaster_client.h"
#endif
#include "hwdisplay.h"
#include "lv_obj_draw.h"
#include "Rtc2C.h"
#include "media_main_interface.h"
#include "ota_main_interface.h"
#include "storageDataApi.h"
#include "myTimer.h"
#ifdef ENABLE_CARPLAY
#include "carplay_display.h"
#include "zlink_client.h"
#include "lvgl_main.h"
#endif


extern void listStatuSwitch(language_t language);
extern void languageItemStatu(language_t language);
extern void mianScreenLanguageUpdata(language_t language);

static lv_timer_t *lockBtnTimer = NULL;

static int num = 0;

static void screen_btn_cartrip_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
	lv_area_t coords;
	lv_point_t point;	
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(g_sys_Data.lockScreenFlag){
			show_label_with_timer(guider_ui.screen_label_Popup, "main_txt_Locked", 1000);
			return;
		}	
		if(!g_sys_Data.carTripSwitch)
			return;
		lv_obj_get_coords(guider_ui.screen_btn_cartrip, &coords);
		lv_indev_get_point(lv_indev_get_act(), &point);
		if(point.x < coords.x1 || point.y < coords.y1 || point.x > coords.x2 || point.y > coords.y2){
			return;
		}

		if(g_sys_Data.lockScreenFlag){
			show_label_with_timer(guider_ui.screen_label_Popup, "main_txt_Locked", 1000);
			break;
		}
		ui_load_scr_animation(&guider_ui, &guider_ui.screen_Tire, guider_ui.screen_Tire_del, &guider_ui.screen_del, setup_scr_screen_Tire, LV_SCR_LOAD_ANIM_NONE, 0, 0, true, true);
		break;
	}
    default:
        break;
    }
}

static void screen_btn_DVR_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
	lv_area_t coords;
	lv_point_t point;
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		lv_obj_get_coords(guider_ui.screen_btn_DVR, &coords);
		lv_indev_get_point(lv_indev_get_act(), &point);
		if(point.x < coords.x1 || point.y < coords.y1 || point.x > coords.x2 || point.y > coords.y2){
			return;
		}
		if(g_sys_Data.lockScreenFlag){
			show_label_with_timer(guider_ui.screen_label_Popup, "main_txt_Locked", 1000);
			return;
		}	
		
		if(!g_sys_Data.frontCamera && !g_sys_Data.rearCamera){
			show_label_with_timer(guider_ui.screen_label_Popup, "dvr_txt_detectCamera", 1000);
			break;
		}		

		ui_load_scr_animation(&guider_ui, &guider_ui.screen_DVR, guider_ui.screen_DVR_del, &guider_ui.screen_del, setup_scr_screen_DVR, LV_SCR_LOAD_ANIM_NONE, 0, 0, true, true);
		break;
	}
    default:
        break;
    }
}

static void screen_btn_set_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
	lv_area_t coords;
	lv_point_t point;
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		lv_obj_get_coords(guider_ui.screen_btn_set, &coords);
		lv_indev_get_point(lv_indev_get_act(), &point);
		if(point.x < coords.x1 || point.y < coords.y1 || point.x > coords.x2 || point.y > coords.y2){
			return;
		}
		printf("-----LV_EVENT_RELEASED---\n");
		if(g_sys_Data.lockScreenFlag){
			show_label_with_timer(guider_ui.screen_label_Popup, "main_txt_Locked", 1000);
			break;
		}
		ui_load_scr_animation(&guider_ui, &guider_ui.screen_SET, guider_ui.screen_SET_del, &guider_ui.screen_del, setup_scr_screen_SET, LV_SCR_LOAD_ANIM_NONE, 0, 0, true, true);
		lv_obj_add_state(guider_ui.screen_SET_list_set_item0,LV_STATE_PRESSED);
		lv_obj_add_state(guider_ui.screen_SET_list_set_item1,LV_STATE_DEFAULT);
		lv_obj_add_state(guider_ui.screen_SET_list_set_item2,LV_STATE_DEFAULT);
		lv_obj_clear_flag(guider_ui.screen_SET_cont_tongyong,LV_OBJ_FLAG_HIDDEN);
		break;
	}
    default:
        break;
    }
}


static void screen_btn_carplay_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(g_sys_Data.lockScreenFlag){
			show_label_with_timer(guider_ui.screen_label_Popup, "main_txt_Locked", 1000);
			return;
		}
#ifdef ENABLE_CARPLAY
		int projection_ok = 0;
		/* 仅当 session 已启动且当前为 CarPlay 连接时才进入投屏界面，否则只进提示界面 */
		if (zlink_client_is_session_started() && g_sys_Data.linktype == LINK_TYPE_CARPLAY) {
			zlink_client_reset_video_prebuffer();
			zlink_client_request_video_focus(1);
			request_link_action(LINK_TYPE_CARPLAY, LINK_ACTION_VIDEO_CTRL, 0, NULL);
			int disp_w = 720;
			int disp_h = 1440;
			projection_ok = (carplay_display_create(0, 0, disp_w, disp_h, 1440, 720) == 0);
			zlink_client_set_video_active(1);
			zlink_client_request_video_focus(0);
			request_link_action(LINK_TYPE_CARPLAY, LINK_ACTION_VIDEO_CTRL, 1, NULL);
			// zlink_client_set_video_dump(1);
		}
#endif
		ui_load_scr_animation(&guider_ui, &guider_ui.screen_carPlay, guider_ui.screen_carPlay_del, &guider_ui.screen_del, setup_scr_screen_carPlay, LV_SCR_LOAD_ANIM_NONE, 0, 0, true, true);
#ifdef ENABLE_CARPLAY
		if (projection_ok)
			link_ui_on_projection_entered();
#endif
		break;
	}
    default:
        break;
    }
}

static void screen_btn_androidauto_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);	
	switch (code)
	{
	case LV_EVENT_CLICKED:
	{
		if(g_sys_Data.lockScreenFlag){
			show_label_with_timer(guider_ui.screen_label_Popup, "main_txt_Locked", 1000);
			return;
		}
#ifdef ENABLE_CARPLAY
		int projection_ok_aa = 0;
		/* 仅当 session 已启动且当前为 Android Auto 连接时才进入投屏界面，否则只进提示界面 */
		if (zlink_client_is_session_started() && g_sys_Data.linktype == LINK_TYPE_ANDROIDAUTO) {
			zlink_client_reset_video_prebuffer();
			zlink_client_request_video_focus(1);
			request_link_action(LINK_TYPE_ANDROIDAUTO, LINK_ACTION_VIDEO_CTRL, 0, NULL);
			int disp_w = 720;
			int disp_h = 1440;
			projection_ok_aa = (carplay_display_create(0, 0, disp_w, disp_h, 1440, 720) == 0);
			zlink_client_set_video_active(1);
			zlink_client_request_video_focus(0);
			request_link_action(LINK_TYPE_ANDROIDAUTO, LINK_ACTION_VIDEO_CTRL, 1, NULL);
			// zlink_client_set_video_dump(1);
		}
#endif
		ui_load_scr_animation(&guider_ui, &guider_ui.screen_androidAuto, guider_ui.screen_androidAuto_del, &guider_ui.screen_del, setup_scr_screen_androidAuto, LV_SCR_LOAD_ANIM_NONE, 0, 0, true, true);
#ifdef ENABLE_CARPLAY
		if (projection_ok_aa)
			link_ui_on_projection_entered();
#endif
		break;
	}
	default:
		break;
	}
}

static int language = LANG_ENGLISH;
static void screen_list_language_itemEN_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(language == LANG_ENGLISH)
			break;
		language = LANG_ENGLISH;
		listStatuSwitch(language);
		mianScreenLanguageUpdata(language);
		break;
	}
    default:
        break;
    }
}

static void screen_list_language_itemCN_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(language == LANG_CHINESE)
			break;
		language = LANG_CHINESE;
		listStatuSwitch(language);
		mianScreenLanguageUpdata(language);
		break;
	}
    default:
        break;
    }
}

static void screen_list_language_itemCN_FT_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(language == LANG_CHINESE_FT)
			break;
		language = LANG_CHINESE_FT;
		listStatuSwitch(language);
		mianScreenLanguageUpdata(language);
		break;
	}
    default:
        break;
    }
}

static void screen_list_language_itemFR_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(language == LANG_FRANCE)
			break;
		language = LANG_FRANCE;
		listStatuSwitch(language);
		mianScreenLanguageUpdata(language);
		break;
	}
    default:
        break;
    }
}

static void screen_list_language_itemGE_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(language == LANG_GERMANY)
			break;
		language = LANG_GERMANY;
		listStatuSwitch(language);
		mianScreenLanguageUpdata(language);
		break;
	}
    default:
        break;
    }
}

static void screen_list_language_itemIT_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(language == LANG_ITALY)
			break;
		language = LANG_ITALY;
		listStatuSwitch(language);
		mianScreenLanguageUpdata(language);
		break;
	}
    default:
        break;
    }
}

static void screen_list_language_itemJA_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(language == LANG_JAPAN)
			break;
		language = LANG_JAPAN;
		listStatuSwitch(language);
		mianScreenLanguageUpdata(language);
		break;
	}
    default:
        break;
    }
}

static void screen_list_language_itemPOR_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(language == LANG_PORTUGUESE)
			break;
		language = LANG_PORTUGUESE;
		listStatuSwitch(language);
		mianScreenLanguageUpdata(language);
		break;
	}
    default:
        break;
    }
}

static void screen_list_language_itemRU_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(language == LANG_RUSSIAN)
			break;
		language = LANG_RUSSIAN;
		listStatuSwitch(language);
		mianScreenLanguageUpdata(language);
		break;
	}
    default:
        break;
    }
}

static void screen_list_language_itemSP_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(language == LANG_SPANISH)
			break;
		language = LANG_SPANISH;
		listStatuSwitch(language);
		mianScreenLanguageUpdata(language);
		break;
	}
    default:
        break;
    }
}

static void screen_list_language_itemPOL_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(language == LANG_POLAND)
			break;
		language = LANG_POLAND;
		listStatuSwitch(language);
		mianScreenLanguageUpdata(language);
		break;
	}
    default:
        break;
    }
}

static void screen_list_language_itemTU_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(language == LANG_TURKISH)
			break;
		language = LANG_TURKISH;
		listStatuSwitch(language);
		mianScreenLanguageUpdata(language);
		break;
	}
    default:
        break;
    }
}

static void screen_list_language_itemKO_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(language == LANG_KOREAN)
			break;
		language = LANG_KOREAN;
		listStatuSwitch(language);
		mianScreenLanguageUpdata(language);
		break;
	}
    default:
        break;
    }
}

static void screen_list_language_itemDU_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(language == LANG_DUTCH)
			break;
		language = LANG_DUTCH;
		listStatuSwitch(language);
		mianScreenLanguageUpdata(language);
		break;
	}
    default:
        break;
    }
}

static void screen_btn_confirm_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		g_sys_Data.current_language = language;
		if(!storageData_setInt("current_language", g_sys_Data.current_language)){
			printf("storage data current_language[%d] fail!\n", g_sys_Data.current_language);
		}
		g_sys_Data.resetFactory = false;
		if(!storageData_setBool("resetFactory", g_sys_Data.resetFactory)){
			printf("storage data resetFactory[%d] fail!\n", g_sys_Data.resetFactory);
		}
		lv_obj_add_flag(guider_ui.screen_cont_languageChoose, LV_OBJ_FLAG_HIDDEN);
		break;
	}
    default:
        break;
    }
}

static void screen_btn_lockScreen_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		lv_obj_clear_flag(guider_ui.screen_label_Popup, LV_OBJ_FLAG_HIDDEN);
		if(g_sys_Data.lockScreenFlag){
			g_sys_Data.lockScreenFlag = false;
			lv_obj_set_style_bg_img_src(guider_ui.screen_btn_lockScreen, &_unlock_select_80x80, LV_PART_MAIN|LV_STATE_DEFAULT);
			show_label_with_timer(guider_ui.screen_label_Popup, "main_txt_Unlocked", 1000);
		}else{
			g_sys_Data.lockScreenFlag = true;
			lv_obj_set_style_bg_img_src(guider_ui.screen_btn_lockScreen, &_lock_select_80x80, LV_PART_MAIN|LV_STATE_DEFAULT);
			show_label_with_timer(guider_ui.screen_label_Popup, "main_txt_Locked", 1000);
		}
		break;
	}
    default:
        break;
    }
}

static void btn_anim(lv_obj_t *btn, int endPoint){
	lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, btn);
    lv_anim_set_exec_cb(&a, lv_obj_set_y);
    lv_anim_set_values(&a, lv_obj_get_y(btn), endPoint);      // 从y=10到y=200
    lv_anim_set_time(&a, 1000);           // 1秒
    lv_anim_set_delay(&a, 100);           // 延迟500ms
    lv_anim_set_playback_time(&a, 0);   // 回放时间
    lv_anim_set_repeat_count(&a, 0);  // 无限循环	
	lv_anim_start(&a);
}

static void lockScreen_Btn(lv_timer_t *timer){
	printf("%s:%d\n",__func__,__LINE__);
	if(lv_obj_is_valid(guider_ui.screen_btn_lockScreen)){
		btn_anim(guider_ui.screen_btn_lockScreen, -80);
	}
}
static void screen_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_indev_t *indev = lv_indev_get_act();
	lv_coord_t coord;
    lv_point_t point;
    switch (code) {
	case LV_EVENT_PRESSED:
	{
		lv_indev_get_point(lv_indev_get_act(), &point);
		if(point.y < 80){
			coord = lv_obj_get_y(guider_ui.screen_btn_lockScreen);
			if(coord == -80){
				btn_anim(guider_ui.screen_btn_lockScreen, 0);
				lockBtnTimer = lv_timer_create(lockScreen_Btn, 3000, NULL);
				lv_timer_set_repeat_count(lockBtnTimer, 1);
			}else{
				btn_anim(guider_ui.screen_btn_lockScreen, -80);
				if(lockBtnTimer != NULL) lv_timer_del(lockBtnTimer);
				lockBtnTimer = NULL;
			}
		}
		break;
	}
	case LV_EVENT_RELEASED:
		

		break;
    default:
        break;
    }
}





void events_init_screen (lv_ui *ui)
{

	lv_obj_add_event_cb(ui->screen, screen_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_btn_cartrip, screen_btn_cartrip_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_btn_DVR, screen_btn_DVR_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_btn_set, screen_btn_set_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_btn_carplay, screen_btn_carplay_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_btn_androidauto, screen_btn_androidauto_event_handler,LV_EVENT_ALL,ui);
	lv_obj_add_event_cb(ui->screen_list_language_itemEN, screen_list_language_itemEN_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_list_language_itemCN, screen_list_language_itemCN_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_list_language_itemCN_FT, screen_list_language_itemCN_FT_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_list_language_itemFR, screen_list_language_itemFR_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_list_language_itemGE, screen_list_language_itemGE_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_list_language_itemIT, screen_list_language_itemIT_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_list_language_itemJA, screen_list_language_itemJA_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_list_language_itemPOR, screen_list_language_itemPOR_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_list_language_itemRU, screen_list_language_itemRU_event_handler, LV_EVENT_ALL, ui);	
	lv_obj_add_event_cb(ui->screen_list_language_itemSP, screen_list_language_itemSP_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_list_language_itemPOL, screen_list_language_itemPOL_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_list_language_itemTU, screen_list_language_itemTU_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_list_language_itemKO, screen_list_language_itemKO_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_list_language_itemDU, screen_list_language_itemDU_event_handler, LV_EVENT_ALL, ui);	
	lv_obj_add_event_cb(ui->screen_btn_confirm, screen_btn_confirm_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_btn_lockScreen, screen_btn_lockScreen_event_handler, LV_EVENT_ALL, ui);
}