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
#include "common.h"
#ifdef ENABLE_CARPLAY
#include "zlink_client.h"
#endif

extern int get_BT_connect_state();

void mianScreenLanguageUpdata(language_t language){
	lv_label_set_text(guider_ui.screen_btn_cartrip_label_fTyre, get_string_for_language(language, "main_txt_fTyre"));
	lv_label_set_text(guider_ui.screen_btn_cartrip_label_bTyre, get_string_for_language(language, "main_txt_bTyre"));
	lv_label_set_text(guider_ui.screen_btn_DVR_label, get_string_for_language(language, "main_txt_DVR"));
	lv_label_set_text(guider_ui.screen_btn_set_label, get_string_for_language(language, "main_txt_sysSet"));
	lv_label_set_text(guider_ui.screen_btn_androidauto_label, get_string_for_language(language,"main_txt_AndroidAuto"));
#ifdef ENABLE_CARPLAY
	lv_label_set_text(guider_ui.screen_btn_androidauto_label_statu,
		(zlink_client_is_session_started() && g_sys_Data.linktype == LINK_TYPE_ANDROIDAUTO)
			? get_string_for_language(language, "main_txt_Connect")
			: get_string_for_language(language, "main_txt_nConnect"));
	lv_label_set_text(guider_ui.screen_btn_carplay_label, get_string_for_language(language,"main_txt_carplay"));
	lv_label_set_text(guider_ui.screen_btn_carplay_label_statu,
		(zlink_client_is_session_started() && g_sys_Data.linktype == LINK_TYPE_CARPLAY)
			? get_string_for_language(language, "main_txt_Connect")
			: get_string_for_language(language, "main_txt_nConnect"));
#else
	lv_label_set_text(guider_ui.screen_btn_androidauto_label_statu, get_string_for_language(language,"main_txt_nConnect"));
	lv_label_set_text(guider_ui.screen_btn_carplay_label, get_string_for_language(language,"main_txt_carplay"));
	lv_label_set_text(guider_ui.screen_btn_carplay_label_statu, get_string_for_language(language,"main_txt_nConnect"));
#endif
	lv_label_set_text(guider_ui.screen_btn_confirm_label, get_string_for_language(language,"sys_txt_confirm"));
	lv_label_set_text(guider_ui.screen_label_textChoose, get_string_for_language(language,"main_txt_SlecetLanguage"));
}


