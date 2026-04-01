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
#include <sys/reboot.h>
#include "bootlogoUp.h"

typedef enum {
	CONT_TONGYONG = 0x0,
	CONT_LANYA,
	CONT_SYS,
	CONT_LANGUAGE,
	CONT_SETTIME,
	CONT_ABOUT,
	CONT_BOOTLOGO,
	CONT_REBOOT,
	CONT_RESETFACTORY,
	CONT_AGINGMODE,
} SYS_PAGE_MANAGE;

static SYS_PAGE_MANAGE sysPageManage = CONT_TONGYONG; 



static void sysPageManageFun(SYS_PAGE_MANAGE page) {
	switch(page) {
		case CONT_TONGYONG:
			lv_obj_add_flag(guider_ui.screen_SET_cont_tongyong, LV_OBJ_FLAG_HIDDEN);
		break;
		case CONT_LANYA:
			lv_obj_add_flag(guider_ui.screen_SET_cont_lanya, LV_OBJ_FLAG_HIDDEN);
		break;
		case CONT_SYS:
			lv_obj_add_flag(guider_ui.screen_SET_cont_sys, LV_OBJ_FLAG_HIDDEN);
		break;
		case CONT_LANGUAGE:
			lv_obj_add_flag(guider_ui.screen_SET_cont_setlanguage, LV_OBJ_FLAG_HIDDEN);
		break;
		case CONT_SETTIME:
			lv_obj_add_flag(guider_ui.screen_SET_cont_settime, LV_OBJ_FLAG_HIDDEN);
		break;
		case CONT_ABOUT:
			lv_obj_add_flag(guider_ui.screen_SET_cont_guanyu, LV_OBJ_FLAG_HIDDEN);
		break;
		case CONT_BOOTLOGO:
			lv_obj_add_flag(guider_ui.screen_SET_cont_bootlogo, LV_OBJ_FLAG_HIDDEN);
		break;
		case CONT_REBOOT:
			lv_obj_add_flag(guider_ui.screen_SET_cont_sys, LV_OBJ_FLAG_HIDDEN);
			lv_obj_add_flag(guider_ui.screen_SET_cont_reboot, LV_OBJ_FLAG_HIDDEN);
		break;
		case CONT_RESETFACTORY:
			lv_obj_add_flag(guider_ui.screen_SET_cont_sys, LV_OBJ_FLAG_HIDDEN);
			lv_obj_add_flag(guider_ui.screen_SET_cont_resetFactory, LV_OBJ_FLAG_HIDDEN);
		
		break;
		case CONT_AGINGMODE:
			lv_obj_add_flag(guider_ui.screen_SET_cont_agingMode, LV_OBJ_FLAG_HIDDEN);
		break;
	}
}

static void screen_SET_btn_return_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		stop_label_timer(guider_ui.screen_label_Popup);
		sysPageManageFun(sysPageManage);
		switch (sysPageManage)
		{
		case CONT_TONGYONG:
		case CONT_LANYA:
		case CONT_SYS:
			ui_load_scr_animation(&guider_ui, &guider_ui.screen, guider_ui.screen_del, &guider_ui.screen_SET_del, setup_scr_screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, true, true);
			sysPageManage = CONT_TONGYONG;
			/* code */
			break;	
		case CONT_LANGUAGE:
		case CONT_SETTIME:
			lv_obj_clear_flag(guider_ui.screen_SET_cont_tongyong, LV_OBJ_FLAG_HIDDEN);
			sysPageManage = CONT_TONGYONG;
		break;
		case CONT_ABOUT:
		case CONT_BOOTLOGO:
		case CONT_REBOOT:
		case CONT_RESETFACTORY:
			lv_obj_clear_flag(guider_ui.screen_SET_cont_sys, LV_OBJ_FLAG_HIDDEN);
			sysPageManage = CONT_SYS;
		break;	
		case CONT_AGINGMODE:
			lv_obj_clear_flag(guider_ui.screen_SET_cont_guanyu, LV_OBJ_FLAG_HIDDEN);
			sysPageManage = CONT_ABOUT;		
		break;
		default:	
			break;
		}
		break;
	}
    default:
        break;
    }
}

