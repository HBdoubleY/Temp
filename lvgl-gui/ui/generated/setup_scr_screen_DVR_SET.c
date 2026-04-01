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

static lv_style_t style_text_align_left_mian_default;
void RecorderTimeBtnSta();
void screenDvrSetBlackNightMode(lv_ui *ui, THEME_MODE mode);
void setup_scr_screen_DVR_SET(lv_ui *ui)
{
	//write all label style_text_align_left_mian_default style
	ui_init_style(&style_text_align_left_mian_default);

	lv_style_set_text_color(&style_text_align_left_mian_default, lv_color_hex(0x000000));
	lv_style_set_text_font(&style_text_align_left_mian_default, &lv_font_harmonyOS_42);
	lv_style_set_text_opa(&style_text_align_left_mian_default, 255);
	lv_style_set_text_align(&style_text_align_left_mian_default, LV_TEXT_ALIGN_LEFT);
	lv_style_set_pad_top(&style_text_align_left_mian_default, 25);
	lv_style_set_pad_left(&style_text_align_left_mian_default, 30);
	lv_style_set_bg_opa(&style_text_align_left_mian_default, 0);
	lv_style_set_bg_color(&style_text_align_left_mian_default, lv_color_hex(0x2195f6));
	lv_style_set_border_opa(&style_text_align_left_mian_default, 0);

	//Write codes screen_DVR_SET
	ui->screen_DVR_SET = lv_obj_create(NULL);
	lv_obj_set_size(ui->screen_DVR_SET, 1440, 720);
	lv_obj_set_scrollbar_mode(ui->screen_DVR_SET, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_DVR_SET, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_DVR_SET, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->screen_DVR_SET, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_DVR_SET_cont_parent
	ui->screen_DVR_SET_cont_parent = lv_obj_create(ui->screen_DVR_SET);
	lv_obj_set_pos(ui->screen_DVR_SET_cont_parent, 0, 80);
	lv_obj_set_size(ui->screen_DVR_SET_cont_parent, 1440, 640);
	lv_obj_set_scrollbar_mode(ui->screen_DVR_SET_cont_parent, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_DVR_SET_cont_parent, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_DVR_SET_cont_parent, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->screen_DVR_SET_cont_parent, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->screen_DVR_SET_cont_parent, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_side(ui->screen_DVR_SET_cont_parent, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_SET_cont_parent, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_cont_parent, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_parent, lv_color_hex(0xE0E0E0), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_cont_parent, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_flex_flow(ui->screen_DVR_SET_cont_parent, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(ui->screen_DVR_SET_cont_parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);	
	lv_obj_set_style_pad_row(ui->screen_DVR_SET_cont_parent, 20,  LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_DVR_SET_cont_rearCameraImage
	ui->screen_DVR_SET_cont_rearCameraImage = lv_obj_create(ui->screen_DVR_SET_cont_parent);
	lv_obj_set_size(ui->screen_DVR_SET_cont_rearCameraImage, 1260, 100);
	lv_obj_set_scrollbar_mode(ui->screen_DVR_SET_cont_rearCameraImage, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_DVR_SET_cont_rearCameraImage, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_DVR_SET_cont_rearCameraImage, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->screen_DVR_SET_cont_rearCameraImage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->screen_DVR_SET_cont_rearCameraImage, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_side(ui->screen_DVR_SET_cont_rearCameraImage, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_SET_cont_rearCameraImage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_cont_rearCameraImage, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_rearCameraImage, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->screen_DVR_SET_cont_rearCameraImage, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_DVR_SET_cont_rearCameraImage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_DVR_SET_cont_rearCameraImage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_DVR_SET_cont_rearCameraImage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_DVR_SET_cont_rearCameraImage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_cont_rearCameraImage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_DVR_SET_label_rearCameraImage
	ui->screen_DVR_SET_label_rearCameraImage = lv_label_create(ui->screen_DVR_SET_cont_rearCameraImage);
	lv_label_set_text(ui->screen_DVR_SET_label_rearCameraImage, get_string_for_language(g_sys_Data.current_language, "dvr_set_txt_rearCameraImage"));
	lv_label_set_long_mode(ui->screen_DVR_SET_label_rearCameraImage, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_DVR_SET_label_rearCameraImage, 0, 0);
	lv_obj_set_size(ui->screen_DVR_SET_label_rearCameraImage, 600, 100);
	lv_obj_add_style(ui->screen_DVR_SET_label_rearCameraImage, &style_text_align_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_DVR_SET_btn_rearCameraImage
	ui->screen_DVR_SET_btn_rearCameraImage = lv_btn_create(ui->screen_DVR_SET_cont_rearCameraImage);
	lv_obj_set_style_pad_all(ui->screen_DVR_SET_btn_rearCameraImage, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(ui->screen_DVR_SET_btn_rearCameraImage, 1100, 15);
	lv_obj_set_size(ui->screen_DVR_SET_btn_rearCameraImage, 135, 61);

	//Write style for screen_DVR_SET_btn_rearCameraImage, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_btn_rearCameraImage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_DVR_SET_btn_rearCameraImage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_SET_btn_rearCameraImage, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_btn_rearCameraImage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_DVR_SET_btn_rearCameraImage, &_sw_off_135x61, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR_SET_btn_rearCameraImage, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR_SET_btn_rearCameraImage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_DVR_SET_btn_rearCameraImage, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_btn_rearCameraImage, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_border_width(ui->screen_DVR_SET_btn_rearCameraImage, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_radius(ui->screen_DVR_SET_btn_rearCameraImage, 5, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_btn_rearCameraImage, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_img_src(ui->screen_DVR_SET_btn_rearCameraImage, &_sw_on_135x61, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR_SET_btn_rearCameraImage, 255, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR_SET_btn_rearCameraImage, 0, LV_PART_MAIN|LV_STATE_CHECKED);


	//Write codes screen_DVR_SET_cont_recorderTime
	ui->screen_DVR_SET_cont_recorderTime = lv_obj_create(ui->screen_DVR_SET_cont_parent);
	lv_obj_set_size(ui->screen_DVR_SET_cont_recorderTime, 1260, 100);
	lv_obj_set_scrollbar_mode(ui->screen_DVR_SET_cont_recorderTime, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_DVR_SET_cont_recorderTime, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_DVR_SET_cont_recorderTime, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->screen_DVR_SET_cont_recorderTime, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->screen_DVR_SET_cont_recorderTime, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_side(ui->screen_DVR_SET_cont_recorderTime, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_SET_cont_recorderTime, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_cont_recorderTime, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_recorderTime, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->screen_DVR_SET_cont_recorderTime, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_DVR_SET_cont_recorderTime, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_DVR_SET_cont_recorderTime, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_DVR_SET_cont_recorderTime, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_DVR_SET_cont_recorderTime, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_cont_recorderTime, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_DVR_SET_label_recorderTime
	ui->screen_DVR_SET_label_recorderTime = lv_label_create(ui->screen_DVR_SET_cont_recorderTime);
	lv_label_set_text(ui->screen_DVR_SET_label_recorderTime, get_string_for_language(g_sys_Data.current_language, "dvr_set_txt_recorderTime"));
	lv_label_set_long_mode(ui->screen_DVR_SET_label_recorderTime, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_DVR_SET_label_recorderTime, 0, 0);
	lv_obj_set_size(ui->screen_DVR_SET_label_recorderTime, 600, 100);
	lv_obj_add_style(ui->screen_DVR_SET_label_recorderTime, &style_text_align_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_DVR_SET_btn_recorder_5min
	ui->screen_DVR_SET_btn_recorder_5min = lv_btn_create(ui->screen_DVR_SET_cont_recorderTime);
	ui->screen_DVR_SET_btn_recorder_5min_label = lv_label_create(ui->screen_DVR_SET_btn_recorder_5min);
	lv_label_set_text(ui->screen_DVR_SET_btn_recorder_5min_label, get_string_for_language(g_sys_Data.current_language, "dvr_set_txt_5min"));
	lv_label_set_long_mode(ui->screen_DVR_SET_btn_recorder_5min_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_align(ui->screen_DVR_SET_btn_recorder_5min_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_DVR_SET_btn_recorder_5min, 0, LV_STATE_DEFAULT);
	lv_obj_set_width(ui->screen_DVR_SET_btn_recorder_5min_label, LV_PCT(100));
	lv_obj_set_pos(ui->screen_DVR_SET_btn_recorder_5min, 1100, 20);
	lv_obj_set_size(ui->screen_DVR_SET_btn_recorder_5min, 150, 60);

	//Write style for screen_DVR_SET_btn_recorder_1min, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_btn_recorder_5min, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_DVR_SET_btn_recorder_5min, lv_color_hex(0xE0E0E0), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_DVR_SET_btn_recorder_5min, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_SET_btn_recorder_5min, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_btn_recorder_5min, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_recorder_5min, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_DVR_SET_btn_recorder_5min, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_DVR_SET_btn_recorder_5min, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_DVR_SET_btn_recorder_5min, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_DVR_SET_btn_urgent_3min, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_btn_recorder_5min, 255, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_color(ui->screen_DVR_SET_btn_recorder_5min, lv_color_hex(0x2196F3), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_border_width(ui->screen_DVR_SET_btn_recorder_5min, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_radius(ui->screen_DVR_SET_btn_recorder_5min, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_btn_recorder_5min, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_recorder_5min, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_font(ui->screen_DVR_SET_btn_recorder_5min, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_opa(ui->screen_DVR_SET_btn_recorder_5min, 255, LV_PART_MAIN|LV_STATE_CHECKED);

	//Write codes screen_DVR_SET_btn_recorder_1min
	ui->screen_DVR_SET_btn_recorder_1min = lv_btn_create(ui->screen_DVR_SET_cont_recorderTime);
	ui->screen_DVR_SET_btn_recorder_1min_label = lv_label_create(ui->screen_DVR_SET_btn_recorder_1min);
	lv_label_set_text(ui->screen_DVR_SET_btn_recorder_1min_label, get_string_for_language(g_sys_Data.current_language, "dvr_set_txt_1min"));
	lv_label_set_long_mode(ui->screen_DVR_SET_btn_recorder_1min_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_align(ui->screen_DVR_SET_btn_recorder_1min_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_DVR_SET_btn_recorder_1min, 0, LV_STATE_DEFAULT);
	lv_obj_set_width(ui->screen_DVR_SET_btn_recorder_1min_label, LV_PCT(100));
	lv_obj_set_pos(ui->screen_DVR_SET_btn_recorder_1min, 800, 20);
	lv_obj_set_size(ui->screen_DVR_SET_btn_recorder_1min, 150, 60);

	//Write style for screen_DVR_SET_btn_recorder_1min, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_btn_recorder_1min, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_DVR_SET_btn_recorder_1min, lv_color_hex(0xE0E0E0), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_DVR_SET_btn_recorder_1min, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_SET_btn_recorder_1min, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_btn_recorder_1min, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_recorder_1min, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_DVR_SET_btn_recorder_1min, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_DVR_SET_btn_recorder_1min, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_DVR_SET_btn_recorder_1min, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_DVR_SET_btn_urgent_3min, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_btn_recorder_1min, 255, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_color(ui->screen_DVR_SET_btn_recorder_1min, lv_color_hex(0x2196F3), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_border_width(ui->screen_DVR_SET_btn_recorder_1min, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_radius(ui->screen_DVR_SET_btn_recorder_1min, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_btn_recorder_1min, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_recorder_1min, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_font(ui->screen_DVR_SET_btn_recorder_1min, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_opa(ui->screen_DVR_SET_btn_recorder_1min, 255, LV_PART_MAIN|LV_STATE_CHECKED);


	//Write codes screen_DVR_SET_btn_recorder_3min
	ui->screen_DVR_SET_btn_recorder_3min = lv_btn_create(ui->screen_DVR_SET_cont_recorderTime);
	ui->screen_DVR_SET_btn_recorder_3min_label = lv_label_create(ui->screen_DVR_SET_btn_recorder_3min);
	lv_label_set_text(ui->screen_DVR_SET_btn_recorder_3min_label, get_string_for_language(g_sys_Data.current_language, "dvr_set_txt_3min"));
	lv_label_set_long_mode(ui->screen_DVR_SET_btn_recorder_3min_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_align(ui->screen_DVR_SET_btn_recorder_3min_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_DVR_SET_btn_recorder_3min, 0, LV_STATE_DEFAULT);
	lv_obj_set_width(ui->screen_DVR_SET_btn_recorder_3min_label, LV_PCT(100));
	lv_obj_set_pos(ui->screen_DVR_SET_btn_recorder_3min, 950, 20);
	lv_obj_set_size(ui->screen_DVR_SET_btn_recorder_3min, 150, 60);

	//Write style for screen_DVR_SET_btn_recorder_3min, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	//Write style for screen_DVR_SET_btn_recorder_1min, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_btn_recorder_3min, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_DVR_SET_btn_recorder_3min, lv_color_hex(0xE0E0E0), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_DVR_SET_btn_recorder_3min, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_SET_btn_recorder_3min, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_btn_recorder_3min, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_recorder_3min, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_DVR_SET_btn_recorder_3min, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_DVR_SET_btn_recorder_3min, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_DVR_SET_btn_recorder_3min, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_DVR_SET_btn_urgent_3min, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_btn_recorder_3min, 255, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_color(ui->screen_DVR_SET_btn_recorder_3min, lv_color_hex(0x2196F3), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_border_width(ui->screen_DVR_SET_btn_recorder_3min, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_radius(ui->screen_DVR_SET_btn_recorder_3min, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_btn_recorder_3min, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_recorder_3min, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_font(ui->screen_DVR_SET_btn_recorder_3min, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_opa(ui->screen_DVR_SET_btn_recorder_3min, 255, LV_PART_MAIN|LV_STATE_CHECKED);

	//Write codes screen_DVR_SET_cont_recorderUrgentTime
	ui->screen_DVR_SET_cont_recorderUrgentTime = lv_obj_create(ui->screen_DVR_SET_cont_parent);
	lv_obj_set_pos(ui->screen_DVR_SET_cont_recorderUrgentTime, 82, 490);
	lv_obj_set_size(ui->screen_DVR_SET_cont_recorderUrgentTime, 1260, 100);
	lv_obj_set_scrollbar_mode(ui->screen_DVR_SET_cont_recorderUrgentTime, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_DVR_SET_cont_recorderUrgentTime, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_DVR_SET_cont_recorderUrgentTime, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->screen_DVR_SET_cont_recorderUrgentTime, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->screen_DVR_SET_cont_recorderUrgentTime, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_side(ui->screen_DVR_SET_cont_recorderUrgentTime, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_SET_cont_recorderUrgentTime, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_cont_recorderUrgentTime, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_recorderUrgentTime, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->screen_DVR_SET_cont_recorderUrgentTime, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_DVR_SET_cont_recorderUrgentTime, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_DVR_SET_cont_recorderUrgentTime, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_DVR_SET_cont_recorderUrgentTime, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_DVR_SET_cont_recorderUrgentTime, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_cont_recorderUrgentTime, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_DVR_SET_label_recorderUrgentTime
	ui->screen_DVR_SET_label_recorderUrgentTime = lv_label_create(ui->screen_DVR_SET_cont_recorderUrgentTime);
	lv_label_set_text(ui->screen_DVR_SET_label_recorderUrgentTime, get_string_for_language(g_sys_Data.current_language, "dvr_set_txt_urgentRecorderTime"));
	lv_label_set_long_mode(ui->screen_DVR_SET_label_recorderUrgentTime, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_DVR_SET_label_recorderUrgentTime, 0, 0);
	lv_obj_set_size(ui->screen_DVR_SET_label_recorderUrgentTime, 600, 100);
	lv_obj_add_style(ui->screen_DVR_SET_label_recorderUrgentTime, &style_text_align_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_DVR_SET_btn_urgent_5min
	ui->screen_DVR_SET_btn_urgent_5min = lv_btn_create(ui->screen_DVR_SET_cont_recorderUrgentTime);
	ui->screen_DVR_SET_btn_urgent_5min_label = lv_label_create(ui->screen_DVR_SET_btn_urgent_5min);
	lv_label_set_text(ui->screen_DVR_SET_btn_urgent_5min_label, get_string_for_language(g_sys_Data.current_language, "dvr_set_txt_5min"));
	lv_label_set_long_mode(ui->screen_DVR_SET_btn_urgent_5min_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_align(ui->screen_DVR_SET_btn_urgent_5min_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_DVR_SET_btn_urgent_5min, 0, LV_STATE_DEFAULT);
	lv_obj_set_width(ui->screen_DVR_SET_btn_urgent_5min_label, LV_PCT(100));
	lv_obj_set_pos(ui->screen_DVR_SET_btn_urgent_5min, 1100, 20);
	lv_obj_set_size(ui->screen_DVR_SET_btn_urgent_5min, 150, 60);

	//Write style for screen_DVR_SET_btn_urgent_5min, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_btn_urgent_5min, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_DVR_SET_btn_urgent_5min, lv_color_hex(0xE0E0E0), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_DVR_SET_btn_urgent_5min, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_SET_btn_urgent_5min, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_btn_urgent_5min, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_urgent_5min, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_DVR_SET_btn_urgent_5min, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_DVR_SET_btn_urgent_5min, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_DVR_SET_btn_urgent_5min, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_DVR_SET_btn_urgent_5min, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_btn_urgent_5min, 255, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_color(ui->screen_DVR_SET_btn_urgent_5min, lv_color_hex(0x2196F3), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_border_width(ui->screen_DVR_SET_btn_urgent_5min, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_radius(ui->screen_DVR_SET_btn_urgent_5min, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_btn_urgent_5min, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_urgent_5min, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_font(ui->screen_DVR_SET_btn_urgent_5min, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_opa(ui->screen_DVR_SET_btn_urgent_5min, 255, LV_PART_MAIN|LV_STATE_CHECKED);

	//Write codes screen_DVR_SET_btn_urgent_1min
	ui->screen_DVR_SET_btn_urgent_1min = lv_btn_create(ui->screen_DVR_SET_cont_recorderUrgentTime);
	ui->screen_DVR_SET_btn_urgent_1min_label = lv_label_create(ui->screen_DVR_SET_btn_urgent_1min);
	lv_label_set_text(ui->screen_DVR_SET_btn_urgent_1min_label, get_string_for_language(g_sys_Data.current_language, "dvr_set_txt_1min"));
	lv_label_set_long_mode(ui->screen_DVR_SET_btn_urgent_1min_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_align(ui->screen_DVR_SET_btn_urgent_1min_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_DVR_SET_btn_urgent_1min, 0, LV_STATE_DEFAULT);
	lv_obj_set_width(ui->screen_DVR_SET_btn_urgent_1min_label, LV_PCT(100));
	lv_obj_set_pos(ui->screen_DVR_SET_btn_urgent_1min, 800, 20);
	lv_obj_set_size(ui->screen_DVR_SET_btn_urgent_1min, 150, 60);

	//Write style for screen_DVR_SET_btn_urgent_1min, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_btn_urgent_1min, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_DVR_SET_btn_urgent_1min, lv_color_hex(0xE0E0E0), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_DVR_SET_btn_urgent_1min, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_SET_btn_urgent_1min, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_btn_urgent_1min, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_urgent_1min, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_DVR_SET_btn_urgent_1min, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_DVR_SET_btn_urgent_1min, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_DVR_SET_btn_urgent_1min, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_DVR_SET_btn_urgent_1min, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_btn_urgent_1min, 255, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_color(ui->screen_DVR_SET_btn_urgent_1min, lv_color_hex(0x2196F3), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_border_width(ui->screen_DVR_SET_btn_urgent_1min, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_radius(ui->screen_DVR_SET_btn_urgent_1min, 0 ,LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_btn_urgent_1min, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_urgent_1min, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_font(ui->screen_DVR_SET_btn_urgent_1min, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_opa(ui->screen_DVR_SET_btn_urgent_1min, 255, LV_PART_MAIN|LV_STATE_CHECKED);

	//Write codes screen_DVR_SET_btn_urgent_3min
	ui->screen_DVR_SET_btn_urgent_3min = lv_btn_create(ui->screen_DVR_SET_cont_recorderUrgentTime);
	ui->screen_DVR_SET_btn_urgent_3min_label = lv_label_create(ui->screen_DVR_SET_btn_urgent_3min);
	lv_label_set_text(ui->screen_DVR_SET_btn_urgent_3min_label, get_string_for_language(g_sys_Data.current_language, "dvr_set_txt_3min"));
	lv_label_set_long_mode(ui->screen_DVR_SET_btn_urgent_3min_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_align(ui->screen_DVR_SET_btn_urgent_3min_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_DVR_SET_btn_urgent_3min, 0, LV_STATE_DEFAULT);
	lv_obj_set_width(ui->screen_DVR_SET_btn_urgent_3min_label, LV_PCT(100));
	lv_obj_set_pos(ui->screen_DVR_SET_btn_urgent_3min, 950, 20);
	lv_obj_set_size(ui->screen_DVR_SET_btn_urgent_3min, 150, 60);

	//Write style for screen_DVR_SET_btn_urgent_3min, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_btn_urgent_3min, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_DVR_SET_btn_urgent_3min, lv_color_hex(0xE0E0E0), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_DVR_SET_btn_urgent_3min, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_SET_btn_urgent_3min, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_btn_urgent_3min, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_urgent_3min, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_DVR_SET_btn_urgent_3min, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_DVR_SET_btn_urgent_3min, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_DVR_SET_btn_urgent_3min, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_DVR_SET_btn_urgent_3min, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_btn_urgent_3min, 255, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_color(ui->screen_DVR_SET_btn_urgent_3min, lv_color_hex(0x2196F3), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_border_width(ui->screen_DVR_SET_btn_urgent_3min, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_radius(ui->screen_DVR_SET_btn_urgent_3min, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_btn_urgent_3min, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_urgent_3min, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_font(ui->screen_DVR_SET_btn_urgent_3min, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_opa(ui->screen_DVR_SET_btn_urgent_3min, 255, LV_PART_MAIN|LV_STATE_CHECKED);

	//Write codes screen_DVR_SET_cont_powerOnRecording
	ui->screen_DVR_SET_cont_powerOnRecording = lv_obj_create(ui->screen_DVR_SET_cont_parent);
	lv_obj_set_pos(ui->screen_DVR_SET_cont_powerOnRecording, 80, 620);
	lv_obj_set_size(ui->screen_DVR_SET_cont_powerOnRecording, 1260, 100);
	lv_obj_set_scrollbar_mode(ui->screen_DVR_SET_cont_powerOnRecording, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_DVR_SET_cont_powerOnRecording, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_DVR_SET_cont_powerOnRecording, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->screen_DVR_SET_cont_powerOnRecording, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->screen_DVR_SET_cont_powerOnRecording, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_side(ui->screen_DVR_SET_cont_powerOnRecording, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_SET_cont_powerOnRecording, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_cont_powerOnRecording, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_powerOnRecording, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->screen_DVR_SET_cont_powerOnRecording, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_DVR_SET_cont_powerOnRecording, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_DVR_SET_cont_powerOnRecording, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_DVR_SET_cont_powerOnRecording, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_DVR_SET_cont_powerOnRecording, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_cont_powerOnRecording, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_DVR_SET_label_powerOnRecording
	ui->screen_DVR_SET_label_powerOnRecording = lv_label_create(ui->screen_DVR_SET_cont_powerOnRecording);
	lv_label_set_text(ui->screen_DVR_SET_label_powerOnRecording, get_string_for_language(g_sys_Data.current_language, "dvr_set_txt_powerOnRecording"));
	lv_label_set_long_mode(ui->screen_DVR_SET_label_powerOnRecording, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_DVR_SET_label_powerOnRecording, 0, 0);
	lv_obj_set_size(ui->screen_DVR_SET_label_powerOnRecording, 600, 100);
	lv_obj_add_style(ui->screen_DVR_SET_label_powerOnRecording, &style_text_align_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_DVR_SET_btn_powerOnRecording
	ui->screen_DVR_SET_btn_powerOnRecording = lv_btn_create(ui->screen_DVR_SET_cont_powerOnRecording);
	lv_obj_set_style_pad_all(ui->screen_DVR_SET_btn_powerOnRecording, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(ui->screen_DVR_SET_btn_powerOnRecording, 1100, 15);
	lv_obj_set_size(ui->screen_DVR_SET_btn_powerOnRecording, 135, 61);

	//Write style for screen_DVR_SET_btn_powerOnRecording, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_btn_powerOnRecording, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_DVR_SET_btn_powerOnRecording, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_SET_btn_powerOnRecording, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_btn_powerOnRecording, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_DVR_SET_btn_powerOnRecording, &_sw_off_135x61, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR_SET_btn_powerOnRecording, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR_SET_btn_powerOnRecording, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_powerOnRecording, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_DVR_SET_btn_powerOnRecording, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_DVR_SET_btn_powerOnRecording, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_DVR_SET_btn_powerOnRecording, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_DVR_SET_btn_powerOnRecording, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_btn_powerOnRecording, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_border_width(ui->screen_DVR_SET_btn_powerOnRecording, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_radius(ui->screen_DVR_SET_btn_powerOnRecording, 5, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_btn_powerOnRecording, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_img_src(ui->screen_DVR_SET_btn_powerOnRecording, &_sw_on_135x61, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR_SET_btn_powerOnRecording, 255, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR_SET_btn_powerOnRecording, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_powerOnRecording, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_font(ui->screen_DVR_SET_btn_powerOnRecording, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_opa(ui->screen_DVR_SET_btn_powerOnRecording, 255, LV_PART_MAIN|LV_STATE_CHECKED);

	//Write codes screen_DVR_SET_cont_about
	ui->screen_DVR_SET_cont_about = lv_obj_create(ui->screen_DVR_SET_cont_parent);
	lv_obj_set_pos(ui->screen_DVR_SET_cont_about, 80, 750);
	lv_obj_set_size(ui->screen_DVR_SET_cont_about, 1260, 100);
	lv_obj_set_scrollbar_mode(ui->screen_DVR_SET_cont_about, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_DVR_SET_cont_about, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_DVR_SET_cont_about, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->screen_DVR_SET_cont_about, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->screen_DVR_SET_cont_about, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_side(ui->screen_DVR_SET_cont_about, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_SET_cont_about, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_cont_about, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_about, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->screen_DVR_SET_cont_about, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_DVR_SET_cont_about, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_DVR_SET_cont_about, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_DVR_SET_cont_about, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_DVR_SET_cont_about, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_cont_about, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_DVR_SET_label_about
	ui->screen_DVR_SET_label_about = lv_label_create(ui->screen_DVR_SET_cont_about);
	lv_label_set_text(ui->screen_DVR_SET_label_about, get_string_for_language(g_sys_Data.current_language, "dvr_set_txt_about"));
	lv_label_set_long_mode(ui->screen_DVR_SET_label_about, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_DVR_SET_label_about, 0, 0);
	lv_obj_set_size(ui->screen_DVR_SET_label_about, 600, 100);
	lv_obj_add_style(ui->screen_DVR_SET_label_about, &style_text_align_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_DVR_SET_label_version
	ui->screen_DVR_SET_label_version = lv_label_create(ui->screen_DVR_SET_cont_about);
	lv_label_set_text(ui->screen_DVR_SET_label_version, DVR_SET_SOFTWARE_VERSION);
	lv_label_set_long_mode(ui->screen_DVR_SET_label_version, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_DVR_SET_label_version, 900, 0);
	lv_obj_set_size(ui->screen_DVR_SET_label_version, 360, 100);

	//Write style for screen_DVR_SET_label_version, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_DVR_SET_label_version, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_SET_label_version, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_DVR_SET_label_version, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_DVR_SET_label_version, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_DVR_SET_label_version, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(ui->screen_DVR_SET_label_version, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(ui->screen_DVR_SET_label_version, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_DVR_SET_label_version, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_label_version, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_DVR_SET_label_version, 25, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_DVR_SET_label_version, 30, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_DVR_SET_label_version, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_DVR_SET_label_version, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_label_version, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_DVR_SET_cont_recorderSoundSet
	ui->screen_DVR_SET_cont_recorderSoundSet = lv_obj_create(ui->screen_DVR_SET_cont_parent);
	lv_obj_set_pos(ui->screen_DVR_SET_cont_recorderSoundSet, 80, 100);
	lv_obj_set_size(ui->screen_DVR_SET_cont_recorderSoundSet, 1260, 100);
	lv_obj_set_scrollbar_mode(ui->screen_DVR_SET_cont_recorderSoundSet, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_DVR_SET_cont_recorderSoundSet, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_DVR_SET_cont_recorderSoundSet, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->screen_DVR_SET_cont_recorderSoundSet, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->screen_DVR_SET_cont_recorderSoundSet, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_side(ui->screen_DVR_SET_cont_recorderSoundSet, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_SET_cont_recorderSoundSet, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_cont_recorderSoundSet, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_recorderSoundSet, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->screen_DVR_SET_cont_recorderSoundSet, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_DVR_SET_cont_recorderSoundSet, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_DVR_SET_cont_recorderSoundSet, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_DVR_SET_cont_recorderSoundSet, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_DVR_SET_cont_recorderSoundSet, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_cont_recorderSoundSet, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_DVR_SET_label_recorderSoundSet
	ui->screen_DVR_SET_label_recorderSoundSet = lv_label_create(ui->screen_DVR_SET_cont_recorderSoundSet);
	lv_label_set_text(ui->screen_DVR_SET_label_recorderSoundSet, get_string_for_language(g_sys_Data.current_language, "dvr_set_txt_recorderSoundSet"));
	lv_label_set_long_mode(ui->screen_DVR_SET_label_recorderSoundSet, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_DVR_SET_label_recorderSoundSet, 0, 0);
	lv_obj_set_size(ui->screen_DVR_SET_label_recorderSoundSet, 600, 100);
	lv_obj_add_style(ui->screen_DVR_SET_label_recorderSoundSet, &style_text_align_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_DVR_SET_btn_recorderSoundSet
	ui->screen_DVR_SET_btn_recorderSoundSet = lv_btn_create(ui->screen_DVR_SET_cont_recorderSoundSet);
	lv_obj_set_style_pad_all(ui->screen_DVR_SET_btn_recorderSoundSet, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(ui->screen_DVR_SET_btn_recorderSoundSet, 1100, 15);
	lv_obj_set_size(ui->screen_DVR_SET_btn_recorderSoundSet, 135, 61);

	//Write style for screen_DVR_SET_btn_recorderSoundSet, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_btn_recorderSoundSet, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_DVR_SET_btn_recorderSoundSet, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_SET_btn_recorderSoundSet, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_btn_recorderSoundSet, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_DVR_SET_btn_recorderSoundSet, &_sw_off_135x61, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR_SET_btn_recorderSoundSet, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR_SET_btn_recorderSoundSet, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_recorderSoundSet, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_DVR_SET_btn_recorderSoundSet, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_DVR_SET_btn_recorderSoundSet, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_DVR_SET_btn_recorderSoundSet, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_DVR_SET_btn_recorderSoundSet, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_btn_recorderSoundSet, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_border_width(ui->screen_DVR_SET_btn_recorderSoundSet, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_radius(ui->screen_DVR_SET_btn_recorderSoundSet, 5, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_btn_recorderSoundSet, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_img_src(ui->screen_DVR_SET_btn_recorderSoundSet, &_sw_on_135x61, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR_SET_btn_recorderSoundSet, 255, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR_SET_btn_recorderSoundSet, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_recorderSoundSet, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_font(ui->screen_DVR_SET_btn_recorderSoundSet, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_opa(ui->screen_DVR_SET_btn_recorderSoundSet, 255, LV_PART_MAIN|LV_STATE_CHECKED);

	//Write codes screen_DVR_SET_cont_recorderTimeMark
	ui->screen_DVR_SET_cont_recorderTimeMark = lv_obj_create(ui->screen_DVR_SET_cont_parent);
	lv_obj_set_pos(ui->screen_DVR_SET_cont_recorderTimeMark, 80, 100);
	lv_obj_set_size(ui->screen_DVR_SET_cont_recorderTimeMark, 1260, 100);
	lv_obj_set_scrollbar_mode(ui->screen_DVR_SET_cont_recorderTimeMark, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_DVR_SET_cont_recorderSoundSet, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_DVR_SET_cont_recorderTimeMark, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->screen_DVR_SET_cont_recorderTimeMark, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->screen_DVR_SET_cont_recorderTimeMark, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_side(ui->screen_DVR_SET_cont_recorderTimeMark, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_SET_cont_recorderTimeMark, 15, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_cont_recorderTimeMark, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_recorderTimeMark, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->screen_DVR_SET_cont_recorderTimeMark, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_DVR_SET_cont_recorderTimeMark, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_DVR_SET_cont_recorderTimeMark, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_DVR_SET_cont_recorderTimeMark, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_DVR_SET_cont_recorderTimeMark, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_cont_recorderTimeMark, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_DVR_SET_label_recorderTimeMark
	ui->screen_DVR_SET_label_recorderTimeMark = lv_label_create(ui->screen_DVR_SET_cont_recorderTimeMark);
	lv_label_set_text(ui->screen_DVR_SET_label_recorderTimeMark, get_string_for_language(g_sys_Data.current_language, "dvr_set_txt_recorderTimeMark"));
	lv_label_set_long_mode(ui->screen_DVR_SET_label_recorderTimeMark, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_DVR_SET_label_recorderTimeMark, 0, 0);
	lv_obj_set_size(ui->screen_DVR_SET_label_recorderTimeMark, 600, 100);
	lv_obj_add_style(ui->screen_DVR_SET_label_recorderTimeMark, &style_text_align_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_DVR_SET_btn_recorderTimeMark
	ui->screen_DVR_SET_btn_recorderTimeMark = lv_btn_create(ui->screen_DVR_SET_cont_recorderTimeMark);
	lv_obj_set_style_pad_all(ui->screen_DVR_SET_btn_recorderTimeMark, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(ui->screen_DVR_SET_btn_recorderTimeMark, 1100, 15);
	lv_obj_set_size(ui->screen_DVR_SET_btn_recorderTimeMark, 135, 61);

	//Write style for screen_DVR_SET_btn_recorderTimeMark, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_btn_recorderTimeMark, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_DVR_SET_btn_recorderTimeMark, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_SET_btn_recorderTimeMark, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_btn_recorderTimeMark, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_DVR_SET_btn_recorderTimeMark, &_sw_off_135x61, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR_SET_btn_recorderTimeMark, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR_SET_btn_recorderTimeMark, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_recorderTimeMark, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_DVR_SET_btn_recorderTimeMark, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_DVR_SET_btn_recorderTimeMark, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_DVR_SET_btn_recorderTimeMark, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_DVR_SET_btn_recorderTimeMark, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_btn_recorderTimeMark, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_border_width(ui->screen_DVR_SET_btn_recorderTimeMark, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_radius(ui->screen_DVR_SET_btn_recorderTimeMark, 5, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_btn_recorderTimeMark, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_img_src(ui->screen_DVR_SET_btn_recorderTimeMark, &_sw_on_135x61, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR_SET_btn_recorderTimeMark, 255, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR_SET_btn_recorderTimeMark, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_recorderTimeMark, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_font(ui->screen_DVR_SET_btn_recorderTimeMark, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_opa(ui->screen_DVR_SET_btn_recorderTimeMark, 255, LV_PART_MAIN|LV_STATE_CHECKED);

	//Write codes screen_DVR_SET_cont_dvrSpeedMark
	ui->screen_DVR_SET_cont_carSpeedMark = lv_obj_create(ui->screen_DVR_SET_cont_parent);
	lv_obj_set_pos(ui->screen_DVR_SET_cont_carSpeedMark, 80, 100);
	lv_obj_set_size(ui->screen_DVR_SET_cont_carSpeedMark, 1260, 100);
	lv_obj_set_scrollbar_mode(ui->screen_DVR_SET_cont_carSpeedMark, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_DVR_SET_cont_recorderSoundSet, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_DVR_SET_cont_carSpeedMark, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->screen_DVR_SET_cont_carSpeedMark, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->screen_DVR_SET_cont_carSpeedMark, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_side(ui->screen_DVR_SET_cont_carSpeedMark, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_SET_cont_carSpeedMark, 15, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_cont_carSpeedMark, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_carSpeedMark, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->screen_DVR_SET_cont_carSpeedMark, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_DVR_SET_cont_carSpeedMark, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_DVR_SET_cont_carSpeedMark, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_DVR_SET_cont_carSpeedMark, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_DVR_SET_cont_carSpeedMark, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_cont_carSpeedMark, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_DVR_SET_label_carSpeedMark
	ui->screen_DVR_SET_label_carSpeedMark = lv_label_create(ui->screen_DVR_SET_cont_carSpeedMark);
	lv_label_set_text(ui->screen_DVR_SET_label_carSpeedMark, get_string_for_language(g_sys_Data.current_language, "dvr_set_txt_carSpeedMark"));
	lv_label_set_long_mode(ui->screen_DVR_SET_label_carSpeedMark, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_DVR_SET_label_carSpeedMark, 0, 0);
	lv_obj_set_size(ui->screen_DVR_SET_label_carSpeedMark, 600, 100);
	lv_obj_add_style(ui->screen_DVR_SET_label_carSpeedMark, &style_text_align_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_DVR_SET_btn_carSpeedMark
	ui->screen_DVR_SET_btn_carSpeedMark = lv_btn_create(ui->screen_DVR_SET_cont_carSpeedMark);
	lv_obj_set_style_pad_all(ui->screen_DVR_SET_btn_carSpeedMark, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(ui->screen_DVR_SET_btn_carSpeedMark, 1100, 15);
	lv_obj_set_size(ui->screen_DVR_SET_btn_carSpeedMark, 135, 61);

	//Write style for screen_DVR_SET_btn_carSpeedMark, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_btn_carSpeedMark, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_DVR_SET_btn_carSpeedMark, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_SET_btn_carSpeedMark, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_btn_carSpeedMark, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_DVR_SET_btn_carSpeedMark, &_sw_off_135x61, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR_SET_btn_carSpeedMark, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR_SET_btn_carSpeedMark, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_carSpeedMark, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_DVR_SET_btn_carSpeedMark, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_DVR_SET_btn_carSpeedMark, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_DVR_SET_btn_carSpeedMark, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_DVR_SET_btn_carSpeedMark, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_btn_carSpeedMark, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_border_width(ui->screen_DVR_SET_btn_carSpeedMark, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_radius(ui->screen_DVR_SET_btn_carSpeedMark, 5, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_btn_carSpeedMark, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_img_src(ui->screen_DVR_SET_btn_carSpeedMark, &_sw_on_135x61, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR_SET_btn_carSpeedMark, 255, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR_SET_btn_carSpeedMark, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_carSpeedMark, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_font(ui->screen_DVR_SET_btn_carSpeedMark, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_opa(ui->screen_DVR_SET_btn_carSpeedMark, 255, LV_PART_MAIN|LV_STATE_CHECKED);	

	//Write codes screen_DVR_SET_cont_carSpeedUnit
	ui->screen_DVR_SET_cont_carSpeedUnit = lv_obj_create(ui->screen_DVR_SET_cont_parent);
	lv_obj_set_pos(ui->screen_DVR_SET_cont_carSpeedUnit, 80, 100);
	lv_obj_set_size(ui->screen_DVR_SET_cont_carSpeedUnit, 1260, 100);
	lv_obj_set_scrollbar_mode(ui->screen_DVR_SET_cont_carSpeedUnit, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_DVR_SET_cont_recorderSoundSet, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_DVR_SET_cont_carSpeedUnit, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->screen_DVR_SET_cont_carSpeedUnit, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->screen_DVR_SET_cont_carSpeedUnit, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_side(ui->screen_DVR_SET_cont_carSpeedUnit, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_SET_cont_carSpeedUnit, 15, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_cont_carSpeedUnit, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_carSpeedUnit, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->screen_DVR_SET_cont_carSpeedUnit, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_DVR_SET_cont_carSpeedUnit, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_DVR_SET_cont_carSpeedUnit, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_DVR_SET_cont_carSpeedUnit, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_DVR_SET_cont_carSpeedUnit, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_cont_carSpeedUnit, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_DVR_SET_label_carSpeedUnit
	ui->screen_DVR_SET_label_carSpeedUnit = lv_label_create(ui->screen_DVR_SET_cont_carSpeedUnit);
	lv_label_set_text(ui->screen_DVR_SET_label_carSpeedUnit, get_string_for_language(g_sys_Data.current_language, "dvr_set_txt_carSpeedUnit"));
	lv_label_set_long_mode(ui->screen_DVR_SET_label_carSpeedUnit, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_DVR_SET_label_carSpeedUnit, 0, 0);
	lv_obj_set_size(ui->screen_DVR_SET_label_carSpeedUnit, 600, 100);
	lv_obj_add_style(ui->screen_DVR_SET_label_carSpeedUnit, &style_text_align_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_DVR_SET_btn_carSpeedUnit
	ui->screen_DVR_SET_btn_carSpeedUnit = lv_btn_create(ui->screen_DVR_SET_cont_carSpeedUnit);
	lv_obj_set_style_pad_all(ui->screen_DVR_SET_btn_carSpeedUnit, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(ui->screen_DVR_SET_btn_carSpeedUnit, 1100, 15);
	lv_obj_set_size(ui->screen_DVR_SET_btn_carSpeedUnit, 135, 61);

	//Write style for screen_DVR_SET_btn_carSpeedUnit, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_btn_carSpeedUnit, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_DVR_SET_btn_carSpeedUnit, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_SET_btn_carSpeedUnit, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_btn_carSpeedUnit, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_DVR_SET_btn_carSpeedUnit, &_sw_off_135x61, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR_SET_btn_carSpeedUnit, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR_SET_btn_carSpeedUnit, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_carSpeedUnit, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_DVR_SET_btn_carSpeedUnit, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_DVR_SET_btn_carSpeedUnit, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_DVR_SET_btn_carSpeedUnit, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_DVR_SET_btn_carSpeedUnit, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_btn_carSpeedUnit, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_border_width(ui->screen_DVR_SET_btn_carSpeedUnit, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_radius(ui->screen_DVR_SET_btn_carSpeedUnit, 5, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_btn_carSpeedUnit, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_img_src(ui->screen_DVR_SET_btn_carSpeedUnit, &_sw_on_135x61, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR_SET_btn_carSpeedUnit, 255, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR_SET_btn_carSpeedUnit, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_carSpeedUnit, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_font(ui->screen_DVR_SET_btn_carSpeedUnit, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_opa(ui->screen_DVR_SET_btn_carSpeedUnit, 255, LV_PART_MAIN|LV_STATE_CHECKED);	


	lv_obj_move_to_index(ui->screen_DVR_SET_cont_recorderSoundSet, 0);
	lv_obj_move_to_index(ui->screen_DVR_SET_cont_rearCameraImage, 1);
	lv_obj_move_to_index(ui->screen_DVR_SET_cont_recorderTime, 2);
	lv_obj_move_to_index(ui->screen_DVR_SET_cont_recorderUrgentTime, 3);
	lv_obj_move_to_index(ui->screen_DVR_SET_cont_powerOnRecording, 4);
	lv_obj_move_to_index(ui->screen_DVR_SET_cont_recorderTimeMark, 5);
	lv_obj_move_to_index(ui->screen_DVR_SET_cont_carSpeedMark, 6);
	lv_obj_move_to_index(ui->screen_DVR_SET_cont_carSpeedUnit, 7);
	lv_obj_move_to_index(ui->screen_DVR_SET_cont_about, 8);

	//Write codes screen_DVR_SET_label_topLabel
	ui->screen_DVR_SET_label_topLabel = lv_label_create(ui->screen_DVR_SET);
	lv_label_set_text(ui->screen_DVR_SET_label_topLabel, get_string_for_language(g_sys_Data.current_language, "dvr_set_txt_DVR"));
	lv_label_set_long_mode(ui->screen_DVR_SET_label_topLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_DVR_SET_label_topLabel, 0, 0);
	lv_obj_set_size(ui->screen_DVR_SET_label_topLabel, 1440, 80);

	//Write style for screen_DVR_SET_label_topLabel, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_DVR_SET_label_topLabel, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_SET_label_topLabel, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_DVR_SET_label_topLabel, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_DVR_SET_label_topLabel, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_DVR_SET_label_topLabel, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(ui->screen_DVR_SET_label_topLabel, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(ui->screen_DVR_SET_label_topLabel, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_DVR_SET_label_topLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_label_topLabel, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_DVR_SET_label_topLabel, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->screen_DVR_SET_label_topLabel, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_DVR_SET_label_topLabel, 20, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_DVR_SET_label_topLabel, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_DVR_SET_label_topLabel, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_DVR_SET_label_topLabel, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_label_topLabel, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_DVR_SET_btn_return
	ui->screen_DVR_SET_btn_return = lv_btn_create(ui->screen_DVR_SET);
	lv_obj_set_style_pad_all(ui->screen_DVR_SET_btn_return, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(ui->screen_DVR_SET_btn_return, 0, 0);
	lv_obj_set_size(ui->screen_DVR_SET_btn_return, 80, 80);

	//Write style for screen_DVR_SET_btn_return, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_btn_return, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_DVR_SET_btn_return, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_SET_btn_return, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_btn_return, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_DVR_SET_btn_return, &_back_dark_80x80, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR_SET_btn_return, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR_SET_btn_return, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_DVR_SET_btn_return, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_DVR_SET_btn_return, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_border_width(ui->screen_DVR_SET_btn_return, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_radius(ui->screen_DVR_SET_btn_return, 5, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_width(ui->screen_DVR_SET_btn_return, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_img_src(ui->screen_DVR_SET_btn_return, &_back_select_80x80, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR_SET_btn_return, 255, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR_SET_btn_return, 0, LV_PART_MAIN|LV_STATE_CHECKED);

	lv_obj_add_flag(ui->screen_DVR_SET_btn_rearCameraImage, LV_OBJ_FLAG_CHECKABLE);
	lv_obj_add_flag(ui->screen_DVR_SET_btn_powerOnRecording, LV_OBJ_FLAG_CHECKABLE);	
	lv_obj_add_flag(ui->screen_DVR_SET_btn_recorderSoundSet, LV_OBJ_FLAG_CHECKABLE);
	lv_obj_add_flag(ui->screen_DVR_SET_btn_recorderTimeMark, LV_OBJ_FLAG_CHECKABLE);
	lv_obj_add_flag(ui->screen_DVR_SET_btn_carSpeedMark, LV_OBJ_FLAG_CHECKABLE);
	lv_obj_add_flag(ui->screen_DVR_SET_btn_carSpeedUnit, LV_OBJ_FLAG_CHECKABLE);
	lv_obj_add_flag(ui->screen_DVR_SET_btn_recorder_1min, LV_OBJ_FLAG_CHECKABLE);
	lv_obj_add_flag(ui->screen_DVR_SET_btn_recorder_3min, LV_OBJ_FLAG_CHECKABLE);
	lv_obj_add_flag(ui->screen_DVR_SET_btn_recorder_5min, LV_OBJ_FLAG_CHECKABLE);
	lv_obj_add_flag(ui->screen_DVR_SET_btn_urgent_1min, LV_OBJ_FLAG_CHECKABLE);
	lv_obj_add_flag(ui->screen_DVR_SET_btn_urgent_3min, LV_OBJ_FLAG_CHECKABLE);
	lv_obj_add_flag(ui->screen_DVR_SET_btn_urgent_5min, LV_OBJ_FLAG_CHECKABLE);
	lv_obj_add_flag(ui->screen_DVR_btn_lock, LV_OBJ_FLAG_CHECKABLE);
	//The custom code of screen_DVR_SET.
	lv_obj_set_scroll_dir(ui->screen_DVR_SET, LV_DIR_NONE);
	lv_obj_set_scroll_dir(ui->screen_DVR_SET_cont_parent, LV_DIR_VER);
	lv_obj_set_scroll_dir(ui->screen_DVR_SET_cont_rearCameraImage, LV_DIR_NONE);
	lv_obj_set_scroll_dir(ui->screen_DVR_SET_cont_recorderTime, LV_DIR_NONE);
	lv_obj_set_scroll_dir(ui->screen_DVR_SET_cont_recorderUrgentTime, LV_DIR_NONE);
	lv_obj_set_scroll_dir(ui->screen_DVR_SET_cont_powerOnRecording, LV_DIR_NONE);
	lv_obj_set_scroll_dir(ui->screen_DVR_SET_cont_about, LV_DIR_NONE);
	lv_obj_set_scroll_dir(ui->screen_DVR_SET_cont_recorderSoundSet, LV_DIR_NONE);
	lv_obj_set_scroll_dir(ui->screen_DVR_SET_cont_recorderTimeMark, LV_DIR_NONE);
	lv_obj_set_scroll_dir(ui->screen_DVR_SET_cont_carSpeedMark, LV_DIR_NONE);
	lv_obj_set_scroll_dir(ui->screen_DVR_SET_cont_carSpeedUnit, LV_DIR_NONE);

	lv_obj_set_style_radius(ui->screen_DVR_SET_cont_rearCameraImage, 15, LV_PART_MAIN);
	lv_obj_set_style_radius(ui->screen_DVR_SET_cont_recorderTime, 15, LV_PART_MAIN);
	lv_obj_set_style_radius(ui->screen_DVR_SET_cont_recorderUrgentTime, 15, LV_PART_MAIN);
	lv_obj_set_style_radius(ui->screen_DVR_SET_cont_powerOnRecording, 15, LV_PART_MAIN);
	lv_obj_set_style_radius(ui->screen_DVR_SET_cont_about, 15, LV_PART_MAIN);
	lv_obj_set_style_radius(ui->screen_DVR_SET_cont_recorderSoundSet, 15, LV_PART_MAIN);
	lv_obj_set_style_radius(ui->screen_DVR_SET_cont_carSpeedUnit, 15, LV_PART_MAIN);

	lv_obj_move_foreground(ui->screen_DVR_SET_btn_return);


	if(g_sys_Data.rearCameraImage) lv_obj_add_state(ui->screen_DVR_SET_btn_rearCameraImage, LV_STATE_CHECKED);
	if(g_sys_Data.powerOnRecorder)	lv_obj_add_state(ui->screen_DVR_SET_btn_powerOnRecording, LV_STATE_CHECKED);	
	if(g_sys_Data.SoundRecorder) lv_obj_add_state(ui->screen_DVR_SET_btn_recorderSoundSet, LV_STATE_CHECKED);
	if(g_sys_Data.TimeMark) lv_obj_add_state(ui->screen_DVR_SET_btn_recorderTimeMark, LV_STATE_CHECKED);
	if(g_sys_Data.SpeedMark) lv_obj_add_state(ui->screen_DVR_SET_btn_carSpeedMark, LV_STATE_CHECKED);
	if(g_sys_Data.SpeedUnit) lv_obj_add_state(ui->screen_DVR_SET_btn_carSpeedUnit, LV_STATE_CHECKED);

	RecorderTimeBtnSta();
	//Update current screen layout.
	lv_obj_update_layout(ui->screen_DVR_SET);
	events_init_screen_DVR_SET(ui);
	THEME_MODE mode = g_sys_Data.themeMode;
	if(mode == THEME_DAY || (mode == THEME_AUTO && g_sys_Data.gpadcVol < 1300)){
		screenDvrSetBlackNightMode(ui, THEME_DAY);
	}else{
		screenDvrSetBlackNightMode(ui, THEME_DARK);
	}
	
}


void RecorderTimeBtnSta(){
	switch (g_sys_Data.recorderTime)
	{
	case RECORDER_1:
		lv_obj_add_state(guider_ui.screen_DVR_SET_btn_recorder_1min, LV_STATE_CHECKED);  // 选中
		lv_obj_clear_state(guider_ui.screen_DVR_SET_btn_recorder_3min, LV_STATE_CHECKED); // 取消选中
		lv_obj_clear_state(guider_ui.screen_DVR_SET_btn_recorder_5min, LV_STATE_CHECKED); // 取消选中
		break;
	case RECORDER_3:
		lv_obj_clear_state(guider_ui.screen_DVR_SET_btn_recorder_1min, LV_STATE_CHECKED);  // 选中
		lv_obj_add_state(guider_ui.screen_DVR_SET_btn_recorder_3min, LV_STATE_CHECKED); // 取消选中
		lv_obj_clear_state(guider_ui.screen_DVR_SET_btn_recorder_5min, LV_STATE_CHECKED); // 取消选中
		break;
	case RECORDER_5:
		lv_obj_clear_state(guider_ui.screen_DVR_SET_btn_recorder_1min, LV_STATE_CHECKED);  // 选中
		lv_obj_clear_state(guider_ui.screen_DVR_SET_btn_recorder_3min, LV_STATE_CHECKED); // 取消选中
		lv_obj_add_state(guider_ui.screen_DVR_SET_btn_recorder_5min, LV_STATE_CHECKED); // 取消选中
		break;	
	default:
		break;
	}
	switch (g_sys_Data.recorderUrgentTime)
	{
	case RECORDER_1:
		lv_obj_add_state(guider_ui.screen_DVR_SET_btn_urgent_1min, LV_STATE_CHECKED);  // 选中
		lv_obj_clear_state(guider_ui.screen_DVR_SET_btn_urgent_3min, LV_STATE_CHECKED); // 取消选中
		lv_obj_clear_state(guider_ui.screen_DVR_SET_btn_urgent_5min, LV_STATE_CHECKED); // 取消选中
		break;
	case RECORDER_3:
		lv_obj_clear_state(guider_ui.screen_DVR_SET_btn_urgent_1min, LV_STATE_CHECKED);  // 选中
		lv_obj_add_state(guider_ui.screen_DVR_SET_btn_urgent_3min, LV_STATE_CHECKED); // 取消选中
		lv_obj_clear_state(guider_ui.screen_DVR_SET_btn_urgent_5min, LV_STATE_CHECKED); // 取消选中
		break;
	case RECORDER_5:
		lv_obj_clear_state(guider_ui.screen_DVR_SET_btn_urgent_1min, LV_STATE_CHECKED);  // 选中
		lv_obj_clear_state(guider_ui.screen_DVR_SET_btn_urgent_3min, LV_STATE_CHECKED); // 取消选中
		lv_obj_add_state(guider_ui.screen_DVR_SET_btn_urgent_5min, LV_STATE_CHECKED); // 取消选中
		break;	
	default:
		break;
	}
}

void screenDvrSetBlackNightMode(lv_ui *ui, THEME_MODE mode){
	printf("-------%s:%d------mode:%d-----\n",__func__,__LINE__,mode);
	switch (mode)
	{
	case THEME_DAY:
		lv_obj_set_style_bg_img_src(ui->screen_DVR_SET_btn_return, &_back_dark_80x80, LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_parent, lv_color_hex(0xE0E0E0), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_rearCameraImage, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_label_rearCameraImage, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_recorderTime, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_label_recorderTime, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_recorder_5min_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_recorder_1min_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_recorder_3min_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_recorderUrgentTime, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_label_recorderUrgentTime, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_urgent_5min_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_urgent_1min_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_urgent_3min_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_powerOnRecording, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_label_powerOnRecording, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_about, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_label_about, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_label_version, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_recorderSoundSet, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_label_recorderSoundSet, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_carSpeedMark, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);	
		lv_obj_set_style_text_color(ui->screen_DVR_SET_label_carSpeedMark, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_carSpeedUnit, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_label_carSpeedUnit, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_DVR_SET_label_topLabel, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_label_topLabel, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_recorderTimeMark, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_DVR_SET_label_recorderTimeMark, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	break;
	case THEME_DARK:
		lv_obj_set_style_bg_img_src(ui->screen_DVR_SET_btn_return, &_back_80x80, LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_parent, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_rearCameraImage, lv_color_hex(0x5f5f5f), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_label_rearCameraImage, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_recorderTime, lv_color_hex(0x5f5f5f), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_label_recorderTime, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_recorder_5min_label, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_recorder_1min_label, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_recorder_3min_label, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_recorderUrgentTime, lv_color_hex(0x5f5f5f), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_label_recorderUrgentTime, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_urgent_5min_label, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_urgent_1min_label, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_btn_urgent_3min_label, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_powerOnRecording, lv_color_hex(0x5f5f5f), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_label_powerOnRecording, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_about, lv_color_hex(0x5f5f5f), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_label_about, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_label_version, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_recorderSoundSet, lv_color_hex(0x5f5f5f), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_label_recorderSoundSet, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_carSpeedMark, lv_color_hex(0x5f5f5f), LV_PART_MAIN|LV_STATE_DEFAULT);	
		lv_obj_set_style_text_color(ui->screen_DVR_SET_label_carSpeedMark, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_carSpeedUnit, lv_color_hex(0x5f5f5f), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_label_carSpeedUnit, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_DVR_SET_label_topLabel, lv_color_hex(0x5f5f5f), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_label_topLabel, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_bg_color(ui->screen_DVR_SET_cont_recorderTimeMark, lv_color_hex(0x5f5f5f), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(ui->screen_DVR_SET_label_recorderTimeMark, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	break;
	default:
	break;
	}	
}