void listStatuSwitch(language_t language){
	lv_obj_set_style_text_color(guider_ui.screen_list_language_itemEN, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(guider_ui.screen_list_language_itemCN,lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(guider_ui.screen_list_language_itemCN_FT,lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(guider_ui.screen_list_language_itemFR, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(guider_ui.screen_list_language_itemGE, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(guider_ui.screen_list_language_itemIT, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(guider_ui.screen_list_language_itemJA, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(guider_ui.screen_list_language_itemPOR, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(guider_ui.screen_list_language_itemRU, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(guider_ui.screen_list_language_itemSP, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(guider_ui.screen_list_language_itemPOL, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(guider_ui.screen_list_language_itemTU, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(guider_ui.screen_list_language_itemKO, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(guider_ui.screen_list_language_itemDU, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	switch (language)
	{
	case LANG_ENGLISH:
		lv_obj_set_style_text_color(guider_ui.screen_list_language_itemEN, lv_color_hex(0xf00000), LV_PART_MAIN|LV_STATE_DEFAULT);
		break;
	case LANG_CHINESE:
		lv_obj_set_style_text_color(guider_ui.screen_list_language_itemCN, lv_color_hex(0xf00000), LV_PART_MAIN|LV_STATE_DEFAULT);
		break;
	case LANG_CHINESE_FT:
		lv_obj_set_style_text_color(guider_ui.screen_list_language_itemCN_FT, lv_color_hex(0xf00000), LV_PART_MAIN|LV_STATE_DEFAULT);
		break;	
	case LANG_FRANCE:
		lv_obj_set_style_text_color(guider_ui.screen_list_language_itemFR, lv_color_hex(0xf00000), LV_PART_MAIN|LV_STATE_DEFAULT);
			break;
	case LANG_GERMANY:
		lv_obj_set_style_text_color(guider_ui.screen_list_language_itemGE, lv_color_hex(0xf00000), LV_PART_MAIN|LV_STATE_DEFAULT);
		break;
	case LANG_ITALY:
		lv_obj_set_style_text_color(guider_ui.screen_list_language_itemIT, lv_color_hex(0xf00000), LV_PART_MAIN|LV_STATE_DEFAULT);
		break;
	case LANG_JAPAN:
		lv_obj_set_style_text_color(guider_ui.screen_list_language_itemJA, lv_color_hex(0xf00000), LV_PART_MAIN|LV_STATE_DEFAULT);
		break;	
	case LANG_PORTUGUESE:
		lv_obj_set_style_text_color(guider_ui.screen_list_language_itemPOR, lv_color_hex(0xf00000), LV_PART_MAIN|LV_STATE_DEFAULT);
		break;
	case LANG_RUSSIAN:
		lv_obj_set_style_text_color(guider_ui.screen_list_language_itemRU, lv_color_hex(0xf00000), LV_PART_MAIN|LV_STATE_DEFAULT);
		break;
	case LANG_SPANISH:
		lv_obj_set_style_text_color(guider_ui.screen_list_language_itemSP, lv_color_hex(0xf00000), LV_PART_MAIN|LV_STATE_DEFAULT);
		break;	
	case LANG_POLAND:
		lv_obj_set_style_text_color(guider_ui.screen_list_language_itemPOL, lv_color_hex(0xf00000), LV_PART_MAIN|LV_STATE_DEFAULT);
			break;
	case LANG_TURKISH:
		lv_obj_set_style_text_color(guider_ui.screen_list_language_itemTU, lv_color_hex(0xf00000), LV_PART_MAIN|LV_STATE_DEFAULT);
		break;
	case LANG_KOREAN:
		lv_obj_set_style_text_color(guider_ui.screen_list_language_itemKO, lv_color_hex(0xf00000), LV_PART_MAIN|LV_STATE_DEFAULT);
		break;
	case LANG_DUTCH:
		lv_obj_set_style_text_color(guider_ui.screen_list_language_itemDU, lv_color_hex(0xf00000), LV_PART_MAIN|LV_STATE_DEFAULT);
		break;			
	default:
		break;
	}
}

static lv_img_dsc_t img_dst = {0};
char *rgb_data =NULL;
void setup_scr_screen(lv_ui *ui)
{
	//Write codes screen
	ui->screen = lv_obj_create(NULL);
	lv_obj_set_size(ui->screen, 1440, 720);
	lv_obj_set_scrollbar_mode(ui->screen, LV_SCROLLBAR_MODE_OFF);
	lv_obj_set_scroll_dir(ui->screen, LV_DIR_NONE);
	//Write style for screen, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->screen, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
	// lv_obj_set_style_bg_img_src(ui->screen, NULL, LV_PART_MAIN|LV_STATE_DEFAULT);
	// lv_obj_set_style_bg_img_opa(ui->screen, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	// lv_obj_set_style_bg_img_recolor_opa(ui->screen, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_btn_cartrip
	ui->screen_btn_cartrip = lv_btn_create(ui->screen);
	ui->screen_btn_cartrip_label_fTyre = lv_label_create(ui->screen_btn_cartrip);
	lv_label_set_text(ui->screen_btn_cartrip_label_fTyre, get_string_for_language(g_sys_Data.current_language, "main_txt_fTyre"));
	lv_label_set_long_mode(ui->screen_btn_cartrip_label_fTyre, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_align(ui->screen_btn_cartrip_label_fTyre, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_btn_cartrip, 0, LV_STATE_DEFAULT);
	lv_obj_set_width(ui->screen_btn_cartrip_label_fTyre, LV_PCT(100));
	lv_obj_set_pos(ui->screen_btn_cartrip, 789, 373);
	lv_obj_set_size(ui->screen_btn_cartrip, 618, 304);

	//write  codes scren_btn_cartrip_label_btyre;
	ui->screen_btn_cartrip_label_bTyre = lv_label_create(ui->screen_btn_cartrip);
	lv_label_set_text(ui->screen_btn_cartrip_label_bTyre, get_string_for_language(g_sys_Data.current_language, "main_txt_bTyre"));
	lv_label_set_long_mode(ui->screen_btn_cartrip_label_bTyre, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_align(ui->screen_btn_cartrip_label_bTyre, LV_ALIGN_CENTER, 0, 0);

	//write codes screen_btn_cartrip_label_ftyre_data 
	ui->screen_btn_cartrip_label_bTyre_data = lv_label_create(ui->screen_btn_cartrip);
	lv_label_set_text(ui->screen_btn_cartrip_label_bTyre_data, "---kpa --℃");
	lv_label_set_long_mode(ui->screen_btn_cartrip_label_bTyre_data, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_align(ui->screen_btn_cartrip_label_bTyre_data, LV_ALIGN_CENTER, 0, 0);
	//write codes screen_btn_cartrip_label_btyre_data 
	ui->screen_btn_cartrip_label_fTyre_data = lv_label_create(ui->screen_btn_cartrip);
	lv_label_set_text(ui->screen_btn_cartrip_label_fTyre_data, "---kpa --℃");
	lv_label_set_long_mode(ui->screen_btn_cartrip_label_fTyre_data, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_align(ui->screen_btn_cartrip_label_fTyre_data, LV_ALIGN_CENTER, 0, 0);

	if(!g_sys_Data.carTripSwitch){
		lv_obj_add_flag(ui->screen_btn_cartrip_label_fTyre, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(ui->screen_btn_cartrip_label_bTyre, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(ui->screen_btn_cartrip_label_fTyre_data, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(ui->screen_btn_cartrip_label_bTyre_data, LV_OBJ_FLAG_HIDDEN);		
	}else{
		lv_obj_clear_flag(ui->screen_btn_cartrip_label_fTyre, LV_OBJ_FLAG_HIDDEN);
		lv_obj_clear_flag(ui->screen_btn_cartrip_label_bTyre, LV_OBJ_FLAG_HIDDEN);
		lv_obj_clear_flag(ui->screen_btn_cartrip_label_fTyre_data, LV_OBJ_FLAG_HIDDEN);
		lv_obj_clear_flag(ui->screen_btn_cartrip_label_bTyre_data, LV_OBJ_FLAG_HIDDEN);
	}

	//Write style for screen_btn_cartrip, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_btn_cartrip, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_btn_cartrip, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_btn_cartrip, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_btn_cartrip, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	if(g_sys_Data.carTripSwitch){
		lv_obj_set_style_bg_img_src(ui->screen_btn_cartrip, &_openTirp_618x304, LV_PART_MAIN|LV_STATE_DEFAULT);
	}else{
		lv_obj_set_style_bg_img_src(ui->screen_btn_cartrip, &_hm_tire_618x304, LV_PART_MAIN|LV_STATE_DEFAULT);
	}
	lv_obj_set_style_bg_img_opa(ui->screen_btn_cartrip, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_btn_cartrip, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_btn_cartrip, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_btn_cartrip, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_btn_cartrip, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_btn_cartrip, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_btn_cartrip, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_btn_cartrip, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_border_width(ui->screen_btn_cartrip, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_radius(ui->screen_btn_cartrip, 5, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_width(ui->screen_btn_cartrip, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	if(g_sys_Data.carTripSwitch){
		lv_obj_set_style_bg_img_src(ui->screen_btn_cartrip, &_openTirp_dn_618x304, LV_PART_MAIN|LV_STATE_PRESSED);
	}else{
		lv_obj_set_style_bg_img_src(ui->screen_btn_cartrip, &_hm_tire_618x304, LV_PART_MAIN|LV_STATE_PRESSED);
	}
	lv_obj_set_style_bg_img_opa(ui->screen_btn_cartrip, 255, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_btn_cartrip, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_text_color(ui->screen_btn_cartrip, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_text_font(ui->screen_btn_cartrip, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_text_opa(ui->screen_btn_cartrip, 255, LV_PART_MAIN|LV_STATE_PRESSED);

//------------------cartrip label--------------------------
	lv_obj_remove_style_all(ui->screen_btn_cartrip_label_fTyre);
	lv_obj_remove_style_all(ui->screen_btn_cartrip_label_fTyre_data);
	lv_obj_remove_style_all(ui->screen_btn_cartrip_label_bTyre);
	lv_obj_remove_style_all(ui->screen_btn_cartrip_label_bTyre_data);
	lv_obj_set_pos(ui->screen_btn_cartrip_label_fTyre, 300, 30);
	lv_obj_set_pos(ui->screen_btn_cartrip_label_fTyre_data, 300, 90);
	lv_obj_set_pos(ui->screen_btn_cartrip_label_bTyre, 300, 170);
	lv_obj_set_pos(ui->screen_btn_cartrip_label_bTyre_data, 300, 230);
	lv_obj_set_size(ui->screen_btn_cartrip_label_fTyre, 300,60);
	lv_obj_set_size(ui->screen_btn_cartrip_label_fTyre_data, 300, 60);
	lv_obj_set_size(ui->screen_btn_cartrip_label_bTyre, 300, 60);
	lv_obj_set_size(ui->screen_btn_cartrip_label_bTyre_data, 300, 60);
	lv_obj_set_style_text_align(ui->screen_btn_cartrip_label_fTyre, LV_TEXT_ALIGN_LEFT,0);
	lv_obj_set_style_text_align(ui->screen_btn_cartrip_label_fTyre_data,LV_TEXT_ALIGN_LEFT,0);
	lv_obj_set_style_text_align(ui->screen_btn_cartrip_label_bTyre,LV_TEXT_ALIGN_LEFT,0);
	lv_obj_set_style_text_align(ui->screen_btn_cartrip_label_bTyre_data,LV_TEXT_ALIGN_LEFT,0);
	lv_obj_set_style_text_font(ui->screen_btn_cartrip_label_bTyre, &lv_font_harmonyOS_42, LV_PART_MAIN);
	lv_obj_set_style_text_font(ui->screen_btn_cartrip_label_bTyre_data, &lv_font_harmonyOS_42, LV_PART_MAIN);
	lv_obj_set_style_text_font(ui->screen_btn_cartrip_label_fTyre, &lv_font_harmonyOS_42, LV_PART_MAIN);
	lv_obj_set_style_text_font(ui->screen_btn_cartrip_label_fTyre_data, &lv_font_harmonyOS_42, LV_PART_MAIN);
//----------------------------------------------


	//Write codes screen_btn_DVR
	ui->screen_btn_DVR = lv_btn_create(ui->screen);
	ui->screen_btn_DVR_label = lv_label_create(ui->screen_btn_DVR);
	lv_label_set_text(ui->screen_btn_DVR_label, get_string_for_language(g_sys_Data.current_language, "main_txt_DVR"));
	lv_label_set_long_mode(ui->screen_btn_DVR_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_align(ui->screen_btn_DVR_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_btn_DVR, 0, LV_STATE_DEFAULT);
	lv_obj_set_width(ui->screen_btn_DVR_label, LV_PCT(100));
	lv_obj_set_pos(ui->screen_btn_DVR, 789, 91);
	lv_obj_set_size(ui->screen_btn_DVR, 304, 264);

	//Write style for screen_btn_DVR, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_btn_DVR, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_btn_DVR, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_btn_DVR, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_btn_DVR, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_btn_DVR, &_hm_dvr_300x264, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_btn_DVR, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_btn_DVR, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_btn_DVR, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_btn_DVR, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_btn_DVR, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_btn_DVR, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_btn_DVR, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_btn_DVR, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_border_width(ui->screen_btn_DVR, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_radius(ui->screen_btn_DVR, 5, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_width(ui->screen_btn_DVR, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_src(ui->screen_btn_DVR, &_hm_dvr_dn_300x264, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_opa(ui->screen_btn_DVR, 255, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_btn_DVR, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_text_color(ui->screen_btn_DVR, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_text_font(ui->screen_btn_DVR, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_text_opa(ui->screen_btn_DVR, 255, LV_PART_MAIN|LV_STATE_PRESSED);

//--------------DVR label----------------
	lv_obj_remove_style_all(ui->screen_btn_DVR_label);
	lv_obj_set_pos(ui->screen_btn_DVR_label, 10, 200);
	lv_obj_set_size(ui->screen_btn_DVR_label, 280, 60);
	lv_obj_set_style_text_align(ui->screen_btn_DVR_label,LV_TEXT_ALIGN_CENTER,0);
	lv_obj_set_style_text_font(ui->screen_btn_DVR_label, &lv_font_harmonyOS_42, LV_PART_MAIN);	

	ui->screen_btn_DVR_label_status = lv_label_create(ui->screen_btn_DVR);
	lv_label_set_text(ui->screen_btn_DVR_label_status, get_string_for_language(g_sys_Data.current_language, "main_txt_DVR_status"));
	lv_label_set_long_mode(ui->screen_btn_DVR_label_status, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_remove_style_all(ui->screen_btn_DVR_label_status);
	lv_obj_set_pos(ui->screen_btn_DVR_label_status, 10, 150);
	lv_obj_set_size(ui->screen_btn_DVR_label_status, 280, 60);
	lv_obj_set_style_text_align(ui->screen_btn_DVR_label_status,LV_TEXT_ALIGN_CENTER,0);
	lv_obj_set_style_text_font(ui->screen_btn_DVR_label_status, &lv_font_harmonyOS_35, LV_PART_MAIN);	
	lv_obj_add_flag(ui->screen_btn_DVR_label_status, LV_OBJ_FLAG_HIDDEN);	
//----------------------------------------

	//Write codes screen_btn_set
	ui->screen_btn_set = lv_btn_create(ui->screen);
	ui->screen_btn_set_label = lv_label_create(ui->screen_btn_set);
	lv_label_set_text(ui->screen_btn_set_label, get_string_for_language(g_sys_Data.current_language, "main_txt_sysSet"));
	lv_label_set_long_mode(ui->screen_btn_set_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_align(ui->screen_btn_set_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_btn_set, 0, LV_STATE_DEFAULT);
	lv_obj_set_width(ui->screen_btn_set_label, LV_PCT(100));
	lv_obj_set_pos(ui->screen_btn_set, 1103, 90);
	lv_obj_set_size(ui->screen_btn_set, 304, 264);

	//Write style for screen_btn_set, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_btn_set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_btn_set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_btn_set, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_btn_set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_btn_set, &_hm_set_300x264, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_btn_set, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_btn_set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_btn_set, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_btn_set, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_btn_set, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_btn_set, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_btn_set, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_btn_set, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_border_width(ui->screen_btn_set, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_radius(ui->screen_btn_set, 5, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_width(ui->screen_btn_set, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_src(ui->screen_btn_set, &_hm_set_dn_300x264, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_opa(ui->screen_btn_set, 255, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_btn_set, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_text_color(ui->screen_btn_set, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_text_font(ui->screen_btn_set, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_text_opa(ui->screen_btn_set, 255, LV_PART_MAIN|LV_STATE_PRESSED);

//--------------SET label----------------
	lv_obj_remove_style_all(ui->screen_btn_set_label);
	lv_obj_set_pos(ui->screen_btn_set_label, 10, 170);
	lv_obj_set_size(ui->screen_btn_set_label, 280, 60);
	lv_obj_set_style_text_align(ui->screen_btn_set_label,LV_TEXT_ALIGN_CENTER,0);
	lv_obj_set_style_text_font(ui->screen_btn_set_label, &lv_font_harmonyOS_42, LV_PART_MAIN);	
//----------------------------------------

	//Write codes screen_btn_androidauto
	ui->screen_btn_androidauto = lv_btn_create(ui->screen);
	ui->screen_btn_androidauto_label = lv_label_create(ui->screen_btn_androidauto);
	lv_label_set_text(ui->screen_btn_androidauto_label, get_string_for_language(g_sys_Data.current_language,"main_txt_AndroidAuto"));
	lv_label_set_long_mode(ui->screen_btn_androidauto_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_align(ui->screen_btn_androidauto_label, LV_ALIGN_TOP_LEFT, 0, 20);

	lv_obj_set_style_pad_all(ui->screen_btn_androidauto, 0, LV_STATE_DEFAULT);
	lv_obj_set_width(ui->screen_btn_androidauto_label, LV_PCT(100));
	lv_obj_set_pos(ui->screen_btn_androidauto, 411, 91);
	lv_obj_set_size(ui->screen_btn_androidauto, 360, 586);

	//write codes screen_btn_AndoriAuto_label_statu
	ui->screen_btn_androidauto_label_statu = lv_label_create(ui->screen_btn_androidauto);
#ifdef ENABLE_CARPLAY
	if (zlink_client_is_session_started() && g_sys_Data.linktype == LINK_TYPE_ANDROIDAUTO) {
		lv_label_set_text(ui->screen_btn_androidauto_label_statu, get_string_for_language(g_sys_Data.current_language,"main_txt_Connect"));
	} else {
		lv_label_set_text(ui->screen_btn_androidauto_label_statu, get_string_for_language(g_sys_Data.current_language,"main_txt_nConnect"));
	}
#else
	if(g_sys_Data.linktype == LINK_TYPE_ANDROIDAUTO){
		lv_label_set_text(ui->screen_btn_androidauto_label_statu, get_string_for_language(g_sys_Data.current_language,"main_txt_Connect"));
	}else{
		lv_label_set_text(ui->screen_btn_androidauto_label_statu, get_string_for_language(g_sys_Data.current_language,"main_txt_nConnect"));
	}
#endif
	lv_label_set_long_mode(ui->screen_btn_androidauto_label_statu, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_align(ui->screen_btn_androidauto_label_statu, LV_ALIGN_CENTER, 0, 100);
	lv_obj_set_width(ui->screen_btn_androidauto_label_statu, LV_PCT(100));

	//Write style for screen_btn_androidauto, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_btn_androidauto, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_btn_androidauto, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_btn_androidauto, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_btn_androidauto, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_btn_androidauto, &_hm_android_360x586, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_btn_androidauto, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_btn_androidauto, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_btn_androidauto, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_btn_androidauto, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_btn_androidauto, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_btn_androidauto, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_btn_androidauto, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_btn_androidauto, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_border_width(ui->screen_btn_androidauto, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_radius(ui->screen_btn_androidauto, 5, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_width(ui->screen_btn_androidauto, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_src(ui->screen_btn_androidauto, &_hm_andro_dn_360x586, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_opa(ui->screen_btn_androidauto, 255, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_btn_androidauto, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_text_color(ui->screen_btn_androidauto, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_text_font(ui->screen_btn_androidauto, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_text_opa(ui->screen_btn_androidauto, 255, LV_PART_MAIN|LV_STATE_PRESSED);

//-----------------label style---------------------------------
	lv_obj_remove_style_all(ui->screen_btn_androidauto_label);
	lv_obj_set_pos(ui->screen_btn_androidauto_label, 80, 20);
	lv_obj_set_size(ui->screen_btn_androidauto_label, 260, 50);
	lv_obj_set_style_text_align(ui->screen_btn_androidauto_label,LV_TEXT_ALIGN_LEFT,0);
	lv_obj_set_style_text_font(ui->screen_btn_androidauto_label, &lv_font_harmonyOS_42, LV_PART_MAIN);	


	lv_obj_remove_style_all(ui->screen_btn_androidauto_label_statu);
	lv_obj_set_pos(ui->screen_btn_androidauto_label_statu, 30, 450);
	lv_obj_set_size(ui->screen_btn_androidauto_label_statu, 300, 70);
	lv_obj_set_style_bg_opa(ui->screen_btn_androidauto_label_statu, 35, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_btn_androidauto_label_statu, lv_color_hex(0xdededf), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_btn_androidauto_label_statu, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_btn_androidauto_label_statu,LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_set_style_text_font(ui->screen_btn_androidauto_label_statu, &lv_font_harmonyOS_42, LV_PART_MAIN);	
	lv_obj_set_style_pad_top(ui->screen_btn_androidauto_label_statu,10, LV_PART_MAIN);

//---------------------------------------------------------

	//Write codes screen_btn_carplay
	ui->screen_btn_carplay = lv_btn_create(ui->screen);
	ui->screen_btn_carplay_label = lv_label_create(ui->screen_btn_carplay);
	lv_label_set_text(ui->screen_btn_carplay_label, get_string_for_language(g_sys_Data.current_language,"main_txt_carplay"));
	lv_label_set_long_mode(ui->screen_btn_carplay_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_align(ui->screen_btn_carplay_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_btn_carplay, 0, LV_STATE_DEFAULT);
	lv_obj_set_width(ui->screen_btn_carplay_label, LV_PCT(100));
	lv_obj_set_pos(ui->screen_btn_carplay, 33, 91);
	lv_obj_set_size(ui->screen_btn_carplay, 360, 586);

	//write codes sscreen_btn_carplay_label_statu
	ui->screen_btn_carplay_label_statu = lv_label_create(ui->screen_btn_carplay);
#ifdef ENABLE_CARPLAY
	if (zlink_client_is_session_started() && g_sys_Data.linktype == LINK_TYPE_CARPLAY) {
		lv_label_set_text(ui->screen_btn_carplay_label_statu, get_string_for_language(g_sys_Data.current_language,"main_txt_Connect"));
	} else {
		lv_label_set_text(ui->screen_btn_carplay_label_statu, get_string_for_language(g_sys_Data.current_language,"main_txt_nConnect"));
	}
#else
	if(g_sys_Data.linktype == LINK_TYPE_CARPLAY){
		lv_label_set_text(ui->screen_btn_carplay_label_statu, get_string_for_language(g_sys_Data.current_language,"main_txt_Connect"));
	}else{
		lv_label_set_text(ui->screen_btn_carplay_label_statu, get_string_for_language(g_sys_Data.current_language,"main_txt_nConnect"));
	}
#endif
	lv_label_set_long_mode(ui->screen_btn_carplay_label_statu, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_align(ui->screen_btn_carplay_label_statu, LV_ALIGN_CENTER, 0, 100);
	lv_obj_set_width(ui->screen_btn_carplay_label_statu, LV_PCT(100));


	//Write style for screen_btn_carplay, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_btn_carplay, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_btn_carplay, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_btn_carplay, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_btn_carplay, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_btn_carplay, &_hm_carplay_360x586, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_btn_carplay, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_btn_carplay, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_btn_carplay, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_btn_carplay, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_btn_carplay, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_btn_carplay, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_btn_carplay, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_btn_carplay, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_border_width(ui->screen_btn_carplay, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_radius(ui->screen_btn_carplay, 5, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_width(ui->screen_btn_carplay, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_src(ui->screen_btn_carplay, &_hm_carpaly_dn_360x586, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_opa(ui->screen_btn_carplay, 255, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_btn_carplay, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_text_color(ui->screen_btn_carplay, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_text_font(ui->screen_btn_carplay, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_text_opa(ui->screen_btn_carplay, 255, LV_PART_MAIN|LV_STATE_PRESSED);

//-----------------label style---------------------------------
	lv_obj_remove_style_all(ui->screen_btn_carplay_label);
	lv_obj_set_pos(ui->screen_btn_carplay_label, 80, 20);
	lv_obj_set_size(ui->screen_btn_carplay_label, 260, 50);
	lv_obj_set_style_text_align(ui->screen_btn_carplay_label,LV_TEXT_ALIGN_LEFT,0);
	lv_obj_set_style_text_font(ui->screen_btn_carplay_label, &lv_font_harmonyOS_42, LV_PART_MAIN);	


	lv_obj_remove_style_all(ui->screen_btn_carplay_label_statu);
	lv_obj_set_pos(ui->screen_btn_carplay_label_statu, 30, 450);
	lv_obj_set_size(ui->screen_btn_carplay_label_statu, 300, 70);
	lv_obj_set_style_bg_opa(ui->screen_btn_carplay_label_statu, 35, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_btn_carplay_label_statu, lv_color_hex(0xdededf), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_btn_carplay_label_statu, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_btn_carplay_label_statu,LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_set_style_text_font(ui->screen_btn_carplay_label_statu, &lv_font_harmonyOS_42, LV_PART_MAIN);	
	lv_obj_set_style_pad_top(ui->screen_btn_carplay_label_statu,10, LV_PART_MAIN);
	
//---------------------------------------------------------

	//
	TTime time;
	memset(&time, 0, sizeof(time));
	RtcGetTime2C(&time);
	char *weekstr = NULL;
	switch (day_of_week_kim_larson(time.nYear,time.nMonth,time.nDay))
	{
	case 0:
		/* code */
		weekstr = "main_txt_Monday";
		break;
	case 1:
		/* code */
		weekstr = "main_txt_Tuesday";
		break;
	case 2:
		weekstr = "main_txt_Wednesday";
		/* code */
		break;
	case 3:
		/* code */
		weekstr = "main_txt_Thursday";
		break;	
	case 4:
		/* code */
		weekstr = "main_txt_Friday";
		break;	
	case 5:
		/* code */
		weekstr = "main_txt_Saturday";
		break;	
	case 6:
		weekstr = "main_txt_Sunday";
		/* code */
		break;	
	default:
		break;
	}
	char Timestr[50];
	sprintf(Timestr,"%d-%d-%d %s %02d:%02d",time.nYear,time.nMonth,time.nDay,get_string_for_language(g_sys_Data.current_language, weekstr), time.nHour, time.nMinute);
	//Write codes screen_datetext_1
	ui->screen_datetext = lv_label_create(ui->screen);
	lv_label_set_text(ui->screen_datetext, Timestr);
	lv_obj_set_style_text_align(ui->screen_datetext, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_add_flag(ui->screen_datetext, LV_OBJ_FLAG_CLICKABLE);
	// lv_obj_add_event_cb(ui->screen_datetext_1, screen_datetext_1_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_set_pos(ui->screen_datetext, 370, 19);
	lv_obj_set_size(ui->screen_datetext, 700, 48);
	lv_obj_set_scrollbar_mode(ui->screen_datetext, LV_SCROLLBAR_MODE_OFF);
	lv_obj_set_scroll_dir(ui->screen_datetext,LV_DIR_NONE);
	//Write style for screen_datetext_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_text_color(ui->screen_datetext, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_datetext, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_datetext, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(ui->screen_datetext, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_datetext, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_datetext, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_datetext, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_datetext, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_datetext, 7, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_datetext, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_datetext, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_datetext, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_digital_clock_1
	static bool screen_digital_clock_1_timer_enabled = false;
	if (!screen_digital_clock_1_timer_enabled) {
		lv_timer_create(screen_digital_clock_1_timer, 1000, NULL);
		screen_digital_clock_1_timer_enabled = true;
	}

	//Write codes screen_cont_languageChoose
	ui->screen_cont_languageChoose = lv_obj_create(ui->screen);
	lv_obj_set_pos(ui->screen_cont_languageChoose, 0, 0);
	lv_obj_set_size(ui->screen_cont_languageChoose, 1440, 720);
	lv_obj_set_scrollbar_mode(ui->screen_cont_languageChoose, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_cont_languageChoose, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_cont_languageChoose, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->screen_cont_languageChoose, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->screen_cont_languageChoose, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_side(ui->screen_cont_languageChoose, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_cont_languageChoose, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_cont_languageChoose, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_cont_languageChoose, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->screen_cont_languageChoose, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_cont_languageChoose, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_cont_languageChoose, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_cont_languageChoose, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_cont_languageChoose, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_cont_languageChoose, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_label_textChoose
	ui->screen_label_textChoose = lv_label_create(ui->screen_cont_languageChoose);
	lv_label_set_text(ui->screen_label_textChoose, "Please select language");
	lv_label_set_long_mode(ui->screen_label_textChoose, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_label_textChoose, 420, 80);
	lv_obj_set_size(ui->screen_label_textChoose, 600, 55);

	//Write style for screen_label_textChoose, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_label_textChoose, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_label_textChoose, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_label_textChoose, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_label_textChoose, &lv_font_harmonyOS_40, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_label_textChoose, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(ui->screen_label_textChoose, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(ui->screen_label_textChoose, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_label_textChoose, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_label_textChoose, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_label_textChoose, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_label_textChoose, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_label_textChoose, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_label_textChoose, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_label_textChoose, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_list_language
	ui->screen_list_language = lv_list_create(ui->screen_cont_languageChoose);
	ui->screen_list_language_itemEN =lv_list_add_btn(ui->screen_list_language, NULL,"English");
	ui->screen_list_language_itemCN =lv_list_add_btn(ui->screen_list_language, NULL,"简体中文");
	ui->screen_list_language_itemCN_FT =lv_list_add_btn(ui->screen_list_language, NULL,"繁體中文");
	ui->screen_list_language_itemFR = lv_list_add_btn(ui->screen_list_language,NULL, "Français");
	ui->screen_list_language_itemGE = lv_list_add_btn(ui->screen_list_language, NULL,"Deutsch");
	ui->screen_list_language_itemIT = lv_list_add_btn(ui->screen_list_language, NULL,"italiano");
	ui->screen_list_language_itemJA = lv_list_add_btn(ui->screen_list_language, NULL,"日本語");
	ui->screen_list_language_itemPOR = lv_list_add_btn(ui->screen_list_language, NULL,"Português");
	ui->screen_list_language_itemRU = lv_list_add_btn(ui->screen_list_language, NULL,"русский язык");
	ui->screen_list_language_itemSP = lv_list_add_btn(ui->screen_list_language, NULL,"Español");
	ui->screen_list_language_itemPOL = lv_list_add_btn(ui->screen_list_language, NULL,"język polski");
	ui->screen_list_language_itemTU = lv_list_add_btn(ui->screen_list_language, NULL,"Türkçe");
	ui->screen_list_language_itemKO = lv_list_add_btn(ui->screen_list_language, NULL,"한국어");
	ui->screen_list_language_itemDU = lv_list_add_btn(ui->screen_list_language, NULL,"Nederlands");
	lv_obj_set_pos(ui->screen_list_language, 420, 140);
	lv_obj_set_size(ui->screen_list_language, 600, 420);
	lv_obj_set_scrollbar_mode(ui->screen_list_language, LV_SCROLLBAR_MODE_OFF);
	lv_obj_set_style_bg_opa(ui->screen_list_language, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->screen_list_language,0,LV_PART_MAIN|LV_STATE_DEFAULT);

	// //Write style state: LV_STATE_DEFAULT for &style_screen_list_language_extra_texts_main_default
	static lv_style_t style_screen_list_language_extra_texts_main_default;
	ui_init_style(&style_screen_list_language_extra_texts_main_default);
	
	lv_style_set_pad_top(&style_screen_list_language_extra_texts_main_default, 20);
	lv_style_set_pad_left(&style_screen_list_language_extra_texts_main_default, 5);
	lv_style_set_pad_right(&style_screen_list_language_extra_texts_main_default, 5);
	lv_style_set_pad_bottom(&style_screen_list_language_extra_texts_main_default, 20);
	lv_style_set_border_width(&style_screen_list_language_extra_texts_main_default, 0);
	lv_style_set_text_color(&style_screen_list_language_extra_texts_main_default, lv_color_hex(0xffffff));
	lv_style_set_text_font(&style_screen_list_language_extra_texts_main_default, &lv_font_harmonyOS_40);
	lv_style_set_text_opa(&style_screen_list_language_extra_texts_main_default, 255);
	// lv_style_set_radius(&style_screen_list_language_extra_texts_main_default, 3);
	// // lv_style_set_transform_width(&style_screen_list_language_extra_texts_main_default, 0);
	lv_style_set_bg_opa(&style_screen_list_language_extra_texts_main_default, 0);
	lv_style_set_bg_color(&style_screen_list_language_extra_texts_main_default, lv_color_hex(0xffffff));
	// lv_style_set_bg_grad_dir(&style_screen_list_language_extra_texts_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_text_align(&style_screen_list_language_extra_texts_main_default,LV_TEXT_ALIGN_CENTER);

	lv_obj_add_style(ui->screen_list_language_itemEN, &style_screen_list_language_extra_texts_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_style(ui->screen_list_language_itemCN, &style_screen_list_language_extra_texts_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_style(ui->screen_list_language_itemCN_FT, &style_screen_list_language_extra_texts_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_style(ui->screen_list_language_itemFR, &style_screen_list_language_extra_texts_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_style(ui->screen_list_language_itemGE, &style_screen_list_language_extra_texts_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_style(ui->screen_list_language_itemIT, &style_screen_list_language_extra_texts_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_style(ui->screen_list_language_itemJA, &style_screen_list_language_extra_texts_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_style(ui->screen_list_language_itemPOR, &style_screen_list_language_extra_texts_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_style(ui->screen_list_language_itemRU, &style_screen_list_language_extra_texts_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_style(ui->screen_list_language_itemSP, &style_screen_list_language_extra_texts_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_style(ui->screen_list_language_itemPOL, &style_screen_list_language_extra_texts_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_style(ui->screen_list_language_itemTU, &style_screen_list_language_extra_texts_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_style(ui->screen_list_language_itemKO, &style_screen_list_language_extra_texts_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_style(ui->screen_list_language_itemDU, &style_screen_list_language_extra_texts_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);


	// lv_obj_set_style_text_color();
	lv_obj_set_style_text_color(ui->screen_list_language_itemEN, lv_color_hex(0xf00000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_list_language_itemCN,lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_list_language_itemCN_FT,lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_list_language_itemFR, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_list_language_itemGE, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_list_language_itemIT, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_list_language_itemJA, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_list_language_itemPOR, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_list_language_itemRU, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_list_language_itemSP, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_list_language_itemPOL, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_list_language_itemTU, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_list_language_itemKO, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_list_language_itemDU, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	// lv_obj_add_state(ui->screen_list_language_itemEN, LV_STATE_CHECKED);

	//Write codes screen_btn_confirm
	ui->screen_btn_confirm = lv_btn_create(ui->screen_cont_languageChoose);
	ui->screen_btn_confirm_label = lv_label_create(ui->screen_btn_confirm);
	lv_label_set_text(ui->screen_btn_confirm_label, "Confirm");
	lv_label_set_long_mode(ui->screen_btn_confirm_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_align(ui->screen_btn_confirm_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_btn_confirm, 0, LV_STATE_DEFAULT);
	lv_obj_set_width(ui->screen_btn_confirm_label, LV_PCT(100));
	lv_obj_set_pos(ui->screen_btn_confirm, 590, 600);
	lv_obj_set_size(ui->screen_btn_confirm, 260, 80);

	//Write style for screen_btn_confirm, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_btn_confirm, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_btn_confirm, lv_color_hex(0x454c52), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->screen_btn_confirm, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_btn_confirm, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_btn_confirm, 15, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_btn_confirm, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_btn_confirm, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_btn_confirm, &lv_font_harmonyOS_40, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_btn_confirm, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_btn_confirm, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

	lv_obj_set_style_bg_color(ui->screen_btn_confirm, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_text_color(ui->screen_btn_confirm, lv_color_hex(0x0dff00), LV_PART_MAIN|LV_STATE_PRESSED);

	ui->screen_btn_lockScreen = lv_btn_create(ui->screen);
	lv_obj_set_pos(ui->screen_btn_lockScreen,150, -80);
	lv_obj_set_size(ui->screen_btn_lockScreen, 80,80);
	lv_obj_set_style_bg_opa(ui->screen_btn_lockScreen, 0 ,LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_btn_lockScreen, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_btn_lockScreen, &_unlock_select_80x80, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_btn_lockScreen, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_btn_lockScreen, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_label_Popup = lv_label_create(ui->screen);
	lv_obj_set_size(ui->screen_label_Popup, 500,60);
	lv_obj_align(ui->screen_label_Popup, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_bg_opa(ui->screen_label_Popup, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_label_Popup, lv_color_hex(0x929292), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_label_Popup, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_label_Popup, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_label_Popup, 255, LV_PART_MAIN|LV_STATE_DEFAULT);	
	lv_obj_set_style_text_align(ui->screen_label_Popup, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_label_set_text(guider_ui.screen_label_Popup, NULL);
	lv_obj_set_style_pad_top(ui->screen_label_Popup, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_flag(ui->screen_label_Popup, LV_OBJ_FLAG_HIDDEN);

	//The custom code of screen.
	lv_obj_add_flag(ui->screen_cont_languageChoose, LV_OBJ_FLAG_HIDDEN);
	if(g_sys_Data.resetFactory){
		lv_obj_clear_flag(ui->screen_cont_languageChoose, LV_OBJ_FLAG_HIDDEN);
	}

	//Write codes screen_cont_sta_icon
	ui->screen_cont_sta_icon = lv_obj_create(ui->screen);
	lv_obj_set_pos(ui->screen_cont_sta_icon, 1060, 10);
	lv_obj_set_size(ui->screen_cont_sta_icon, 400, 70);
	lv_obj_set_scrollbar_mode(ui->screen_cont_sta_icon, LV_SCROLLBAR_MODE_OFF);
	lv_obj_clear_flag(ui->screen_cont_sta_icon, LV_OBJ_FLAG_SCROLLABLE);

	//Write style for screen_cont_sta_icon, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_cont_sta_icon, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->screen_cont_sta_icon, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->screen_cont_sta_icon, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_side(ui->screen_cont_sta_icon, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_cont_sta_icon, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_cont_sta_icon, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_cont_sta_icon, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_cont_sta_icon, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_cont_sta_icon, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_cont_sta_icon, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_cont_sta_icon, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_img_rec
	ui->screen_img_rec = lv_img_create(ui->screen);
	lv_img_set_src(ui->screen_img_rec, &_rec_sta_60x60);
	lv_img_set_pivot(ui->screen_img_rec, 50,50);
	lv_img_set_angle(ui->screen_img_rec, 0);
	lv_obj_set_pos(ui->screen_img_rec, 20, 20);
	lv_obj_set_size(ui->screen_img_rec, 60, 60);

	//Write codes screen_img_GPS
	ui->screen_img_GPS = lv_img_create(ui->screen_cont_sta_icon);
	lv_img_set_src(ui->screen_img_GPS, &_gps_sta_70x70);
	lv_img_set_pivot(ui->screen_img_GPS, 50,50);
	lv_img_set_angle(ui->screen_img_GPS, 0);
	lv_obj_set_pos(ui->screen_img_GPS, 0, 0);
	lv_obj_set_size(ui->screen_img_GPS, 70, 70);

	//Write codes screen_img_carPLay
	ui->screen_img_carPLay = lv_img_create(ui->screen_cont_sta_icon);
	lv_img_set_src(ui->screen_img_carPLay, &_CarPlay_sta_70x70);
	lv_img_set_pivot(ui->screen_img_carPLay, 50,50);
	lv_img_set_angle(ui->screen_img_carPLay, 0);
	lv_obj_set_pos(ui->screen_img_carPLay, 60, 0);
	lv_obj_set_size(ui->screen_img_carPLay, 70, 70);

	//Write codes screen_img_androiAuto
	ui->screen_img_androiAuto = lv_img_create(ui->screen_cont_sta_icon);
	lv_img_set_src(ui->screen_img_androiAuto, &_AUTO_sta_70x70);
	lv_img_set_pivot(ui->screen_img_androiAuto, 50,50);
	lv_img_set_angle(ui->screen_img_androiAuto, 0);
	lv_obj_set_pos(ui->screen_img_androiAuto, 120, 0);
	lv_obj_set_size(ui->screen_img_androiAuto, 70, 70);

	//Write codes screen_img_bt
	ui->screen_img_bt = lv_img_create(ui->screen_cont_sta_icon);
	lv_img_set_src(ui->screen_img_bt, &_bt_sta_60x60);
	lv_img_set_pivot(ui->screen_img_bt, 50,50);
	lv_img_set_angle(ui->screen_img_bt, 0);
	lv_obj_set_pos(ui->screen_img_bt, 180, 5);
	lv_obj_set_size(ui->screen_img_bt, 60, 60);

	//Write codes screen_img_wifi
	ui->screen_img_wifi = lv_img_create(ui->screen_cont_sta_icon);
	lv_img_set_src(ui->screen_img_wifi, &_wifi_sta_72x72);
	lv_img_set_pivot(ui->screen_img_wifi, 50,50);
	lv_img_set_angle(ui->screen_img_wifi, 0);
	lv_obj_set_pos(ui->screen_img_wifi, 240, 0);
	lv_obj_set_size(ui->screen_img_wifi, 72, 72);
	
	//Write codes screen_img_TF
	ui->screen_img_TF = lv_img_create(ui->screen_cont_sta_icon);
	lv_img_set_src(ui->screen_img_TF, &_tf_sta_70x70);
	lv_img_set_pivot(ui->screen_img_TF, 50,50);
	lv_img_set_angle(ui->screen_img_TF, 0);
	lv_obj_set_pos(ui->screen_img_TF, 300, 0);
	lv_obj_set_size(ui->screen_img_TF, 70, 70);

	lv_obj_add_flag (ui->screen_img_rec, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag (ui->screen_img_GPS, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag (ui->screen_img_carPLay, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag (ui->screen_img_androiAuto, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag (ui->screen_img_bt, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag (ui->screen_img_wifi, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag (ui->screen_img_TF, LV_OBJ_FLAG_HIDDEN);

	g_sys_Data.TFmounted ? lv_obj_clear_flag(guider_ui.screen_img_TF, LV_OBJ_FLAG_HIDDEN) : lv_obj_add_flag(guider_ui.screen_img_TF, LV_OBJ_FLAG_HIDDEN);

	//Update current screen layout.
	lv_obj_update_layout(ui->screen);

	//Init events for screen.
	events_init_screen(ui);
	mianScreenLanguageUpdata(g_sys_Data.current_language);
}