static void screen_SET_list_set_item0_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		//by jy:genaral click event
		sysPageManageFun(sysPageManage);
		sysPageManage = CONT_TONGYONG;
		lv_obj_add_state(guider_ui.screen_SET_list_set_item0,LV_STATE_PRESSED);
		lv_obj_clear_state(guider_ui.screen_SET_list_set_item1,LV_STATE_PRESSED);
		lv_obj_clear_state(guider_ui.screen_SET_list_set_item2,LV_STATE_PRESSED);
		lv_obj_clear_flag(guider_ui.screen_SET_cont_tongyong, LV_OBJ_FLAG_HIDDEN);
		break;
	}
    default:
        break;
    }
}

static void screen_SET_list_set_item1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		//by jy:bt click enevnt
		sysPageManageFun(sysPageManage);
		sysPageManage = CONT_LANYA;
		lv_obj_clear_state(guider_ui.screen_SET_list_set_item0,LV_STATE_PRESSED);
		lv_obj_add_state(guider_ui.screen_SET_list_set_item1,LV_STATE_PRESSED);
		lv_obj_clear_state(guider_ui.screen_SET_list_set_item2,LV_STATE_PRESSED);
		lv_obj_clear_flag(guider_ui.screen_SET_cont_lanya, LV_OBJ_FLAG_HIDDEN);
		break;
	}
    default:
        break;
    }
}

static void screen_SET_list_set_item2_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		//by jy:sys click event
		sysPageManageFun(sysPageManage);
		sysPageManage = CONT_SYS;
		lv_obj_clear_state(guider_ui.screen_SET_list_set_item0,LV_STATE_PRESSED);
		lv_obj_clear_state(guider_ui.screen_SET_list_set_item1,LV_STATE_PRESSED);
		lv_obj_add_state(guider_ui.screen_SET_list_set_item2,LV_STATE_PRESSED);
		lv_obj_clear_flag(guider_ui.screen_SET_cont_sys, LV_OBJ_FLAG_HIDDEN);
		break;
	}
    default:
        break;
    }
}
static const char* themeMode[] = {"sys_txt_day", "sys_txt_night", "sys_txt_auto"};
lv_timer_t *autoModeTimer = NULL;
static THEME_MODE theme_Mode = THEME_NONO;
void creatAutoModeTimerCbk(lv_timer_t *timer){
	lv_obj_t* cur_screen = lv_scr_act();
	THEME_MODE mode = THEME_NONO;
	if(g_sys_Data.gpadcVol > 1600){
		mode = THEME_DARK;

	}else if(g_sys_Data.gpadcVol < 1300){
		mode = THEME_DAY;
	}
	if(theme_Mode != mode){
		theme_Mode = mode;
		if(theme_Mode == THEME_DAY){
			g_sys_Data.backlight = 80;
		}else if(theme_Mode == THEME_DARK){
			g_sys_Data.backlight = 20;
		}
		SetBackLight(g_sys_Data.backlight);
		if(cur_screen == guider_ui.screen_SET && lv_obj_is_valid(guider_ui.screen_SET_slider_backlight)){
			char str[5] = {0};
			sprintf(str,"%d",g_sys_Data.backlight);
			lv_label_set_text(guider_ui.screen_SET_label_beiguang1, str);
			lv_slider_set_value(guider_ui.screen_SET_slider_backlight, g_sys_Data.backlight, 1);
		}
		
		if(!storageData_setInt("backlight", g_sys_Data.backlight)){
			printf("storage data backlight[%d] fail!\n", g_sys_Data.backlight);
		}
		if(cur_screen == guider_ui.screen_SET && lv_obj_is_valid(guider_ui.screen_SET_btn_about)){
			ScreenSetBlackNightMode(&guider_ui, theme_Mode);
			languageItemStatu(g_sys_Data.current_language);
		}else if(cur_screen == guider_ui.screen_Tire && lv_obj_is_valid(guider_ui.screen_Tire_btn_bPair)){
			ScreenTireBlackNightMode(&guider_ui, theme_Mode);
		}else if(cur_screen == guider_ui.screen_DVR_SET && lv_obj_is_valid(guider_ui.screen_DVR_SET_btn_carSpeedMark)){
			screenDvrSetBlackNightMode(&guider_ui, theme_Mode);
		}		
	}
}

