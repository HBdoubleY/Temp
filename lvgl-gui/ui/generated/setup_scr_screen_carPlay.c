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

static lv_style_t style_text_align_left_mian_default;

void setup_scr_screen_carPlay(lv_ui *ui)
{
	//write all label style_text_align_center_mian_default style
	ui_init_style(&style_text_align_left_mian_default);

	lv_style_set_text_color(&style_text_align_left_mian_default, lv_color_hex(0xFFFFFF));
	lv_style_set_text_font(&style_text_align_left_mian_default, &lv_font_harmonyOS_42);
	lv_style_set_text_opa(&style_text_align_left_mian_default, 255);
	lv_style_set_text_align(&style_text_align_left_mian_default, LV_TEXT_ALIGN_LEFT);
	lv_style_set_pad_top(&style_text_align_left_mian_default, 10);
	lv_style_set_bg_opa(&style_text_align_left_mian_default, 0);
	lv_style_set_bg_color(&style_text_align_left_mian_default, lv_color_hex(0x2195f6));
	lv_style_set_border_opa(&style_text_align_left_mian_default, 0);	

	//Write codes screen_androidAuto
	ui->screen_carPlay = lv_obj_create(NULL);
	lv_obj_set_size(ui->screen_carPlay, 1440, 720);
	lv_obj_set_scrollbar_mode(ui->screen_carPlay, LV_SCROLLBAR_MODE_OFF);


	//Write style for screen_androidAuto, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_carPlay, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_carPlay, NULL, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_carPlay, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_carPlay, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_clear_flag(ui->screen_carPlay, LV_OBJ_FLAG_SCROLLABLE);

	//Write codes screen_androidAuto_btn_return
	ui->screen_carPlay_btn_return = lv_obj_create(ui->screen_carPlay);
	lv_obj_set_size(ui->screen_carPlay_btn_return, 80, 80);
	lv_obj_set_pos(ui->screen_carPlay_btn_return, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_carPlay_btn_return, 0, LV_STATE_DEFAULT);

	//Write style for screen_androidAuto_btn_return, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_carPlay_btn_return, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_carPlay_btn_return, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_carPlay_btn_return, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_carPlay_btn_return, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_carPlay_btn_return, &_back_80x80, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_carPlay_btn_return, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_carPlay_btn_return, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_carPlay_btn_return, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_carPlay_btn_return, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_carPlay_btn_return, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_carPlay_btn_return, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_DVR_btn_prev, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_carPlay_btn_return, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_border_width(ui->screen_carPlay_btn_return, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_radius(ui->screen_carPlay_btn_return, 5, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_width(ui->screen_carPlay_btn_return, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_src(ui->screen_carPlay_btn_return, &_back_select_80x80, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_opa(ui->screen_carPlay_btn_return, 255, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_carPlay_btn_return, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_text_color(ui->screen_carPlay_btn_return, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_text_font(ui->screen_carPlay_btn_return, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_text_opa(ui->screen_carPlay_btn_return, 255, LV_PART_MAIN|LV_STATE_PRESSED);

	//write code screen_androidAuto_prompt_cont;
	ui->screen_carPlay_cont_prompt = lv_obj_create(ui->screen_carPlay);
	lv_obj_set_pos(ui->screen_carPlay_cont_prompt,70,110);
	lv_obj_set_size(ui->screen_carPlay_cont_prompt,1300,500);
	lv_obj_set_scrollbar_mode(ui->screen_carPlay_cont_prompt, LV_SCROLLBAR_MODE_OFF);
	lv_obj_clear_flag(ui->screen_carPlay_cont_prompt, LV_OBJ_FLAG_HIDDEN);
	// lv_obj_add_flag(ui->screen_carPlay_cont_prompt, LV_OBJ_FLAG_HIDDEN);

	//Write style for screen_Tire_cont_behind, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_carPlay_cont_prompt, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->screen_carPlay_cont_prompt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->screen_carPlay_cont_prompt, lv_color_hex(0x5C8AB8), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_side(ui->screen_carPlay_cont_prompt, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_carPlay_cont_prompt, 30, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_carPlay_cont_prompt, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_carPlay_cont_prompt, lv_color_hex(0x5C8AB8), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_carPlay_cont_prompt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_carPlay_cont_prompt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_carPlay_cont_prompt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_carPlay_cont_prompt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_carPlay_cont_prompt, 0, LV_PART_MAIN|LV_STATE_DEFAULT); 

		//write code screen_androidAuto_prompt_lable
	ui->screen_carPlay_label1_prompt = lv_label_create(ui->screen_carPlay_cont_prompt);
	ui->screen_carPlay_label2_prompt = lv_label_create(ui->screen_carPlay_cont_prompt);
	ui->screen_carPlay_label3_prompt = lv_label_create(ui->screen_carPlay_cont_prompt);
	ui->screen_carPlay_label4_prompt = lv_label_create(ui->screen_carPlay_cont_prompt);
	ui->screen_carPlay_label5_prompt = lv_label_create(ui->screen_carPlay_cont_prompt);
	ui->screen_carPlay_label6_prompt = lv_label_create(ui->screen_carPlay_cont_prompt);
	
	lv_label_set_text(ui->screen_carPlay_label1_prompt, get_string_for_language(g_sys_Data.current_language,"carplay_txt_label1"));
	lv_label_set_long_mode(ui->screen_carPlay_label1_prompt, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_carPlay_label1_prompt, 50, 50);
	lv_obj_set_size(ui->screen_carPlay_label1_prompt, 1200, 60);
	lv_obj_add_style(ui->screen_carPlay_label1_prompt, &style_text_align_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	lv_label_set_text(ui->screen_carPlay_label2_prompt, get_string_for_language(g_sys_Data.current_language,"carplay_txt_label2"));
	lv_label_set_long_mode(ui->screen_carPlay_label2_prompt, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_carPlay_label2_prompt, 50, 120);
	lv_obj_set_size(ui->screen_carPlay_label2_prompt, 1200, 60);
	lv_obj_add_style(ui->screen_carPlay_label2_prompt, &style_text_align_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	lv_label_set_text(ui->screen_carPlay_label3_prompt, get_string_for_language(g_sys_Data.current_language,"carplay_txt_label3"));
	lv_label_set_long_mode(ui->screen_carPlay_label3_prompt, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_carPlay_label3_prompt, 50, 190);
	lv_obj_set_size(ui->screen_carPlay_label3_prompt, 1200, 60);
	lv_obj_add_style(ui->screen_carPlay_label3_prompt, &style_text_align_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	lv_label_set_text(ui->screen_carPlay_label4_prompt, get_string_for_language(g_sys_Data.current_language,"carplay_txt_label4"));
	lv_label_set_long_mode(ui->screen_carPlay_label4_prompt, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_carPlay_label4_prompt, 50, 260);
	lv_obj_set_size(ui->screen_carPlay_label4_prompt, 1200, 60);
	lv_obj_add_style(ui->screen_carPlay_label4_prompt, &style_text_align_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

    char str[50];
    sprintf(str, "BLUETOOTH: %s", "RTL8733BS");
	lv_label_set_text(ui->screen_carPlay_label5_prompt, str);
	lv_label_set_long_mode(ui->screen_carPlay_label5_prompt, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_carPlay_label5_prompt, 50, 350);
	lv_obj_set_size(ui->screen_carPlay_label5_prompt, 1200, 60);
	lv_obj_add_style(ui->screen_carPlay_label5_prompt, &style_text_align_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	lv_label_set_text(ui->screen_carPlay_label6_prompt, get_string_for_language(g_sys_Data.current_language,"carplay_txt_label6"));
	lv_label_set_long_mode(ui->screen_carPlay_label6_prompt, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_carPlay_label6_prompt, 50, 420);
	lv_obj_set_size(ui->screen_carPlay_label6_prompt, 1200, 60);
	lv_obj_add_style(ui->screen_carPlay_label6_prompt, &style_text_align_left_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_carPlay_label6_prompt, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

	if(g_sys_Data.linktype == LINK_TYPE_CARPLAY){
		lv_obj_set_style_bg_img_src(ui->screen_carPlay, NULL, LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_obj_add_flag(ui->screen_carPlay_btn_return, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(ui->screen_carPlay_cont_prompt, LV_OBJ_FLAG_HIDDEN);
	}
	//Update current screen layout.
	lv_obj_update_layout(ui->screen_carPlay);
	//nit events for screen.
	events_init_screen_carPlay(ui);
}

