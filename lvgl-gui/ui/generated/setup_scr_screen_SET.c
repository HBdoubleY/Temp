/*
* Copyright 2025 NXP
* NXP Confidential and Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"
#include "lvgl_system.h"
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "bootlogoUp.h"
#include "bt_serial.h"
static const char* themeMode[] = {"sys_txt_day", "sys_txt_night", "sys_txt_auto"}; 
const char* app_language[] = {"sys_txt_zh", "sys_txt_en", "sys_txt_zh_ft", "sys_txt_fr", "sys_txt_ge", 
                        "sys_txt_it", "sys_txt_jp", "sys_txt_por", "sys_txt_ru", "sys_txt_sp",
                        "sys_txt_pol", "sys_txt_tu", "sys_txt_ko", "sys_txt_du"};

static void create_bootLogo_cont(lv_ui *ui);
static void create_reboot_cont(lv_ui *ui);
static void create_resetFactory_cont(lv_ui *ui);
static void create_aging_mode_cont(lv_ui *ui);
static lv_style_t style_screen_SET_text_left_mian_default;
static lv_style_t style_screen_SET_text_center_mian_default;
static lv_style_t style_screen_SET_text_right_align_mian_default;

void languageItemStatu(language_t language){
	printf("%s:%d\n",__func__,__LINE__);
	if(g_sys_Data.themeMode == THEME_DAY || (g_sys_Data.themeMode == THEME_AUTO && g_sys_Data.gpadcVol < 1300)){
		lv_img_set_src(guider_ui.screen_SET_btn_ZH_img, &_sel_checkbox_off_day_60_60x60);
		lv_img_set_src(guider_ui.screen_SET_btn_EN_img, &_sel_checkbox_off_day_60_60x60);
		lv_img_set_src(guider_ui.screen_SET_btn_ZH_FT_img, &_sel_checkbox_off_day_60_60x60);
		lv_img_set_src(guider_ui.screen_SET_btn_FR_img, &_sel_checkbox_off_day_60_60x60);
		lv_img_set_src(guider_ui.screen_SET_btn_GE_img, &_sel_checkbox_off_day_60_60x60);
		lv_img_set_src(guider_ui.screen_SET_btn_IT_img, &_sel_checkbox_off_day_60_60x60);
		lv_img_set_src(guider_ui.screen_SET_btn_JP_img, &_sel_checkbox_off_day_60_60x60);
		lv_img_set_src(guider_ui.screen_SET_btn_POR_img, &_sel_checkbox_off_day_60_60x60);
		lv_img_set_src(guider_ui.screen_SET_btn_RU_img, &_sel_checkbox_off_day_60_60x60);
		lv_img_set_src(guider_ui.screen_SET_btn_SP_img, &_sel_checkbox_off_day_60_60x60);
		lv_img_set_src(guider_ui.screen_SET_btn_POL_img, &_sel_checkbox_off_day_60_60x60);
		lv_img_set_src(guider_ui.screen_SET_btn_TU_img, &_sel_checkbox_off_day_60_60x60);
		lv_img_set_src(guider_ui.screen_SET_btn_KO_img, &_sel_checkbox_off_day_60_60x60);
		lv_img_set_src(guider_ui.screen_SET_btn_DU_img, &_sel_checkbox_off_day_60_60x60);
	}else if(g_sys_Data.themeMode == THEME_DARK || (g_sys_Data.themeMode == THEME_AUTO && g_sys_Data.gpadcVol > 1600)){
		lv_img_set_src(guider_ui.screen_SET_btn_ZH_img, &_sel_checkbox_off_night_60_60x60);
		lv_img_set_src(guider_ui.screen_SET_btn_EN_img, &_sel_checkbox_off_night_60_60x60);
		lv_img_set_src(guider_ui.screen_SET_btn_ZH_FT_img, &_sel_checkbox_off_night_60_60x60);
		lv_img_set_src(guider_ui.screen_SET_btn_FR_img, &_sel_checkbox_off_night_60_60x60);
		lv_img_set_src(guider_ui.screen_SET_btn_GE_img, &_sel_checkbox_off_night_60_60x60);
		lv_img_set_src(guider_ui.screen_SET_btn_IT_img, &_sel_checkbox_off_night_60_60x60);
		lv_img_set_src(guider_ui.screen_SET_btn_JP_img, &_sel_checkbox_off_night_60_60x60);
		lv_img_set_src(guider_ui.screen_SET_btn_POR_img, &_sel_checkbox_off_night_60_60x60);
		lv_img_set_src(guider_ui.screen_SET_btn_RU_img, &_sel_checkbox_off_night_60_60x60);
		lv_img_set_src(guider_ui.screen_SET_btn_SP_img, &_sel_checkbox_off_night_60_60x60);
		lv_img_set_src(guider_ui.screen_SET_btn_POL_img, &_sel_checkbox_off_night_60_60x60);
		lv_img_set_src(guider_ui.screen_SET_btn_TU_img, &_sel_checkbox_off_night_60_60x60);
		lv_img_set_src(guider_ui.screen_SET_btn_KO_img, &_sel_checkbox_off_night_60_60x60);
		lv_img_set_src(guider_ui.screen_SET_btn_DU_img, &_sel_checkbox_off_night_60_60x60);
	}

	switch (g_sys_Data.current_language)
	{
	case LANG_CHINESE:
		lv_img_set_src(guider_ui.screen_SET_btn_ZH_img, &_sel_checkbox_on_60_60x60);
		break;
	case LANG_ENGLISH:
		lv_img_set_src(guider_ui.screen_SET_btn_EN_img, &_sel_checkbox_on_60_60x60);
		break;	
	case LANG_CHINESE_FT:
		lv_img_set_src(guider_ui.screen_SET_btn_ZH_FT_img, &_sel_checkbox_on_60_60x60);
		break;
	case LANG_FRANCE:
		lv_img_set_src(guider_ui.screen_SET_btn_FR_img, &_sel_checkbox_on_60_60x60);
		break;
	case LANG_GERMANY:
		lv_img_set_src(guider_ui.screen_SET_btn_GE_img, &_sel_checkbox_on_60_60x60);
		break;
	case LANG_ITALY:
		lv_img_set_src(guider_ui.screen_SET_btn_IT_img, &_sel_checkbox_on_60_60x60);
		break;
	case LANG_JAPAN:
		lv_img_set_src(guider_ui.screen_SET_btn_JP_img, &_sel_checkbox_on_60_60x60);
		break;
	case LANG_PORTUGUESE:
		lv_img_set_src(guider_ui.screen_SET_btn_POR_img, &_sel_checkbox_on_60_60x60);
		break;
	case LANG_RUSSIAN:
		lv_img_set_src(guider_ui.screen_SET_btn_RU_img, &_sel_checkbox_on_60_60x60);
		break;
	case LANG_SPANISH:
		lv_img_set_src(guider_ui.screen_SET_btn_SP_img, &_sel_checkbox_on_60_60x60);
		break;
	case LANG_POLAND:
		lv_img_set_src(guider_ui.screen_SET_btn_POL_img, &_sel_checkbox_on_60_60x60);
		break;
	case LANG_TURKISH:
		lv_img_set_src(guider_ui.screen_SET_btn_TU_img, &_sel_checkbox_on_60_60x60);
		break;
	case LANG_KOREAN:
		lv_img_set_src(guider_ui.screen_SET_btn_KO_img, &_sel_checkbox_on_60_60x60);
		break;
	case LANG_DUTCH:
		lv_img_set_src(guider_ui.screen_SET_btn_DU_img, &_sel_checkbox_on_60_60x60);
		break;
	default:
		break;
	}
	lv_obj_t * label0 = lv_obj_get_child(guider_ui.screen_SET_list_set_item0, 0);
	lv_obj_t * label1 = lv_obj_get_child(guider_ui.screen_SET_list_set_item1, 0);
	lv_obj_t * label2 = lv_obj_get_child(guider_ui.screen_SET_list_set_item2, 0);
	lv_label_set_text(label0,get_string_for_language(g_sys_Data.current_language, "sys_txt_generate"));
	lv_label_set_text(label1,get_string_for_language(g_sys_Data.current_language, "sys_txt_bt"));
	lv_label_set_text(label2,get_string_for_language(g_sys_Data.current_language, "sys_txt_sys"));	
	lv_label_set_text(guider_ui.screen_SET_btn_taiya_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_tireInfo"));
	if(g_sys_Data.carTripSwitch){
		lv_label_set_text(guider_ui.screen_SET_btn_taiya_sw_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_open"));
	}else{
		lv_label_set_text(guider_ui.screen_SET_btn_taiya_sw_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_close"));
	}
	lv_label_set_text(guider_ui.screen_SET_label_beiguang, get_string_for_language(g_sys_Data.current_language,"sys_txt_backLight"));
	lv_label_set_text(guider_ui.screen_SET_btn_shijian_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_setTime"));
	lv_label_set_text(guider_ui.screen_SET_btn_yuyan_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_language"));
	lv_label_set_text(guider_ui.screen_SET_btn_yuyan_label1, get_string_for_language(g_sys_Data.current_language,app_language[g_sys_Data.current_language]));
	lv_label_set_text(guider_ui.screen_SET_label_zhuti, get_string_for_language(g_sys_Data.current_language,"sys_txt_topic"));
	lv_label_set_text(guider_ui.screen_SET_label_zhuti1, get_string_for_language(g_sys_Data.current_language,themeMode[g_sys_Data.themeMode]));
	// if(get_BT_connect_state()){
	// 	lv_label_set_text(guider_ui.screen_SET_label_connect, get_phone_name());
	// }else{
	// 	lv_label_set_text(guider_ui.screen_SET_label_connect, get_string_for_language(g_sys_Data.current_language,"main_txt_nConnect"));
	// }
	lv_label_set_text(guider_ui.screen_SET_label_pairdev, get_string_for_language(g_sys_Data.current_language,"sys_txt_btPairDev"));
	lv_label_set_text(guider_ui.screen_SET_label_btname, get_string_for_language(g_sys_Data.current_language,"sys_txt_btName"));
	lv_label_set_text(guider_ui.screen_SET_btn_refactory_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_reFactory"));
	lv_label_set_text(guider_ui.screen_SET_btn_reboot_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_rebootSys"));
	lv_label_set_text(guider_ui.screen_SET_btn_logo_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_logo"));
	lv_label_set_text(guider_ui.screen_SET_btn_about_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_about"));
	lv_label_set_text(guider_ui.screen_SET_label_year, get_string_for_language(g_sys_Data.current_language,"sys_txt_year") );
	lv_label_set_text(guider_ui.screen_SET_label_month, get_string_for_language(g_sys_Data.current_language,"sys_txt_mon") );
	lv_label_set_text(guider_ui.screen_SET_label_day, get_string_for_language(g_sys_Data.current_language,"sys_txt_today") );
	lv_label_set_text(guider_ui.screen_SET_label_hour, get_string_for_language(g_sys_Data.current_language,"sys_txt_hour") );
	lv_label_set_text(guider_ui.screen_SET_label_minute, get_string_for_language(g_sys_Data.current_language,"sys_txt_min") );
	lv_label_set_text(guider_ui.screen_SET_label_devModel,get_string_for_language(g_sys_Data.current_language,"sys_txt_devModel") );
	lv_label_set_text(guider_ui.screen_SET_label_devNum,get_string_for_language(g_sys_Data.current_language,"sys_txt_devSerNum") );
	lv_label_set_text(guider_ui.screen_SET_label_appVersion,get_string_for_language(g_sys_Data.current_language,"sys_txt_appVersion") );
	lv_label_set_text(guider_ui.screen_SET_label_hwVersion,get_string_for_language(g_sys_Data.current_language,"sys_txt_hwVersion"));
	lv_label_set_text(guider_ui.screen_SET_btn_otaUpdata_label,get_string_for_language(g_sys_Data.current_language,"sys_txt_otaUpdata"));
	lv_label_set_text(guider_ui.screen_SET_label_popup, get_string_for_language(g_sys_Data.current_language,"sys_txt_invalid"));
	lv_label_set_text(guider_ui.screen_SET_btn_bootlogo_path_label,get_string_for_language(g_sys_Data.current_language,"sys_txt_choose_path"));
	lv_label_set_text(guider_ui.screen_SET_btn_bootlogo_path_label1, get_string_for_language(g_sys_Data.current_language,"sys_txt_internal_path"));
	lv_label_set_text(guider_ui.screen_SET_label_bootlogo_choose,get_string_for_language(g_sys_Data.current_language,"sys_txt_choose_logo"));
	lv_label_set_text(guider_ui.screen_SET_label_reboot_text,get_string_for_language(g_sys_Data.current_language,"sys_txt_rebootSys_label"));
	lv_label_set_text(guider_ui.screen_SET_btn_reboot_confirm_label,get_string_for_language(g_sys_Data.current_language,"sys_txt_confirm"));
	lv_label_set_text(guider_ui.screen_SET_btn_reboot_cancel_label,get_string_for_language(g_sys_Data.current_language,"sys_txt_cancel"));
	lv_label_set_text(guider_ui.screen_SET_label_resetFactory_text,get_string_for_language(g_sys_Data.current_language,"sys_txt_reFactory_label"));
	lv_label_set_text(guider_ui.screen_SET_btn_resetFactory_confirm_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_confirm"));
	lv_label_set_text(guider_ui.screen_SET_btn_resetFactory_cancel_label,get_string_for_language(g_sys_Data.current_language,"sys_txt_cancel"));
	lv_label_set_text(guider_ui.screen_SET_btn_agingMode_memInfo_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_memInfo_label"));
	lv_label_set_text(guider_ui.screen_SET_btn_agingMode_netBandWidth_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_netBandWidth_label"));
	lv_label_set_text(guider_ui.screen_SET_btn_agingMode_touchPoint_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_touchPoint_label"));
	lv_label_set_text(guider_ui.screen_SET_btn_agingMode_touchPoint_labelSwitch, get_string_for_language(g_sys_Data.current_language,"sys_txt_close"));
	lv_label_set_text(guider_ui.screen_SET_btn_agingMode_screenSave_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_screenSave_label"));
	if(g_sys_Data.agingMode.screenSaveSw){
		lv_label_set_text(guider_ui.screen_SET_btn_agingMode_screenSave_labelSwitch,get_string_for_language(g_sys_Data.current_language,"sys_txt_open") );
	}else{
		lv_label_set_text(guider_ui.screen_SET_btn_agingMode_screenSave_labelSwitch,get_string_for_language(g_sys_Data.current_language,"sys_txt_close") );
	}
	lv_label_set_text(guider_ui.screen_SET_btn_agingMode_screenSaveTime_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_screenSaveTime_label") );
	char *str = NULL;
	switch (g_sys_Data.agingMode.screenSaveTime)
	{
	case T_10S :
		str = "10s";
		break;
	case T_30S :
		str = "30s";
	case T_1MIN :
		str = "1min";
	case T_5MIN :
		str = "5min";
	case T_10MIN :
		str = "10min";
	case T_30MIN :
		str = "30min";	
	default:
		str = "10s";
		break;
	}
	lv_label_set_text(guider_ui.screen_SET_btn_agingMode_screenSaveTime_labelTime, str);
	lv_label_set_text(guider_ui.screen_SET_btn_agingMode_scrennShot_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_screenShot_label"));
	if(g_sys_Data.agingMode.scrennShotSW){
		lv_label_set_text(guider_ui.screen_SET_btn_agingMode_scrennShot_labelSW, get_string_for_language(g_sys_Data.current_language,"sys_txt_open"));
	}else{
		lv_label_set_text(guider_ui.screen_SET_btn_agingMode_scrennShot_labelSW, get_string_for_language(g_sys_Data.current_language,"sys_txt_close"));
	}
}


// 检查闰年
static bool is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// 获取月份天数
static int get_days_in_month(int year, int month) {
    static const int days_in_month[] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    
    if (month == 2) {
        return is_leap_year(year) ? 29 : 28;
    }
    
    return days_in_month[month - 1];
}

static char* CreateYearOptions(void){
    static char yearOptions[1000];
    int ops = 0;
    for(int i = 1980; i <= 2079; i++){
        ops += sprintf(yearOptions+ops,"%d\n",i);
    }
    yearOptions[ops-1] = '\0';
    return yearOptions;
}

static char* CreateMonthOptions(void){
    static char MonthOptions[200];
    int ops = 0;
    for(int i = 1; i <= 12; i++){
        ops += sprintf(MonthOptions+ops,"%d\n",i);
    }
    MonthOptions[ops-1] = '\0';
    return MonthOptions;
}

char* CreateDayOptions(void){
    static char DayOptions[200];
    int ops = 0;
	TTime time;
	memset(&time, 0, sizeof(TTime));
	RtcGetTime2C(&time);
    for(int i = 1; i <= get_days_in_month(time.nYear, time.nMonth); i++){
        ops += sprintf(DayOptions+ops,"%d\n",i);
    }
    DayOptions[ops-1] = '\0';
    return DayOptions;
}

static char* CreateHourOptions(void){
    static char HourOptions[200];
    int ops = 0;
    for(int i = 0; i <= 23; i++){
        ops += sprintf(HourOptions+ops,"%d\n",i);
    }
    HourOptions[ops-1] = '\0';
    return HourOptions;
}

static char* CreateMinuteOptions(void){
    static char MinuteOptions[200];
    int ops = 0;
    for(int i = 0; i <= 59; i++){
        ops += sprintf(MinuteOptions+ops,"%d\n",i);
    }
    MinuteOptions[ops-1] = '\0';
    return MinuteOptions;
}

void setup_scr_screen_SET(lv_ui *ui)
{

	//Write codes screen_SET
	ui->screen_SET = lv_obj_create(NULL);
	lv_obj_set_size(ui->screen_SET, 1440, 720);
	lv_obj_set_scrollbar_mode(ui->screen_SET, LV_SCROLLBAR_MODE_AUTO);

	//Write style for screen_SET, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_SET, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_SET, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);

	//write all label mian_default style
	ui_init_style(&style_screen_SET_text_left_mian_default);

	lv_style_set_text_color(&style_screen_SET_text_left_mian_default, lv_color_hex(0x000000));
	lv_style_set_text_font(&style_screen_SET_text_left_mian_default, &lv_font_harmonyOS_40);
	lv_style_set_text_opa(&style_screen_SET_text_left_mian_default, 255);
	lv_style_set_pad_top(&style_screen_SET_text_left_mian_default, 25);
	lv_style_set_pad_left(&style_screen_SET_text_left_mian_default, 50);
	lv_style_set_pad_right(&style_screen_SET_text_left_mian_default, 0);
	lv_style_set_pad_bottom(&style_screen_SET_text_left_mian_default, 35);
	lv_style_set_bg_opa(&style_screen_SET_text_left_mian_default, 0);
	lv_style_set_bg_color(&style_screen_SET_text_left_mian_default, lv_color_hex(0x2195f6));
	lv_style_set_border_opa(&style_screen_SET_text_left_mian_default, 0);

	//write all label mian_align_default style
	ui_init_style(&style_screen_SET_text_center_mian_default);

	lv_style_set_text_color(&style_screen_SET_text_center_mian_default, lv_color_hex(0x000000));
	lv_style_set_text_font(&style_screen_SET_text_center_mian_default, &lv_font_harmonyOS_40);
	lv_style_set_text_opa(&style_screen_SET_text_center_mian_default, 255);
	lv_style_set_text_align(&style_screen_SET_text_center_mian_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_top(&style_screen_SET_text_center_mian_default, 25);
	lv_style_set_pad_bottom(&style_screen_SET_text_center_mian_default, 15);
	lv_style_set_bg_opa(&style_screen_SET_text_center_mian_default, 0);
	lv_style_set_bg_color(&style_screen_SET_text_center_mian_default, lv_color_hex(0x2195f6));
	lv_style_set_border_opa(&style_screen_SET_text_center_mian_default, 0);

	//write all label style_screen_SET_text_right_align_mian_default style
	ui_init_style(&style_screen_SET_text_right_align_mian_default);

	lv_style_set_text_color(&style_screen_SET_text_right_align_mian_default, lv_color_hex(0x000000));
	lv_style_set_text_font(&style_screen_SET_text_right_align_mian_default, &lv_font_harmonyOS_40);
	lv_style_set_text_opa(&style_screen_SET_text_right_align_mian_default, 255);
	lv_style_set_text_align(&style_screen_SET_text_right_align_mian_default, LV_TEXT_ALIGN_RIGHT);
	lv_style_set_pad_top(&style_screen_SET_text_right_align_mian_default, 25);
	lv_style_set_pad_bottom(&style_screen_SET_text_right_align_mian_default, 35);
	lv_style_set_bg_opa(&style_screen_SET_text_right_align_mian_default, 0);
	lv_style_set_bg_color(&style_screen_SET_text_right_align_mian_default, lv_color_hex(0x2195f6));
	lv_style_set_border_opa(&style_screen_SET_text_right_align_mian_default, 0);

	//Write codes screen_SET_btn_return
	ui->screen_SET_btn_return = lv_btn_create(ui->screen_SET);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_return, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(ui->screen_SET_btn_return, 50, 20);
	lv_obj_set_size(ui->screen_SET_btn_return, 60, 75);

	//Write style for screen_SET_btn_return, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_return, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_SET_btn_return, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_SET_btn_return, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_SET_btn_return, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_SET_btn_return, &_left_but_normal_day_60x75, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_SET_btn_return, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_SET_btn_return, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_SET_btn_return, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_SET_btn_return, &lv_font_harmonyOS_40, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_SET_btn_return, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_SET_btn_return, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_SET_btn_return, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_return, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_border_width(ui->screen_SET_btn_return, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_radius(ui->screen_SET_btn_return, 5, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_width(ui->screen_SET_btn_return, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_src(ui->screen_SET_btn_return, &_left_but_down_60x75, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_opa(ui->screen_SET_btn_return, 255, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_SET_btn_return, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_text_color(ui->screen_SET_btn_return, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_text_font(ui->screen_SET_btn_return, &lv_font_harmonyOS_40, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_text_opa(ui->screen_SET_btn_return, 255, LV_PART_MAIN|LV_STATE_PRESSED);

	// Write codes screen_SET_list_set
	ui->screen_SET_list_set = lv_list_create(ui->screen_SET);
	ui->screen_SET_list_set_item0 =lv_list_add_btn(ui->screen_SET_list_set, NULL,get_string_for_language(g_sys_Data.current_language, "sys_txt_generate"));
	ui->screen_SET_list_set_item1 =lv_list_add_btn(ui->screen_SET_list_set, NULL,get_string_for_language(g_sys_Data.current_language, "sys_txt_bt"));
	ui->screen_SET_list_set_item2 =lv_list_add_btn(ui->screen_SET_list_set, NULL,get_string_for_language(g_sys_Data.current_language, "sys_txt_sys"));	
	lv_obj_set_pos(ui->screen_SET_list_set, 0, 130);
	lv_obj_set_size(ui->screen_SET_list_set, 240, 420);
	lv_obj_set_scrollbar_mode(ui->screen_SET_list_set, LV_SCROLLBAR_MODE_AUTO);
	lv_obj_set_style_bg_opa(ui->screen_SET_list_set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->screen_SET_list_set,0,LV_PART_MAIN|LV_STATE_DEFAULT);

	//write codes set_list Attribute
	static lv_style_t style_screen_SET_list_set_texts_main_default;
	ui_init_style(&style_screen_SET_list_set_texts_main_default);

	lv_style_set_pad_top(&style_screen_SET_list_set_texts_main_default, 45);
	lv_style_set_pad_left(&style_screen_SET_list_set_texts_main_default, 5);
	lv_style_set_pad_right(&style_screen_SET_list_set_texts_main_default, 5);
	lv_style_set_pad_bottom(&style_screen_SET_list_set_texts_main_default, 45);
	lv_style_set_border_width(&style_screen_SET_list_set_texts_main_default, 0);
	lv_style_set_text_color(&style_screen_SET_list_set_texts_main_default, lv_color_hex(0x000000));
	lv_style_set_text_font(&style_screen_SET_list_set_texts_main_default, &lv_font_harmonyOS_40);
	lv_style_set_text_opa(&style_screen_SET_list_set_texts_main_default, 255);
	lv_style_set_bg_opa(&style_screen_SET_list_set_texts_main_default, 255);
	lv_style_set_bg_color(&style_screen_SET_list_set_texts_main_default, lv_color_hex(0xffffff));
	lv_style_set_text_align(&style_screen_SET_list_set_texts_main_default,LV_TEXT_ALIGN_CENTER);
	lv_obj_add_style(ui->screen_SET_list_set_item0, &style_screen_SET_list_set_texts_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_style(ui->screen_SET_list_set_item1, &style_screen_SET_list_set_texts_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_style(ui->screen_SET_list_set_item2, &style_screen_SET_list_set_texts_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	static lv_style_t style_screen_SET_list_set_texts_main_pressed;
	ui_init_style(&style_screen_SET_list_set_texts_main_pressed);

	lv_style_set_pad_top(&style_screen_SET_list_set_texts_main_pressed, 45);
	lv_style_set_pad_left(&style_screen_SET_list_set_texts_main_pressed, 5);
	lv_style_set_pad_right(&style_screen_SET_list_set_texts_main_pressed, 5);
	lv_style_set_pad_bottom(&style_screen_SET_list_set_texts_main_pressed, 45);
	lv_style_set_border_width(&style_screen_SET_list_set_texts_main_pressed, 0);
	lv_style_set_text_color(&style_screen_SET_list_set_texts_main_pressed, lv_color_hex(0x000000));
	lv_style_set_text_font(&style_screen_SET_list_set_texts_main_pressed, &lv_font_harmonyOS_40);
	lv_style_set_text_opa(&style_screen_SET_list_set_texts_main_pressed, 255);
	lv_style_set_bg_opa(&style_screen_SET_list_set_texts_main_pressed, 255);
	lv_style_set_bg_color(&style_screen_SET_list_set_texts_main_pressed, lv_color_hex(0x1da7ff));
	lv_style_set_text_align(&style_screen_SET_list_set_texts_main_pressed,LV_TEXT_ALIGN_CENTER);
	lv_obj_add_style(ui->screen_SET_list_set_item0, &style_screen_SET_list_set_texts_main_pressed, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_add_style(ui->screen_SET_list_set_item1, &style_screen_SET_list_set_texts_main_pressed, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_add_style(ui->screen_SET_list_set_item2, &style_screen_SET_list_set_texts_main_pressed, LV_PART_MAIN|LV_STATE_PRESSED);	

	//Write codes screen_SET_img_lineX
	ui->screen_SET_img_lineY = lv_img_create(ui->screen_SET);
	lv_obj_add_flag(ui->screen_SET_img_lineY, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_img_lineY, &_lineY_alpha_5x720);
	lv_img_set_pivot(ui->screen_SET_img_lineY, 50,50);
	lv_img_set_angle(ui->screen_SET_img_lineY, 0);
	lv_obj_set_pos(ui->screen_SET_img_lineY, 240, 0);
	lv_obj_set_size(ui->screen_SET_img_lineY, 5, 720);

	//Write style for screen_SET_img_lineX, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_recolor_opa(ui->screen_SET_img_lineY, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_opa(ui->screen_SET_img_lineY, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_SET_img_lineY, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_clip_corner(ui->screen_SET_img_lineY, true, LV_PART_MAIN|LV_STATE_DEFAULT);


	//Write codes screen_SET_cont_tongyong
	ui->screen_SET_cont_tongyong = lv_obj_create(ui->screen_SET);
	lv_obj_set_pos(ui->screen_SET_cont_tongyong, 240, 60);
	lv_obj_set_size(ui->screen_SET_cont_tongyong, 1200, 600);
	lv_obj_set_scrollbar_mode(ui->screen_SET_cont_tongyong, LV_SCROLLBAR_MODE_OFF);
	lv_obj_set_scroll_dir(ui->screen_SET_cont_tongyong,LV_DIR_TOP|LV_DIR_BOTTOM);
	lv_obj_clear_flag(ui->screen_SET_cont_tongyong, LV_OBJ_FLAG_HIDDEN);
	lv_obj_set_scroll_dir(ui->screen_SET_cont_tongyong,LV_DIR_NONE);
 
	//Write style for screen_SET_cont_tongyong, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_SET_cont_tongyong, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->screen_SET_cont_tongyong, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->screen_SET_cont_tongyong, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_side(ui->screen_SET_cont_tongyong, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_SET_cont_tongyong, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_SET_cont_tongyong, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_cont_tongyong, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_SET_cont_tongyong, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_SET_cont_tongyong, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_SET_cont_tongyong, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_SET_cont_tongyong, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_SET_img_lineY
	ui->screen_SET_cont_tongyong_img_lineX0 = lv_img_create(ui->screen_SET_cont_tongyong);
	lv_obj_add_flag(ui->screen_SET_cont_tongyong_img_lineX0, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_tongyong_img_lineX0, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_tongyong_img_lineX0, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_tongyong_img_lineX0, 0);
	lv_obj_set_pos(ui->screen_SET_cont_tongyong_img_lineX0, 0, 120-5);
	lv_obj_set_size(ui->screen_SET_cont_tongyong_img_lineX0, 1200, 5);

	//Write style for screen_SET_img_lineX, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_recolor_opa(ui->screen_SET_cont_tongyong_img_lineX0, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_opa(ui->screen_SET_cont_tongyong_img_lineX0, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_SET_cont_tongyong_img_lineX0, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_clip_corner(ui->screen_SET_cont_tongyong_img_lineX0, true, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_SET_img_lineY
	ui->screen_SET_cont_tongyong_img_lineX1 = lv_img_create(ui->screen_SET_cont_tongyong);
	lv_obj_add_flag(ui->screen_SET_cont_tongyong_img_lineX1, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_tongyong_img_lineX1, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_tongyong_img_lineX1, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_tongyong_img_lineX1, 0);
	lv_obj_set_pos(ui->screen_SET_cont_tongyong_img_lineX1, 0, 240-5);
	lv_obj_set_size(ui->screen_SET_cont_tongyong_img_lineX1, 1200, 5);

	//Write style for screen_SET_img_lineX, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_recolor_opa(ui->screen_SET_cont_tongyong_img_lineX1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_opa(ui->screen_SET_cont_tongyong_img_lineX1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_SET_cont_tongyong_img_lineX1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_clip_corner(ui->screen_SET_cont_tongyong_img_lineX1, true, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_SET_img_lineY
	ui->screen_SET_cont_tongyong_img_lineX2 = lv_img_create(ui->screen_SET_cont_tongyong);
	lv_obj_add_flag(ui->screen_SET_cont_tongyong_img_lineX2, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_tongyong_img_lineX2, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_tongyong_img_lineX2, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_tongyong_img_lineX2, 0);
	lv_obj_set_pos(ui->screen_SET_cont_tongyong_img_lineX2, 0, 360-5);
	lv_obj_set_size(ui->screen_SET_cont_tongyong_img_lineX2, 1200, 5);

	//Write style for screen_SET_img_lineX, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_recolor_opa(ui->screen_SET_cont_tongyong_img_lineX2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_opa(ui->screen_SET_cont_tongyong_img_lineX2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_SET_cont_tongyong_img_lineX2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_clip_corner(ui->screen_SET_cont_tongyong_img_lineX2, true, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_SET_img_lineY
	ui->screen_SET_cont_tongyong_img_lineX3 = lv_img_create(ui->screen_SET_cont_tongyong);
	lv_obj_add_flag(ui->screen_SET_cont_tongyong_img_lineX3, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_tongyong_img_lineX3, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_tongyong_img_lineX3, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_tongyong_img_lineX3, 0);
	lv_obj_set_pos(ui->screen_SET_cont_tongyong_img_lineX3, 0, 480-5);
	lv_obj_set_size(ui->screen_SET_cont_tongyong_img_lineX3, 1200, 5);

	//Write style for screen_SET_img_lineX, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_recolor_opa(ui->screen_SET_cont_tongyong_img_lineX3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_opa(ui->screen_SET_cont_tongyong_img_lineX3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_SET_cont_tongyong_img_lineX3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_clip_corner(ui->screen_SET_cont_tongyong_img_lineX3, true, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_SET_img_lineY
	ui->screen_SET_cont_tongyong_img_lineX4 = lv_img_create(ui->screen_SET_cont_tongyong);
	lv_obj_add_flag(ui->screen_SET_cont_tongyong_img_lineX4, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_tongyong_img_lineX4, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_tongyong_img_lineX4, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_tongyong_img_lineX4, 0);
	lv_obj_set_pos(ui->screen_SET_cont_tongyong_img_lineX4, 0, 600-5);
	lv_obj_set_size(ui->screen_SET_cont_tongyong_img_lineX4, 1200, 5);

	//Write style for screen_SET_img_lineX, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_recolor_opa(ui->screen_SET_cont_tongyong_img_lineX4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_opa(ui->screen_SET_cont_tongyong_img_lineX4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_SET_cont_tongyong_img_lineX4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_clip_corner(ui->screen_SET_cont_tongyong_img_lineX4, true, LV_PART_MAIN|LV_STATE_DEFAULT);


	//Write codes screen_SET_label_taiya
	ui->screen_SET_btn_taiya = lv_btn_create(ui->screen_SET_cont_tongyong);
	ui->screen_SET_btn_taiya_label = lv_label_create(ui->screen_SET_btn_taiya);
	ui->screen_SET_btn_taiya_sw_label = lv_label_create(ui->screen_SET_btn_taiya);
	ui->screen_SET_btn_taiya_img = lv_img_create(ui->screen_SET_btn_taiya);
	lv_label_set_text(ui->screen_SET_btn_taiya_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_tireInfo"));
	if(g_sys_Data.carTripSwitch){
		lv_label_set_text(ui->screen_SET_btn_taiya_sw_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_open"));
	}else{
		lv_label_set_text(ui->screen_SET_btn_taiya_sw_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_close"));
	}
	lv_label_set_long_mode(ui->screen_SET_btn_taiya_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_label_set_long_mode(ui->screen_SET_btn_taiya_sw_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_SET_btn_taiya, 0, 480);
	lv_obj_set_size(ui->screen_SET_btn_taiya, 1200, 120);

	lv_img_set_src(ui->screen_SET_btn_taiya_img, &_right_but_normal_day_60x75);
	lv_img_set_pivot(ui->screen_SET_btn_taiya_img, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_taiya_img, 0);
	lv_obj_set_pos(ui->screen_SET_btn_taiya_img, 1050, 12);
	lv_obj_set_size(ui->screen_SET_btn_taiya_img, 60, 75);	
 
	//Write style for screen_SET_label_taiya, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_SET_btn_taiya, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_SET_btn_taiya, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_taiya, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_SET_btn_taiya, lv_color_hex(0x1da7ff), LV_PART_MAIN|LV_STATE_DEFAULT);
	

	lv_obj_remove_style_all(ui->screen_SET_btn_taiya_label);
	lv_obj_remove_style_all(ui->screen_SET_btn_taiya_sw_label);
	lv_obj_set_pos(ui->screen_SET_btn_taiya_label, 0, 0);
	lv_obj_set_size(ui->screen_SET_btn_taiya_label, 600, 120);
	lv_obj_set_pos(ui->screen_SET_btn_taiya_sw_label, 900, 0);
	lv_obj_set_size(ui->screen_SET_btn_taiya_sw_label, 150, 120);
	lv_obj_add_style(ui->screen_SET_btn_taiya_label, &style_screen_SET_text_left_mian_default, LV_PART_MAIN);
	lv_obj_add_style(ui->screen_SET_btn_taiya_sw_label, &style_screen_SET_text_center_mian_default, LV_PART_MAIN);

	//Write codes screen_SET_label_beiguang
	ui->screen_SET_label_beiguang = lv_label_create(ui->screen_SET_cont_tongyong);
	lv_label_set_text(ui->screen_SET_label_beiguang, get_string_for_language(g_sys_Data.current_language,"sys_txt_backLight"));
	lv_label_set_long_mode(ui->screen_SET_label_beiguang, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_SET_label_beiguang, 20, 360);
	lv_obj_set_size(ui->screen_SET_label_beiguang, 370, 120);
	lv_obj_add_style(ui->screen_SET_label_beiguang, &style_screen_SET_text_left_mian_default, LV_PART_MAIN);
	lv_obj_set_style_pad_top(ui->screen_SET_label_beiguang, 35, LV_PART_MAIN);

	//Write codes screen_SET_slider_backlight
	ui->screen_SET_slider_backlight = lv_slider_create(ui->screen_SET_cont_tongyong);
	lv_slider_set_range(ui->screen_SET_slider_backlight, 0, 100);
	lv_slider_set_mode(ui->screen_SET_slider_backlight, LV_SLIDER_MODE_NORMAL);
	lv_slider_set_value(ui->screen_SET_slider_backlight, g_sys_Data.backlight, LV_ANIM_OFF);
	lv_obj_set_pos(ui->screen_SET_slider_backlight, 400, 420);
	lv_obj_set_size(ui->screen_SET_slider_backlight, 600, 8);
 
	//Write style for screen_SET_slider_backlight, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_SET_slider_backlight, 60, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_SET_slider_backlight, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->screen_SET_slider_backlight, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_SET_slider_backlight, 50, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_outline_width(ui->screen_SET_slider_backlight, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_SET_slider_backlight, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_SET_slider_backlight, Part: LV_PART_INDICATOR, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_SET_slider_backlight, 255, LV_PART_INDICATOR|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_SET_slider_backlight, lv_color_hex(0x2195f6), LV_PART_INDICATOR|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->screen_SET_slider_backlight, LV_GRAD_DIR_NONE, LV_PART_INDICATOR|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_SET_slider_backlight, 50, LV_PART_INDICATOR|LV_STATE_DEFAULT);

	//Write style for screen_SET_slider_backlight, Part: LV_PART_KNOB, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_SET_slider_backlight, 255, LV_PART_KNOB|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_SET_slider_backlight, lv_color_hex(0x2195f6), LV_PART_KNOB|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->screen_SET_slider_backlight, LV_GRAD_DIR_NONE, LV_PART_KNOB|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_SET_slider_backlight, 50, LV_PART_KNOB|LV_STATE_DEFAULT);

	//Write codes screen_SET_label_beiguang1
	ui->screen_SET_label_beiguang1 = lv_label_create(ui->screen_SET_cont_tongyong);
	char bl_str[5] = {0};
	sprintf(bl_str,"%d",g_sys_Data.backlight);
	lv_label_set_text(ui->screen_SET_label_beiguang1, bl_str);
	lv_label_set_long_mode(ui->screen_SET_label_beiguang1, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_SET_label_beiguang1, 1000, 360);
	lv_obj_set_size(ui->screen_SET_label_beiguang1, 100, 120);
	lv_obj_add_style(ui->screen_SET_label_beiguang1, &style_screen_SET_text_center_mian_default, LV_PART_MAIN);
	lv_obj_set_style_pad_top(ui->screen_SET_label_beiguang1, 35, LV_PART_MAIN);

	//Write codes screen_SET_label_shijian
	ui->screen_SET_btn_shijian = lv_btn_create(ui->screen_SET_cont_tongyong);
	ui->screen_SET_btn_shijian_label = lv_label_create(ui->screen_SET_btn_shijian);
	lv_label_set_text(ui->screen_SET_btn_shijian_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_setTime"));
	lv_label_set_long_mode(ui->screen_SET_btn_shijian_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_SET_btn_shijian, 0, 240);
	lv_obj_set_size(ui->screen_SET_btn_shijian, 1200, 120);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_shijian, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	lv_obj_remove_style_all(ui->screen_SET_btn_shijian_label);
	lv_obj_set_pos(ui->screen_SET_btn_shijian_label, 0, 0);
	lv_obj_set_size(ui->screen_SET_btn_shijian_label, 500, 120);
	lv_obj_add_style(ui->screen_SET_btn_shijian_label, &style_screen_SET_text_left_mian_default, LV_PART_MAIN);

	//write screen_SET_btn_shijian_img
	ui->screen_SET_btn_shijian_img = lv_img_create(ui->screen_SET_btn_shijian);
	lv_img_set_src(ui->screen_SET_btn_shijian_img, &_right_but_normal_day_60x75);
	lv_img_set_pivot(ui->screen_SET_btn_shijian_img, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_shijian_img, 0);
	lv_obj_set_pos(ui->screen_SET_btn_shijian_img, 1050, 12);
	lv_obj_set_size(ui->screen_SET_btn_shijian_img, 60, 75);	


	// Write codes screen_SET_label_yuyan
	ui->screen_SET_btn_yuyan = lv_btn_create(ui->screen_SET_cont_tongyong);
	ui->screen_SET_btn_yuyan_label = lv_label_create(ui->screen_SET_btn_yuyan);
	ui->screen_SET_btn_yuyan_label1 = lv_label_create(ui->screen_SET_btn_yuyan);
	lv_label_set_text(ui->screen_SET_btn_yuyan_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_language"));
	lv_label_set_text(ui->screen_SET_btn_yuyan_label1, get_string_for_language(g_sys_Data.current_language,app_language[g_sys_Data.current_language]));
	lv_label_set_long_mode(ui->screen_SET_btn_yuyan_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_SET_btn_yuyan, 0, 120);
	lv_obj_set_size(ui->screen_SET_btn_yuyan, 1200, 120);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_yuyan, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	lv_obj_remove_style_all(ui->screen_SET_btn_yuyan_label);
	lv_obj_set_pos(ui->screen_SET_btn_yuyan_label, 0, 0);
	lv_obj_set_size(ui->screen_SET_btn_yuyan_label, 500, 120);
	lv_obj_add_style(ui->screen_SET_btn_yuyan_label, &style_screen_SET_text_left_mian_default, LV_PART_MAIN);

	lv_obj_remove_style_all(ui->screen_SET_btn_yuyan_label1);
	lv_obj_set_pos(ui->screen_SET_btn_yuyan_label1, 800, 0);
	lv_obj_set_size(ui->screen_SET_btn_yuyan_label1, 250, 120);
	lv_obj_add_style(ui->screen_SET_btn_yuyan_label1, &style_screen_SET_text_center_mian_default, LV_PART_MAIN);

	//write screen_SET_btn_shijian_img
	ui->screen_SET_btn_yuyan_img = lv_img_create(ui->screen_SET_btn_yuyan);
	lv_img_set_src(ui->screen_SET_btn_yuyan_img, &_right_but_normal_day_60x75);
	lv_img_set_pivot(ui->screen_SET_btn_yuyan_img, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_yuyan_img, 0);
	lv_obj_set_pos(ui->screen_SET_btn_yuyan_img, 1050, 12);
	lv_obj_set_size(ui->screen_SET_btn_yuyan_img, 60, 75);	


	//Write codes screen_SET_label_zhuti
	ui->screen_SET_label_zhuti = lv_label_create(ui->screen_SET_cont_tongyong);
	lv_label_set_text(ui->screen_SET_label_zhuti, get_string_for_language(g_sys_Data.current_language,"sys_txt_topic"));
	lv_label_set_long_mode(ui->screen_SET_label_zhuti, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_SET_label_zhuti, 20, 0);
	lv_obj_set_size(ui->screen_SET_label_zhuti, 300, 120);
	lv_obj_add_style(ui->screen_SET_label_zhuti, &style_screen_SET_text_left_mian_default, LV_PART_MAIN);

	//Write codes screen_SET_label_zhuti1
	ui->screen_SET_label_zhuti1 = lv_label_create(ui->screen_SET_cont_tongyong);
	lv_label_set_text(ui->screen_SET_label_zhuti1, get_string_for_language(g_sys_Data.current_language,themeMode[g_sys_Data.themeMode]));
	lv_label_set_long_mode(ui->screen_SET_label_zhuti1, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_SET_label_zhuti1, 900, 0);
	lv_obj_set_size(ui->screen_SET_label_zhuti1, 170, 120);
	// lv_obj_add_style(ui->screen_SET_label_zhuti1, &style_screen_SET_text_left_mian_default, LV_PART_MAIN);
	lv_obj_add_style(ui->screen_SET_label_zhuti1, &style_screen_SET_text_center_mian_default, LV_PART_MAIN);

	//write codes screen_SET_btn_zhuti_left
	ui->screen_SET_btn_zhuti_left = lv_btn_create(ui->screen_SET_cont_tongyong);
	lv_obj_set_pos(ui->screen_SET_btn_zhuti_left, 840, 12);
	lv_obj_set_size(ui->screen_SET_btn_zhuti_left, 60, 75);
	lv_obj_set_style_bg_img_src(ui->screen_SET_btn_zhuti_left, &_left_but_normal_day_60x75, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_SET_btn_zhuti_left, &_left_but_down_60x75, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_zhuti_left, 0, LV_PART_MAIN);

	//write codes screen_SET_btn_zhuti_left
	ui->screen_SET_btn_zhuti_right = lv_btn_create(ui->screen_SET_cont_tongyong);
	lv_obj_set_pos(ui->screen_SET_btn_zhuti_right, 1070, 12);
	lv_obj_set_size(ui->screen_SET_btn_zhuti_right, 60, 75);
	lv_obj_set_style_bg_img_src(ui->screen_SET_btn_zhuti_right, &_right_but_normal_day_60x75, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_SET_btn_zhuti_right, &_right_but_down_60x75, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_zhuti_right, 0, LV_PART_MAIN);	

	//Write codes screen_SET_cont_lanya
	ui->screen_SET_cont_lanya = lv_obj_create(ui->screen_SET);
	lv_obj_set_pos(ui->screen_SET_cont_lanya, 240, 60);
	lv_obj_set_size(ui->screen_SET_cont_lanya, 1200, 600);
	lv_obj_set_scrollbar_mode(ui->screen_SET_cont_lanya, LV_SCROLLBAR_MODE_OFF);
	lv_obj_set_scroll_dir(ui->screen_SET_cont_lanya,LV_DIR_TOP|LV_DIR_BOTTOM);
	lv_obj_add_flag(ui->screen_SET_cont_lanya, LV_OBJ_FLAG_HIDDEN);

	//Write style for screen_SET_cont_lanya, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_SET_cont_lanya, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->screen_SET_cont_lanya, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->screen_SET_cont_lanya, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_side(ui->screen_SET_cont_lanya, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_SET_cont_lanya, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_SET_cont_lanya, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_cont_lanya, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_SET_cont_lanya, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_SET_cont_lanya, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_SET_cont_lanya, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_SET_cont_lanya, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_SET_img_lineY
	ui->screen_SET_cont_lanya_img_lineX0 = lv_img_create(ui->screen_SET_cont_lanya);
	lv_obj_add_flag(ui->screen_SET_cont_lanya_img_lineX0, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_lanya_img_lineX0, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_lanya_img_lineX0, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_lanya_img_lineX0, 0);
	lv_obj_set_pos(ui->screen_SET_cont_lanya_img_lineX0, 0, 120-5);
	lv_obj_set_size(ui->screen_SET_cont_lanya_img_lineX0, 1200, 5);

	//Write style for screen_SET_img_lineX, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_recolor_opa(ui->screen_SET_cont_lanya_img_lineX0, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_opa(ui->screen_SET_cont_lanya_img_lineX0, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_SET_cont_lanya_img_lineX0, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_clip_corner(ui->screen_SET_cont_lanya_img_lineX0, true, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_SET_img_lineY
	ui->screen_SET_cont_lanya_img_lineX1 = lv_img_create(ui->screen_SET_cont_lanya);
	lv_obj_add_flag(ui->screen_SET_cont_lanya_img_lineX1, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_lanya_img_lineX1, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_lanya_img_lineX1, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_lanya_img_lineX1, 0);
	lv_obj_set_pos(ui->screen_SET_cont_lanya_img_lineX1, 0, 240-5);
	lv_obj_set_size(ui->screen_SET_cont_lanya_img_lineX1, 1200, 5);

	//Write style for screen_SET_img_lineX, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_recolor_opa(ui->screen_SET_cont_lanya_img_lineX1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_opa(ui->screen_SET_cont_lanya_img_lineX1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_SET_cont_lanya_img_lineX1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_clip_corner(ui->screen_SET_cont_lanya_img_lineX1, true, LV_PART_MAIN|LV_STATE_DEFAULT);

 
	//Write codes screen_SET_label_connect
	ui->screen_SET_label_connect = lv_label_create(ui->screen_SET_cont_lanya);
	const char *bt_connect_text = NULL;
	if (get_BT_connect_state()) {
		const char *name = get_BT_connected_name();
		if (name && name[0] != '\0') {
			bt_connect_text = name;
		} else {
			bt_connect_text = get_string_for_language(g_sys_Data.current_language, "main_txt_nConnect");
		}
	} else {
		bt_connect_text = get_string_for_language(g_sys_Data.current_language, "main_txt_nConnect");
	}
	lv_label_set_text(ui->screen_SET_label_connect, bt_connect_text);
	lv_label_set_long_mode(ui->screen_SET_label_connect, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_SET_label_connect, 700, 120);
	lv_obj_set_size(ui->screen_SET_label_connect, 400, 120);
	lv_obj_add_style(ui->screen_SET_label_connect, &style_screen_SET_text_right_align_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_label_connect, 35, LV_PART_MAIN);
 
	//Write codes screen_SET_label_pairdev
	ui->screen_SET_label_pairdev = lv_label_create(ui->screen_SET_cont_lanya);
	lv_label_set_text(ui->screen_SET_label_pairdev, get_string_for_language(g_sys_Data.current_language,"sys_txt_btPairDev"));
	lv_label_set_long_mode(ui->screen_SET_label_pairdev, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_SET_label_pairdev, 0, 120);
	lv_obj_set_size(ui->screen_SET_label_pairdev, 500, 120);
	lv_obj_add_style(ui->screen_SET_label_pairdev, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_label_pairdev, 35, LV_PART_MAIN);
 
	//Write codes screen_SET_label_btname
	ui->screen_SET_label_btname = lv_label_create(ui->screen_SET_cont_lanya);
	lv_label_set_text(ui->screen_SET_label_btname, get_string_for_language(g_sys_Data.current_language,"sys_txt_btName"));
	lv_label_set_long_mode(ui->screen_SET_label_btname, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_SET_label_btname, 0, 0);
	lv_obj_set_size(ui->screen_SET_label_btname, 500, 120);
	lv_obj_add_style(ui->screen_SET_label_btname, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_label_btname, 35, LV_PART_MAIN);

	//Write codes screen_SET_label_btname1
	ui->screen_SET_label_btname1 = lv_label_create(ui->screen_SET_cont_lanya);
	lv_label_set_text(ui->screen_SET_label_btname1, "RTL8733BS");
	lv_label_set_long_mode(ui->screen_SET_label_btname1, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_SET_label_btname1, 700, 0);
	lv_obj_set_size(ui->screen_SET_label_btname1, 400, 120);
	lv_obj_add_style(ui->screen_SET_label_btname1, &style_screen_SET_text_right_align_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_label_btname1, 35, LV_PART_MAIN);

	//Write codes screen_SET_cont_sys
	ui->screen_SET_cont_sys = lv_obj_create(ui->screen_SET);
	lv_obj_set_pos(ui->screen_SET_cont_sys, 240, 60);
	lv_obj_set_size(ui->screen_SET_cont_sys, 1200, 600);
	lv_obj_set_scrollbar_mode(ui->screen_SET_cont_sys, LV_SCROLLBAR_MODE_OFF);
	lv_obj_set_scroll_dir(ui->screen_SET_cont_sys,LV_DIR_TOP|LV_DIR_BOTTOM);
	lv_obj_add_flag(ui->screen_SET_cont_sys, LV_OBJ_FLAG_HIDDEN);
 
	//Write style for screen_SET_cont_sys, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_SET_cont_sys, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->screen_SET_cont_sys, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->screen_SET_cont_sys, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_side(ui->screen_SET_cont_sys, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_SET_cont_sys, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_SET_cont_sys, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_cont_sys, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_SET_cont_sys, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_SET_cont_sys, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_SET_cont_sys, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_SET_cont_sys, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_SET_img_lineY
	ui->screen_SET_cont_sys_img_lineX0 = lv_img_create(ui->screen_SET_cont_sys);
	lv_obj_add_flag(ui->screen_SET_cont_sys_img_lineX0, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_sys_img_lineX0, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_sys_img_lineX0, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_sys_img_lineX0, 0);
	lv_obj_set_pos(ui->screen_SET_cont_sys_img_lineX0, 0, 120-5);
	lv_obj_set_size(ui->screen_SET_cont_sys_img_lineX0, 1200, 5);

	//Write style for screen_SET_img_lineX, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_recolor_opa(ui->screen_SET_cont_sys_img_lineX0, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_opa(ui->screen_SET_cont_sys_img_lineX0, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_SET_cont_sys_img_lineX0, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_clip_corner(ui->screen_SET_cont_sys_img_lineX0, true, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_SET_img_lineY
	ui->screen_SET_cont_sys_img_lineX1 = lv_img_create(ui->screen_SET_cont_sys);
	lv_obj_add_flag(ui->screen_SET_cont_sys_img_lineX1, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_sys_img_lineX1, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_sys_img_lineX1, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_sys_img_lineX1, 0);
	lv_obj_set_pos(ui->screen_SET_cont_sys_img_lineX1, 0, 240-5);
	lv_obj_set_size(ui->screen_SET_cont_sys_img_lineX1, 1200, 5);

	//Write style for screen_SET_img_lineX, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_recolor_opa(ui->screen_SET_cont_sys_img_lineX1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_opa(ui->screen_SET_cont_sys_img_lineX1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_SET_cont_sys_img_lineX1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_clip_corner(ui->screen_SET_cont_sys_img_lineX1, true, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_SET_img_lineY
	ui->screen_SET_cont_sys_img_lineX2 = lv_img_create(ui->screen_SET_cont_sys);
	lv_obj_add_flag(ui->screen_SET_cont_sys_img_lineX2, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_sys_img_lineX2, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_sys_img_lineX2, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_sys_img_lineX2, 0);
	lv_obj_set_pos(ui->screen_SET_cont_sys_img_lineX2, 0, 360-5);
	lv_obj_set_size(ui->screen_SET_cont_sys_img_lineX2, 1200, 5);

	//Write style for screen_SET_img_lineX, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_recolor_opa(ui->screen_SET_cont_sys_img_lineX2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_opa(ui->screen_SET_cont_sys_img_lineX2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_SET_cont_sys_img_lineX2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_clip_corner(ui->screen_SET_cont_sys_img_lineX2, true, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_SET_img_lineY
	ui->screen_SET_cont_sys_img_lineX3 = lv_img_create(ui->screen_SET_cont_sys);
	lv_obj_add_flag(ui->screen_SET_cont_sys_img_lineX3, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_sys_img_lineX3, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_sys_img_lineX3, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_sys_img_lineX3, 0);
	lv_obj_set_pos(ui->screen_SET_cont_sys_img_lineX3, 0, 480-5);
	lv_obj_set_size(ui->screen_SET_cont_sys_img_lineX3, 1200, 5);

	//Write style for screen_SET_img_lineX, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_recolor_opa(ui->screen_SET_cont_sys_img_lineX3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_opa(ui->screen_SET_cont_sys_img_lineX3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_SET_cont_sys_img_lineX3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_clip_corner(ui->screen_SET_cont_sys_img_lineX3, true, LV_PART_MAIN|LV_STATE_DEFAULT);	

	//Write codes screen_SET_label_refactory
	ui->screen_SET_btn_refactory = lv_btn_create(ui->screen_SET_cont_sys);
	ui->screen_SET_btn_refactory_label = lv_label_create(ui->screen_SET_btn_refactory);
	lv_label_set_text(ui->screen_SET_btn_refactory_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_reFactory"));
	lv_label_set_long_mode(ui->screen_SET_btn_refactory_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_SET_btn_refactory, 0, 360);
	lv_obj_set_size(ui->screen_SET_btn_refactory, 1200, 120);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_refactory, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_remove_style_all(ui->screen_SET_btn_refactory_label);
	lv_obj_add_style(ui->screen_SET_btn_refactory_label, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_refactory_img = lv_img_create(ui->screen_SET_btn_refactory);
	lv_img_set_src(ui->screen_SET_btn_refactory_img, &_right_but_normal_day_60x75);
	lv_img_set_pivot(ui->screen_SET_btn_refactory_img, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_refactory_img, 0);
	lv_obj_set_pos(ui->screen_SET_btn_refactory_img, 1050, 12);
	lv_obj_set_size(ui->screen_SET_btn_refactory_img, 60, 75);	

	// Write codes screen_SET_label_reboot
	ui->screen_SET_btn_reboot = lv_btn_create(ui->screen_SET_cont_sys);
	ui->screen_SET_btn_reboot_label = lv_label_create(ui->screen_SET_btn_reboot);
	lv_label_set_text(ui->screen_SET_btn_reboot_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_rebootSys"));
	lv_label_set_long_mode(ui->screen_SET_btn_reboot_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_SET_btn_reboot, 0, 240);
	lv_obj_set_size(ui->screen_SET_btn_reboot, 1200, 120);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_reboot, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_remove_style_all(ui->screen_SET_btn_reboot_label);
	lv_obj_add_style(ui->screen_SET_btn_reboot_label, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_reboot_img = lv_img_create(ui->screen_SET_btn_reboot);
	lv_img_set_src(ui->screen_SET_btn_reboot_img, &_right_but_normal_day_60x75);
	lv_img_set_pivot(ui->screen_SET_btn_reboot_img, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_reboot_img, 0);
	lv_obj_set_pos(ui->screen_SET_btn_reboot_img, 1050, 12);
	lv_obj_set_size(ui->screen_SET_btn_reboot_img, 60, 75);	

	//Write codes screen_SET_label_logo
	ui->screen_SET_btn_logo = lv_btn_create(ui->screen_SET_cont_sys);
	ui->screen_SET_btn_logo_label = lv_label_create(ui->screen_SET_btn_logo);
	lv_label_set_text(ui->screen_SET_btn_logo_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_logo"));
	lv_label_set_long_mode(ui->screen_SET_btn_logo_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_SET_btn_logo, 0, 120);
	lv_obj_set_size(ui->screen_SET_btn_logo, 1200, 120);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_logo, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_remove_style_all(ui->screen_SET_btn_logo_label);
	lv_obj_add_style(ui->screen_SET_btn_logo_label, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_logo_img = lv_img_create(ui->screen_SET_btn_logo);
	lv_img_set_src(ui->screen_SET_btn_logo_img, &_right_but_normal_day_60x75);
	lv_img_set_pivot(ui->screen_SET_btn_logo_img, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_logo_img, 0);
	lv_obj_set_pos(ui->screen_SET_btn_logo_img, 1050, 0);
	lv_obj_set_size(ui->screen_SET_btn_logo_img, 60, 75);	

	//Write codes screen_SET_label_about
	ui->screen_SET_btn_about = lv_btn_create(ui->screen_SET_cont_sys);
	ui->screen_SET_btn_about_label = lv_label_create(ui->screen_SET_btn_about);
	lv_label_set_text(ui->screen_SET_btn_about_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_about"));
	lv_label_set_long_mode(ui->screen_SET_btn_about_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_SET_btn_about, 0, 0);
	lv_obj_set_size(ui->screen_SET_btn_about, 1200, 120);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_about, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_remove_style_all(ui->screen_SET_btn_about_label);
	lv_obj_add_style(ui->screen_SET_btn_about_label, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_about_img = lv_img_create(ui->screen_SET_btn_about);
	lv_img_set_src(ui->screen_SET_btn_about_img, &_right_but_normal_day_60x75);
	lv_img_set_pivot(ui->screen_SET_btn_about_img, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_about_img, 0);
	lv_obj_set_pos(ui->screen_SET_btn_about_img, 1050, 12);
	lv_obj_set_size(ui->screen_SET_btn_about_img, 60, 75);	

	//Write codes screen_SET_cont_setlanguage
	ui->screen_SET_cont_setlanguage = lv_obj_create(ui->screen_SET);
	lv_obj_set_pos(ui->screen_SET_cont_setlanguage, 240, 60);
	lv_obj_set_size(ui->screen_SET_cont_setlanguage, 1200, 600);
	lv_obj_set_scrollbar_mode(ui->screen_SET_cont_setlanguage, LV_SCROLLBAR_MODE_OFF);
	lv_obj_set_scroll_dir(ui->screen_SET_cont_setlanguage, LV_DIR_VER);
	lv_obj_add_flag(ui->screen_SET_cont_setlanguage,LV_OBJ_FLAG_HIDDEN);
 
	//Write style for screen_SET_cont_setlanguage, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_SET_cont_setlanguage, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->screen_SET_cont_setlanguage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->screen_SET_cont_setlanguage, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_side(ui->screen_SET_cont_setlanguage, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_SET_cont_setlanguage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_SET_cont_setlanguage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_cont_setlanguage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_SET_cont_setlanguage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_SET_cont_setlanguage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_SET_cont_setlanguage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_SET_cont_setlanguage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_flex_flow(ui->screen_SET_cont_setlanguage, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(ui->screen_SET_cont_setlanguage, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_row(ui->screen_SET_cont_setlanguage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_column(ui->screen_SET_cont_setlanguage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_gap(ui->screen_SET_cont_setlanguage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_SET_imgbtn_ZH
	ui->screen_SET_btn_ZH = lv_btn_create(ui->screen_SET_cont_setlanguage);
	ui->screen_SET_btn_ZH_label = lv_label_create(ui->screen_SET_btn_ZH);
	lv_label_set_text(ui->screen_SET_btn_ZH_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_zh"));
	lv_label_set_long_mode(ui->screen_SET_btn_ZH_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_ZH, 0, LV_STATE_DEFAULT);
	lv_obj_set_size(ui->screen_SET_btn_ZH, 1200, 115);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_ZH, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_remove_style_all(ui->screen_SET_btn_ZH_label);
	lv_obj_add_style(ui->screen_SET_btn_ZH_label, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_btn_ZH_label, 40, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_ZH_img = lv_img_create(ui->screen_SET_btn_ZH);
	lv_img_set_src(ui->screen_SET_btn_ZH_img, &_sel_checkbox_off_day_60_60x60);
	lv_img_set_pivot(ui->screen_SET_btn_ZH_img, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_ZH_img, 0);
	lv_obj_set_pos(ui->screen_SET_btn_ZH_img, 1050, 27);
	lv_obj_set_size(ui->screen_SET_btn_ZH_img, 60, 60);	
 
	//Write codes screen_SET_imgbtn_EN
	ui->screen_SET_btn_EN = lv_btn_create(ui->screen_SET_cont_setlanguage);
	ui->screen_SET_btn_EN_label = lv_label_create(ui->screen_SET_btn_EN);
	lv_label_set_text(ui->screen_SET_btn_EN_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_en"));
	lv_label_set_long_mode(ui->screen_SET_btn_EN_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_EN, 0, LV_STATE_DEFAULT);
	// lv_obj_set_pos(ui->screen_SET_btn_EN, 0, 110);
	lv_obj_set_size(ui->screen_SET_btn_EN, 1200, 115);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_EN, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_remove_style_all(ui->screen_SET_btn_EN_label);
	lv_obj_add_style(ui->screen_SET_btn_EN_label, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_btn_EN_label, 40, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_EN_img = lv_img_create(ui->screen_SET_btn_EN);
	lv_img_set_src(ui->screen_SET_btn_EN_img, &_sel_checkbox_off_day_60_60x60);
	lv_img_set_pivot(ui->screen_SET_btn_EN_img, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_EN_img, 0);
	lv_obj_set_pos(ui->screen_SET_btn_EN_img, 1050, 27);
	lv_obj_set_size(ui->screen_SET_btn_EN_img, 60, 60);	


	//Write codes screen_SET_btn_ZH_FT
	ui->screen_SET_btn_ZH_FT = lv_btn_create(ui->screen_SET_cont_setlanguage);
	ui->screen_SET_btn_ZH_FT_label = lv_label_create(ui->screen_SET_btn_ZH_FT);
	lv_label_set_text(ui->screen_SET_btn_ZH_FT_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_zh_ft"));
	lv_label_set_long_mode(ui->screen_SET_btn_ZH_FT_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_ZH_FT, 0, LV_STATE_DEFAULT);
	// lv_obj_set_pos(ui->screen_SET_btn_EN, 0, 110);
	lv_obj_set_size(ui->screen_SET_btn_ZH_FT, 1200, 115);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_ZH_FT, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_remove_style_all(ui->screen_SET_btn_ZH_FT_label);
	lv_obj_add_style(ui->screen_SET_btn_ZH_FT_label, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_btn_ZH_FT_label, 40, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_ZH_FT_img = lv_img_create(ui->screen_SET_btn_ZH_FT);
	lv_img_set_src(ui->screen_SET_btn_ZH_FT_img, &_sel_checkbox_off_day_60_60x60);
	lv_img_set_pivot(ui->screen_SET_btn_ZH_FT_img, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_ZH_FT_img, 0);
	lv_obj_set_pos(ui->screen_SET_btn_ZH_FT_img, 1050, 27);
	lv_obj_set_size(ui->screen_SET_btn_ZH_FT_img, 60, 60);		

	//Write codes screen_SET_imgbtn_FR
	ui->screen_SET_btn_FR = lv_btn_create(ui->screen_SET_cont_setlanguage);
	ui->screen_SET_btn_FR_label = lv_label_create(ui->screen_SET_btn_FR);
	lv_label_set_text(ui->screen_SET_btn_FR_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_fr") );
	lv_label_set_long_mode(ui->screen_SET_btn_FR_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	// lv_obj_align(ui->screen_SET_btn_FR_label, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_FR, 0, LV_STATE_DEFAULT);
	// lv_obj_set_pos(ui->screen_SET_btn_FR, 0, 330);
	lv_obj_set_size(ui->screen_SET_btn_FR, 1200, 115);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_FR, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_remove_style_all(ui->screen_SET_btn_FR_label);
	lv_obj_add_style(ui->screen_SET_btn_FR_label, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_btn_FR_label, 40, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_FR_img = lv_img_create(ui->screen_SET_btn_FR);
	lv_img_set_src(ui->screen_SET_btn_FR_img, &_sel_checkbox_off_day_60_60x60);
	lv_img_set_pivot(ui->screen_SET_btn_FR_img, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_FR_img, 0);
	lv_obj_set_pos(ui->screen_SET_btn_FR_img, 1050, 27);
	lv_obj_set_size(ui->screen_SET_btn_FR_img, 60, 60);		

	//Write codes screen_SET_btn_GE
	ui->screen_SET_btn_GE = lv_btn_create(ui->screen_SET_cont_setlanguage);
	ui->screen_SET_btn_GE_label = lv_label_create(ui->screen_SET_btn_GE);
	lv_label_set_text(ui->screen_SET_btn_GE_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_ge") );
	lv_label_set_long_mode(ui->screen_SET_btn_GE_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	// lv_obj_align(ui->screen_SET_btn_AR_label, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_GE, 0, LV_STATE_DEFAULT);
	// lv_obj_set_pos(ui->screen_SET_btn_AR, 0, 440);
	lv_obj_set_size(ui->screen_SET_btn_GE, 1200, 115);	
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_GE, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_remove_style_all(ui->screen_SET_btn_GE_label);
	lv_obj_add_style(ui->screen_SET_btn_GE_label, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_btn_GE_label, 40, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_GE_img = lv_img_create(ui->screen_SET_btn_GE);
	lv_img_set_src(ui->screen_SET_btn_GE_img, &_sel_checkbox_off_day_60_60x60);
	lv_img_set_pivot(ui->screen_SET_btn_GE_img, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_GE_img, 0);
	lv_obj_set_pos(ui->screen_SET_btn_GE_img, 1050, 27);
	lv_obj_set_size(ui->screen_SET_btn_GE_img, 60, 60);		
	
	//Write codes screen_SET_btn_IT
	ui->screen_SET_btn_IT = lv_btn_create(ui->screen_SET_cont_setlanguage);
	ui->screen_SET_btn_IT_label = lv_label_create(ui->screen_SET_btn_IT);
	lv_label_set_text(ui->screen_SET_btn_IT_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_it") );
	lv_label_set_long_mode(ui->screen_SET_btn_IT_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	// lv_obj_align(ui->screen_SET_btn_AR_label, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_IT, 0, LV_STATE_DEFAULT);
	// lv_obj_set_pos(ui->screen_SET_btn_AR, 0, 440);
	lv_obj_set_size(ui->screen_SET_btn_IT, 1200, 115);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_IT, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_remove_style_all(ui->screen_SET_btn_IT_label);
	lv_obj_add_style(ui->screen_SET_btn_IT_label, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_btn_IT_label, 40, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_IT_img = lv_img_create(ui->screen_SET_btn_IT);
	lv_img_set_src(ui->screen_SET_btn_IT_img, &_sel_checkbox_off_day_60_60x60);
	lv_img_set_pivot(ui->screen_SET_btn_IT_img, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_IT_img, 0);
	lv_obj_set_pos(ui->screen_SET_btn_IT_img, 1050, 27);
	lv_obj_set_size(ui->screen_SET_btn_IT_img, 60, 60);				

	//Write codes screen_SET_imgbtn_JP
	ui->screen_SET_btn_JP = lv_btn_create(ui->screen_SET_cont_setlanguage);
	ui->screen_SET_btn_JP_label = lv_label_create(ui->screen_SET_btn_JP);
	lv_label_set_text(ui->screen_SET_btn_JP_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_jp") );
	lv_label_set_long_mode(ui->screen_SET_btn_JP_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	// lv_obj_align(ui->screen_SET_btn_JP_label, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_JP, 0, LV_STATE_DEFAULT);
	// lv_obj_set_pos(ui->screen_SET_btn_JP, 0, 660);
	lv_obj_set_size(ui->screen_SET_btn_JP, 1200, 115);	
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_JP, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_remove_style_all(ui->screen_SET_btn_JP_label);
	lv_obj_add_style(ui->screen_SET_btn_JP_label, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_btn_JP_label, 40, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_JP_img = lv_img_create(ui->screen_SET_btn_JP);
	lv_img_set_src(ui->screen_SET_btn_JP_img, &_sel_checkbox_off_day_60_60x60);
	lv_img_set_pivot(ui->screen_SET_btn_JP_img, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_JP_img, 0);
	lv_obj_set_pos(ui->screen_SET_btn_JP_img, 1050, 27);
	lv_obj_set_size(ui->screen_SET_btn_JP_img, 60, 60);	

	//Write codes screen_SET_imgbtn_JP
	ui->screen_SET_btn_POR = lv_btn_create(ui->screen_SET_cont_setlanguage);
	ui->screen_SET_btn_POR_label = lv_label_create(ui->screen_SET_btn_POR);
	lv_label_set_text(ui->screen_SET_btn_POR_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_por") );
	lv_label_set_long_mode(ui->screen_SET_btn_POR_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	// lv_obj_align(ui->screen_SET_btn_JP_label, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_POR, 0, LV_STATE_DEFAULT);
	// lv_obj_set_pos(ui->screen_SET_btn_JP, 0, 660);
	lv_obj_set_size(ui->screen_SET_btn_POR, 1200, 115);		
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_POR, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_remove_style_all(ui->screen_SET_btn_POR_label);
	lv_obj_add_style(ui->screen_SET_btn_POR_label, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_btn_POR_label, 40, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_POR_img = lv_img_create(ui->screen_SET_btn_POR);
	lv_img_set_src(ui->screen_SET_btn_POR_img, &_sel_checkbox_off_day_60_60x60);
	lv_img_set_pivot(ui->screen_SET_btn_POR_img, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_POR_img, 0);
	lv_obj_set_pos(ui->screen_SET_btn_POR_img, 1050, 27);
	lv_obj_set_size(ui->screen_SET_btn_POR_img, 60, 60);	

	//Write codes screen_SET_imgbtn_RU
	ui->screen_SET_btn_RU = lv_btn_create(ui->screen_SET_cont_setlanguage);
	ui->screen_SET_btn_RU_label = lv_label_create(ui->screen_SET_btn_RU);
	lv_label_set_text(ui->screen_SET_btn_RU_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_ru") );
	lv_label_set_long_mode(ui->screen_SET_btn_RU_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	// lv_obj_align(ui->screen_SET_btn_RU_label, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_RU, 0, LV_STATE_DEFAULT);
	// lv_obj_set_pos(ui->screen_SET_btn_RU, 0, 550);
	lv_obj_set_size(ui->screen_SET_btn_RU, 1200, 115);	
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_RU, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_remove_style_all(ui->screen_SET_btn_RU_label);
	lv_obj_add_style(ui->screen_SET_btn_RU_label, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_btn_RU_label, 40, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_RU_img = lv_img_create(ui->screen_SET_btn_RU);
	lv_img_set_src(ui->screen_SET_btn_RU_img, &_sel_checkbox_off_day_60_60x60);
	lv_img_set_pivot(ui->screen_SET_btn_RU_img, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_RU_img, 0);
	lv_obj_set_pos(ui->screen_SET_btn_RU_img, 1050, 27);
	lv_obj_set_size(ui->screen_SET_btn_RU_img, 60, 60);		

	//Write codes screen_SET_imgbtn_SP
	ui->screen_SET_btn_SP = lv_btn_create(ui->screen_SET_cont_setlanguage);
	ui->screen_SET_btn_SP_label = lv_label_create(ui->screen_SET_btn_SP);
	lv_label_set_text(ui->screen_SET_btn_SP_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_sp") );
	lv_label_set_long_mode(ui->screen_SET_btn_SP_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	// lv_obj_align(ui->screen_SET_btn_SP, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_SP, 0, LV_STATE_DEFAULT);
	// lv_obj_set_pos(ui->screen_SET_btn_SP, 0, 220);
	lv_obj_set_size(ui->screen_SET_btn_SP, 1200, 115);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_SP, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_remove_style_all(ui->screen_SET_btn_SP_label);
	lv_obj_add_style(ui->screen_SET_btn_SP_label, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_btn_SP_label, 40, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_SP_img = lv_img_create(ui->screen_SET_btn_SP);
	lv_img_set_src(ui->screen_SET_btn_SP_img, &_sel_checkbox_off_day_60_60x60);
	lv_img_set_pivot(ui->screen_SET_btn_SP_img, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_SP_img, 0);
	lv_obj_set_pos(ui->screen_SET_btn_SP_img, 1050, 27);
	lv_obj_set_size(ui->screen_SET_btn_SP_img, 60, 60);			

	//Write codes screen_SET_btn_POL
	ui->screen_SET_btn_POL = lv_btn_create(ui->screen_SET_cont_setlanguage);
	ui->screen_SET_btn_POL_label = lv_label_create(ui->screen_SET_btn_POL);
	lv_label_set_text(ui->screen_SET_btn_POL_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_pol") );
	lv_label_set_long_mode(ui->screen_SET_btn_POL_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	// lv_obj_align(ui->screen_SET_btn_SP, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_POL, 0, LV_STATE_DEFAULT);
	// lv_obj_set_pos(ui->screen_SET_btn_SP, 0, 220);
	lv_obj_set_size(ui->screen_SET_btn_POL, 1200, 115);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_POL, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_remove_style_all(ui->screen_SET_btn_POL_label);
	lv_obj_add_style(ui->screen_SET_btn_POL_label, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_btn_POL_label, 40, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_POL_img = lv_img_create(ui->screen_SET_btn_POL);
	lv_img_set_src(ui->screen_SET_btn_POL_img, &_sel_checkbox_off_day_60_60x60);
	lv_img_set_pivot(ui->screen_SET_btn_POL_img, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_POL_img, 0);
	lv_obj_set_pos(ui->screen_SET_btn_POL_img, 1050, 27);
	lv_obj_set_size(ui->screen_SET_btn_POL_img, 60, 60);		

	//Write codes screen_SET_btn_TU
	ui->screen_SET_btn_TU = lv_btn_create(ui->screen_SET_cont_setlanguage);
	ui->screen_SET_btn_TU_label = lv_label_create(ui->screen_SET_btn_TU);
	lv_label_set_text(ui->screen_SET_btn_TU_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_tu") );
	lv_label_set_long_mode(ui->screen_SET_btn_TU_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	// lv_obj_align(ui->screen_SET_btn_SP, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_TU, 0, LV_STATE_DEFAULT);
	// lv_obj_set_pos(ui->screen_SET_btn_SP, 0, 220);
	lv_obj_set_size(ui->screen_SET_btn_TU, 1200, 115); 
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_TU, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_remove_style_all(ui->screen_SET_btn_TU_label);
	lv_obj_add_style(ui->screen_SET_btn_TU_label, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_btn_TU_label, 40, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_TU_img = lv_img_create(ui->screen_SET_btn_TU);
	lv_img_set_src(ui->screen_SET_btn_TU_img, &_sel_checkbox_off_day_60_60x60);
	lv_img_set_pivot(ui->screen_SET_btn_TU_img, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_TU_img, 0);
	lv_obj_set_pos(ui->screen_SET_btn_TU_img, 1050, 27);
	lv_obj_set_size(ui->screen_SET_btn_TU_img, 60, 60);		

	//Write codes screen_SET_btn_KO
	ui->screen_SET_btn_KO = lv_btn_create(ui->screen_SET_cont_setlanguage);
	ui->screen_SET_btn_KO_label = lv_label_create(ui->screen_SET_btn_KO);
	lv_label_set_text(ui->screen_SET_btn_KO_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_ko") );
	lv_label_set_long_mode(ui->screen_SET_btn_KO_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	// lv_obj_align(ui->screen_SET_btn_SP, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_KO, 0, LV_STATE_DEFAULT);
	// lv_obj_set_pos(ui->screen_SET_btn_SP, 0, 220);
	lv_obj_set_size(ui->screen_SET_btn_KO, 1200, 115); 	
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_KO, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_remove_style_all(ui->screen_SET_btn_KO_label);
	lv_obj_add_style(ui->screen_SET_btn_KO_label, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_btn_KO_label, 40, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_KO_img = lv_img_create(ui->screen_SET_btn_KO);
	lv_img_set_src(ui->screen_SET_btn_KO_img, &_sel_checkbox_off_day_60_60x60);
	lv_img_set_pivot(ui->screen_SET_btn_KO_img, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_KO_img, 0);
	lv_obj_set_pos(ui->screen_SET_btn_KO_img, 1050, 27);
	lv_obj_set_size(ui->screen_SET_btn_KO_img, 60, 60);			

	//Write codes screen_SET_btn_DU_label
	ui->screen_SET_btn_DU = lv_btn_create(ui->screen_SET_cont_setlanguage);
	ui->screen_SET_btn_DU_label = lv_label_create(ui->screen_SET_btn_DU);
	lv_label_set_text(ui->screen_SET_btn_DU_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_du") );
	lv_label_set_long_mode(ui->screen_SET_btn_DU_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	// lv_obj_align(ui->screen_SET_btn_SP, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_DU, 0, LV_STATE_DEFAULT);
	// lv_obj_set_pos(ui->screen_SET_btn_SP, 0, 220);
	lv_obj_set_size(ui->screen_SET_btn_DU, 1200, 115); 	
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_DU, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_remove_style_all(ui->screen_SET_btn_DU_label);
	lv_obj_add_style(ui->screen_SET_btn_DU_label, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_btn_DU_label, 40, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_btn_DU_label, 40, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_DU_img = lv_img_create(ui->screen_SET_btn_DU);
	lv_img_set_src(ui->screen_SET_btn_DU_img, &_sel_checkbox_off_day_60_60x60);
	lv_img_set_pivot(ui->screen_SET_btn_DU_img, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_DU_img, 0);
	lv_obj_set_pos(ui->screen_SET_btn_DU_img, 1050, 27);
	lv_obj_set_size(ui->screen_SET_btn_DU_img, 60, 60);		

	ui->screen_SET_cont_setlanguage_img_lineX0 = lv_img_create(ui->screen_SET_cont_setlanguage);
	lv_obj_add_flag(ui->screen_SET_cont_setlanguage_img_lineX0, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX0, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_setlanguage_img_lineX0, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_setlanguage_img_lineX0, 0);
	lv_obj_set_size(ui->screen_SET_cont_setlanguage_img_lineX0, 1200, 5);

	ui->screen_SET_cont_setlanguage_img_lineX1 = lv_img_create(ui->screen_SET_cont_setlanguage);
	lv_obj_add_flag(ui->screen_SET_cont_setlanguage_img_lineX1, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX1, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_setlanguage_img_lineX1, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_setlanguage_img_lineX1, 0);
	lv_obj_set_size(ui->screen_SET_cont_setlanguage_img_lineX1, 1200, 5);

	ui->screen_SET_cont_setlanguage_img_lineX2 = lv_img_create(ui->screen_SET_cont_setlanguage);
	lv_obj_add_flag(ui->screen_SET_cont_setlanguage_img_lineX2, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX2, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_setlanguage_img_lineX2, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_setlanguage_img_lineX2, 0);
	lv_obj_set_size(ui->screen_SET_cont_setlanguage_img_lineX2, 1200, 5);

	ui->screen_SET_cont_setlanguage_img_lineX3 = lv_img_create(ui->screen_SET_cont_setlanguage);
	lv_obj_add_flag(ui->screen_SET_cont_setlanguage_img_lineX3, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX3, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_setlanguage_img_lineX3, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_setlanguage_img_lineX3, 0);
	lv_obj_set_size(ui->screen_SET_cont_setlanguage_img_lineX3, 1200, 5);

	ui->screen_SET_cont_setlanguage_img_lineX4 = lv_img_create(ui->screen_SET_cont_setlanguage);
	lv_obj_add_flag(ui->screen_SET_cont_setlanguage_img_lineX4, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX4, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_setlanguage_img_lineX4, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_setlanguage_img_lineX4, 0);
	lv_obj_set_size(ui->screen_SET_cont_setlanguage_img_lineX4, 1200, 5);

	ui->screen_SET_cont_setlanguage_img_lineX5 = lv_img_create(ui->screen_SET_cont_setlanguage);
	lv_obj_add_flag(ui->screen_SET_cont_setlanguage_img_lineX5, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX5, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_setlanguage_img_lineX5, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_setlanguage_img_lineX5, 0);
	lv_obj_set_size(ui->screen_SET_cont_setlanguage_img_lineX5, 1200, 5);

	ui->screen_SET_cont_setlanguage_img_lineX6 = lv_img_create(ui->screen_SET_cont_setlanguage);
	lv_obj_add_flag(ui->screen_SET_cont_setlanguage_img_lineX6, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX6, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_setlanguage_img_lineX6, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_setlanguage_img_lineX6, 0);
	lv_obj_set_size(ui->screen_SET_cont_setlanguage_img_lineX6, 1200, 5);

	ui->screen_SET_cont_setlanguage_img_lineX7 = lv_img_create(ui->screen_SET_cont_setlanguage);
	lv_obj_add_flag(ui->screen_SET_cont_setlanguage_img_lineX7, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX7, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_setlanguage_img_lineX7, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_setlanguage_img_lineX7, 0);
	lv_obj_set_size(ui->screen_SET_cont_setlanguage_img_lineX7, 1200, 5);

	ui->screen_SET_cont_setlanguage_img_lineX8 = lv_img_create(ui->screen_SET_cont_setlanguage);
	lv_obj_add_flag(ui->screen_SET_cont_setlanguage_img_lineX8, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX8, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_setlanguage_img_lineX8, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_setlanguage_img_lineX8, 0);
	lv_obj_set_size(ui->screen_SET_cont_setlanguage_img_lineX8, 1200, 5);

	ui->screen_SET_cont_setlanguage_img_lineX9 = lv_img_create(ui->screen_SET_cont_setlanguage);
	lv_obj_add_flag(ui->screen_SET_cont_setlanguage_img_lineX9, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX9, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_setlanguage_img_lineX9, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_setlanguage_img_lineX9, 0);
	lv_obj_set_size(ui->screen_SET_cont_setlanguage_img_lineX9, 1200, 5);

	ui->screen_SET_cont_setlanguage_img_lineX10 = lv_img_create(ui->screen_SET_cont_setlanguage);
	lv_obj_add_flag(ui->screen_SET_cont_setlanguage_img_lineX10, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX10, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_setlanguage_img_lineX10, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_setlanguage_img_lineX10, 0);
	lv_obj_set_size(ui->screen_SET_cont_setlanguage_img_lineX10, 1200, 5);

	ui->screen_SET_cont_setlanguage_img_lineX11 = lv_img_create(ui->screen_SET_cont_setlanguage);
	lv_obj_add_flag(ui->screen_SET_cont_setlanguage_img_lineX11, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX11, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_setlanguage_img_lineX11, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_setlanguage_img_lineX11, 0);
	lv_obj_set_size(ui->screen_SET_cont_setlanguage_img_lineX11, 1200, 5);

	ui->screen_SET_cont_setlanguage_img_lineX12 = lv_img_create(ui->screen_SET_cont_setlanguage);
	lv_obj_add_flag(ui->screen_SET_cont_setlanguage_img_lineX12, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX12, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_setlanguage_img_lineX12, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_setlanguage_img_lineX12, 0);
	lv_obj_set_size(ui->screen_SET_cont_setlanguage_img_lineX12, 1200, 5);

	ui->screen_SET_cont_setlanguage_img_lineX13 = lv_img_create(ui->screen_SET_cont_setlanguage);
	lv_obj_add_flag(ui->screen_SET_cont_setlanguage_img_lineX13, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX13, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_setlanguage_img_lineX13, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_setlanguage_img_lineX13, 0);
	lv_obj_set_size(ui->screen_SET_cont_setlanguage_img_lineX13, 1200, 5);

	lv_obj_move_to_index(ui->screen_SET_btn_ZH, 0);
	lv_obj_move_to_index(ui->screen_SET_cont_setlanguage_img_lineX0,1);
	lv_obj_move_to_index(ui->screen_SET_btn_EN, 2);
	lv_obj_move_to_index(ui->screen_SET_cont_setlanguage_img_lineX1,3);
	lv_obj_move_to_index(ui->screen_SET_btn_ZH_FT, 4);
	lv_obj_move_to_index(ui->screen_SET_cont_setlanguage_img_lineX2,5);
	lv_obj_move_to_index(ui->screen_SET_btn_FR, 6);
	lv_obj_move_to_index(ui->screen_SET_cont_setlanguage_img_lineX3,7);
	lv_obj_move_to_index(ui->screen_SET_btn_GE, 8);
	lv_obj_move_to_index(ui->screen_SET_cont_setlanguage_img_lineX4,9);
	lv_obj_move_to_index(ui->screen_SET_btn_IT, 10);
	lv_obj_move_to_index(ui->screen_SET_cont_setlanguage_img_lineX5,11);
	lv_obj_move_to_index(ui->screen_SET_btn_JP, 12);
	lv_obj_move_to_index(ui->screen_SET_cont_setlanguage_img_lineX6,13);
	lv_obj_move_to_index(ui->screen_SET_btn_POR, 14);
	lv_obj_move_to_index(ui->screen_SET_cont_setlanguage_img_lineX7,15);
	lv_obj_move_to_index(ui->screen_SET_btn_RU, 16);
	lv_obj_move_to_index(ui->screen_SET_cont_setlanguage_img_lineX8,17);
	lv_obj_move_to_index(ui->screen_SET_btn_SP, 18);
	lv_obj_move_to_index(ui->screen_SET_cont_setlanguage_img_lineX9,19);
	lv_obj_move_to_index(ui->screen_SET_btn_POL, 20);
	lv_obj_move_to_index(ui->screen_SET_cont_setlanguage_img_lineX10,21);
	lv_obj_move_to_index(ui->screen_SET_btn_TU, 22);
	lv_obj_move_to_index(ui->screen_SET_cont_setlanguage_img_lineX11,23);
	lv_obj_move_to_index(ui->screen_SET_btn_KO, 24);
	lv_obj_move_to_index(ui->screen_SET_cont_setlanguage_img_lineX12,25);
	lv_obj_move_to_index(ui->screen_SET_btn_DU, 26);
	lv_obj_move_to_index(ui->screen_SET_cont_setlanguage_img_lineX13,27);

	//Write codes screen_SET_cont_settime
	ui->screen_SET_cont_settime = lv_obj_create(ui->screen_SET);
	lv_obj_set_pos(ui->screen_SET_cont_settime, 240, 60);
	lv_obj_set_size(ui->screen_SET_cont_settime, 1200, 600);
	lv_obj_set_scrollbar_mode(ui->screen_SET_cont_settime, LV_SCROLLBAR_MODE_OFF);
	lv_obj_set_scroll_dir(ui->screen_SET_cont_settime,LV_DIR_TOP|LV_DIR_BOTTOM);
	lv_obj_add_flag(ui->screen_SET_cont_settime, LV_OBJ_FLAG_HIDDEN);
 
	//Write style for screen_SET_cont_settime, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_SET_cont_settime, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->screen_SET_cont_settime, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->screen_SET_cont_settime, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_side(ui->screen_SET_cont_settime, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_SET_cont_settime, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_SET_cont_settime, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_cont_settime, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_SET_cont_settime, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_SET_cont_settime, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_SET_cont_settime, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_SET_cont_settime, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_cont_settime_img_lineX0 = lv_img_create(ui->screen_SET_cont_settime);
	lv_obj_add_flag(ui->screen_SET_cont_settime_img_lineX0, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_settime_img_lineX0, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_settime_img_lineX0, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_settime_img_lineX0, 0);
	lv_obj_set_size(ui->screen_SET_cont_settime_img_lineX0, 1200, 5);
	lv_obj_set_pos(ui->screen_SET_cont_settime_img_lineX0, 0, 115);

	ui->screen_SET_cont_settime_img_lineX1 = lv_img_create(ui->screen_SET_cont_settime);
	lv_obj_add_flag(ui->screen_SET_cont_settime_img_lineX1, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_settime_img_lineX1, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_settime_img_lineX1, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_settime_img_lineX1, 0);
	lv_obj_set_size(ui->screen_SET_cont_settime_img_lineX1, 1200, 5);
	lv_obj_set_pos(ui->screen_SET_cont_settime_img_lineX1, 0, 235);

	ui->screen_SET_cont_settime_img_lineX2 = lv_img_create(ui->screen_SET_cont_settime);
	lv_obj_add_flag(ui->screen_SET_cont_settime_img_lineX2, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_settime_img_lineX2, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_settime_img_lineX2, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_settime_img_lineX2, 0);
	lv_obj_set_size(ui->screen_SET_cont_settime_img_lineX2, 1200, 5);
	lv_obj_set_pos(ui->screen_SET_cont_settime_img_lineX2, 0, 350);

	ui->screen_SET_cont_settime_img_lineX3 = lv_img_create(ui->screen_SET_cont_settime);
	lv_obj_add_flag(ui->screen_SET_cont_settime_img_lineX3, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_settime_img_lineX3, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_settime_img_lineX3, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_settime_img_lineX3, 0);
	lv_obj_set_size(ui->screen_SET_cont_settime_img_lineX3, 1200, 5);
	lv_obj_set_pos(ui->screen_SET_cont_settime_img_lineX3, 0, 475);	

	//Write codes screen_SET_label_year
	ui->screen_SET_label_year = lv_label_create(ui->screen_SET_cont_settime);
	lv_label_set_text(ui->screen_SET_label_year, get_string_for_language(g_sys_Data.current_language,"sys_txt_year") );
	lv_label_set_long_mode(ui->screen_SET_label_year, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_SET_label_year, 150, 0);
	lv_obj_set_size(ui->screen_SET_label_year, 150, 120);
	lv_obj_add_style(ui->screen_SET_label_year, &style_screen_SET_text_center_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_SET_label_month
	ui->screen_SET_label_month = lv_label_create(ui->screen_SET_cont_settime);
	lv_label_set_text(ui->screen_SET_label_month, get_string_for_language(g_sys_Data.current_language,"sys_txt_mon") );
	lv_label_set_long_mode(ui->screen_SET_label_month, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_SET_label_month, 350, 0);
	lv_obj_set_size(ui->screen_SET_label_month, 150, 120);
	lv_obj_add_style(ui->screen_SET_label_month, &style_screen_SET_text_center_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_SET_label_day
	ui->screen_SET_label_day = lv_label_create(ui->screen_SET_cont_settime);
	lv_label_set_text(ui->screen_SET_label_day, get_string_for_language(g_sys_Data.current_language,"sys_txt_today") );
	lv_label_set_long_mode(ui->screen_SET_label_day, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_SET_label_day, 550, 0);
	lv_obj_set_size(ui->screen_SET_label_day, 150, 120);
	lv_obj_add_style(ui->screen_SET_label_day, &style_screen_SET_text_center_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_SET_label_hour
	ui->screen_SET_label_hour = lv_label_create(ui->screen_SET_cont_settime);
	lv_label_set_text(ui->screen_SET_label_hour, get_string_for_language(g_sys_Data.current_language,"sys_txt_hour") );
	lv_label_set_long_mode(ui->screen_SET_label_hour, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_SET_label_hour, 750, 0);
	lv_obj_set_size(ui->screen_SET_label_hour, 150, 120);
	lv_obj_add_style(ui->screen_SET_label_hour, &style_screen_SET_text_center_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_SET_label_minute
	ui->screen_SET_label_minute = lv_label_create(ui->screen_SET_cont_settime);
	lv_label_set_text(ui->screen_SET_label_minute, get_string_for_language(g_sys_Data.current_language,"sys_txt_min") );
	lv_label_set_long_mode(ui->screen_SET_label_minute, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_SET_label_minute, 950, 0);
	lv_obj_set_size(ui->screen_SET_label_minute, 150, 120);
	lv_obj_add_style(ui->screen_SET_label_minute, &style_screen_SET_text_center_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);


	//Write codes screen_SET_img_left
	ui->screen_SET_cont_settime_img_left = lv_img_create(ui->screen_SET_cont_settime);
	lv_obj_add_flag(ui->screen_SET_cont_settime_img_left, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_settime_img_left, &_left_but_normal_day_60x75);
	lv_img_set_pivot(ui->screen_SET_cont_settime_img_left, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_settime_img_left, 0);
	lv_obj_set_pos(ui->screen_SET_cont_settime_img_left, 1100, 260);
	lv_obj_set_size(ui->screen_SET_cont_settime_img_left, 60, 75);

	//Write codes screen_SET_img_right
	ui->screen_SET_cont_settime_img_right = lv_img_create(ui->screen_SET_cont_settime);
	lv_obj_add_flag(ui->screen_SET_cont_settime_img_right, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_settime_img_right, &_right_but_normal_day_60x75);
	lv_img_set_pivot(ui->screen_SET_cont_settime_img_right, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_settime_img_right, 0);
	lv_obj_set_pos(ui->screen_SET_cont_settime_img_right, 50, 260);
	lv_obj_set_size(ui->screen_SET_cont_settime_img_right, 60, 75);

	//Write codes screen_SET_roller_year
	ui->screen_SET_roller_year = lv_roller_create(ui->screen_SET_cont_settime);
	lv_roller_set_options(ui->screen_SET_roller_year, CreateYearOptions(), LV_ROLLER_MODE_INFINITE);
	lv_obj_set_pos(ui->screen_SET_roller_year, 150, 120);
	lv_obj_set_width(ui->screen_SET_roller_year, 150);
 
	//Write style for screen_SET_roller_year, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_radius(ui->screen_SET_roller_year, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_SET_roller_year, 1, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_SET_roller_year, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->screen_SET_roller_year, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_SET_roller_year, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_SET_roller_year, &lv_font_harmonyOS_40, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_SET_roller_year, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_SET_roller_year, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_SET_roller_year, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->screen_SET_roller_year, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->screen_SET_roller_year, lv_color_hex(0xe6e6e6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_side(ui->screen_SET_roller_year, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_SET_roller_year, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_SET_roller_year, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_SET_roller_year, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_SET_roller_year, Part: LV_PART_SELECTED, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_SET_roller_year, 0, LV_PART_SELECTED|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_SET_roller_year, lv_color_hex(0xff0027), LV_PART_SELECTED|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_SET_roller_year, &lv_font_harmonyOS_40, LV_PART_SELECTED|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_SET_roller_year, 255, LV_PART_SELECTED|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_SET_roller_year, LV_TEXT_ALIGN_CENTER, LV_PART_SELECTED|LV_STATE_DEFAULT);
	
	lv_obj_set_style_text_line_space(ui->screen_SET_roller_year, 70, LV_PART_MAIN);
	lv_roller_set_visible_row_count(ui->screen_SET_roller_year, 3);
	//Write codes screen_SET_roller_month
	ui->screen_SET_roller_month = lv_roller_create(ui->screen_SET_cont_settime);
	lv_roller_set_options(ui->screen_SET_roller_month, CreateMonthOptions(), LV_ROLLER_MODE_INFINITE);
	lv_obj_set_pos(ui->screen_SET_roller_month, 350, 120);
	lv_obj_set_width(ui->screen_SET_roller_month, 150);
 
	//Write style for screen_SET_roller_month, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_radius(ui->screen_SET_roller_month, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_SET_roller_month, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_SET_roller_month, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_SET_roller_month, &lv_font_harmonyOS_40, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_SET_roller_month, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_SET_roller_month, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_SET_roller_month, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->screen_SET_roller_month, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->screen_SET_roller_month, lv_color_hex(0xe6e6e6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_side(ui->screen_SET_roller_month, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_SET_roller_month, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_SET_roller_month, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_SET_roller_month, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_SET_roller_month, Part: LV_PART_SELECTED, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_SET_roller_month, 3, LV_PART_SELECTED|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_SET_roller_month, lv_color_hex(0x2195f6), LV_PART_SELECTED|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->screen_SET_roller_month, LV_GRAD_DIR_NONE, LV_PART_SELECTED|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_SET_roller_month, lv_color_hex(0xff0027), LV_PART_SELECTED|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_SET_roller_month, &lv_font_harmonyOS_40, LV_PART_SELECTED|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_SET_roller_month, 255, LV_PART_SELECTED|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_SET_roller_month, LV_TEXT_ALIGN_CENTER, LV_PART_SELECTED|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(ui->screen_SET_roller_month, 70, LV_PART_MAIN);
	lv_roller_set_visible_row_count(ui->screen_SET_roller_month, 3);
	//Write codes screen_SET_roller_day
	ui->screen_SET_roller_day = lv_roller_create(ui->screen_SET_cont_settime);
	lv_roller_set_options(ui->screen_SET_roller_day, CreateDayOptions(), LV_ROLLER_MODE_INFINITE);
	lv_obj_set_pos(ui->screen_SET_roller_day, 550, 120);
	lv_obj_set_width(ui->screen_SET_roller_day, 150);
 
	//Write style for screen_SET_roller_day, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_radius(ui->screen_SET_roller_day, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_SET_roller_day, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_SET_roller_day, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_SET_roller_day, &lv_font_harmonyOS_40, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_SET_roller_day, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_SET_roller_day, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_SET_roller_day, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->screen_SET_roller_day, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->screen_SET_roller_day, lv_color_hex(0xe6e6e6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_side(ui->screen_SET_roller_day, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_SET_roller_day, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_SET_roller_day, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_SET_roller_day, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_SET_roller_day, Part: LV_PART_SELECTED, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_SET_roller_day, 0, LV_PART_SELECTED|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_SET_roller_day, lv_color_hex(0xff0027), LV_PART_SELECTED|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_SET_roller_day, &lv_font_harmonyOS_40, LV_PART_SELECTED|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_SET_roller_day, 255, LV_PART_SELECTED|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_SET_roller_day, LV_TEXT_ALIGN_CENTER, LV_PART_SELECTED|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(ui->screen_SET_roller_day, 70, LV_PART_MAIN);
	lv_roller_set_visible_row_count(ui->screen_SET_roller_day, 3);
	//Write codes screen_SET_roller_hour
	ui->screen_SET_roller_hour = lv_roller_create(ui->screen_SET_cont_settime);
	lv_roller_set_options(ui->screen_SET_roller_hour, CreateHourOptions(), LV_ROLLER_MODE_INFINITE);
	lv_obj_set_pos(ui->screen_SET_roller_hour, 750, 120);
	lv_obj_set_width(ui->screen_SET_roller_hour, 150);
 
	//Write style for screen_SET_roller_hour, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_radius(ui->screen_SET_roller_hour, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_SET_roller_hour, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_SET_roller_hour, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_SET_roller_hour, &lv_font_harmonyOS_40, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_SET_roller_hour, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_SET_roller_hour, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_SET_roller_hour, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->screen_SET_roller_hour, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->screen_SET_roller_hour, lv_color_hex(0xe6e6e6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_side(ui->screen_SET_roller_hour, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_SET_roller_hour, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_SET_roller_hour, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_SET_roller_hour, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_SET_roller_hour, Part: LV_PART_SELECTED, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_SET_roller_hour, 0, LV_PART_SELECTED|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_SET_roller_hour, lv_color_hex(0xff0027), LV_PART_SELECTED|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_SET_roller_hour, &lv_font_harmonyOS_40, LV_PART_SELECTED|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_SET_roller_hour, 255, LV_PART_SELECTED|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_SET_roller_hour, LV_TEXT_ALIGN_CENTER, LV_PART_SELECTED|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(ui->screen_SET_roller_hour, 70, LV_PART_MAIN);
	lv_roller_set_visible_row_count(ui->screen_SET_roller_hour, 3);
	//Write codes screen_SET_roller_minute
	ui->screen_SET_roller_minute = lv_roller_create(ui->screen_SET_cont_settime);
	lv_roller_set_options(ui->screen_SET_roller_minute, CreateMinuteOptions(), LV_ROLLER_MODE_INFINITE);
	lv_obj_set_pos(ui->screen_SET_roller_minute, 950, 120);
	lv_obj_set_width(ui->screen_SET_roller_minute, 150);
 
	//Write style for screen_SET_roller_minute, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_radius(ui->screen_SET_roller_minute, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_SET_roller_minute, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_SET_roller_minute, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_SET_roller_minute, &lv_font_harmonyOS_40, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_SET_roller_minute, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_SET_roller_minute, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_SET_roller_minute, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->screen_SET_roller_minute, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->screen_SET_roller_minute, lv_color_hex(0xe6e6e6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_side(ui->screen_SET_roller_minute, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_SET_roller_minute, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_SET_roller_minute, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_SET_roller_minute, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_SET_roller_minute, Part: LV_PART_SELECTED, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_SET_roller_minute, 0, LV_PART_SELECTED|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_SET_roller_minute, lv_color_hex(0xff0027), LV_PART_SELECTED|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_SET_roller_minute, &lv_font_harmonyOS_40, LV_PART_SELECTED|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_SET_roller_minute, 255, LV_PART_SELECTED|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_SET_roller_minute, LV_TEXT_ALIGN_CENTER, LV_PART_SELECTED|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(ui->screen_SET_roller_minute, 70, LV_PART_MAIN);
	lv_roller_set_visible_row_count(ui->screen_SET_roller_minute, 3);

	//Write codes screen_SET_cont_guanyu
	ui->screen_SET_cont_guanyu = lv_obj_create(ui->screen_SET);
	lv_obj_set_pos(ui->screen_SET_cont_guanyu, 240, 60);
	lv_obj_set_size(ui->screen_SET_cont_guanyu, 1200, 600);
	lv_obj_set_style_bg_opa(ui->screen_SET_cont_guanyu, 0 , LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_scrollbar_mode(ui->screen_SET_cont_guanyu, LV_SCROLLBAR_MODE_OFF);
	lv_obj_set_scroll_dir(ui->screen_SET_cont_guanyu,LV_DIR_TOP|LV_DIR_BOTTOM);
	lv_obj_add_flag(ui->screen_SET_cont_guanyu, LV_OBJ_FLAG_HIDDEN);

	lv_obj_set_style_border_opa(ui->screen_SET_cont_guanyu, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_flex_flow(ui->screen_SET_cont_guanyu, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(ui->screen_SET_cont_guanyu, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_row(ui->screen_SET_cont_guanyu, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_column(ui->screen_SET_cont_guanyu, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_gap(ui->screen_SET_cont_guanyu, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
 
	//wirte codes for screen_SET_label_devModel
	ui->screen_SET_btn_devModel = lv_btn_create(ui->screen_SET_cont_guanyu);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_devModel, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	ui->screen_SET_label_devModel = lv_label_create(ui->screen_SET_btn_devModel);
	ui->screen_SET_label_devModel1 = lv_label_create(ui->screen_SET_btn_devModel);
	lv_obj_set_size(ui->screen_SET_btn_devModel,1200,120);
	lv_label_set_text(ui->screen_SET_label_devModel,get_string_for_language(g_sys_Data.current_language,"sys_txt_devModel") );
	lv_label_set_text(ui->screen_SET_label_devModel1, DEVICE_MODEL);
	lv_label_set_long_mode(ui->screen_SET_label_devModel, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_label_set_long_mode(ui->screen_SET_label_devModel1, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_devModel, 0, LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_devModel, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	lv_obj_remove_style_all(ui->screen_SET_label_devModel);
	lv_obj_add_style(ui->screen_SET_label_devModel, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_label_devModel, 40, LV_PART_MAIN|LV_STATE_DEFAULT);

	lv_obj_remove_style_all(ui->screen_SET_label_devModel1);
	lv_obj_set_pos(ui->screen_SET_label_devModel1, 450, 0);
	lv_obj_set_size(ui->screen_SET_label_devModel1, 600,120);
	lv_obj_add_style(ui->screen_SET_label_devModel1, &style_screen_SET_text_right_align_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT); 
	lv_obj_set_style_pad_top(ui->screen_SET_label_devModel, 40, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_SET_label_devModel, 30, LV_PART_MAIN|LV_STATE_DEFAULT);

	//wirte codes for screen_SET_label_devNum
	ui->screen_SET_btn_devNum = lv_btn_create(ui->screen_SET_cont_guanyu);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_devNum, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	ui->screen_SET_label_devNum = lv_label_create(ui->screen_SET_btn_devNum);
	ui->screen_SET_label_devNum1 = lv_label_create(ui->screen_SET_btn_devNum);
	lv_obj_set_size(ui->screen_SET_btn_devNum,1200,120);
	lv_label_set_text(ui->screen_SET_label_devNum,get_string_for_language(g_sys_Data.current_language,"sys_txt_devSerNum") );
	lv_label_set_text(ui->screen_SET_label_devNum1, DEVICE_SERIAL_NUMBER);
	lv_label_set_long_mode(ui->screen_SET_label_devNum, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_label_set_long_mode(ui->screen_SET_label_devNum1, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_devNum, 0, LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_devNum, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	lv_obj_remove_style_all(ui->screen_SET_label_devNum);
	lv_obj_add_style(ui->screen_SET_label_devNum, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_label_devNum, 40, LV_PART_MAIN|LV_STATE_DEFAULT);

	lv_obj_remove_style_all(ui->screen_SET_label_devNum1);
	lv_obj_set_pos(ui->screen_SET_label_devNum1, 450, 0);
	lv_obj_set_size(ui->screen_SET_label_devNum1, 600,120);
	lv_obj_add_style(ui->screen_SET_label_devNum1, &style_screen_SET_text_right_align_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_label_devNum1, 40, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_SET_label_devNum1, 30, LV_PART_MAIN|LV_STATE_DEFAULT);
 
	//wirte codes for screen_SET_label_appVersion1
	ui->screen_SET_btn_appVersion = lv_btn_create(ui->screen_SET_cont_guanyu);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_appVersion, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	ui->screen_SET_label_appVersion = lv_label_create(ui->screen_SET_btn_appVersion);
	ui->screen_SET_btn_appVersion_label = lv_label_create(ui->screen_SET_btn_appVersion);
	lv_obj_set_size(ui->screen_SET_btn_appVersion,1200,120);
	lv_label_set_text(ui->screen_SET_label_appVersion,get_string_for_language(g_sys_Data.current_language,"sys_txt_appVersion") );
	lv_label_set_text(ui->screen_SET_btn_appVersion_label, APP_VERSION);
	lv_label_set_long_mode(ui->screen_SET_label_appVersion, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_label_set_long_mode(ui->screen_SET_btn_appVersion_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_appVersion, 0, LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_appVersion, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	lv_obj_remove_style_all(ui->screen_SET_label_appVersion);
	lv_obj_add_style(ui->screen_SET_label_appVersion, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_label_appVersion, 40, LV_PART_MAIN|LV_STATE_DEFAULT);

	lv_obj_remove_style_all(ui->screen_SET_btn_appVersion_label);
	lv_obj_set_pos(ui->screen_SET_btn_appVersion_label, 450, 0);
	lv_obj_set_size(ui->screen_SET_btn_appVersion_label, 600,120);
	lv_obj_add_style(ui->screen_SET_btn_appVersion_label, &style_screen_SET_text_right_align_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_btn_appVersion_label, 40, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_SET_btn_appVersion_label, 30, LV_PART_MAIN|LV_STATE_DEFAULT);

	//wirte codes for screen_SET_label_hwVersion
	ui->screen_SET_btn_hwVersion = lv_btn_create(ui->screen_SET_cont_guanyu);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_hwVersion, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	ui->screen_SET_label_hwVersion = lv_label_create(ui->screen_SET_btn_hwVersion);
	lv_label_set_text(ui->screen_SET_label_hwVersion,get_string_for_language(g_sys_Data.current_language,"sys_txt_hwVersion"));
	ui->screen_SET_label_hwVersion1 = lv_label_create(ui->screen_SET_btn_hwVersion);
	lv_label_set_text(ui->screen_SET_label_hwVersion1, HARDWARE_VERSION);
	lv_obj_set_size(ui->screen_SET_btn_hwVersion,1200,120);
	lv_label_set_long_mode(ui->screen_SET_label_hwVersion, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_label_set_long_mode(ui->screen_SET_label_hwVersion1, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_hwVersion, 0, LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_hwVersion, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	lv_obj_remove_style_all(ui->screen_SET_label_hwVersion);
	lv_obj_add_style(ui->screen_SET_label_hwVersion, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_label_hwVersion, 40, LV_PART_MAIN|LV_STATE_DEFAULT);

	lv_obj_remove_style_all(ui->screen_SET_label_hwVersion1);
	lv_obj_set_pos(ui->screen_SET_label_hwVersion1, 450, 0);
	lv_obj_set_size(ui->screen_SET_label_hwVersion1, 600,120);
	lv_obj_add_style(ui->screen_SET_label_hwVersion1, &style_screen_SET_text_right_align_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_label_hwVersion1, 40, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_SET_label_hwVersion1, 30, LV_PART_MAIN|LV_STATE_DEFAULT);

	//wirte codes for screen_SET_btn_otaUpdata
	ui->screen_SET_btn_otaUpdata = lv_btn_create(ui->screen_SET_cont_guanyu);
	ui->screen_SET_btn_otaUpdata_label = lv_label_create(ui->screen_SET_btn_otaUpdata);
	lv_obj_set_size(ui->screen_SET_btn_otaUpdata,1200,120);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_otaUpdata,0,LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_label_set_text(ui->screen_SET_btn_otaUpdata_label,get_string_for_language(g_sys_Data.current_language,"sys_txt_otaUpdata"));
	lv_obj_set_style_pad_all(ui->screen_SET_btn_otaUpdata,0,LV_PART_MAIN);
	lv_label_set_long_mode(ui->screen_SET_btn_otaUpdata_label, LV_LABEL_LONG_SCROLL_CIRCULAR);

	lv_obj_remove_style_all(ui->screen_SET_btn_otaUpdata_label);
	lv_obj_add_style(ui->screen_SET_btn_otaUpdata_label, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_btn_otaUpdata_label, 40, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_otaUpdata_img = lv_img_create(ui->screen_SET_btn_otaUpdata);
	lv_img_set_src(ui->screen_SET_btn_otaUpdata_img, &_right_but_normal_day_60x75);
	lv_img_set_pivot(ui->screen_SET_btn_otaUpdata_img, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_otaUpdata_img, 0);
	lv_obj_set_pos(ui->screen_SET_btn_otaUpdata_img, 1050, 27);
	lv_obj_set_size(ui->screen_SET_btn_otaUpdata_img, 60, 75);	

	ui->screen_SET_cont_guanyu_img_lineX0 = lv_img_create(ui->screen_SET_cont_guanyu);
	lv_obj_add_flag(ui->screen_SET_cont_guanyu_img_lineX0, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_guanyu_img_lineX0, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_guanyu_img_lineX0, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_guanyu_img_lineX0, 0);
	lv_obj_set_size(ui->screen_SET_cont_guanyu_img_lineX0, 1200, 5);

	ui->screen_SET_cont_guanyu_img_lineX1 = lv_img_create(ui->screen_SET_cont_guanyu);
	lv_obj_add_flag(ui->screen_SET_cont_guanyu_img_lineX1, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_guanyu_img_lineX1, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_guanyu_img_lineX1, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_guanyu_img_lineX1, 0);
	lv_obj_set_size(ui->screen_SET_cont_guanyu_img_lineX1, 1200, 5);

	ui->screen_SET_cont_guanyu_img_lineX2 = lv_img_create(ui->screen_SET_cont_guanyu);
	lv_obj_add_flag(ui->screen_SET_cont_guanyu_img_lineX2, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_guanyu_img_lineX2, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_guanyu_img_lineX2, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_guanyu_img_lineX2, 0);
	lv_obj_set_size(ui->screen_SET_cont_guanyu_img_lineX2, 1200, 5);

	ui->screen_SET_cont_guanyu_img_lineX3 = lv_img_create(ui->screen_SET_cont_guanyu);
	lv_obj_add_flag(ui->screen_SET_cont_guanyu_img_lineX3, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_guanyu_img_lineX3, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_guanyu_img_lineX3, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_guanyu_img_lineX3, 0);
	lv_obj_set_size(ui->screen_SET_cont_guanyu_img_lineX3, 1200, 5);

	ui->screen_SET_cont_guanyu_img_lineX4 = lv_img_create(ui->screen_SET_cont_guanyu);
	lv_obj_add_flag(ui->screen_SET_cont_guanyu_img_lineX4, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_guanyu_img_lineX4, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_guanyu_img_lineX4, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_guanyu_img_lineX4, 0);
	lv_obj_set_size(ui->screen_SET_cont_guanyu_img_lineX4, 1200, 5);

	lv_obj_move_to_index(ui->screen_SET_btn_devModel,0);
	lv_obj_move_to_index(ui->screen_SET_cont_guanyu_img_lineX0,1);
	lv_obj_move_to_index(ui->screen_SET_btn_devNum,2);
	lv_obj_move_to_index(ui->screen_SET_cont_guanyu_img_lineX1,3);
	lv_obj_move_to_index(ui->screen_SET_btn_appVersion,4);
	lv_obj_move_to_index(ui->screen_SET_cont_guanyu_img_lineX2,5);
	lv_obj_move_to_index(ui->screen_SET_btn_hwVersion,6);
	lv_obj_move_to_index(ui->screen_SET_cont_guanyu_img_lineX3,7);
	lv_obj_move_to_index(ui->screen_SET_btn_otaUpdata,8);
	lv_obj_move_to_index(ui->screen_SET_cont_guanyu_img_lineX4,9);

	create_bootLogo_cont(ui);
	create_reboot_cont(ui);
	create_resetFactory_cont(ui);
	create_aging_mode_cont(ui);

	ui->screen_SET_label_popup = lv_label_create(ui->screen_SET);
	lv_obj_set_size(ui->screen_SET_label_popup, 500,60);
	lv_label_set_text(ui->screen_SET_label_popup, get_string_for_language(g_sys_Data.current_language,"sys_txt_invalid"));
	lv_obj_align(ui->screen_SET_label_popup, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_bg_opa(ui->screen_SET_label_popup, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_SET_label_popup, lv_color_hex(0x929292), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_SET_label_popup, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_SET_label_popup, &lv_font_harmonyOS_45, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_SET_label_popup, 255, LV_PART_MAIN|LV_STATE_DEFAULT);	
	lv_obj_set_style_text_align(ui->screen_SET_label_popup, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_label_set_text(guider_ui.screen_SET_label_popup, NULL);
	lv_obj_add_flag(ui->screen_SET_label_popup, LV_OBJ_FLAG_HIDDEN);

//--------------------------------
	TTime time;
	memset(&time, 0, sizeof(TTime));
	RtcGetTime2C(&time);
	printf("%d-%d-%d  %d:%d\n",time.nYear,time.nMonth,time.nDay,time.nHour,time.nMinute);
	lv_roller_set_selected(ui->screen_SET_roller_year, time.nYear - 1980, LV_ANIM_OFF);
	lv_roller_set_selected(ui->screen_SET_roller_month, time.nMonth-1, LV_ANIM_OFF);
	lv_roller_set_selected(ui->screen_SET_roller_day, time.nDay-1, LV_ANIM_OFF);
	lv_roller_set_selected(ui->screen_SET_roller_hour, time.nHour, LV_ANIM_OFF);
	lv_roller_set_selected(ui->screen_SET_roller_minute, time.nMinute, LV_ANIM_OFF);
	printf("%d-%d-%d  %d:%d\n",lv_roller_get_selected(ui->screen_SET_roller_year),lv_roller_get_selected(ui->screen_SET_roller_month),lv_roller_get_selected(ui->screen_SET_roller_day),lv_roller_get_selected(ui->screen_SET_roller_hour),lv_roller_get_selected(ui->screen_SET_roller_minute));
//--------------------------------
 
	//Update current screen layout.
	lv_obj_update_layout(ui->screen_SET);

	//Init events for screen.
	events_init_screen_SET(ui);
	THEME_MODE mode = g_sys_Data.themeMode;
	if(mode == THEME_DAY || (mode == THEME_AUTO && g_sys_Data.gpadcVol < 1300)){
		ScreenSetBlackNightMode(ui, THEME_DAY);
	}else{
		ScreenSetBlackNightMode(ui, THEME_DARK);
	}
	languageItemStatu(g_sys_Data.current_language);
}

//-------------------------开机logo ui-----------------------------------------------
// 图片信息结构
typedef struct {
    char path[256];  // 图片完整路径
    lv_obj_t *btn;   // 对应的按钮对象
} image_info_t;

static image_info_t *g_btn_list = NULL;
static int g_image_count = 0;
#define MAX_ROW 100
static int scan_image_files(const char *dir_path) {
    DIR *dir;
    struct dirent *entry;
    int count = 0;
    
    // 第一次扫描，统计文件数量
    dir = opendir(dir_path);
    if (dir == NULL) {
        LV_LOG_ERROR("Failed to open directory: %s", dir_path);
        return 0;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        char *ext = strrchr(entry->d_name, '.');
        if (ext != NULL && strcasecmp(ext, ".jpg") == 0) {
            count++;
        }
    }
    closedir(dir);
    
    if (count == 0) {
        return 0;
    }
    
    // 分配内存存储图片信息
    g_btn_list = (image_info_t *)malloc(count * sizeof(image_info_t));
    if (g_btn_list == NULL) {
        return 0;
    }
    
    memset(g_btn_list, 0, count * sizeof(image_info_t));
    
    // 第二次扫描，保存文件信息
    dir = opendir(dir_path);
    int index = 0;
    while ((entry = readdir(dir)) != NULL && index < count) {
        char *ext = strrchr(entry->d_name, '.');
        if (ext != NULL && strcasecmp(ext, ".jpg") == 0) {
            snprintf(g_btn_list[index].path, sizeof(g_btn_list[index].path), 
                    "%s/%s", dir_path, entry->d_name);
            index++;
        }
    }
    closedir(dir);
    
    g_image_count = index;
    return index;
}

static void btn_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    
    if (code == LV_EVENT_CLICKED) {
        // 在所有图片中查找对应的按钮
        for (int i = 0; i < g_image_count; i++) {
            if (g_btn_list[i].btn == btn) {
                // 调用更换开机logo的函数
                bootlogoUp(g_btn_list[i].path);
            }else if(g_btn_list[i].btn != btn){
				lv_obj_clear_state(g_btn_list[i].btn, LV_STATE_CHECKED);
			}

        }
    }
}

static void create_bootlogoBtn_cont(lv_ui *ui){
	ui->screen_SET_cont_externalBootlogo = lv_obj_create(ui->screen_SET_cont_bootlogo);
	lv_obj_set_pos(ui->screen_SET_cont_externalBootlogo, 100, 240);
	lv_obj_set_size(ui->screen_SET_cont_externalBootlogo, 700, 360);
	lv_obj_set_style_bg_opa(ui->screen_SET_cont_externalBootlogo, 0 , LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_scrollbar_mode(ui->screen_SET_cont_externalBootlogo, LV_SCROLLBAR_MODE_ACTIVE);
	lv_obj_set_scroll_dir(ui->screen_SET_cont_externalBootlogo,LV_DIR_VER);
	lv_obj_set_style_border_opa(ui->screen_SET_cont_externalBootlogo, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_flag(ui->screen_SET_cont_externalBootlogo, LV_OBJ_FLAG_HIDDEN);	

	ui->screen_SET_btn_externalBootlogo = lv_btn_create(ui->screen_SET_cont_externalBootlogo);
	lv_obj_set_pos(ui->screen_SET_btn_externalBootlogo, 0, 0);
	lv_obj_set_size(ui->screen_SET_btn_externalBootlogo,300,150);
	// lv_obj_set_style_bg_img_src(ui->screen_SET_btn_externalBootlogo, "S:/opt/work/app/logo/bootlogo.jpg", LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_SET_btn_externalBootlogo, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->screen_SET_btn_externalBootlogo, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_SET_btn_externalBootlogo, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->screen_SET_btn_externalBootlogo, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->screen_SET_btn_externalBootlogo, lv_color_hex(0x00ff00), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_add_flag(ui->screen_SET_btn_externalBootlogo, LV_OBJ_FLAG_CHECKABLE);

	ui->screen_SET_cont_internalBootlogo = lv_obj_create(ui->screen_SET_cont_bootlogo);
	lv_obj_set_pos(ui->screen_SET_cont_internalBootlogo, 100, 240);
	lv_obj_set_size(ui->screen_SET_cont_internalBootlogo, 700, 360);
	lv_obj_set_style_bg_opa(ui->screen_SET_cont_internalBootlogo, 0 , LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->screen_SET_cont_internalBootlogo, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(ui->screen_SET_cont_internalBootlogo, 10, 0);
    lv_obj_set_style_pad_column(ui->screen_SET_cont_internalBootlogo, 10, 0);
    lv_obj_set_scroll_dir(ui->screen_SET_cont_internalBootlogo, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(ui->screen_SET_cont_internalBootlogo, LV_SCROLLBAR_MODE_ACTIVE);
    lv_obj_set_layout(ui->screen_SET_cont_internalBootlogo, LV_LAYOUT_GRID);
	// lv_obj_add_flag(ui->screen_SET_cont_internalBootlogo, LV_OBJ_FLAG_HIDDEN);
	
	int count = scan_image_files(INTERN_LOGO_PATH);
	if(count > MAX_ROW) count = MAX_ROW;
    // 网格配置
    static lv_coord_t col_dsc[] = {300, 300, LV_GRID_TEMPLATE_LAST};
    int rows = (count + 1) / 2;
    static lv_coord_t row_dsc[MAX_ROW + 1];
    for (int i = 0; i < rows; i++) {
        row_dsc[i] = 150;
    }
    row_dsc[rows] = LV_GRID_TEMPLATE_LAST;
    
    lv_obj_set_grid_dsc_array(ui->screen_SET_cont_internalBootlogo, col_dsc, row_dsc);
	
	for(int i = 0; i < count; i++){
        int col = i % 2;
        int row = i / 2;
		lv_obj_t * btn = lv_btn_create(ui->screen_SET_cont_internalBootlogo);
    	lv_obj_set_size(btn, 300, 150);
		char str[256] = {};
		snprintf(str, sizeof(str), "S:%s", g_btn_list[i].path);
		printf("%s\n",str);
		lv_obj_set_style_bg_img_src(btn, str, LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_opa(btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_radius(btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_border_opa(btn, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_border_width(btn, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_border_color(btn, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_border_color(btn, lv_color_hex(0x00ff00), LV_PART_MAIN|LV_STATE_CHECKED);
		lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
        g_btn_list[i].btn = btn;
        lv_obj_set_grid_cell(btn, LV_GRID_ALIGN_CENTER, col, 1, LV_GRID_ALIGN_CENTER, row, 1);

		lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, ui);
	}
}


static void create_bootLogo_cont(lv_ui *ui){
	ui->screen_SET_cont_bootlogo = lv_obj_create(ui->screen_SET);
	lv_obj_set_pos(ui->screen_SET_cont_bootlogo, 240, 60);
	lv_obj_set_size(ui->screen_SET_cont_bootlogo, 1200, 600);
	lv_obj_set_style_bg_opa(ui->screen_SET_cont_bootlogo, 0 , LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_scrollbar_mode(ui->screen_SET_cont_bootlogo, LV_SCROLLBAR_MODE_OFF);
	lv_obj_set_scroll_dir(ui->screen_SET_cont_bootlogo,LV_DIR_NONE);
	lv_obj_add_flag(ui->screen_SET_cont_bootlogo, LV_OBJ_FLAG_HIDDEN);
	lv_obj_set_style_border_opa(ui->screen_SET_cont_bootlogo, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//wirte codes for screen_SET_label_hwVersion
	ui->screen_SET_btn_bootlogo_path = lv_btn_create(ui->screen_SET_cont_bootlogo);
	ui->screen_SET_btn_bootlogo_path_label = lv_label_create(ui->screen_SET_btn_bootlogo_path);
	ui->screen_SET_btn_bootlogo_path_label1 = lv_label_create(ui->screen_SET_btn_bootlogo_path);
	lv_label_set_text(ui->screen_SET_btn_bootlogo_path_label,get_string_for_language(g_sys_Data.current_language,"sys_txt_choose_path"));
	lv_label_set_text(ui->screen_SET_btn_bootlogo_path_label1, get_string_for_language(g_sys_Data.current_language,"sys_txt_internal_path"));
	lv_obj_set_pos(ui->screen_SET_btn_bootlogo_path, 0, 0);
	lv_obj_set_size(ui->screen_SET_btn_bootlogo_path,1200,120);
	lv_label_set_long_mode(ui->screen_SET_btn_bootlogo_path_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_label_set_long_mode(ui->screen_SET_btn_bootlogo_path_label1, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_bootlogo_path, 0, LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_bootlogo_path, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	lv_obj_remove_style_all(ui->screen_SET_btn_bootlogo_path_label);
	lv_obj_add_style(ui->screen_SET_btn_bootlogo_path_label, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_btn_bootlogo_path_label, 40, LV_PART_MAIN|LV_STATE_DEFAULT);

	lv_obj_remove_style_all(ui->screen_SET_btn_bootlogo_path_label1);
	lv_obj_set_pos(ui->screen_SET_btn_bootlogo_path_label1, 750, 0);
	lv_obj_set_size(ui->screen_SET_btn_bootlogo_path_label1, 300,120);
	lv_obj_add_style(ui->screen_SET_btn_bootlogo_path_label1, &style_screen_SET_text_right_align_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);	

	ui->screen_SET_btn_bootlogo_path_img = lv_img_create(ui->screen_SET_btn_bootlogo_path);
	lv_img_set_src(ui->screen_SET_btn_bootlogo_path_img, &_right_but_normal_day_60x75);
	lv_img_set_pivot(ui->screen_SET_btn_bootlogo_path_img, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_bootlogo_path_img, 0);
	lv_obj_set_pos(ui->screen_SET_btn_bootlogo_path_img, 1050, 12);
	lv_obj_set_size(ui->screen_SET_btn_bootlogo_path_img, 60, 75);	

	ui->screen_SET_cont_bootlogo_img_lineX0 = lv_img_create(ui->screen_SET_cont_bootlogo);
	lv_obj_add_flag(ui->screen_SET_cont_bootlogo_img_lineX0, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_cont_bootlogo_img_lineX0, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_cont_bootlogo_img_lineX0, 50,50);
	lv_img_set_angle(ui->screen_SET_cont_bootlogo_img_lineX0, 0);
	lv_obj_set_pos(ui->screen_SET_cont_bootlogo_img_lineX0, 0, 115);
	lv_obj_set_size(ui->screen_SET_cont_bootlogo_img_lineX0, 1200, 5);

	ui->screen_SET_label_bootlogo_choose = lv_label_create(ui->screen_SET_cont_bootlogo);
	lv_obj_set_pos(ui->screen_SET_label_bootlogo_choose, 0, 120);
	lv_obj_set_size(ui->screen_SET_label_bootlogo_choose, 500,120);	
	lv_label_set_text(ui->screen_SET_label_bootlogo_choose,get_string_for_language(g_sys_Data.current_language,"sys_txt_choose_logo"));
	lv_obj_add_style(ui->screen_SET_label_bootlogo_choose, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_SET_label_bootlogo_choose, 20, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_label_bootlogo_choose, 40, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_SET_label_bootlogo_choose, 50, LV_PART_MAIN|LV_STATE_DEFAULT);

	create_bootlogoBtn_cont(ui);
}
//----------------------------------------------------------------------------
static void create_reboot_cont(lv_ui *ui){
	ui->screen_SET_cont_reboot = lv_obj_create(ui->screen_SET);
	lv_obj_set_size(ui->screen_SET_cont_reboot, 600, 300);
	lv_obj_set_style_bg_opa(ui->screen_SET_cont_reboot, 255 , LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_SET_cont_reboot, lv_color_hex(0xe9fafe) , LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_scrollbar_mode(ui->screen_SET_cont_reboot, LV_SCROLLBAR_MODE_OFF);
	lv_obj_set_scroll_dir(ui->screen_SET_cont_reboot,LV_DIR_NONE);
	lv_obj_add_flag(ui->screen_SET_cont_reboot, LV_OBJ_FLAG_HIDDEN);
	lv_obj_set_style_border_opa(ui->screen_SET_cont_reboot, 0, LV_PART_MAIN|LV_STATE_DEFAULT);	
	lv_obj_center(ui->screen_SET_cont_reboot);	

	ui->screen_SET_label_reboot_text = lv_label_create(ui->screen_SET_cont_reboot);
	lv_obj_set_size(ui->screen_SET_label_reboot_text, 600, 80);
	lv_obj_set_pos(ui->screen_SET_label_reboot_text, 0, 50);
	lv_label_set_long_mode(ui->screen_SET_label_reboot_text, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_label_set_text(ui->screen_SET_label_reboot_text,get_string_for_language(g_sys_Data.current_language,"sys_txt_rebootSys_label"));
	lv_obj_set_style_text_color(ui->screen_SET_label_reboot_text, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_SET_label_reboot_text, &lv_font_harmonyOS_40, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_SET_label_reboot_text, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_SET_label_reboot_text, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_reboot_confirm = lv_btn_create(ui->screen_SET_cont_reboot);
	ui->screen_SET_btn_reboot_confirm_label = lv_label_create(ui->screen_SET_btn_reboot_confirm);
	lv_obj_set_size(ui->screen_SET_btn_reboot_confirm, 265, 60);
	lv_obj_set_pos(ui->screen_SET_btn_reboot_confirm, 10, 150);
	lv_label_set_long_mode(ui->screen_SET_btn_reboot_confirm_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_label_set_text(ui->screen_SET_btn_reboot_confirm_label,get_string_for_language(g_sys_Data.current_language,"sys_txt_confirm"));
	lv_obj_set_style_text_color(ui->screen_SET_btn_reboot_confirm_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_SET_btn_reboot_confirm_label, &lv_font_harmonyOS_40, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_reboot_confirm, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_SET_btn_reboot_confirm, lv_color_hex(0x7d7d7d), LV_PART_MAIN|LV_STATE_DEFAULT);	
	lv_obj_set_style_bg_color(ui->screen_SET_btn_reboot_confirm, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_PRESSED);	
	lv_obj_set_style_radius(ui->screen_SET_btn_reboot_confirm, 30, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_SET_btn_reboot_confirm_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_center(ui->screen_SET_btn_reboot_confirm_label);	

	ui->screen_SET_btn_reboot_cancel = lv_btn_create(ui->screen_SET_cont_reboot);
	ui->screen_SET_btn_reboot_cancel_label = lv_label_create(ui->screen_SET_btn_reboot_cancel);
	lv_obj_set_size(ui->screen_SET_btn_reboot_cancel, 265, 60);
	lv_obj_set_pos(ui->screen_SET_btn_reboot_cancel, 285, 150);
	lv_label_set_long_mode(ui->screen_SET_btn_reboot_cancel_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_label_set_text(ui->screen_SET_btn_reboot_cancel_label,get_string_for_language(g_sys_Data.current_language,"sys_txt_cancel"));
	lv_obj_set_style_text_color(ui->screen_SET_btn_reboot_cancel_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_SET_btn_reboot_cancel_label, &lv_font_harmonyOS_40, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_reboot_cancel, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_SET_btn_reboot_cancel, lv_color_hex(0x7d7d7d), LV_PART_MAIN|LV_STATE_DEFAULT);	
	lv_obj_set_style_bg_color(ui->screen_SET_btn_reboot_cancel, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_PRESSED);	
	lv_obj_set_style_radius(ui->screen_SET_btn_reboot_cancel, 30, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_SET_btn_reboot_cancel_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_center(ui->screen_SET_btn_reboot_cancel_label);
}

static void create_resetFactory_cont(lv_ui *ui){
	ui->screen_SET_cont_resetFactory = lv_obj_create(ui->screen_SET);
	lv_obj_set_size(ui->screen_SET_cont_resetFactory, 600, 300);
	lv_obj_set_style_bg_opa(ui->screen_SET_cont_resetFactory, 255 , LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_SET_cont_resetFactory, lv_color_hex(0xe9fafe) , LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_scrollbar_mode(ui->screen_SET_cont_resetFactory, LV_SCROLLBAR_MODE_OFF);
	lv_obj_set_scroll_dir(ui->screen_SET_cont_resetFactory,LV_DIR_NONE);
	lv_obj_add_flag(ui->screen_SET_cont_resetFactory, LV_OBJ_FLAG_HIDDEN);
	lv_obj_set_style_border_opa(ui->screen_SET_cont_resetFactory, 0, LV_PART_MAIN|LV_STATE_DEFAULT);	
	lv_obj_center(ui->screen_SET_cont_resetFactory);

	ui->screen_SET_label_resetFactory_text = lv_label_create(ui->screen_SET_cont_resetFactory);
	lv_obj_set_size(ui->screen_SET_label_resetFactory_text, 600, 80);
	lv_obj_set_pos(ui->screen_SET_label_resetFactory_text, 0, 50);
	lv_label_set_long_mode(ui->screen_SET_label_resetFactory_text, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_label_set_text(ui->screen_SET_label_resetFactory_text,get_string_for_language(g_sys_Data.current_language,"sys_txt_reFactory_label"));
	lv_obj_set_style_text_color(ui->screen_SET_label_resetFactory_text, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_SET_label_resetFactory_text, &lv_font_harmonyOS_40, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_SET_label_resetFactory_text, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_SET_label_resetFactory_text, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_resetFactory_confirm = lv_btn_create(ui->screen_SET_cont_resetFactory);
	ui->screen_SET_btn_resetFactory_confirm_label = lv_label_create(ui->screen_SET_btn_resetFactory_confirm);
	lv_obj_set_size(ui->screen_SET_btn_resetFactory_confirm, 265, 60);
	lv_obj_set_pos(ui->screen_SET_btn_resetFactory_confirm, 10, 150);
	lv_label_set_long_mode(ui->screen_SET_btn_resetFactory_confirm_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_label_set_text(ui->screen_SET_btn_resetFactory_confirm_label, get_string_for_language(g_sys_Data.current_language,"sys_txt_confirm"));
	lv_obj_set_style_text_color(ui->screen_SET_btn_resetFactory_confirm_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_SET_btn_resetFactory_confirm_label, &lv_font_harmonyOS_40, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_resetFactory_confirm, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_SET_btn_resetFactory_confirm, lv_color_hex(0x7d7d7d), LV_PART_MAIN|LV_STATE_DEFAULT);	
	lv_obj_set_style_bg_color(ui->screen_SET_btn_resetFactory_confirm, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_PRESSED);	
	lv_obj_set_style_radius(ui->screen_SET_btn_resetFactory_confirm, 30, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_SET_btn_resetFactory_confirm_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_center(ui->screen_SET_btn_resetFactory_confirm_label);	

	ui->screen_SET_btn_resetFactory_cancel = lv_btn_create(ui->screen_SET_cont_resetFactory);
	ui->screen_SET_btn_resetFactory_cancel_label = lv_label_create(ui->screen_SET_btn_resetFactory_cancel);
	lv_obj_set_size(ui->screen_SET_btn_resetFactory_cancel, 265, 60);
	lv_obj_set_pos(ui->screen_SET_btn_resetFactory_cancel, 285, 150);
	lv_label_set_long_mode(ui->screen_SET_btn_resetFactory_cancel_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_label_set_text(ui->screen_SET_btn_resetFactory_cancel_label,get_string_for_language(g_sys_Data.current_language,"sys_txt_cancel"));
	lv_obj_set_style_text_color(ui->screen_SET_btn_resetFactory_cancel_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_SET_btn_resetFactory_cancel_label, &lv_font_harmonyOS_40, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_resetFactory_cancel, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_SET_btn_resetFactory_cancel, lv_color_hex(0x7d7d7d), LV_PART_MAIN|LV_STATE_DEFAULT);	
	lv_obj_set_style_bg_color(ui->screen_SET_btn_resetFactory_cancel, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_PRESSED);	
	lv_obj_set_style_radius(ui->screen_SET_btn_resetFactory_cancel, 30, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_SET_btn_resetFactory_cancel_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_center(ui->screen_SET_btn_resetFactory_cancel_label);
}

static void create_aging_mode_cont(lv_ui *ui){
	ui->screen_SET_cont_agingMode = lv_obj_create(ui->screen_SET);
	lv_obj_set_pos(ui->screen_SET_cont_agingMode, 240, 60);
	lv_obj_set_size(ui->screen_SET_cont_agingMode, 1200, 600);
	lv_obj_set_style_bg_opa(ui->screen_SET_cont_agingMode, 0 , LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_scrollbar_mode(ui->screen_SET_cont_agingMode, LV_SCROLLBAR_MODE_OFF);
	lv_obj_set_scroll_dir(ui->screen_SET_cont_agingMode,LV_DIR_TOP|LV_DIR_BOTTOM);
	lv_obj_add_flag(ui->screen_SET_cont_agingMode, LV_OBJ_FLAG_HIDDEN);

	lv_obj_set_style_border_opa(ui->screen_SET_cont_agingMode, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_flex_flow(ui->screen_SET_cont_agingMode, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(ui->screen_SET_cont_agingMode, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_row(ui->screen_SET_cont_agingMode, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_column(ui->screen_SET_cont_agingMode, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_gap(ui->screen_SET_cont_agingMode, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//write screen_SET_btn_agingMode_memInfo
	ui->screen_SET_btn_agingMode_memInfo = lv_btn_create(ui->screen_SET_cont_agingMode);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_agingMode_memInfo, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_size(ui->screen_SET_btn_agingMode_memInfo,1200,120);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_agingMode_memInfo, 0, LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_agingMode_memInfo, 0, LV_PART_MAIN|LV_STATE_DEFAULT);


	ui->screen_SET_btn_agingMode_memInfo_label = lv_label_create(ui->screen_SET_btn_agingMode_memInfo);
	lv_label_set_text(ui->screen_SET_btn_agingMode_memInfo_label,get_string_for_language(g_sys_Data.current_language,"sys_txt_memInfo_label") );
	lv_label_set_long_mode(ui->screen_SET_btn_agingMode_memInfo_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_remove_style_all(ui->screen_SET_btn_agingMode_memInfo_label);
	lv_obj_add_style(ui->screen_SET_btn_agingMode_memInfo_label, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_btn_agingMode_memInfo_label, 40, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_agingMode_memInfo_img = lv_img_create(ui->screen_SET_btn_agingMode_memInfo);
	lv_img_set_src(ui->screen_SET_btn_agingMode_memInfo_img, &_right_but_normal_day_60x75);
	lv_img_set_pivot(ui->screen_SET_btn_agingMode_memInfo_img, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_agingMode_memInfo_img, 0);
	lv_obj_set_pos(ui->screen_SET_btn_agingMode_memInfo_img, 1050, 25);
	lv_obj_set_size(ui->screen_SET_btn_agingMode_memInfo_img, 60, 75);			

	//write screen_SET_btn_agingMode_netBandWidth
	ui->screen_SET_btn_agingMode_netBandWidth = lv_btn_create(ui->screen_SET_cont_agingMode);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_agingMode_netBandWidth, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_size(ui->screen_SET_btn_agingMode_netBandWidth,1200,120);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_agingMode_netBandWidth, 0, LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_agingMode_netBandWidth, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_agingMode_netBandWidth_label = lv_label_create(ui->screen_SET_btn_agingMode_netBandWidth);
	lv_label_set_text(ui->screen_SET_btn_agingMode_netBandWidth_label,get_string_for_language(g_sys_Data.current_language,"sys_txt_netBandWidth_label") );
	lv_label_set_long_mode(ui->screen_SET_btn_agingMode_netBandWidth_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_remove_style_all(ui->screen_SET_btn_agingMode_netBandWidth_label);
	lv_obj_add_style(ui->screen_SET_btn_agingMode_netBandWidth_label, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_btn_agingMode_netBandWidth_label, 40, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_agingMode_netBandWidth_img = lv_img_create(ui->screen_SET_btn_agingMode_netBandWidth);
	lv_img_set_src(ui->screen_SET_btn_agingMode_netBandWidth_img, &_right_but_normal_day_60x75);
	lv_img_set_pivot(ui->screen_SET_btn_agingMode_netBandWidth_img, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_agingMode_netBandWidth_img, 0);
	lv_obj_set_pos(ui->screen_SET_btn_agingMode_netBandWidth_img, 1050, 25);
	lv_obj_set_size(ui->screen_SET_btn_agingMode_netBandWidth_img, 60, 75);	

	//write screen_SET_btn_agingMode_touchPoint
	ui->screen_SET_btn_agingMode_touchPoint = lv_btn_create(ui->screen_SET_cont_agingMode);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_agingMode_touchPoint, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_size(ui->screen_SET_btn_agingMode_touchPoint,1200,120);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_agingMode_touchPoint, 0, LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_agingMode_touchPoint, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_agingMode_touchPoint_label = lv_label_create(ui->screen_SET_btn_agingMode_touchPoint);
	lv_label_set_text(ui->screen_SET_btn_agingMode_touchPoint_label,get_string_for_language(g_sys_Data.current_language,"sys_txt_touchPoint_label") );
	lv_label_set_long_mode(ui->screen_SET_btn_agingMode_touchPoint_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_remove_style_all(ui->screen_SET_btn_agingMode_touchPoint_label);
	lv_obj_add_style(ui->screen_SET_btn_agingMode_touchPoint_label, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_btn_agingMode_touchPoint_label, 40, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_agingMode_touchPoint_labelSwitch = lv_label_create(ui->screen_SET_btn_agingMode_touchPoint);
	lv_label_set_text(ui->screen_SET_btn_agingMode_touchPoint_labelSwitch,get_string_for_language(g_sys_Data.current_language,"sys_txt_close") );
	lv_label_set_long_mode(ui->screen_SET_btn_agingMode_touchPoint_labelSwitch, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_remove_style_all(ui->screen_SET_btn_agingMode_touchPoint_labelSwitch);
	lv_obj_add_style(ui->screen_SET_btn_agingMode_touchPoint_labelSwitch, &style_screen_SET_text_center_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_pos(ui->screen_SET_btn_agingMode_touchPoint_labelSwitch, 900, 0);
	lv_obj_set_size(ui->screen_SET_btn_agingMode_touchPoint_labelSwitch, 150, 120);
	lv_obj_set_style_pad_top(ui->screen_SET_btn_agingMode_touchPoint_labelSwitch, 40, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_agingMode_touchPoint_img = lv_img_create(ui->screen_SET_btn_agingMode_touchPoint);
	lv_img_set_src(ui->screen_SET_btn_agingMode_touchPoint_img, &_right_but_normal_day_60x75);
	lv_img_set_pivot(ui->screen_SET_btn_agingMode_touchPoint_img, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_agingMode_touchPoint_img, 0);
	lv_obj_set_pos(ui->screen_SET_btn_agingMode_touchPoint_img, 1050, 25);
	lv_obj_set_size(ui->screen_SET_btn_agingMode_touchPoint_img, 60, 75);	

	//write screen_SET_btn_agingMode_screenSave
	ui->screen_SET_btn_agingMode_screenSave = lv_btn_create(ui->screen_SET_cont_agingMode);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_agingMode_screenSave, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_size(ui->screen_SET_btn_agingMode_screenSave,1200,120);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_agingMode_screenSave, 0, LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_agingMode_screenSave, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_agingMode_screenSave_label = lv_label_create(ui->screen_SET_btn_agingMode_screenSave);
	lv_label_set_text(ui->screen_SET_btn_agingMode_screenSave_label,get_string_for_language(g_sys_Data.current_language,"sys_txt_screenSave_label") );
	lv_label_set_long_mode(ui->screen_SET_btn_agingMode_screenSave_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_remove_style_all(ui->screen_SET_btn_agingMode_screenSave_label);
	lv_obj_add_style(ui->screen_SET_btn_agingMode_screenSave_label, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_btn_agingMode_screenSave_label, 40, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_agingMode_screenSave_labelSwitch = lv_label_create(ui->screen_SET_btn_agingMode_screenSave);
	if(g_sys_Data.agingMode.screenSaveSw){
		lv_label_set_text(ui->screen_SET_btn_agingMode_screenSave_labelSwitch,get_string_for_language(g_sys_Data.current_language,"sys_txt_open") );
	}else{
		lv_label_set_text(ui->screen_SET_btn_agingMode_screenSave_labelSwitch,get_string_for_language(g_sys_Data.current_language,"sys_txt_close") );
	}
	
	lv_label_set_long_mode(ui->screen_SET_btn_agingMode_screenSave_labelSwitch, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_remove_style_all(ui->screen_SET_btn_agingMode_screenSave_labelSwitch);
	lv_obj_add_style(ui->screen_SET_btn_agingMode_screenSave_labelSwitch, &style_screen_SET_text_center_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_pos(ui->screen_SET_btn_agingMode_screenSave_labelSwitch, 900, 0);
	lv_obj_set_size(ui->screen_SET_btn_agingMode_screenSave_labelSwitch, 150, 120);
	lv_obj_set_style_pad_top(ui->screen_SET_btn_agingMode_screenSave_labelSwitch, 40, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_agingMode_screenSave_img = lv_img_create(ui->screen_SET_btn_agingMode_screenSave);
	lv_img_set_src(ui->screen_SET_btn_agingMode_screenSave_img, &_right_but_normal_day_60x75);
	lv_img_set_pivot(ui->screen_SET_btn_agingMode_screenSave_img, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_agingMode_screenSave_img, 0);
	lv_obj_set_pos(ui->screen_SET_btn_agingMode_screenSave_img, 1050, 25);
	lv_obj_set_size(ui->screen_SET_btn_agingMode_screenSave_img, 60, 75);	

	//write screen_SET_btn_agingMode_screenSaveTime
	ui->screen_SET_btn_agingMode_screenSaveTime = lv_btn_create(ui->screen_SET_cont_agingMode);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_agingMode_screenSaveTime, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_size(ui->screen_SET_btn_agingMode_screenSaveTime,1200,120);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_agingMode_screenSaveTime, 0, LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_agingMode_screenSaveTime, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_agingMode_screenSaveTime_label = lv_label_create(ui->screen_SET_btn_agingMode_screenSaveTime);
	lv_label_set_text(ui->screen_SET_btn_agingMode_screenSaveTime_label,get_string_for_language(g_sys_Data.current_language,"sys_txt_screenSaveTime_label") );
	lv_label_set_long_mode(ui->screen_SET_btn_agingMode_screenSaveTime_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_remove_style_all(ui->screen_SET_btn_agingMode_screenSaveTime_label);
	lv_obj_add_style(ui->screen_SET_btn_agingMode_screenSaveTime_label, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_btn_agingMode_screenSaveTime_label, 40, LV_PART_MAIN|LV_STATE_DEFAULT);


	char *str = NULL;
	switch (g_sys_Data.agingMode.screenSaveTime)
	{
	case T_10S :
		str = "10s";
		break;
	case T_30S :
		str = "30s";
	case T_1MIN :
		str = "1min";
	case T_5MIN :
		str = "5min";
	case T_10MIN :
		str = "10min";
	case T_30MIN :
		str = "30min";	
	default:
		str = "10s";
		break;
	}
	ui->screen_SET_btn_agingMode_screenSaveTime_labelTime = lv_label_create(ui->screen_SET_btn_agingMode_screenSaveTime);
	lv_label_set_text(ui->screen_SET_btn_agingMode_screenSaveTime_labelTime, str);
	lv_label_set_long_mode(ui->screen_SET_btn_agingMode_screenSaveTime_labelTime, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_remove_style_all(ui->screen_SET_btn_agingMode_screenSaveTime_labelTime);
	lv_obj_add_style(ui->screen_SET_btn_agingMode_screenSaveTime_labelTime, &style_screen_SET_text_center_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_pos(ui->screen_SET_btn_agingMode_screenSaveTime_labelTime, 900, 0);
	lv_obj_set_size(ui->screen_SET_btn_agingMode_screenSaveTime_labelTime, 150, 120);
	lv_obj_set_style_pad_top(ui->screen_SET_btn_agingMode_screenSaveTime_labelTime, 40, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_agingMode_screenSaveTime_img = lv_img_create(ui->screen_SET_btn_agingMode_screenSaveTime);
	lv_img_set_src(ui->screen_SET_btn_agingMode_screenSaveTime_img, &_right_but_normal_day_60x75);
	lv_img_set_pivot(ui->screen_SET_btn_agingMode_screenSaveTime_img, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_agingMode_screenSaveTime_img, 0);
	lv_obj_set_pos(ui->screen_SET_btn_agingMode_screenSaveTime_img, 1050, 25);
	lv_obj_set_size(ui->screen_SET_btn_agingMode_screenSaveTime_img, 60, 75);	

	//write screen_SET_btn_agingMode_scrennShot
	ui->screen_SET_btn_agingMode_scrennShot = lv_btn_create(ui->screen_SET_cont_agingMode);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_agingMode_scrennShot, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_size(ui->screen_SET_btn_agingMode_scrennShot,1200,120);
	lv_obj_set_style_pad_all(ui->screen_SET_btn_agingMode_scrennShot, 0, LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_SET_btn_agingMode_scrennShot, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_agingMode_scrennShot_label = lv_label_create(ui->screen_SET_btn_agingMode_scrennShot);
	lv_label_set_text(ui->screen_SET_btn_agingMode_scrennShot_label,get_string_for_language(g_sys_Data.current_language,"sys_txt_screenShot_label") );
	lv_label_set_long_mode(ui->screen_SET_btn_agingMode_scrennShot_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_remove_style_all(ui->screen_SET_btn_agingMode_scrennShot_label);
	lv_obj_add_style(ui->screen_SET_btn_agingMode_scrennShot_label, &style_screen_SET_text_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_SET_btn_agingMode_scrennShot_label, 40, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_SET_btn_agingMode_scrennShot_labelSW = lv_label_create(ui->screen_SET_btn_agingMode_scrennShot);
	if(g_sys_Data.agingMode.scrennShotSW){
		lv_label_set_text(ui->screen_SET_btn_agingMode_scrennShot_labelSW, get_string_for_language(g_sys_Data.current_language,"sys_txt_open"));
	}else{
		lv_label_set_text(ui->screen_SET_btn_agingMode_scrennShot_labelSW, get_string_for_language(g_sys_Data.current_language,"sys_txt_close"));
	}
	
	lv_label_set_long_mode(ui->screen_SET_btn_agingMode_scrennShot_labelSW, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_remove_style_all(ui->screen_SET_btn_agingMode_scrennShot_labelSW);
	lv_obj_add_style(ui->screen_SET_btn_agingMode_scrennShot_labelSW, &style_screen_SET_text_center_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_pos(ui->screen_SET_btn_agingMode_scrennShot_labelSW, 900, 0);
	lv_obj_set_size(ui->screen_SET_btn_agingMode_scrennShot_labelSW, 150, 120);
	lv_obj_set_style_pad_top(ui->screen_SET_btn_agingMode_scrennShot_labelSW, 40, LV_PART_MAIN|LV_STATE_DEFAULT);	

	ui->screen_SET_btn_agingMode_scrennShot_img = lv_img_create(ui->screen_SET_btn_agingMode_scrennShot);
	lv_img_set_src(ui->screen_SET_btn_agingMode_scrennShot_img, &_right_but_normal_day_60x75);
	lv_img_set_pivot(ui->screen_SET_btn_agingMode_scrennShot_img, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_agingMode_scrennShot_img, 0);
	lv_obj_set_pos(ui->screen_SET_btn_agingMode_scrennShot_img, 1050, 25);
	lv_obj_set_size(ui->screen_SET_btn_agingMode_scrennShot_img, 60, 75);	

//write lineX
	ui->screen_SET_btn_agingMode_lineX0 = lv_img_create(ui->screen_SET_cont_agingMode);
	lv_obj_add_flag(ui->screen_SET_btn_agingMode_lineX0, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_btn_agingMode_lineX0, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_btn_agingMode_lineX0, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_agingMode_lineX0, 0);
	lv_obj_set_size(ui->screen_SET_btn_agingMode_lineX0, 1200, 5);	

	ui->screen_SET_btn_agingMode_lineX1 = lv_img_create(ui->screen_SET_cont_agingMode);
	lv_obj_add_flag(ui->screen_SET_btn_agingMode_lineX1, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_btn_agingMode_lineX1, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_btn_agingMode_lineX1, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_agingMode_lineX1, 0);
	lv_obj_set_size(ui->screen_SET_btn_agingMode_lineX1, 1200, 5);	

	ui->screen_SET_btn_agingMode_lineX2 = lv_img_create(ui->screen_SET_cont_agingMode);
	lv_obj_add_flag(ui->screen_SET_btn_agingMode_lineX2, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_btn_agingMode_lineX2, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_btn_agingMode_lineX2, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_agingMode_lineX2, 0);
	lv_obj_set_size(ui->screen_SET_btn_agingMode_lineX2, 1200, 5);	

	ui->screen_SET_btn_agingMode_lineX3 = lv_img_create(ui->screen_SET_cont_agingMode);
	lv_obj_add_flag(ui->screen_SET_btn_agingMode_lineX3, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_btn_agingMode_lineX3, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_btn_agingMode_lineX3, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_agingMode_lineX3, 0);
	lv_obj_set_size(ui->screen_SET_btn_agingMode_lineX3, 1200, 5);	

	ui->screen_SET_btn_agingMode_lineX4 = lv_img_create(ui->screen_SET_cont_agingMode);
	lv_obj_add_flag(ui->screen_SET_btn_agingMode_lineX4, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_btn_agingMode_lineX4, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_btn_agingMode_lineX4, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_agingMode_lineX4, 0);
	lv_obj_set_size(ui->screen_SET_btn_agingMode_lineX4, 1200, 5);	

	ui->screen_SET_btn_agingMode_lineX5 = lv_img_create(ui->screen_SET_cont_agingMode);
	lv_obj_add_flag(ui->screen_SET_btn_agingMode_lineX5, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_SET_btn_agingMode_lineX5, &_lineX_alpha_1200x5);
	lv_img_set_pivot(ui->screen_SET_btn_agingMode_lineX5, 50,50);
	lv_img_set_angle(ui->screen_SET_btn_agingMode_lineX5, 0);
	lv_obj_set_size(ui->screen_SET_btn_agingMode_lineX5, 1200, 5);	

	//index
	lv_obj_move_to_index(ui->screen_SET_btn_agingMode_memInfo,0);
	lv_obj_move_to_index(ui->screen_SET_btn_agingMode_lineX0,1);
	lv_obj_move_to_index(ui->screen_SET_btn_agingMode_netBandWidth,2);
	lv_obj_move_to_index(ui->screen_SET_btn_agingMode_lineX1,3);
	lv_obj_move_to_index(ui->screen_SET_btn_agingMode_touchPoint,4);
	lv_obj_move_to_index(ui->screen_SET_btn_agingMode_lineX2,5);
	lv_obj_move_to_index(ui->screen_SET_btn_agingMode_screenSave,6);
	lv_obj_move_to_index(ui->screen_SET_btn_agingMode_lineX3,7);
	lv_obj_move_to_index(ui->screen_SET_btn_agingMode_screenSaveTime,8);
	lv_obj_move_to_index(ui->screen_SET_btn_agingMode_lineX4,9);
	lv_obj_move_to_index(ui->screen_SET_btn_agingMode_scrennShot,10);
	lv_obj_move_to_index(ui->screen_SET_btn_agingMode_lineX5,11);
}

void ScreenSetBlackNightMode(lv_ui *ui, THEME_MODE mode){
	printf("-------%s:%d------mode:%d-----\n",__func__,__LINE__,mode);
	switch (mode)
	{
	case THEME_DAY:
		lv_obj_set_style_bg_img_src(ui->screen_SET_btn_return, &_left_but_normal_day_60x75, LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_img_src(ui->screen_SET_btn_zhuti_left, &_left_but_normal_day_60x75, LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_img_src(ui->screen_SET_btn_zhuti_right, &_right_but_normal_day_60x75, LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_img_set_src(ui->screen_SET_btn_yuyan_img, &_right_but_normal_day_60x75);
		lv_img_set_src(ui->screen_SET_btn_shijian_img, &_right_but_normal_day_60x75);
		lv_img_set_src(ui->screen_SET_btn_taiya_img, &_right_but_normal_day_60x75);
		lv_img_set_src(ui->screen_SET_cont_settime_img_left, &_left_but_normal_day_60x75);
		lv_img_set_src(ui->screen_SET_cont_settime_img_right, &_right_but_normal_day_60x75);
		lv_img_set_src(ui->screen_SET_btn_refactory_img, &_right_but_normal_day_60x75);
		lv_img_set_src(ui->screen_SET_btn_reboot_img, &_right_but_normal_day_60x75);
		lv_img_set_src(ui->screen_SET_btn_logo_img, &_right_but_normal_day_60x75);
		lv_img_set_src(ui->screen_SET_btn_about_img, &_right_but_normal_day_60x75);
		lv_img_set_src(ui->screen_SET_btn_otaUpdata_img, &_right_but_normal_day_60x75);
		lv_img_set_src(ui->screen_SET_btn_agingMode_memInfo_img, &_right_but_normal_day_60x75);
		lv_img_set_src(ui->screen_SET_btn_agingMode_netBandWidth_img, &_right_but_normal_day_60x75);
		lv_img_set_src(ui->screen_SET_btn_agingMode_touchPoint_img, &_right_but_normal_day_60x75);
		lv_img_set_src(ui->screen_SET_btn_agingMode_screenSave_img, &_right_but_normal_day_60x75);
		lv_img_set_src(ui->screen_SET_btn_agingMode_screenSaveTime_img, &_right_but_normal_day_60x75);
		lv_img_set_src(ui->screen_SET_btn_agingMode_scrennShot_img, &_right_but_normal_day_60x75);
		lv_img_set_src(ui->screen_SET_btn_bootlogo_path_img, &_right_but_normal_day_60x75);
		lv_img_set_src(ui->screen_SET_img_lineY, &_lineY_alpha_5x720);
		lv_img_set_src(ui->screen_SET_cont_tongyong_img_lineX0, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_tongyong_img_lineX1, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_tongyong_img_lineX2, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_tongyong_img_lineX3, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_tongyong_img_lineX4, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_lanya_img_lineX1, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_lanya_img_lineX0, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_sys_img_lineX0, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_sys_img_lineX1, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_sys_img_lineX2, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_sys_img_lineX3, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_guanyu_img_lineX0, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_guanyu_img_lineX1, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_guanyu_img_lineX2, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_guanyu_img_lineX3, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_guanyu_img_lineX4, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX0, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX1, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX2, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX3, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX4, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX5, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX6, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX7, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX8, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX9, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX10, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX11, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX12, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX13, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_settime_img_lineX0, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_settime_img_lineX1, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_settime_img_lineX2, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_settime_img_lineX3, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_bootlogo_img_lineX0, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_btn_agingMode_lineX0, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_btn_agingMode_lineX1, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_btn_agingMode_lineX2, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_btn_agingMode_lineX3, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_btn_agingMode_lineX4, &_lineX_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_btn_agingMode_lineX5, &_lineX_alpha_1200x5);

		lv_obj_set_style_bg_color(ui->screen_SET_cont_reboot, lv_color_hex(0xe9fafe) , LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_reboot_text, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_SET_btn_reboot_confirm, lv_color_hex(0x7d7d7d), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_SET_btn_reboot_cancel, lv_color_hex(0x7d7d7d), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_SET_cont_resetFactory, lv_color_hex(0xe9fafe) , LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_resetFactory_text, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_SET_btn_resetFactory_confirm, lv_color_hex(0x7d7d7d), LV_PART_MAIN|LV_STATE_DEFAULT); 
		lv_obj_set_style_bg_color(ui->screen_SET_btn_resetFactory_cancel, lv_color_hex(0x7d7d7d), LV_PART_MAIN|LV_STATE_DEFAULT);


		lv_obj_set_style_bg_color(ui->screen_SET, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_list_set_item0, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_list_set_item1, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_list_set_item2, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_SET_list_set_item0, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_SET_list_set_item1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_SET_list_set_item2, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_list_set_item0, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_PRESSED);
		lv_obj_set_style_text_color(ui->screen_SET_list_set_item1, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_PRESSED);
		lv_obj_set_style_text_color(ui->screen_SET_list_set_item2, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_PRESSED);
		lv_obj_set_style_text_color(ui->screen_SET_btn_taiya_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_taiya_sw_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_beiguang, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_shijian_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_yuyan_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_yuyan_label1, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_zhuti, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_zhuti1, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_beiguang1, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_connect, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_pairdev, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_btname1, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_btname, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_refactory_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_reboot_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_logo_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_about_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_devModel, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_devModel1, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_devNum, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_devNum1, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_appVersion, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_appVersion_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_hwVersion, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_hwVersion1, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_otaUpdata_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_ZH_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_EN_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_ZH_FT_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_FR_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_GE_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_IT_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_JP_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_POR_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_RU_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_SP_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_POL_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_TU_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_KO_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_DU_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_year, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_month, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_day, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_hour, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_minute, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_roller_year, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_roller_month, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_roller_day, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_roller_hour, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_roller_minute, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_bootlogo_path_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_bootlogo_path_label1, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_bootlogo_choose, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_reboot_text, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_reboot_confirm_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_reboot_cancel_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_resetFactory_text, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_resetFactory_confirm_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_resetFactory_cancel_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_popup, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_agingMode_memInfo_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_agingMode_netBandWidth_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_agingMode_touchPoint_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_agingMode_touchPoint_labelSwitch, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_agingMode_screenSave_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_agingMode_screenSave_labelSwitch, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_agingMode_screenSaveTime_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);	
		lv_obj_set_style_text_color(ui->screen_SET_btn_agingMode_screenSaveTime_labelTime, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_agingMode_scrennShot_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_agingMode_scrennShot_labelSW, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);		
	break;
	case THEME_DARK:
		lv_obj_set_style_bg_img_src(ui->screen_SET_btn_return, &_left_but_normal_night_60x75, LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_img_src(ui->screen_SET_btn_zhuti_left, &_left_but_normal_night_60x75, LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_img_src(ui->screen_SET_btn_zhuti_right, &_right_but_normal_night_60x75, LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_img_set_src(ui->screen_SET_btn_yuyan_img, &_right_but_normal_night_60x75);
		lv_img_set_src(ui->screen_SET_btn_shijian_img, &_right_but_normal_night_60x75);
		lv_img_set_src(ui->screen_SET_btn_taiya_img, &_right_but_normal_night_60x75);
		lv_img_set_src(ui->screen_SET_cont_settime_img_left, &_left_but_normal_night_60x75);
		lv_img_set_src(ui->screen_SET_cont_settime_img_right, &_right_but_normal_night_60x75);
		lv_img_set_src(ui->screen_SET_btn_refactory_img, &_right_but_normal_night_60x75);
		lv_img_set_src(ui->screen_SET_btn_reboot_img, &_right_but_normal_night_60x75);
		lv_img_set_src(ui->screen_SET_btn_logo_img, &_right_but_normal_night_60x75);
		lv_img_set_src(ui->screen_SET_btn_about_img, &_right_but_normal_night_60x75);
		lv_img_set_src(ui->screen_SET_btn_otaUpdata_img, &_right_but_normal_night_60x75);
		lv_img_set_src(ui->screen_SET_btn_agingMode_memInfo_img, &_right_but_normal_night_60x75);
		lv_img_set_src(ui->screen_SET_btn_agingMode_netBandWidth_img, &_right_but_normal_night_60x75);
		lv_img_set_src(ui->screen_SET_btn_agingMode_touchPoint_img, &_right_but_normal_night_60x75);
		lv_img_set_src(ui->screen_SET_btn_agingMode_screenSave_img, &_right_but_normal_night_60x75);
		lv_img_set_src(ui->screen_SET_btn_agingMode_screenSaveTime_img, &_right_but_normal_night_60x75);
		lv_img_set_src(ui->screen_SET_btn_agingMode_scrennShot_img, &_right_but_normal_night_60x75);
		lv_img_set_src(ui->screen_SET_btn_bootlogo_path_img, &_right_but_normal_night_60x75);
		lv_img_set_src(ui->screen_SET_img_lineY, &_lineY_dark_5x720);
		lv_img_set_src(ui->screen_SET_cont_tongyong_img_lineX0, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_tongyong_img_lineX1, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_tongyong_img_lineX2, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_tongyong_img_lineX3, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_tongyong_img_lineX4, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_lanya_img_lineX1, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_lanya_img_lineX0, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_sys_img_lineX0, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_sys_img_lineX1, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_sys_img_lineX2, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_sys_img_lineX3, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_guanyu_img_lineX0, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_guanyu_img_lineX1, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_guanyu_img_lineX2, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_guanyu_img_lineX3, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_guanyu_img_lineX4, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX0, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX1, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX2, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX3, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX4, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX5, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX6, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX7, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX8, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX9, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX10, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX11, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX12, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_setlanguage_img_lineX13, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_settime_img_lineX0, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_settime_img_lineX1, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_settime_img_lineX2, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_settime_img_lineX3, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_cont_bootlogo_img_lineX0, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_btn_agingMode_lineX0, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_btn_agingMode_lineX1, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_btn_agingMode_lineX2, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_btn_agingMode_lineX3, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_btn_agingMode_lineX4, &_linx_dark_alpha_1200x5);
		lv_img_set_src(ui->screen_SET_btn_agingMode_lineX5, &_linx_dark_alpha_1200x5);

		lv_obj_set_style_bg_color(ui->screen_SET_cont_reboot, lv_color_hex(0x5f5f5f) , LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_reboot_text, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_SET_btn_reboot_confirm, lv_color_hex(0xcfcfcf), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_SET_btn_reboot_cancel, lv_color_hex(0xcfcfcf), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_SET_cont_resetFactory, lv_color_hex(0x5f5f5f) , LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_resetFactory_text, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_SET_btn_resetFactory_confirm, lv_color_hex(0xcfcfcf), LV_PART_MAIN|LV_STATE_DEFAULT); 
		lv_obj_set_style_bg_color(ui->screen_SET_btn_resetFactory_cancel, lv_color_hex(0xcfcfcf), LV_PART_MAIN|LV_STATE_DEFAULT);

		lv_obj_set_style_bg_color(ui->screen_SET, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_list_set_item0, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_list_set_item1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_list_set_item2, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_SET_list_set_item0, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_SET_list_set_item1, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_SET_list_set_item2, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);		
		lv_obj_set_style_text_color(ui->screen_SET_list_set_item0, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_PRESSED);
		lv_obj_set_style_text_color(ui->screen_SET_list_set_item1, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_PRESSED);
		lv_obj_set_style_text_color(ui->screen_SET_list_set_item2, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_PRESSED);
		lv_obj_set_style_text_color(ui->screen_SET_btn_taiya_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_taiya_sw_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_beiguang, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_shijian_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_yuyan_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_yuyan_label1, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_zhuti, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_zhuti1, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_beiguang1, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_connect, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_pairdev, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_btname1, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_btname, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_refactory_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_reboot_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_logo_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_about_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_devModel, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_devModel1, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_devNum, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_devNum1, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_appVersion, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_appVersion_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_hwVersion, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_hwVersion1, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_otaUpdata_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_ZH_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_EN_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_ZH_FT_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_FR_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_GE_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_IT_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_JP_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_POR_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_RU_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_SP_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_POL_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_TU_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_KO_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_DU_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_year, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_month, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_day, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_hour, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_minute, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_roller_year, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_roller_month, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_roller_day, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_roller_hour, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_roller_minute, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_bootlogo_path_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_bootlogo_path_label1, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_bootlogo_choose, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_reboot_text, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_reboot_confirm_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_reboot_cancel_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_resetFactory_text, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_resetFactory_confirm_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_resetFactory_cancel_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_label_popup, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_agingMode_memInfo_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_agingMode_netBandWidth_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_agingMode_touchPoint_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_agingMode_touchPoint_labelSwitch, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_agingMode_screenSave_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_agingMode_screenSave_labelSwitch, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_agingMode_screenSaveTime_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);	
		lv_obj_set_style_text_color(ui->screen_SET_btn_agingMode_screenSaveTime_labelTime, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_agingMode_scrennShot_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_SET_btn_agingMode_scrennShot_labelSW, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
	break;
	default:
	break;	
	}
	
}