void screen_SET_btn_zhuti_left_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		g_sys_Data.themeMode = g_sys_Data.themeMode==1?THEME_DAY:(g_sys_Data.themeMode > 1 ? THEME_DARK : THEME_AUTO);
		if(!storageData_setInt("themeMode", g_sys_Data.themeMode)){
			printf("storage data themeMode[%d] fail!\n", g_sys_Data.themeMode);
		}
		if(g_sys_Data.themeMode == THEME_DAY){
			g_sys_Data.backlight = 100;
		}else if(g_sys_Data.themeMode == THEME_DARK){
			g_sys_Data.backlight = 40;
		}
		SetBackLight(g_sys_Data.backlight);
		char str[5] = {0};
		sprintf(str,"%d",g_sys_Data.backlight);
		lv_label_set_text(guider_ui.screen_SET_label_beiguang1, str);
		lv_slider_set_value(guider_ui.screen_SET_slider_backlight, g_sys_Data.backlight, 1);
		if(!storageData_setInt("backlight", g_sys_Data.backlight)){
			printf("storage data backlight[%d] fail!\n", g_sys_Data.backlight);
		}

		lv_label_set_text(guider_ui.screen_SET_label_zhuti1,get_string_for_language(g_sys_Data.current_language,themeMode[g_sys_Data.themeMode]));
		ScreenSetBlackNightMode(&guider_ui, g_sys_Data.themeMode);
		languageItemStatu(g_sys_Data.current_language);
		if(g_sys_Data.themeMode == THEME_AUTO){
			createLightPerceptionThread();
			autoModeTimer = lv_timer_create(creatAutoModeTimerCbk, 1000, NULL);
		}else{
			destoryLightPerceptionThread();
			if(autoModeTimer){
				lv_timer_del(autoModeTimer);
				autoModeTimer = NULL;
			}
			
		}
		break;
	default:
		break;
	}
}
void screen_SET_btn_zhuti_right_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		g_sys_Data.themeMode = g_sys_Data.themeMode==1?THEME_AUTO:(g_sys_Data.themeMode > 1 ? THEME_DAY : THEME_DARK);
		if(!storageData_setInt("themeMode", g_sys_Data.themeMode)){
			printf("storage data themeMode[%d] fail!\n", g_sys_Data.themeMode);
		}
		if(g_sys_Data.themeMode == THEME_DAY){
			g_sys_Data.backlight = 100;
		}else if(g_sys_Data.themeMode == THEME_DARK){
			g_sys_Data.backlight = 40;
		}
		SetBackLight(g_sys_Data.backlight);
		char str[5] = {0};
		sprintf(str,"%d",g_sys_Data.backlight);
		lv_label_set_text(guider_ui.screen_SET_label_beiguang1, str);
		lv_slider_set_value(guider_ui.screen_SET_slider_backlight, g_sys_Data.backlight, 1);
		if(!storageData_setInt("backlight", g_sys_Data.backlight)){
			printf("storage data backlight[%d] fail!\n", g_sys_Data.backlight);
		}

		lv_label_set_text(guider_ui.screen_SET_label_zhuti1,get_string_for_language(g_sys_Data.current_language,themeMode[g_sys_Data.themeMode]));

		ScreenSetBlackNightMode(&guider_ui, g_sys_Data.themeMode);
		languageItemStatu(g_sys_Data.current_language);

		if(g_sys_Data.themeMode == THEME_AUTO){
			createLightPerceptionThread();
			autoModeTimer = lv_timer_create(creatAutoModeTimerCbk, 1000, NULL);
		}else{
			destoryLightPerceptionThread();
			if(autoModeTimer){
				lv_timer_del(autoModeTimer);
				autoModeTimer = NULL;
			}
		}
		break;
	default:
		break;
	}
}

void screen_SET_btn_yuyan1_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		/* code */
		sysPageManageFun(sysPageManage);
		sysPageManage = CONT_LANGUAGE;
		lv_obj_clear_flag(guider_ui.screen_SET_cont_setlanguage, LV_OBJ_FLAG_HIDDEN);
		break;
	
	default:
		break;
	}
}

