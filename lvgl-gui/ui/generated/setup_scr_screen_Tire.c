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

static lv_style_t style_text_align_center_mian_default;
static lv_style_t style_text_align_left_mian_default;

void setup_scr_screen_Tire(lv_ui *ui)
{
	//Write codes screen_Tire
	ui->screen_Tire = lv_obj_create(NULL);
	lv_obj_set_size(ui->screen_Tire, 1440, 720);
	lv_obj_set_scrollbar_mode(ui->screen_Tire, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_Tire, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_Tire, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->screen_Tire, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);	

	//write all label style_text_align_center_mian_default style
	ui_init_style(&style_text_align_center_mian_default);

	lv_style_set_text_color(&style_text_align_center_mian_default, lv_color_hex(0x000000));
	lv_style_set_text_font(&style_text_align_center_mian_default, &lv_font_harmonyOS_42);
	lv_style_set_text_opa(&style_text_align_center_mian_default, 255);
	lv_style_set_text_align(&style_text_align_center_mian_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_top(&style_text_align_center_mian_default, 8);
	lv_style_set_bg_opa(&style_text_align_center_mian_default, 0);
	lv_style_set_bg_color(&style_text_align_center_mian_default, lv_color_hex(0x2195f6));
	lv_style_set_border_opa(&style_text_align_center_mian_default, 0);	

	//write all label style_text_align_left_mian_default style
	ui_init_style(&style_text_align_left_mian_default);

	lv_style_set_text_color(&style_text_align_left_mian_default, lv_color_hex(0x000000));
	lv_style_set_text_font(&style_text_align_left_mian_default, &lv_font_harmonyOS_42);
	lv_style_set_text_opa(&style_text_align_left_mian_default, 255);
	lv_style_set_text_align(&style_text_align_left_mian_default, LV_TEXT_ALIGN_LEFT);
	lv_style_set_pad_top(&style_text_align_left_mian_default, 15);
	lv_style_set_bg_opa(&style_text_align_left_mian_default, 0);
	lv_style_set_bg_color(&style_text_align_left_mian_default, lv_color_hex(0x2195f6));
	lv_style_set_border_opa(&style_text_align_left_mian_default, 0);		

	//Write codes screen_Tire_btn_return
	ui->screen_Tire_btn_return = lv_btn_create(ui->screen_Tire);
	lv_obj_set_pos(ui->screen_Tire_btn_return, 0, 0);
	lv_obj_set_size(ui->screen_Tire_btn_return, 80, 80);

	//Write style for screen_Tire_btn_return, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_Tire_btn_return, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_Tire_btn_return, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_Tire_btn_return, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_Tire_btn_return, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_Tire_btn_return, &_back_dark_80x80, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_Tire_btn_return, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_Tire_btn_return, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_Tire_btn_return, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_Tire_btn_return, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_border_width(ui->screen_Tire_btn_return, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_radius(ui->screen_Tire_btn_return, 5, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_width(ui->screen_Tire_btn_return, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_src(ui->screen_Tire_btn_return, &_back_select_80x80, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_opa(ui->screen_Tire_btn_return, 255, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_Tire_btn_return, 0, LV_PART_MAIN|LV_STATE_PRESSED);

	//Write codes screen_Tire_cont_behind
	ui->screen_Tire_cont_behind = lv_obj_create(ui->screen_Tire);
	lv_obj_set_pos(ui->screen_Tire_cont_behind, 80, 385);
	lv_obj_set_size(ui->screen_Tire_cont_behind, 500, 285);
	lv_obj_set_scrollbar_mode(ui->screen_Tire_cont_behind, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_Tire_cont_behind, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_opa(ui->screen_Tire_cont_behind, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_Tire_cont_behind, 15, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_cont_behind, lv_color_hex(0xecf5f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_Tire_cont_behind, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_clear_flag(ui->screen_Tire_cont_behind, LV_OBJ_FLAG_SCROLLABLE);

	//Write codes screen_Tire_label_bTemp
	ui->screen_Tire_label_bTemp = lv_label_create(ui->screen_Tire_cont_behind);
	lv_label_set_text(ui->screen_Tire_label_bTemp, "--℉");
	lv_label_set_long_mode(ui->screen_Tire_label_bTemp, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_Tire_label_bTemp, 240, 100);
	lv_obj_set_size(ui->screen_Tire_label_bTemp, 230, 70);
	lv_obj_add_style(ui->screen_Tire_label_bTemp, &style_text_align_center_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_Tire_label_bPressure
	ui->screen_Tire_label_bPressure = lv_label_create(ui->screen_Tire_cont_behind);
	lv_label_set_text(ui->screen_Tire_label_bPressure, "--Bar");
	lv_label_set_long_mode(ui->screen_Tire_label_bPressure, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_Tire_label_bPressure, 0, 100);
	lv_obj_set_size(ui->screen_Tire_label_bPressure, 230, 70);
	lv_obj_add_style(ui->screen_Tire_label_bPressure, &style_text_align_center_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);


	//Write codes screen_Tire_label_bLable
	ui->screen_Tire_label_bLable = lv_label_create(ui->screen_Tire_cont_behind);
	lv_label_set_text(ui->screen_Tire_label_bLable, get_string_for_language(g_sys_Data.current_language,"main_txt_bTyre"));
	lv_label_set_long_mode(ui->screen_Tire_label_bLable, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_Tire_label_bLable, 0, 0);
	lv_obj_set_size(ui->screen_Tire_label_bLable, 300, 70);
	lv_obj_add_style(ui->screen_Tire_label_bLable, &style_text_align_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_Tire_btn_bPair
	ui->screen_Tire_btn_bPair = lv_btn_create(ui->screen_Tire_cont_behind);
	ui->screen_Tire_btn_bPair_label = lv_label_create(ui->screen_Tire_btn_bPair);
	lv_label_set_text(ui->screen_Tire_btn_bPair_label, get_string_for_language(g_sys_Data.current_language,"tire_txt_nPair"));
	lv_label_set_long_mode(ui->screen_Tire_btn_bPair_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_align(ui->screen_Tire_btn_bPair_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_Tire_btn_bPair, 0, LV_STATE_DEFAULT);
	lv_obj_set_width(ui->screen_Tire_btn_bPair_label, LV_PCT(100));
	lv_obj_set_pos(ui->screen_Tire_btn_bPair, 0, 180);
	lv_obj_set_size(ui->screen_Tire_btn_bPair, 460, 60);

	//Write style for screen_Tire_btn_bPair, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_Tire_btn_bPair, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_btn_bPair, lv_color_hex(0xfcfcfc), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_Tire_btn_bPair, 15, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_Tire_btn_bPair, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_btn_bPair, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_Tire_btn_bPair, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_Tire_btn_bPair, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_Tire_btn_bPair, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_Tire_cont_front
	ui->screen_Tire_cont_front = lv_obj_create(ui->screen_Tire);
	lv_obj_set_pos(ui->screen_Tire_cont_front, 80, 50);
	lv_obj_set_size(ui->screen_Tire_cont_front, 500, 285);
	lv_obj_set_scrollbar_mode(ui->screen_Tire_cont_front, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_Tire_cont_front, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_opa(ui->screen_Tire_cont_front, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_Tire_cont_front, 15, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_Tire_cont_front, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_cont_front, lv_color_hex(0xecf5f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	// lv_obj_set_style_pad_all(ui->screen_Tire_cont_front, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_clear_flag(ui->screen_Tire_cont_front, LV_OBJ_FLAG_SCROLLABLE);


	//Write codes screen_Tire_label_fLabel
	ui->screen_Tire_label_fLabel = lv_label_create(ui->screen_Tire_cont_front);
	lv_label_set_text(ui->screen_Tire_label_fLabel, get_string_for_language(g_sys_Data.current_language,"main_txt_fTyre"));
	lv_label_set_long_mode(ui->screen_Tire_label_fLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_Tire_label_fLabel, 0, 0);
	lv_obj_set_size(ui->screen_Tire_label_fLabel, 300, 70);
	lv_obj_add_style(ui->screen_Tire_label_fLabel, &style_text_align_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);


	//Write codes screen_Tire_label_fTemp
	ui->screen_Tire_label_fTemp = lv_label_create(ui->screen_Tire_cont_front);
	lv_label_set_text(ui->screen_Tire_label_fTemp, "--℉");
	lv_label_set_long_mode(ui->screen_Tire_label_fTemp, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_Tire_label_fTemp, 240, 100);
	lv_obj_set_size(ui->screen_Tire_label_fTemp, 230, 70);
	lv_obj_add_style(ui->screen_Tire_label_fTemp, &style_text_align_center_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);


	//Write codes screen_Tire_label_fPressure
	ui->screen_Tire_label_fPressure = lv_label_create(ui->screen_Tire_cont_front);
	lv_label_set_text(ui->screen_Tire_label_fPressure, "--Bar");
	lv_label_set_long_mode(ui->screen_Tire_label_fPressure, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_Tire_label_fPressure, 0, 100);
	lv_obj_set_size(ui->screen_Tire_label_fPressure, 230, 70);
	lv_obj_add_style(ui->screen_Tire_label_fPressure, &style_text_align_center_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_Tire_btn_fPair
	ui->screen_Tire_btn_fPair = lv_btn_create(ui->screen_Tire_cont_front);
	ui->screen_Tire_btn_fPair_label = lv_label_create(ui->screen_Tire_btn_fPair);
	lv_label_set_text(ui->screen_Tire_btn_fPair_label,  get_string_for_language(g_sys_Data.current_language,"tire_txt_nPair"));
	lv_label_set_long_mode(ui->screen_Tire_btn_fPair_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_align(ui->screen_Tire_btn_fPair_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_Tire_btn_fPair, 0, LV_STATE_DEFAULT);
	lv_obj_set_width(ui->screen_Tire_btn_fPair_label, LV_PCT(100));
	lv_obj_set_pos(ui->screen_Tire_btn_fPair, 0, 180);
	lv_obj_set_size(ui->screen_Tire_btn_fPair, 460, 60);

	//Write style for screen_Tire_btn_fPair, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_Tire_btn_fPair, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_btn_fPair, lv_color_hex(0xfcfcfc), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_Tire_btn_fPair, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_Tire_btn_fPair, 15, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_Tire_btn_fPair, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_btn_fPair, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_Tire_btn_fPair, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_Tire_btn_fPair, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_Tire_btn_fPair, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_Tire_cont_flexLabel
	ui->screen_Tire_cont_flexLabel = lv_obj_create(ui->screen_Tire);
	lv_obj_set_pos(ui->screen_Tire_cont_flexLabel, 630, 50);
	lv_obj_set_size(ui->screen_Tire_cont_flexLabel, 400, 620);
	lv_obj_set_scrollbar_mode(ui->screen_Tire_cont_flexLabel, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_Tire_cont_front, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_opa(ui->screen_Tire_cont_flexLabel, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_Tire_cont_flexLabel, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_Tire_cont_flexLabel, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_cont_flexLabel, lv_color_hex(0xb7cbd4), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_clear_flag(ui->screen_Tire_cont_flexLabel, LV_OBJ_FLAG_SCROLLABLE);	
	lv_obj_set_flex_flow(ui->screen_Tire_cont_flexLabel, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(ui->screen_Tire_cont_flexLabel, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

	//Write codes screen_Tire_label_clearPair
	ui->screen_Tire_label_clearPair = lv_label_create(ui->screen_Tire_cont_flexLabel);
	lv_label_set_text(ui->screen_Tire_label_clearPair, get_string_for_language(g_sys_Data.current_language,"tire_txt_clearPair"));
	lv_label_set_long_mode(ui->screen_Tire_label_clearPair, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_size(ui->screen_Tire_label_clearPair, 400, 70);
	lv_obj_add_style(ui->screen_Tire_label_clearPair, &style_text_align_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	//Write codes screen_Tire_label_paraReset
	ui->screen_Tire_label_paraReset = lv_label_create(ui->screen_Tire_cont_flexLabel);
	lv_label_set_text(ui->screen_Tire_label_paraReset, get_string_for_language(g_sys_Data.current_language,"tire_txt_paraReset"));
	lv_label_set_long_mode(ui->screen_Tire_label_paraReset, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_size(ui->screen_Tire_label_paraReset, 400, 70);
	lv_obj_add_style(ui->screen_Tire_label_paraReset, &style_text_align_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	//Write codes screen_Tire_label_tempUnit
	ui->screen_Tire_label_tempUnit = lv_label_create(ui->screen_Tire_cont_flexLabel);
	lv_label_set_text(ui->screen_Tire_label_tempUnit, get_string_for_language(g_sys_Data.current_language,"tire_txt_tempUnit"));
	lv_label_set_long_mode(ui->screen_Tire_label_tempUnit, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_size(ui->screen_Tire_label_tempUnit, 400, 70);
	lv_obj_add_style(ui->screen_Tire_label_tempUnit, &style_text_align_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	//Write codes screen_Tire_label_pressureUnit
	ui->screen_Tire_label_pressureUnit = lv_label_create(ui->screen_Tire_cont_flexLabel);
	lv_label_set_text(ui->screen_Tire_label_pressureUnit, get_string_for_language(g_sys_Data.current_language,"tire_txt_pressUnit"));
	lv_label_set_long_mode(ui->screen_Tire_label_pressureUnit, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_size(ui->screen_Tire_label_pressureUnit, 400, 70);
	lv_obj_add_style(ui->screen_Tire_label_pressureUnit, &style_text_align_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_Tire_label_pressureMin
	ui->screen_Tire_label_pressureMin = lv_label_create(ui->screen_Tire_cont_flexLabel);
	lv_label_set_text(ui->screen_Tire_label_pressureMin,  get_string_for_language(g_sys_Data.current_language,"tire_txt_pressMin"));
	lv_label_set_long_mode(ui->screen_Tire_label_pressureMin, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_size(ui->screen_Tire_label_pressureMin, 400, 70);
	lv_obj_add_style(ui->screen_Tire_label_pressureMin, &style_text_align_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_Tire_label_pressureMax
	ui->screen_Tire_label_pressureMax = lv_label_create(ui->screen_Tire_cont_flexLabel);
	lv_label_set_text(ui->screen_Tire_label_pressureMax, get_string_for_language(g_sys_Data.current_language,"tire_txt_pressMax"));
	lv_label_set_long_mode(ui->screen_Tire_label_pressureMax, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_size(ui->screen_Tire_label_pressureMax, 400, 70);
	lv_obj_add_style(ui->screen_Tire_label_pressureMax, &style_text_align_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	//Write codes screen_Tire_label_tempMax
	ui->screen_Tire_label_tempMax = lv_label_create(ui->screen_Tire_cont_flexLabel);
	lv_label_set_text(ui->screen_Tire_label_tempMax, get_string_for_language(g_sys_Data.current_language,"tire_txt_tempMax"));
	lv_label_set_long_mode(ui->screen_Tire_label_tempMax, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_size(ui->screen_Tire_label_tempMax, 400, 70);
	lv_obj_add_style(ui->screen_Tire_label_tempMax, &style_text_align_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	lv_obj_move_to_index(ui->screen_Tire_label_tempMax, 0);
	lv_obj_move_to_index(ui->screen_Tire_label_pressureMax, 1);
	lv_obj_move_to_index(ui->screen_Tire_label_pressureMin, 2);
	lv_obj_move_to_index(ui->screen_Tire_label_pressureUnit, 3);
	lv_obj_move_to_index(ui->screen_Tire_label_tempUnit, 4);
	lv_obj_move_to_index(ui->screen_Tire_label_paraReset, 5);
	lv_obj_move_to_index(ui->screen_Tire_label_clearPair, 6);
	
	//Write codes screen_Tire_cont_flexBtn
	ui->screen_Tire_cont_flexBtn = lv_obj_create(ui->screen_Tire);
	lv_obj_set_pos(ui->screen_Tire_cont_flexBtn, 1040, 50);
	lv_obj_set_size(ui->screen_Tire_cont_flexBtn, 350, 620);
	lv_obj_set_scrollbar_mode(ui->screen_Tire_cont_flexBtn, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_Tire_cont_front, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_opa(ui->screen_Tire_cont_flexBtn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_Tire_cont_flexBtn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_Tire_cont_flexBtn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_cont_flexBtn, lv_color_hex(0xb7cbd4), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_clear_flag(ui->screen_Tire_cont_flexBtn, LV_OBJ_FLAG_SCROLLABLE);	
	lv_obj_set_flex_flow(ui->screen_Tire_cont_flexBtn, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(ui->screen_Tire_cont_flexBtn, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

	//Write codes screen_Tire_cont_pressureMin
	ui->screen_Tire_cont_pressureMin = lv_obj_create(ui->screen_Tire_cont_flexBtn);
	lv_obj_set_size(ui->screen_Tire_cont_pressureMin, 300, 70);
	lv_obj_set_style_border_opa(ui->screen_Tire_cont_pressureMin, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_Tire_cont_pressureMin, 15, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_Tire_cont_pressureMin, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_cont_pressureMin, lv_color_hex(0xecf5f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_all(ui->screen_Tire_cont_pressureMin, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_clear_flag(ui->screen_Tire_cont_pressureMin, LV_OBJ_FLAG_SCROLLABLE);

	//Write codes screen_Tire_btn_rPressMin
	ui->screen_Tire_btn_rPressMin = lv_btn_create(ui->screen_Tire_cont_pressureMin);
	lv_obj_set_pos(ui->screen_Tire_btn_rPressMin, 240, 0);
	lv_obj_set_size(ui->screen_Tire_btn_rPressMin, 60, 75);

	//Write style for screen_Tire_btn_rPressMin, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_Tire_btn_rPressMin, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_Tire_btn_rPressMin, &_right_but_normal_day_60x75, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_Tire_btn_rPressMin, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_Tire_btn_rPressMin, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_Tire_btn_rPressMin, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_src(ui->screen_Tire_btn_rPressMin, &_right_but_down_60x75, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_opa(ui->screen_Tire_btn_rPressMin, 255, LV_PART_MAIN|LV_STATE_PRESSED);

	//Write codes screen_Tire_label_pressMin1
	ui->screen_Tire_label_pressMin1 = lv_label_create(ui->screen_Tire_cont_pressureMin);
	lv_label_set_text(ui->screen_Tire_label_pressMin1, "--");
	lv_label_set_long_mode(ui->screen_Tire_label_pressMin1, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_Tire_label_pressMin1, 60, 0);
	lv_obj_set_size(ui->screen_Tire_label_pressMin1, 180, 70);
	lv_obj_add_style(ui->screen_Tire_label_pressMin1 , &style_text_align_center_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_Tire_btn_lPressMin
	ui->screen_Tire_btn_lPressMin = lv_btn_create(ui->screen_Tire_cont_pressureMin);
	lv_obj_set_pos(ui->screen_Tire_btn_lPressMin, 0, 0);
	lv_obj_set_size(ui->screen_Tire_btn_lPressMin, 60, 75);

	//Write style for screen_Tire_btn_lPressMin, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_Tire_btn_lPressMin, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_Tire_btn_lPressMin, &_left_but_normal_day_60x75, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_Tire_btn_lPressMin, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_Tire_btn_lPressMin, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_Tire_btn_lPressMin, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_src(ui->screen_Tire_btn_lPressMin, &_left_but_down_60x75, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_opa(ui->screen_Tire_btn_lPressMin, 255, LV_PART_MAIN|LV_STATE_PRESSED);

	//Write codes screen_Tire_cont_pressureMax
	ui->screen_Tire_cont_pressureMax = lv_obj_create(ui->screen_Tire_cont_flexBtn);
	lv_obj_set_size(ui->screen_Tire_cont_pressureMax, 300, 70);
	//Write style for screen_Tire_cont_pressureMax, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_opa(ui->screen_Tire_cont_pressureMax, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_Tire_cont_pressureMax, 15, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_Tire_cont_pressureMax, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_cont_pressureMax, lv_color_hex(0xecf5f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_all(ui->screen_Tire_cont_pressureMax, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_clear_flag(ui->screen_Tire_cont_pressureMax, LV_OBJ_FLAG_SCROLLABLE);	

	//Write codes screen_Tire_btn_lPressMax
	ui->screen_Tire_btn_lPressMax = lv_btn_create(ui->screen_Tire_cont_pressureMax);
	lv_obj_set_pos(ui->screen_Tire_btn_lPressMax, 0, 0);
	lv_obj_set_size(ui->screen_Tire_btn_lPressMax, 60, 75);
	lv_obj_set_style_bg_opa(ui->screen_Tire_btn_lPressMax, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_Tire_btn_lPressMax, &_left_but_normal_day_60x75, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_Tire_btn_lPressMax, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_Tire_btn_lPressMax, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_Tire_btn_lPressMax, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_src(ui->screen_Tire_btn_lPressMax, &_left_but_down_60x75, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_opa(ui->screen_Tire_btn_lPressMax, 255, LV_PART_MAIN|LV_STATE_PRESSED);

	//Write codes screen_Tire_btn_rPressMax
	ui->screen_Tire_btn_rPressMax = lv_btn_create(ui->screen_Tire_cont_pressureMax);
	lv_obj_set_pos(ui->screen_Tire_btn_rPressMax, 240, 0);
	lv_obj_set_size(ui->screen_Tire_btn_rPressMax, 60, 75);

	//Write style for screen_Tire_btn_rPressMax, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_Tire_btn_rPressMax, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_Tire_btn_rPressMax, &_right_but_normal_day_60x75, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_Tire_btn_rPressMax, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_Tire_btn_rPressMax, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_Tire_btn_rPressMax, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_src(ui->screen_Tire_btn_rPressMax, &_right_but_down_60x75, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_opa(ui->screen_Tire_btn_rPressMax, 255, LV_PART_MAIN|LV_STATE_PRESSED);

	//Write codes screen_Tire_label_pressMax1
	ui->screen_Tire_label_pressMax1 = lv_label_create(ui->screen_Tire_cont_pressureMax);
	lv_label_set_text(ui->screen_Tire_label_pressMax1, "--");
	lv_label_set_long_mode(ui->screen_Tire_label_pressMax1, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_Tire_label_pressMax1, 60, 0);
	lv_obj_set_size(ui->screen_Tire_label_pressMax1, 180, 70);
	lv_obj_add_style(ui->screen_Tire_label_pressMax1, &style_text_align_center_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_Tire_cont_tempMax
	ui->screen_Tire_cont_tempMax = lv_obj_create(ui->screen_Tire_cont_flexBtn);
	lv_obj_set_size(ui->screen_Tire_cont_tempMax, 300, 70);
	//Write style for screen_Tire_cont_tempMax, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_opa(ui->screen_Tire_cont_tempMax, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_Tire_cont_tempMax, 15, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_Tire_cont_tempMax, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_cont_tempMax, lv_color_hex(0xecf5f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_all(ui->screen_Tire_cont_tempMax, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_clear_flag(ui->screen_Tire_cont_tempMax, LV_OBJ_FLAG_SCROLLABLE);

	//Write codes screen_Tire_btn_lTemp
	ui->screen_Tire_btn_lTemp = lv_btn_create(ui->screen_Tire_cont_tempMax);
	lv_obj_set_pos(ui->screen_Tire_btn_lTemp, 0, 0);
	lv_obj_set_size(ui->screen_Tire_btn_lTemp, 60, 75);

	//Write style for screen_Tire_btn_lTemp, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_Tire_btn_lTemp, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_Tire_btn_lTemp, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_Tire_btn_lTemp, &_left_but_normal_day_60x75, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_Tire_btn_lTemp, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_Tire_btn_lTemp, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_Tire_btn_lTemp, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_radius(ui->screen_Tire_btn_lTemp, 5, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_src(ui->screen_Tire_btn_lTemp, &_left_but_down_60x75, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_opa(ui->screen_Tire_btn_lTemp, 255, LV_PART_MAIN|LV_STATE_PRESSED);

	//Write codes screen_Tire_btn_rTemp
	ui->screen_Tire_btn_rTemp = lv_btn_create(ui->screen_Tire_cont_tempMax);
	lv_obj_set_pos(ui->screen_Tire_btn_rTemp, 240, 0);
	lv_obj_set_size(ui->screen_Tire_btn_rTemp, 60, 75);

	//Write style for screen_Tire_btn_rTemp, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_Tire_btn_rTemp, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_Tire_btn_rTemp, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_Tire_btn_rTemp, &_right_but_normal_day_60x75, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_Tire_btn_rTemp, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_Tire_btn_rTemp, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_Tire_btn_rTemp, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_radius(ui->screen_Tire_btn_rTemp, 5, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_src(ui->screen_Tire_btn_rTemp, &_right_but_down_60x75, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_opa(ui->screen_Tire_btn_rTemp, 255, LV_PART_MAIN|LV_STATE_PRESSED);

	//Write codes screen_Tire_label_tempMax1
	ui->screen_Tire_label_tempMax1 = lv_label_create(ui->screen_Tire_cont_tempMax);
	lv_label_set_text(ui->screen_Tire_label_tempMax1, "--");
	lv_label_set_long_mode(ui->screen_Tire_label_tempMax1, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_Tire_label_tempMax1, 60, 0);
	lv_obj_set_size(ui->screen_Tire_label_tempMax1, 180, 70);
	lv_obj_add_style(ui->screen_Tire_label_tempMax1, &style_text_align_center_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_Tire_cont_pressBtn
	ui->screen_Tire_cont_pressBtn = lv_obj_create(ui->screen_Tire_cont_flexBtn);
	lv_obj_set_size(ui->screen_Tire_cont_pressBtn, 300, 70);
	//Write style for screen_Tire_cont_pressBtn, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_opa(ui->screen_Tire_cont_pressBtn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_Tire_cont_pressBtn, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_Tire_cont_pressBtn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_cont_pressBtn, lv_color_hex(0xdef6ff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_all(ui->screen_Tire_cont_pressBtn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_clear_flag(ui->screen_Tire_cont_pressBtn, LV_OBJ_FLAG_SCROLLABLE);	
	lv_obj_set_flex_flow(ui->screen_Tire_cont_pressBtn, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(ui->screen_Tire_cont_pressBtn, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
	lv_obj_set_style_pad_row(ui->screen_Tire_cont_pressBtn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_column(ui->screen_Tire_cont_pressBtn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_gap(ui->screen_Tire_cont_pressBtn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_Tire_btn_pressBar
	ui->screen_Tire_btn_pressBar = lv_btn_create(ui->screen_Tire_cont_pressBtn);
	ui->screen_Tire_btn_pressBar_label = lv_label_create(ui->screen_Tire_btn_pressBar);
	lv_label_set_text(ui->screen_Tire_btn_pressBar_label, "Bar");
	lv_label_set_long_mode(ui->screen_Tire_btn_pressBar_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_align(ui->screen_Tire_btn_pressBar_label, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_Tire_btn_pressBar, 0, LV_STATE_DEFAULT);
	lv_obj_set_width(ui->screen_Tire_btn_pressBar_label, LV_PCT(100));
	lv_obj_set_pos(ui->screen_Tire_btn_pressBar, 0, 0);
	lv_obj_set_size(ui->screen_Tire_btn_pressBar, 150, 70);

	//Write style for screen_Tire_btn_pressBar, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_Tire_btn_pressBar, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_Tire_btn_pressBar, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_btn_pressBar, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_Tire_btn_pressBar, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_Tire_btn_pressBar, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_Tire_btn_pressBar, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_Tire_btn_pressBar_label, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
	ui->screen_Tire_img_pressBar = lv_img_create(ui->screen_Tire_btn_pressBar);
	lv_obj_set_size(ui->screen_Tire_img_pressBar,40,40);
	lv_obj_set_pos(ui->screen_Tire_img_pressBar,100,10);
	lv_obj_set_style_bg_img_opa(ui->screen_Tire_img_pressBar,0,LV_PART_MAIN|LV_STATE_DEFAULT|LV_STATE_PRESSED);

	//Write codes screen_Tire_btn_pressPsj
	ui->screen_Tire_btn_pressPsj = lv_btn_create(ui->screen_Tire_cont_pressBtn);
	ui->screen_Tire_btn_pressPsj_label = lv_label_create(ui->screen_Tire_btn_pressPsj);
	lv_label_set_text(ui->screen_Tire_btn_pressPsj_label, "Psi");
	lv_label_set_long_mode(ui->screen_Tire_btn_pressPsj_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_align(ui->screen_Tire_btn_pressPsj_label, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_Tire_btn_pressPsj, 0, LV_STATE_DEFAULT);
	lv_obj_set_width(ui->screen_Tire_btn_pressPsj_label, LV_PCT(100));
	lv_obj_set_pos(ui->screen_Tire_btn_pressPsj, 150, 0);
	lv_obj_set_size(ui->screen_Tire_btn_pressPsj, 150, 70);
	
	//Write style for screen_Tire_btn_pressPsj, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_Tire_btn_pressPsj, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_Tire_btn_pressPsj, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_btn_pressPsj, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_Tire_btn_pressPsj, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_Tire_btn_pressPsj, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_Tire_btn_pressPsj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_Tire_btn_pressPsj_label, 10, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_Tire_img_pressPsj = lv_img_create(ui->screen_Tire_btn_pressPsj);
	lv_obj_set_size(ui->screen_Tire_img_pressPsj,40,40);
	lv_obj_set_pos(ui->screen_Tire_img_pressPsj,100,10);
	lv_obj_set_style_bg_img_opa(ui->screen_Tire_img_pressPsj,0,LV_PART_MAIN|LV_STATE_DEFAULT|LV_STATE_PRESSED);


	//Write codes screen_Tire_cont_tempBtn
	ui->screen_Tire_cont_tempBtn = lv_obj_create(ui->screen_Tire_cont_flexBtn);
	lv_obj_set_size(ui->screen_Tire_cont_tempBtn, 300, 70);
	//Write style for screen_Tire_cont_tempBtn, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_opa(ui->screen_Tire_cont_tempBtn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_Tire_cont_tempBtn, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_Tire_cont_tempBtn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_cont_tempBtn, lv_color_hex(0xdef6ff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_all(ui->screen_Tire_cont_tempBtn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_clear_flag(ui->screen_Tire_cont_tempBtn, LV_OBJ_FLAG_SCROLLABLE);	
	lv_obj_set_flex_flow(ui->screen_Tire_cont_tempBtn, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(ui->screen_Tire_cont_tempBtn, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
	lv_obj_set_style_pad_row(ui->screen_Tire_cont_tempBtn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_column(ui->screen_Tire_cont_tempBtn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_gap(ui->screen_Tire_cont_tempBtn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_Tire_btn_cUnit
	ui->screen_Tire_btn_tempC = lv_btn_create(ui->screen_Tire_cont_tempBtn);
	ui->screen_Tire_btn_tempC_label = lv_label_create(ui->screen_Tire_btn_tempC);
	lv_label_set_text(ui->screen_Tire_btn_tempC_label, "℃");
	lv_label_set_long_mode(ui->screen_Tire_btn_tempC_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_align(ui->screen_Tire_btn_tempC_label, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_Tire_btn_tempC, 0, LV_STATE_DEFAULT);
	lv_obj_set_width(ui->screen_Tire_btn_tempC_label, LV_PCT(100));
	lv_obj_set_size(ui->screen_Tire_btn_tempC, 150, 70);
	lv_obj_set_pos(ui->screen_Tire_btn_tempC, 0, 0);
	//Write style for screen_Tire_btn_cUnit, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_Tire_btn_tempC, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_btn_tempC, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_Tire_btn_tempC, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_Tire_btn_tempC, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_Tire_btn_tempC, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_Tire_btn_tempC_label, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
	//write codes for screen_Tire_pic_tempF
	ui->screen_Tire_img_tempC = lv_img_create(ui->screen_Tire_btn_tempC);
	lv_obj_set_size(ui->screen_Tire_img_tempC,40,40);
	lv_obj_set_pos(ui->screen_Tire_img_tempC,100,10);
	lv_obj_set_style_bg_img_opa(ui->screen_Tire_img_tempC,0,LV_PART_MAIN|LV_STATE_DEFAULT|LV_STATE_PRESSED);

	//Write codes screen_Tire_btn_fUnit
	ui->screen_Tire_btn_tempF = lv_btn_create(ui->screen_Tire_cont_tempBtn);
	ui->screen_Tire_btn_tempF_label = lv_label_create(ui->screen_Tire_btn_tempF);
	lv_label_set_text(ui->screen_Tire_btn_tempF_label, "℉");
	lv_label_set_long_mode(ui->screen_Tire_btn_tempF_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_align(ui->screen_Tire_btn_tempF_label, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_Tire_btn_tempF, 0, LV_STATE_DEFAULT);
	lv_obj_set_width(ui->screen_Tire_btn_tempF_label, LV_PCT(100));
	lv_obj_set_size(ui->screen_Tire_btn_tempF, 150, 70);
	lv_obj_set_pos(ui->screen_Tire_btn_tempF, 150, 0);
	//Write style for screen_Tire_btn_fUnit, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_Tire_btn_tempF, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_Tire_btn_tempF, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_btn_tempF, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_Tire_btn_tempF, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_Tire_btn_tempF, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_Tire_btn_tempF, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_Tire_btn_tempF_label, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
	//write codes for screen_Tire_pic_tempF
	ui->screen_Tire_img_tempF = lv_img_create(ui->screen_Tire_btn_tempF);
	lv_obj_set_size(ui->screen_Tire_img_tempF,40,40);
	lv_obj_set_pos(ui->screen_Tire_img_tempF,100,10);
	lv_obj_set_style_bg_img_opa(ui->screen_Tire_img_tempF,0,LV_PART_MAIN|LV_STATE_DEFAULT|LV_STATE_PRESSED);

	//Write codes screen_Tire_btn_paraReset
	ui->screen_Tire_btn_paraReset = lv_btn_create(ui->screen_Tire_cont_flexBtn);
	ui->screen_Tire_btn_paraReset_label = lv_label_create(ui->screen_Tire_btn_paraReset);
	lv_label_set_text(ui->screen_Tire_btn_paraReset_label, get_string_for_language(g_sys_Data.current_language,"tire_txt_paraReset1"));
	lv_label_set_long_mode(ui->screen_Tire_btn_paraReset_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_align(ui->screen_Tire_btn_paraReset_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_Tire_btn_paraReset, 0, LV_STATE_DEFAULT);
	lv_obj_set_width(ui->screen_Tire_btn_paraReset_label, LV_PCT(100));
	lv_obj_set_size(ui->screen_Tire_btn_paraReset, 250, 70);

	//Write style for screen_Tire_btn_paraReset, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_Tire_btn_paraReset, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_btn_paraReset, lv_color_hex(0x737373), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_Tire_btn_paraReset, 30, LV_PART_MAIN|LV_STATE_DEFAULT);

	lv_obj_set_style_bg_color(ui->screen_Tire_btn_paraReset, lv_color_hex(0x2d73e3), LV_PART_MAIN|LV_STATE_PRESSED);

	lv_obj_remove_style_all(ui->screen_Tire_btn_paraReset_label);
	lv_obj_center(ui->screen_Tire_btn_paraReset_label);
	lv_obj_add_style(ui->screen_Tire_btn_paraReset_label, &style_text_align_center_mian_default, LV_PART_MAIN);

	//Write codes screen_Tire_btn_clearPair
	ui->screen_Tire_btn_clearPair = lv_btn_create(ui->screen_Tire_cont_flexBtn);
	ui->screen_Tire_btn_clearPair_label = lv_label_create(ui->screen_Tire_btn_clearPair);
	lv_label_set_text(ui->screen_Tire_btn_clearPair_label, get_string_for_language(g_sys_Data.current_language,"tire_txt_clearPair1"));
	lv_label_set_long_mode(ui->screen_Tire_btn_clearPair_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_align(ui->screen_Tire_btn_clearPair_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_Tire_btn_clearPair, 0, LV_STATE_DEFAULT);
	lv_obj_set_width(ui->screen_Tire_btn_clearPair_label, LV_PCT(100));
	lv_obj_set_size(ui->screen_Tire_btn_clearPair, 250, 70);

	//Write style for screen_Tire_btn_clearPair, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_Tire_btn_clearPair, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_btn_clearPair, lv_color_hex(0x737373), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_Tire_btn_clearPair, 30, LV_PART_MAIN|LV_STATE_DEFAULT);

	lv_obj_set_style_bg_color(ui->screen_Tire_btn_clearPair, lv_color_hex(0x2d73e3), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_remove_style_all(ui->screen_Tire_btn_clearPair_label);
	lv_obj_center(ui->screen_Tire_btn_clearPair_label);
	lv_obj_add_style(ui->screen_Tire_btn_clearPair_label, &style_text_align_center_mian_default, LV_PART_MAIN);

	lv_obj_move_to_index(ui->screen_Tire_cont_tempMax, 0);
	lv_obj_move_to_index(ui->screen_Tire_cont_pressureMax, 1);
	lv_obj_move_to_index(ui->screen_Tire_cont_pressureMin, 2);
	lv_obj_move_to_index(ui->screen_Tire_cont_pressBtn, 3);
	lv_obj_move_to_index(ui->screen_Tire_cont_tempBtn, 4);
	lv_obj_move_to_index(ui->screen_Tire_btn_paraReset, 5);
	lv_obj_move_to_index(ui->screen_Tire_btn_clearPair, 6);

	//The custom code of screen_Tire.


	//Update current screen layout.
	lv_obj_update_layout(ui->screen_Tire);

	//Init events for screen.
	events_init_screen_Tire(ui);

	THEME_MODE mode = g_sys_Data.themeMode;
	if(mode == THEME_DAY || (mode == THEME_AUTO && g_sys_Data.gpadcVol < 1300)){
		ScreenTireBlackNightMode(ui, THEME_DAY);
	}else{
		ScreenTireBlackNightMode(ui, THEME_DARK);
	}
}
void ScreenTireBlackNightMode(lv_ui *ui, THEME_MODE mode){
	printf("%s:%d\n",__func__,__LINE__);
	switch (mode)
	{
	case THEME_DAY:
	lv_obj_set_style_bg_img_src(ui->screen_Tire_btn_return, &_back_dark_80x80, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_Tire_btn_rPressMin, &_right_but_normal_day_60x75, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_Tire_btn_rPressMax, &_right_but_normal_day_60x75, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_Tire_btn_rTemp, &_right_but_normal_day_60x75, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_Tire_btn_lPressMin, &_left_but_normal_day_60x75, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_Tire_btn_lPressMax, &_left_but_normal_day_60x75, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_Tire_btn_lTemp, &_left_but_normal_day_60x75, LV_PART_MAIN|LV_STATE_DEFAULT);
	if(!g_sys_Data.pressureUnit){
		char str[20];
		sprintf(str,"%.1f Bar",g_sys_Data.fPressure);
		lv_label_set_text(ui->screen_Tire_label_fPressure,str);
		sprintf(str,"%.1f Bar",g_sys_Data.bPressure);
		lv_label_set_text(ui->screen_Tire_label_bPressure,str);
		lv_img_set_src(ui->screen_Tire_img_pressBar,&_selected_40x40);
		lv_img_set_src(ui->screen_Tire_img_pressPsj,&_not_select_40x40);
	}else{
		char str[20];
		sprintf(str,"%.0f Psi",g_sys_Data.fPressure);
		lv_label_set_text(ui->screen_Tire_label_fPressure,str);
		sprintf(str,"%.0f Psi",g_sys_Data.bPressure);
		lv_label_set_text(ui->screen_Tire_label_bPressure,str);
		lv_img_set_src(ui->screen_Tire_img_pressBar,&_not_select_40x40);
		lv_img_set_src(ui->screen_Tire_img_pressPsj,&_selected_40x40);
	}
	if(g_sys_Data.tempUnit){
		char str[20];
		sprintf(str,"%d ℉",g_sys_Data.fTemp);
		lv_label_set_text(ui->screen_Tire_label_fTemp,str);
		sprintf(str,"%d ℉",g_sys_Data.bTemp);
		lv_label_set_text(ui->screen_Tire_label_bTemp,str);		
		lv_img_set_src(ui->screen_Tire_img_tempC,&_not_select_40x40);
		lv_img_set_src(ui->screen_Tire_img_tempF,&_selected_40x40);
	}else{
		char str[20];
		sprintf(str,"%d ℃",g_sys_Data.fTemp);
		lv_label_set_text(ui->screen_Tire_label_fTemp,str);
		sprintf(str,"%d ℃",g_sys_Data.bTemp);
		lv_label_set_text(ui->screen_Tire_label_bTemp,str);
		lv_img_set_src(ui->screen_Tire_img_tempC,&_selected_40x40);
		lv_img_set_src(ui->screen_Tire_img_tempF,&_not_select_40x40);
	}

	lv_obj_set_style_bg_color(ui->screen_Tire, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_cont_front, lv_color_hex(0xecf5f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_cont_behind, lv_color_hex(0xecf5f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_btn_bPair, lv_color_hex(0xfcfcfc), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_btn_fPair, lv_color_hex(0xfcfcfc), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_cont_pressureMin, lv_color_hex(0xecf5f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_cont_pressureMax, lv_color_hex(0xecf5f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_cont_tempMax, lv_color_hex(0xecf5f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_btn_paraReset, lv_color_hex(0x737373), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_btn_clearPair, lv_color_hex(0x737373), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_bTemp, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_bPressure, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_bLable, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_btn_bPair_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_fLabel, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_fTemp, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_fPressure, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_btn_fPair_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_clearPair, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_paraReset, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_tempUnit, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_pressureUnit, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_pressureMin, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_pressureMax, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_tempMax, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_pressMax1, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_pressMin1, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_tempMax1, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_btn_pressBar_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_btn_pressPsj_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_btn_tempC_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_btn_tempF_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_btn_paraReset_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_btn_clearPair_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);	
		break;
	case THEME_DARK:
	lv_obj_set_style_bg_img_src(ui->screen_Tire_btn_return, &_back_60x60, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_Tire_btn_rPressMin, &_right_but_normal_night_60x75, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_Tire_btn_rPressMax, &_right_but_normal_night_60x75, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_Tire_btn_rTemp, &_right_but_normal_night_60x75, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_Tire_btn_lPressMin, &_left_but_normal_night_60x75, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_Tire_btn_lPressMax, &_left_but_normal_night_60x75, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_Tire_btn_lTemp, &_left_but_normal_night_60x75, LV_PART_MAIN|LV_STATE_DEFAULT);
	if(!g_sys_Data.pressureUnit){
		char str[20];
		sprintf(str,"%.1f Bar",g_sys_Data.fPressure);
		lv_label_set_text(ui->screen_Tire_label_fPressure,str);
		sprintf(str,"%.1f Bar",g_sys_Data.bPressure);
		lv_label_set_text(ui->screen_Tire_label_bPressure,str);
		lv_img_set_src(ui->screen_Tire_img_pressBar,&_selected_dark_40x40);
		lv_img_set_src(ui->screen_Tire_img_pressPsj,&_not_select_dark_40x40);
	}else{
		char str[20];
		sprintf(str,"%.0f Psi",g_sys_Data.fPressure);
		lv_label_set_text(ui->screen_Tire_label_fPressure,str);
		sprintf(str,"%.0f Psi",g_sys_Data.bPressure);
		lv_label_set_text(ui->screen_Tire_label_bPressure,str);
		lv_img_set_src(ui->screen_Tire_img_pressBar,&_not_select_dark_40x40);
		lv_img_set_src(ui->screen_Tire_img_pressPsj,&_selected_dark_40x40);
	}
	if(g_sys_Data.tempUnit){
		char str[20];
		sprintf(str,"%d ℉",g_sys_Data.fTemp);
		lv_label_set_text(ui->screen_Tire_label_fTemp,str);
		sprintf(str,"%d ℉",g_sys_Data.bTemp);
		lv_label_set_text(ui->screen_Tire_label_bTemp,str);		
		lv_img_set_src(ui->screen_Tire_img_tempC,&_not_select_dark_40x40);
		lv_img_set_src(ui->screen_Tire_img_tempF,&_selected_dark_40x40);
	}else{
		char str[20];
		sprintf(str,"%d ℃",g_sys_Data.fTemp);
		lv_label_set_text(ui->screen_Tire_label_fTemp,str);
		sprintf(str,"%d ℃",g_sys_Data.bTemp);
		lv_label_set_text(ui->screen_Tire_label_bTemp,str);
		lv_img_set_src(ui->screen_Tire_img_tempC,&_selected_dark_40x40);
		lv_img_set_src(ui->screen_Tire_img_tempF,&_not_select_dark_40x40);
	}

	lv_obj_set_style_bg_color(ui->screen_Tire, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_cont_front, lv_color_hex(0x5f5f5f), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_cont_behind, lv_color_hex(0x5f5f5f), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_btn_bPair, lv_color_hex(0xcfcfcf), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_btn_fPair, lv_color_hex(0xcfcfcf), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_cont_pressureMin, lv_color_hex(0x5f5f5f), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_cont_pressureMax, lv_color_hex(0x5f5f5f), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_cont_tempMax, lv_color_hex(0x5f5f5f), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_btn_paraReset, lv_color_hex(0xcfcfcf), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_Tire_btn_clearPair, lv_color_hex(0xcfcfcf), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_bTemp, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_bPressure, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_bLable, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_btn_bPair_label, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_fLabel, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_fTemp, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_fPressure, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_btn_fPair_label, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_clearPair, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_paraReset, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_tempUnit, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_pressureUnit, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_pressureMin, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_pressureMax, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_tempMax, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_pressMax1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_pressMin1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_label_tempMax1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_btn_pressBar_label, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_btn_pressPsj_label, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_btn_tempC_label, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_btn_tempF_label, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_btn_paraReset_label, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_Tire_btn_clearPair_label, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		break;	
	default:
		break;
	}
}