void screen_SET_btn_shijian_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		if(g_sys_Data.linktype == LINK_TYPE_ANDROIDAUTO || g_sys_Data.linktype == LINK_TYPE_CARPLAY){
			show_label_with_timer(guider_ui.screen_label_Popup, "sys_txt_invalid", 1000);
			return;
		}
		sysPageManageFun(sysPageManage);
		sysPageManage = CONT_SETTIME;
		lv_obj_clear_flag(guider_ui.screen_SET_cont_settime, LV_OBJ_FLAG_HIDDEN);
		break;
	
	default:
		break;
	}
}

void screen_SET_btn_about_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		/* code */
		sysPageManageFun(sysPageManage);
		sysPageManage = CONT_ABOUT;
		lv_obj_clear_flag(guider_ui.screen_SET_cont_guanyu, LV_OBJ_FLAG_HIDDEN);
		break;
	
	default:
		break;
	}
}

void screen_SET_btn_ZH_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		g_sys_Data.current_language = LANG_CHINESE;
		languageItemStatu(g_sys_Data.current_language);
		if(!storageData_setInt("current_language", g_sys_Data.current_language)){
			printf("storage data language[%d] fail!\n", g_sys_Data.current_language);
		}
		break;
	default:
		break;
	}
}
void screen_SET_btn_EN_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		g_sys_Data.current_language = LANG_ENGLISH;
		languageItemStatu(g_sys_Data.current_language);
		if(!storageData_setInt("current_language", g_sys_Data.current_language)){
			printf("storage data language[%d] fail!\n", g_sys_Data.current_language);
		}
		break;
	default:
		break;
	}
}
void screen_SET_btn_ZH_FT_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		g_sys_Data.current_language = LANG_CHINESE_FT;
		languageItemStatu(g_sys_Data.current_language);
		if(!storageData_setInt("current_language", g_sys_Data.current_language)){
			printf("storage data language[%d] fail!\n", g_sys_Data.current_language);
		}
		break;
	default:
		break;
	}
}
void screen_SET_btn_FR_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		g_sys_Data.current_language = LANG_FRANCE;
		languageItemStatu(g_sys_Data.current_language);
		if(!storageData_setInt("current_language", g_sys_Data.current_language)){
			printf("storage data language[%d] fail!\n", g_sys_Data.current_language);
		}
		break;
	default:
		break;
	}
}
void screen_SET_btn_GE_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		g_sys_Data.current_language = LANG_GERMANY;
		languageItemStatu(g_sys_Data.current_language);
		if(!storageData_setInt("current_language", g_sys_Data.current_language)){
			printf("storage data language[%d] fail!\n", g_sys_Data.current_language);
		}
		break;
	default:
		break;
	}
}
void screen_SET_btn_IT_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		g_sys_Data.current_language = LANG_ITALY;
		languageItemStatu(g_sys_Data.current_language);
		if(!storageData_setInt("current_language", g_sys_Data.current_language)){
			printf("storage data language[%d] fail!\n", g_sys_Data.current_language);
		}
		break;
	default:
		break;
	}
}
void screen_SET_btn_JP_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		g_sys_Data.current_language = LANG_JAPAN;
		languageItemStatu(g_sys_Data.current_language);
		if(!storageData_setInt("current_language", g_sys_Data.current_language)){
			printf("storage data language[%d] fail!\n", g_sys_Data.current_language);
		}
		break;
	default:
		break;
	}
}
void screen_SET_btn_POR_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		g_sys_Data.current_language = LANG_PORTUGUESE;
		languageItemStatu(g_sys_Data.current_language);
		if(!storageData_setInt("current_language", g_sys_Data.current_language)){
			printf("storage data language[%d] fail!\n", g_sys_Data.current_language);
		}
		break;
	default:
		break;
	}
}
void screen_SET_btn_RU_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		g_sys_Data.current_language = LANG_RUSSIAN;
		languageItemStatu(g_sys_Data.current_language);
		if(!storageData_setInt("current_language", g_sys_Data.current_language)){
			printf("storage data language[%d] fail!\n", g_sys_Data.current_language);
		}
		break;
	default:
		break;
	}
}
void screen_SET_btn_SP_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		g_sys_Data.current_language = LANG_SPANISH;
		languageItemStatu(g_sys_Data.current_language);
		if(!storageData_setInt("current_language", g_sys_Data.current_language)){
			printf("storage data language[%d] fail!\n", g_sys_Data.current_language);
		}
		break;
	default:
		break;
	}
}
void screen_SET_btn_POL_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		g_sys_Data.current_language = LANG_POLAND;
		languageItemStatu(g_sys_Data.current_language);
		if(!storageData_setInt("current_language", g_sys_Data.current_language)){
			printf("storage data language[%d] fail!\n", g_sys_Data.current_language);
		}
		break;
	default:
		break;
	}
}
void screen_SET_btn_TU_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		g_sys_Data.current_language = LANG_TURKISH;
		languageItemStatu(g_sys_Data.current_language);
		if(!storageData_setInt("current_language", g_sys_Data.current_language)){
			printf("storage data language[%d] fail!\n", g_sys_Data.current_language);
		}
		break;
	default:
		break;
	}
}
void screen_SET_btn_KO_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		g_sys_Data.current_language = LANG_KOREAN;
		languageItemStatu(g_sys_Data.current_language);
		if(!storageData_setInt("current_language", g_sys_Data.current_language)){
			printf("storage data language[%d] fail!\n", g_sys_Data.current_language);
		}
		break;
	default:
		break;
	}
}
void screen_SET_btn_DU_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		g_sys_Data.current_language = LANG_DUTCH;
		languageItemStatu(g_sys_Data.current_language);
		if(!storageData_setInt("current_language", g_sys_Data.current_language)){
			printf("storage data language[%d] fail!\n", g_sys_Data.current_language);
		}
		break;
	default:
		break;
	}
}


void screen_SET_slider_backlight_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
	g_sys_Data.backlight = lv_slider_get_value(obj);
     if(code == LV_EVENT_REFR_EXT_DRAW_SIZE) {
         lv_coord_t * s = lv_event_get_param(e);
         *s = LV_MAX(*s, 60);
     }
    else if(code == LV_EVENT_VALUE_CHANGED)
    {	
		SetBackLight(g_sys_Data.backlight);
		char str[5] = {0};
		sprintf(str,"%d",g_sys_Data.backlight);
		lv_label_set_text(guider_ui.screen_SET_label_beiguang1, str);
		if(!storageData_setInt("backlight", g_sys_Data.backlight)){
			printf("storage data backlight[%d] fail!\n", g_sys_Data.backlight);
		}
    }
     else if(code == LV_EVENT_DRAW_PART_END) {
         lv_obj_draw_part_dsc_t * dsc = lv_event_get_param(e);
         if(dsc->part == LV_PART_KNOB && lv_obj_has_state(obj, LV_STATE_PRESSED)) {
             char buf[8];
		}
	}
}

//--------------------data set ----------------------
static void screen_SET_roller_year_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        char buf[32];
        lv_roller_get_selected_str(obj, buf, sizeof(buf));
        printf("Selected years: %s\n", buf);
		TTime time;
		memset(&time, 0 ,sizeof(TTime));
		RtcGetTime2C(&time);
		time.nYear = atoi(buf);
		RtcSetTime2C(&time);
		printf("Selected years: %d\n", time.nYear);
    }
}
extern char* CreateDayOptions(void);
static void screen_SET_roller_month_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        char buf[32];
        lv_roller_get_selected_str(obj, buf, sizeof(buf));
        printf("Selected month: %s\n", buf);
		TTime time;
		memset(&time, 0 ,sizeof(TTime));
		RtcGetTime2C(&time);
		time.nMonth = atoi(buf);
		time.nDay = 1;
		RtcSetTime2C(&time);
		lv_roller_set_options(guider_ui.screen_SET_roller_day, CreateDayOptions(), LV_ROLLER_MODE_INFINITE);
		lv_roller_set_selected(guider_ui.screen_SET_roller_day,0,LV_ANIM_OFF);

		printf("Selected nMonth: %d\n", time.nMonth);
    }
}

static void screen_SET_roller_day_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        char buf[32];
        lv_roller_get_selected_str(obj, buf, sizeof(buf));
        printf("Selected day: %s\n", buf);
		TTime time;
		memset(&time, 0 ,sizeof(TTime));
		RtcGetTime2C(&time);
		time.nDay = atoi(buf);
		RtcSetTime2C(&time);
		printf("Selected day: %d\n", time.nDay);
    }
}

static void screen_SET_roller_hour_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        char buf[32];
        lv_roller_get_selected_str(obj, buf, sizeof(buf));
        printf("Selected hour: %s\n", buf);
		TTime time;
		memset(&time, 0 ,sizeof(TTime));
		RtcGetTime2C(&time);
		time.nHour = atoi(buf);
		RtcSetTime2C(&time);
		printf("Selected hour: %d\n", time.nHour);
    }
}

static void screen_SET_roller_minute_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        char buf[32];
        lv_roller_get_selected_str(obj, buf, sizeof(buf));
        printf("Selected minute: %s\n", buf);
		TTime time;
		memset(&time, 0 ,sizeof(TTime));
		RtcGetTime2C(&time);
		time.nMinute = atoi(buf);
		RtcSetTime2C(&time);
		printf("Selected minute: %d\n", time.nMinute);
    }
}
//----
static void screen_SET_screen_SET_btn_taiya_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_PRESSED:
		lv_img_set_src(guider_ui.screen_SET_btn_taiya_img, &_right_but_down_60x75);
		break;
	case LV_EVENT_RELEASED:
		lv_img_set_src(guider_ui.screen_SET_btn_taiya_img, &_right_but_normal_day_60x75);
		if(g_sys_Data.carTripSwitch){
			g_sys_Data.carTripSwitch = false;
			lv_label_set_text(guider_ui.screen_SET_btn_taiya_sw_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_close"));
		}else{
			g_sys_Data.carTripSwitch = true;
			lv_label_set_text(guider_ui.screen_SET_btn_taiya_sw_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_open"));
		}
		if(!storageData_setBool("carTripSwitch", g_sys_Data.carTripSwitch)){
			printf("storage data carTripSwitch[%d] fail!\n", g_sys_Data.carTripSwitch);
		}
		break;
	default:
		break;
	}
}

void screen_SET_btn_logo_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		sysPageManageFun(sysPageManage);
		sysPageManage = CONT_BOOTLOGO;
		lv_obj_clear_flag(guider_ui.screen_SET_cont_bootlogo, LV_OBJ_FLAG_HIDDEN);
		break;
	default:
		break;
	}
}

void screen_SET_btn_externalBootlogo_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		bootlogoUp(EXTERN_LOGO_PATH);
		break;
	default:
		break;
	}
}

void screen_SET_btn_reboot_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		sysPageManageFun(sysPageManage);
		sysPageManage = CONT_REBOOT;
		lv_obj_clear_flag(guider_ui.screen_SET_cont_sys, LV_OBJ_FLAG_HIDDEN);
		lv_obj_clear_flag(guider_ui.screen_SET_cont_reboot, LV_OBJ_FLAG_HIDDEN);
		break;
	default:
		break;
	}
}

void screen_SET_btn_refactory_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		sysPageManageFun(sysPageManage);
		sysPageManage = CONT_RESETFACTORY;
		lv_obj_clear_flag(guider_ui.screen_SET_cont_sys, LV_OBJ_FLAG_HIDDEN);
		lv_obj_clear_flag(guider_ui.screen_SET_cont_resetFactory, LV_OBJ_FLAG_HIDDEN);
		break;
	default:
		break;
	}
}

void screen_SET_btn_reboot_confirm_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		lv_obj_add_flag(guider_ui.screen_SET_cont_reboot, LV_OBJ_FLAG_HIDDEN);
		sysPageManage = CONT_SYS;
		sync();
		reboot(RB_AUTOBOOT);
		break;
	default:
		break;
	}
}

void screen_SET_btn_reboot_cancel_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		lv_obj_add_flag(guider_ui.screen_SET_cont_reboot, LV_OBJ_FLAG_HIDDEN);
		sysPageManage = CONT_SYS;
		break;
	default:
		break;
	}
}

void screen_SET_btn_resetFactory_confirm_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		lv_obj_add_flag(guider_ui.screen_SET_cont_resetFactory, LV_OBJ_FLAG_HIDDEN);
		sysPageManage = CONT_SYS;
		RestorefactorySettings();
		break;
	default:
		break;
	}
}

void screen_SET_btn_resetFactory_cancel_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		lv_obj_add_flag(guider_ui.screen_SET_cont_resetFactory, LV_OBJ_FLAG_HIDDEN);
		sysPageManage = CONT_SYS;
		break;
	default:
		break;
	}
}

void screen_SET_btn_otaUpdata_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		// if(g_sys_Data.linktype == LINK_TYPE_ANDROIDAUTO || g_sys_Data.linktype == LINK_TYPE_CARPLAY){
		// 	show_label_with_timer(guider_ui.screen_label_Popup, "sys_txt_invalid", 1000);
		// 	return;
		// }

		stop_label_timer(guider_ui.screen_label_Popup);

		ui_load_scr_animation(&guider_ui, &guider_ui.screen_OTA,
					guider_ui.screen_OTA_del, &guider_ui.screen_SET_del,
					setup_scr_ota, LV_SCR_LOAD_ANIM_NONE,
					0, 0, true, true);
		break;
	default:
		break;
	}
}

void screen_SET_btn_appVersion_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_LONG_PRESSED:
		sysPageManageFun(sysPageManage);
		sysPageManage = CONT_AGINGMODE;
		lv_obj_clear_flag(guider_ui.screen_SET_cont_agingMode, LV_OBJ_FLAG_HIDDEN);		
		break;
	default:
		break;
	}
}

void screen_SET_btn_agingMode_memInfo_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		g_sys_Data.agingMode.memInfoSW = (g_sys_Data.agingMode.memInfoSW ? false : true);
		break;
	default:
		break;
	}
}

void screen_SET_btn_agingMode_netBandWidth_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		g_sys_Data.agingMode.netBandWidth = (g_sys_Data.agingMode.netBandWidth ? false : true);
		break;
	default:
		break;
	}
}


void screen_SET_btn_agingMode_touchPoint_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		g_sys_Data.agingMode.touchPointSW = (g_sys_Data.agingMode.touchPointSW ? false : true);	
		break;
	default:
		break;
	}
}

void screen_SET_btn_agingMode_screenSave_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		g_sys_Data.agingMode.screenSaveSw = (g_sys_Data.agingMode.screenSaveSw ? false : true);		
		if(g_sys_Data.agingMode.screenSaveSw){
			createDashAnalogTimer(g_sys_Data.agingMode.screenSaveTime*1000);
			lv_label_set_text(guider_ui.screen_SET_btn_agingMode_screenSave_labelSwitch,get_string_for_language(g_sys_Data.current_language,"sys_txt_open") );
		}else{
			destoryDashAnalogTimer();
			lv_label_set_text(guider_ui.screen_SET_btn_agingMode_screenSave_labelSwitch,get_string_for_language(g_sys_Data.current_language,"sys_txt_close") );
		}
		break;
	default:
		break;
	}
}

void screen_SET_btn_agingMode_screenSaveTime_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		switch (g_sys_Data.agingMode.screenSaveTime)
		{
		case T_10S:
			g_sys_Data.agingMode.screenSaveTime = T_30S;
			lv_label_set_text(guider_ui.screen_SET_btn_agingMode_screenSaveTime_labelTime, "30s");
			break;
		case T_30S:
			g_sys_Data.agingMode.screenSaveTime = T_1MIN;
			lv_label_set_text(guider_ui.screen_SET_btn_agingMode_screenSaveTime_labelTime, "1min");
			break;	
		case T_1MIN:
			g_sys_Data.agingMode.screenSaveTime = T_5MIN;
			lv_label_set_text(guider_ui.screen_SET_btn_agingMode_screenSaveTime_labelTime, "5min");
			break;	
		case T_5MIN:
			g_sys_Data.agingMode.screenSaveTime = T_10MIN;
			lv_label_set_text(guider_ui.screen_SET_btn_agingMode_screenSaveTime_labelTime, "10min");
			break;	
		case T_10MIN:
			g_sys_Data.agingMode.screenSaveTime = T_30MIN;
			lv_label_set_text(guider_ui.screen_SET_btn_agingMode_screenSaveTime_labelTime, "30min");
			break;		
		default:
			g_sys_Data.agingMode.screenSaveTime = T_10S;
			lv_label_set_text(guider_ui.screen_SET_btn_agingMode_screenSaveTime_labelTime, "10s");
			break;
		}
		if(!storageData_setInt("screenSaveTime", g_sys_Data.agingMode.screenSaveTime)){
			printf("storage data screenSaveTime[%d] fail!\n", g_sys_Data.agingMode.screenSaveTime);
		}
		if(g_sys_Data.agingMode.screenSaveSw){
			destoryDashAnalogTimer();
			createDashAnalogTimer(g_sys_Data.agingMode.screenSaveTime*1000);
		}	
		break;
	default:
		break;
	}
}

void screen_SET_btn_agingMode_scrennShot_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		g_sys_Data.agingMode.scrennShotSW = (g_sys_Data.agingMode.scrennShotSW ? false : true);
		if(g_sys_Data.agingMode.scrennShotSW){
			screenshot_thread_start();
			lv_label_set_text(guider_ui.screen_SET_btn_agingMode_scrennShot_labelSW, get_string_for_language(g_sys_Data.current_language,"sys_txt_open"));
		}else{
			screenshot_thread_stop();
			lv_label_set_text(guider_ui.screen_SET_btn_agingMode_scrennShot_labelSW, get_string_for_language(g_sys_Data.current_language,"sys_txt_close"));
		}				
		break;
	default:
		break;
	}
}



void events_init_screen_SET (lv_ui *ui)
{

	lv_obj_add_event_cb(ui->screen_SET_btn_return, screen_SET_btn_return_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_list_set_item0, screen_SET_list_set_item0_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_list_set_item1, screen_SET_list_set_item1_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_list_set_item2, screen_SET_list_set_item2_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_taiya, screen_SET_screen_SET_btn_taiya_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_zhuti_left, screen_SET_btn_zhuti_left_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_zhuti_right, screen_SET_btn_zhuti_right_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_yuyan, screen_SET_btn_yuyan1_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_shijian, screen_SET_btn_shijian_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_about, screen_SET_btn_about_event_handler, LV_EVENT_ALL, ui);
	
	lv_obj_add_event_cb(ui->screen_SET_btn_ZH, screen_SET_btn_ZH_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_EN, screen_SET_btn_EN_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_ZH_FT, screen_SET_btn_ZH_FT_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_FR, screen_SET_btn_FR_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_GE, screen_SET_btn_GE_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_IT, screen_SET_btn_IT_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_JP, screen_SET_btn_JP_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_POR, screen_SET_btn_POR_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_RU, screen_SET_btn_RU_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_SP, screen_SET_btn_SP_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_POL, screen_SET_btn_POL_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_TU, screen_SET_btn_TU_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_KO, screen_SET_btn_KO_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_DU, screen_SET_btn_DU_event_handler, LV_EVENT_ALL, ui);


	lv_obj_add_event_cb(ui->screen_SET_slider_backlight, screen_SET_slider_backlight_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_roller_year, screen_SET_roller_year_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_roller_month, screen_SET_roller_month_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_roller_day, screen_SET_roller_day_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_roller_hour, screen_SET_roller_hour_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_roller_minute, screen_SET_roller_minute_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_logo, screen_SET_btn_logo_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_externalBootlogo, screen_SET_btn_externalBootlogo_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_reboot, screen_SET_btn_reboot_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_refactory, screen_SET_btn_refactory_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_reboot_confirm, screen_SET_btn_reboot_confirm_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_reboot_cancel, screen_SET_btn_reboot_cancel_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_resetFactory_confirm, screen_SET_btn_resetFactory_confirm_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_resetFactory_cancel, screen_SET_btn_resetFactory_cancel_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_otaUpdata, screen_SET_btn_otaUpdata_event_handler, LV_EVENT_ALL, ui);	
	lv_obj_add_event_cb(ui->screen_SET_btn_appVersion, screen_SET_btn_appVersion_event_handler, LV_EVENT_LONG_PRESSED, ui);

	lv_obj_add_event_cb(ui->screen_SET_btn_agingMode_memInfo, screen_SET_btn_agingMode_memInfo_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_agingMode_netBandWidth,screen_SET_btn_agingMode_netBandWidth_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_agingMode_touchPoint, screen_SET_btn_agingMode_touchPoint_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_agingMode_screenSave, screen_SET_btn_agingMode_screenSave_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_agingMode_screenSaveTime, screen_SET_btn_agingMode_screenSaveTime_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_SET_btn_agingMode_scrennShot, screen_SET_btn_agingMode_scrennShot_event_handler, LV_EVENT_ALL, ui);

}