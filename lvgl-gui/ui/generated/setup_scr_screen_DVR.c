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
#include "myTimer.h"
#include "mppFileManager.h"

lv_timer_t *HideBtnContTimer = NULL;

static int nSecond = 0;
static int recorderTime = 0;
extern lv_timer_t *DVRstaTimer;

void HideBtnCont(void){
	printf("%s:%d\n",__func__,__LINE__);
	if(lv_obj_is_valid(guider_ui.screen_DVR_cont_bottom))
		lv_obj_add_flag(guider_ui.screen_DVR_cont_bottom, LV_OBJ_FLAG_HIDDEN);
	HideBtnContTimer = NULL;
}
static lv_style_t style_text_align_center_mian_default;

void recorder_status_timer(lv_timer_t *timer){
	if(nSecond == 0){
		switch(g_sys_Data.recorderMode){
			case RECORDER_NORMAL:
				if(nSecond == 0) recorderTime = g_sys_Data.recorderTime;
					
			break;
			case RECORDER_URGENT:
				if(nSecond == 0) recorderTime = g_sys_Data.recorderUrgentTime;
			break;
		}
	}
	

	if(lv_obj_is_valid(guider_ui.screen_img_rec)){
		if(lv_obj_has_flag(guider_ui.screen_img_rec, LV_OBJ_FLAG_HIDDEN)){
			lv_obj_clear_flag(guider_ui.screen_img_rec, LV_OBJ_FLAG_HIDDEN);
		}else{
			lv_obj_add_flag(guider_ui.screen_img_rec, LV_OBJ_FLAG_HIDDEN);
		}
	}
/*
	if(lv_obj_is_valid(guider_ui.screen_DVR_img_rec)){
		if(lv_obj_has_flag(guider_ui.screen_DVR_img_rec, LV_OBJ_FLAG_HIDDEN)){
			lv_obj_clear_flag(guider_ui.screen_DVR_img_rec, LV_OBJ_FLAG_HIDDEN);
		}else{
			lv_obj_add_flag(guider_ui.screen_DVR_img_rec, LV_OBJ_FLAG_HIDDEN);
		}
	}
*/
	if(lv_obj_is_valid(guider_ui.screen_DVR_label_Front)){	
		char secondstr[10];
		sprintf(secondstr, "%02d:%02d", nSecond/60, nSecond%60);
		lv_label_set_text(guider_ui.screen_DVR_label_Front1, secondstr);
		lv_label_set_text(guider_ui.screen_DVR_label_Rear1, secondstr);
		TTime time;
		memset(&time, 0, sizeof(time));
		RtcGetTime2C(&time);
		char Timestr[50];
		sprintf(Timestr,"%d/%d/%d %02d:%02d:%02d %s",time.nYear,time.nMonth,time.nDay, time.nHour, time.nMinute, time.nSecond, get_string_for_language(g_sys_Data.current_language, "dvr_txt_front"));
		lv_label_set_text(guider_ui.screen_DVR_label_Front, Timestr);
		sprintf(Timestr,"%d/%d/%d %02d:%02d:%02d %s",time.nYear,time.nMonth,time.nDay, time.nHour, time.nMinute, time.nSecond, get_string_for_language(g_sys_Data.current_language, "dvr_txt_rear"));
		lv_label_set_text(guider_ui.screen_DVR_label_Rear, Timestr);				
	}
	nSecond ++;

	if(nSecond == recorderTime*60) nSecond = 0;

}

void clearRecorderStatu(void){
	if(lv_obj_is_valid(guider_ui.screen_DVR_img_rec)){
		if(!lv_obj_has_flag(guider_ui.screen_DVR_img_rec, LV_OBJ_FLAG_HIDDEN)) lv_obj_add_flag(guider_ui.screen_DVR_img_rec, LV_OBJ_FLAG_HIDDEN);
		lv_label_set_text(guider_ui.screen_DVR_label_Front, get_string_for_language(g_sys_Data.current_language,"dvr_txt_front"));
		lv_label_set_text(guider_ui.screen_DVR_label_Front1, "");
		lv_label_set_text(guider_ui.screen_DVR_label_Rear, get_string_for_language(g_sys_Data.current_language,"dvr_txt_rear"));
		lv_label_set_text(guider_ui.screen_DVR_label_Rear1, "");
	}
	nSecond = 0;
}
void prviewModeLabelDash(preview_mode mode){
	switch(mode){
		case PREVIEW_FRONT:
			lv_obj_add_flag(guider_ui.screen_DVR_label_Rear, LV_OBJ_FLAG_HIDDEN);
			lv_obj_add_flag(guider_ui.screen_DVR_label_Rear1, LV_OBJ_FLAG_HIDDEN);
			lv_obj_clear_flag(guider_ui.screen_DVR_label_Front, LV_OBJ_FLAG_HIDDEN);
			lv_obj_clear_flag(guider_ui.screen_DVR_label_Front1, LV_OBJ_FLAG_HIDDEN);
			lv_obj_set_pos(guider_ui.screen_DVR_label_Front, 360, 0);
			lv_obj_set_pos(guider_ui.screen_DVR_label_Front1, 360, 60);
		break;
		case PREVIEW_REAR:
			lv_obj_add_flag(guider_ui.screen_DVR_label_Front, LV_OBJ_FLAG_HIDDEN);
			lv_obj_add_flag(guider_ui.screen_DVR_label_Front1, LV_OBJ_FLAG_HIDDEN);
			lv_obj_clear_flag(guider_ui.screen_DVR_label_Rear, LV_OBJ_FLAG_HIDDEN);	
			lv_obj_clear_flag(guider_ui.screen_DVR_label_Rear1, LV_OBJ_FLAG_HIDDEN);
			lv_obj_set_pos(guider_ui.screen_DVR_label_Rear, 360, 0);
			lv_obj_set_pos(guider_ui.screen_DVR_label_Rear1, 360, 60);
		break;
		case PREVIEW_FRONT_REAR:
			lv_obj_clear_flag(guider_ui.screen_DVR_label_Front, LV_OBJ_FLAG_HIDDEN);
			lv_obj_clear_flag(guider_ui.screen_DVR_label_Front1, LV_OBJ_FLAG_HIDDEN);
			lv_obj_clear_flag(guider_ui.screen_DVR_label_Rear, LV_OBJ_FLAG_HIDDEN);
			lv_obj_clear_flag(guider_ui.screen_DVR_label_Rear1, LV_OBJ_FLAG_HIDDEN);
			lv_obj_set_pos(guider_ui.screen_DVR_label_Front, 0, 0);	
			lv_obj_set_pos(guider_ui.screen_DVR_label_Front1, 0, 60);
			lv_obj_set_pos(guider_ui.screen_DVR_label_Rear, 720, 0);
			lv_obj_set_pos(guider_ui.screen_DVR_label_Rear1, 720, 60);	
		break;
	}
	lv_obj_invalidate(guider_ui.screen_DVR_label_Front);
	lv_obj_invalidate(guider_ui.screen_DVR_label_Rear);
	lv_obj_invalidate(guider_ui.screen_DVR_label_Front1);
	lv_obj_invalidate(guider_ui.screen_DVR_label_Rear1);	
}
void setup_scr_screen_DVR(lv_ui *ui)
{
	ui_init_style(&style_text_align_center_mian_default);
	lv_style_set_text_color(&style_text_align_center_mian_default, lv_color_hex(0xFFFFFF));
	lv_style_set_text_font(&style_text_align_center_mian_default, &lv_font_harmonyOS_42);
	lv_style_set_text_opa(&style_text_align_center_mian_default, 255);
	lv_style_set_text_align(&style_text_align_center_mian_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_bg_opa(&style_text_align_center_mian_default, 0);
	lv_style_set_bg_color(&style_text_align_center_mian_default, lv_color_hex(0x2195f6));
	lv_style_set_border_opa(&style_text_align_center_mian_default, 0);
	//Write codes screen_DVR
	ui->screen_DVR = lv_obj_create(NULL);
	lv_obj_set_size(ui->screen_DVR, 1440, 720);
	lv_obj_set_scrollbar_mode(ui->screen_DVR, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_DVR, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_DVR, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_DVR, NULL, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_DVR_btn_return
	ui->screen_DVR_btn_return = lv_btn_create(ui->screen_DVR);
	lv_obj_set_style_pad_all(ui->screen_DVR_btn_return, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(ui->screen_DVR_btn_return, 0, 0);
	lv_obj_set_size(ui->screen_DVR_btn_return, 80, 80);

	//Write style for screen_DVR_btn_return, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_DVR_btn_return, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_DVR_btn_return, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_btn_return, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_btn_return, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_DVR_btn_return, &_back_80x80, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR_btn_return, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR_btn_return, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_DVR_btn_return, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_DVR_btn_return, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_border_width(ui->screen_DVR_btn_return, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_radius(ui->screen_DVR_btn_return, 5, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_width(ui->screen_DVR_btn_return, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_src(ui->screen_DVR_btn_return, &_back_select_80x80, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR_btn_return, 255, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR_btn_return, 0, LV_PART_MAIN|LV_STATE_PRESSED);


	//Write codes screen_DVR_img_rec
	ui->screen_DVR_img_rec = lv_img_create(ui->screen_DVR);
	lv_img_set_src(ui->screen_DVR_img_rec, &_rec_sta_40x40);
	lv_img_set_pivot(ui->screen_DVR_img_rec, 50,50);
	lv_img_set_angle(ui->screen_DVR_img_rec, 0);
	lv_obj_set_pos(ui->screen_DVR_img_rec, 1370, 20);
	lv_obj_set_size(ui->screen_DVR_img_rec, 40, 40);
	lv_obj_add_flag(ui->screen_DVR_img_rec, LV_OBJ_FLAG_HIDDEN);

	//Write style for screen_DVR_img_rec, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_recolor_opa(ui->screen_DVR_img_rec, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_opa(ui->screen_DVR_img_rec, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_img_rec, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_clip_corner(ui->screen_DVR_img_rec, true, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_DVR_cont_bottom
	ui->screen_DVR_cont_bottom = lv_obj_create(ui->screen_DVR);
	lv_obj_set_pos(ui->screen_DVR_cont_bottom, 80, 580);
	lv_obj_set_size(ui->screen_DVR_cont_bottom, 1280, 100);
	lv_obj_set_scrollbar_mode(ui->screen_DVR_cont_bottom, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_DVR_cont_bottom, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_DVR_cont_bottom, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->screen_DVR_cont_bottom, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->screen_DVR_cont_bottom, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_side(ui->screen_DVR_cont_bottom, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_cont_bottom, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_DVR_cont_bottom, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_DVR_cont_bottom, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->screen_DVR_cont_bottom, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_DVR_cont_bottom, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_DVR_cont_bottom, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_DVR_cont_bottom, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_DVR_cont_bottom, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_DVR_cont_bottom, &_dvr_btn_bg_1280x100, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR_cont_bottom, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR_cont_bottom, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_cont_bottom, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_scroll_dir(ui->screen_DVR_cont_bottom, LV_DIR_HOR);


	//Write codes screen_DVR_btn_lock
	ui->screen_DVR_btn_lock = lv_btn_create(ui->screen_DVR_cont_bottom);
	lv_obj_set_style_pad_all(ui->screen_DVR_btn_lock, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(ui->screen_DVR_btn_lock, 46, 0);
	lv_obj_set_size(ui->screen_DVR_btn_lock, 198, 100);
	lv_obj_add_flag(ui->screen_DVR_btn_lock, LV_OBJ_FLAG_CHECKABLE);

	//Write style for screen_DVR_btn_lock, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_DVR_btn_lock, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_DVR_btn_lock, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_btn_lock, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_btn_lock, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_DVR_btn_lock, &_lock_btn_dn_198x100, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR_btn_lock, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR_btn_lock, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_DVR_btn_lock, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_DVR_btn_lock, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_border_width(ui->screen_DVR_btn_lock, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_radius(ui->screen_DVR_btn_lock, 5, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_width(ui->screen_DVR_btn_lock, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_img_src(ui->screen_DVR_btn_lock, &_lock_btn_198x100, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR_btn_lock, 255, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR_btn_lock, 0, LV_PART_MAIN|LV_STATE_CHECKED);

	//Write codes screen_DVR_btn_takepic
	ui->screen_DVR_btn_takepic = lv_btn_create(ui->screen_DVR_cont_bottom);
	lv_obj_set_style_pad_all(ui->screen_DVR_btn_takepic, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(ui->screen_DVR_btn_takepic, 198+46, 0);
	lv_obj_set_size(ui->screen_DVR_btn_takepic, 198, 100);

	//Write style for screen_DVR_btn_takepic, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_DVR_btn_takepic, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_DVR_btn_takepic, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_btn_takepic, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_btn_takepic, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_DVR_btn_takepic, &_takepic_btn_198x100, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR_btn_takepic, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR_btn_takepic, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_DVR_btn_takepic, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_DVR_btn_takepic, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_border_width(ui->screen_DVR_btn_takepic, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_radius(ui->screen_DVR_btn_takepic, 5, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_width(ui->screen_DVR_btn_takepic, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_src(ui->screen_DVR_btn_takepic, &_takepic_btn_dn_198x100, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR_btn_takepic, 255, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR_btn_takepic, 0, LV_PART_MAIN|LV_STATE_PRESSED);

	//Write codes screen_DVR_btn_rec_mode
	ui->screen_DVR_btn_preview_mode = lv_btn_create(ui->screen_DVR_cont_bottom);
	lv_obj_set_style_pad_all(ui->screen_DVR_btn_preview_mode, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(ui->screen_DVR_btn_preview_mode, 198*2+46, 0);
	lv_obj_set_size(ui->screen_DVR_btn_preview_mode, 198, 100);

	//Write style for screen_DVR_btn_rec_mode, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_DVR_btn_preview_mode, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_DVR_btn_preview_mode, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_btn_preview_mode, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_btn_preview_mode, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_DVR_btn_preview_mode, &_preview_btn_198x100, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR_btn_preview_mode, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR_btn_preview_mode, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_DVR_btn_rec_mode, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_DVR_btn_preview_mode, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_border_width(ui->screen_DVR_btn_preview_mode, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_radius(ui->screen_DVR_btn_preview_mode, 5, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_width(ui->screen_DVR_btn_preview_mode, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_src(ui->screen_DVR_btn_preview_mode, &_preview_btn_dn_198x100, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR_btn_preview_mode, 255, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR_btn_preview_mode, 0, LV_PART_MAIN|LV_STATE_PRESSED);


	//Write codes screen_DVR_btn_recorder
	ui->screen_DVR_btn_recorder = lv_btn_create(ui->screen_DVR_cont_bottom);
	lv_obj_set_style_pad_all(ui->screen_DVR_btn_recorder, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(ui->screen_DVR_btn_recorder, 198*3+46, 0);
	lv_obj_set_size(ui->screen_DVR_btn_recorder, 198, 100);
	lv_obj_add_flag(ui->screen_DVR_btn_recorder, LV_OBJ_FLAG_CHECKABLE);

	//Write style for screen_DVR_btn_recorder, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_DVR_btn_recorder, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_DVR_btn_recorder, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_btn_recorder, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_btn_recorder, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_DVR_btn_recorder, &_recorder_btn_198x100, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR_btn_recorder, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR_btn_recorder, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_DVR_btn_recorder, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_DVR_btn_recorder, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_border_width(ui->screen_DVR_btn_recorder, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_radius(ui->screen_DVR_btn_recorder, 5, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_width(ui->screen_DVR_btn_recorder, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_img_src(ui->screen_DVR_btn_recorder, &_recorder_btn_dn_198x100, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR_btn_recorder, 255, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR_btn_recorder, 0, LV_PART_MAIN|LV_STATE_CHECKED);

	//Write codes screen_DVR_btn_playback
	ui->screen_DVR_btn_playback = lv_btn_create(ui->screen_DVR_cont_bottom);
	lv_obj_set_style_pad_all(ui->screen_DVR_btn_playback, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(ui->screen_DVR_btn_playback, 198*4+46, 0);
	lv_obj_set_size(ui->screen_DVR_btn_playback, 198, 100);

	//Write style for screen_DVR_btn_playback, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_DVR_btn_playback, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_DVR_btn_playback, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_btn_playback, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_btn_playback, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_DVR_btn_playback, &_menu_btn_198x100, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR_btn_playback, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR_btn_playback, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_DVR_btn_playback, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_DVR_btn_playback, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_border_width(ui->screen_DVR_btn_playback, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_radius(ui->screen_DVR_btn_playback, 5, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_width(ui->screen_DVR_btn_playback, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_src(ui->screen_DVR_btn_playback, &_menu_btn_dn_198x100, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR_btn_playback, 255, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR_btn_playback, 0, LV_PART_MAIN|LV_STATE_PRESSED);

	//Write codes screen_DVR_btn_set
	ui->screen_DVR_btn_set = lv_btn_create(ui->screen_DVR_cont_bottom);
	lv_obj_set_style_pad_all(ui->screen_DVR_btn_set, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(ui->screen_DVR_btn_set, 198*5+46, 0);
	lv_obj_set_size(ui->screen_DVR_btn_set, 198, 100);

	//Write style for screen_DVR_btn_set, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_DVR_btn_set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_DVR_btn_set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_btn_set, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_btn_set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_DVR_btn_set, &_set_btn_198x100, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR_btn_set, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR_btn_set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_DVR_btn_set, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
	lv_obj_set_style_bg_opa(ui->screen_DVR_btn_set, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_border_width(ui->screen_DVR_btn_set, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_radius(ui->screen_DVR_btn_set, 5, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_width(ui->screen_DVR_btn_set, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_src(ui->screen_DVR_btn_set, &_set_btn_dn_198x100, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_opa(ui->screen_DVR_btn_set, 255, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_bg_img_recolor_opa(ui->screen_DVR_btn_set, 0, LV_PART_MAIN|LV_STATE_PRESSED);

		//Write codes screen_DVR_label_Popup
	ui->screen_DVR_label_Popup = lv_label_create(ui->screen_DVR);
	lv_label_set_text(ui->screen_DVR_label_Popup, get_string_for_language(g_sys_Data.current_language,"dvr_set_txt_tfDetected"));
	lv_label_set_long_mode(ui->screen_DVR_label_Popup, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(ui->screen_DVR_label_Popup, 570, 200);
	lv_obj_set_size(ui->screen_DVR_label_Popup, 400, 250);

	//Write style for screen_DVR_label_Popup, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_DVR_label_Popup, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_DVR_label_Popup, 15, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_DVR_label_Popup, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_DVR_label_Popup, &lv_font_harmonyOS_42, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui->screen_DVR_label_Popup, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(ui->screen_DVR_label_Popup, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(ui->screen_DVR_label_Popup, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	
	lv_obj_set_style_bg_opa(ui->screen_DVR_label_Popup, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_DVR_label_Popup, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->screen_DVR_label_Popup, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_DVR_label_Popup, 80, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_DVR_label_Popup, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_DVR_label_Popup, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_DVR_label_Popup, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_DVR_label_Popup, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_DVR_label_Popup, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_flag(guider_ui.screen_DVR_label_Popup, LV_OBJ_FLAG_HIDDEN);

	ui->screen_DVR_label_Front= lv_label_create(ui->screen_DVR);
	lv_label_set_text(ui->screen_DVR_label_Front, get_string_for_language(g_sys_Data.current_language,"dvr_txt_front"));
	lv_label_set_long_mode(ui->screen_DVR_label_Front, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_DVR_label_Front, 0, 80);
	lv_obj_set_size(ui->screen_DVR_label_Front, 720, 60);
	lv_obj_add_style(ui->screen_DVR_label_Front, &style_text_align_center_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_DVR_label_Front1= lv_label_create(ui->screen_DVR);
	lv_label_set_text(ui->screen_DVR_label_Front1, "");
	lv_label_set_long_mode(ui->screen_DVR_label_Front1, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_DVR_label_Front1, 0, 80);
	lv_obj_set_size(ui->screen_DVR_label_Front1, 720, 60);
	lv_obj_add_style(ui->screen_DVR_label_Front1, &style_text_align_center_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_DVR_label_Rear= lv_label_create(ui->screen_DVR);
	lv_label_set_text(ui->screen_DVR_label_Rear, get_string_for_language(g_sys_Data.current_language,"dvr_txt_rear"));
	lv_label_set_long_mode(ui->screen_DVR_label_Rear, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_DVR_label_Rear, 0, 80);
	lv_obj_set_size(ui->screen_DVR_label_Rear, 720, 60);
	lv_obj_add_style(ui->screen_DVR_label_Rear, &style_text_align_center_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_flag(ui->screen_DVR_label_Rear, LV_OBJ_FLAG_HIDDEN);
	ui->screen_DVR_label_Rear1= lv_label_create(ui->screen_DVR);
	lv_label_set_text(ui->screen_DVR_label_Rear1, "");
	lv_label_set_long_mode(ui->screen_DVR_label_Rear1, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_DVR_label_Rear1, 0, 80);
	lv_obj_set_size(ui->screen_DVR_label_Rear1, 720, 60);
	lv_obj_add_style(ui->screen_DVR_label_Rear1, &style_text_align_center_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);


	char str[10];
	sprintf(str, "%d", g_sys_Data.carSpeed);
	ui->screen_DVR_label_carSpeedMark= lv_label_create(ui->screen_DVR);
	lv_label_set_text(ui->screen_DVR_label_carSpeedMark, str);
	lv_label_set_long_mode(ui->screen_DVR_label_carSpeedMark, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_DVR_label_carSpeedMark, 0, 340);
	lv_obj_set_size(ui->screen_DVR_label_carSpeedMark, 300, 60);
	lv_obj_add_style(ui->screen_DVR_label_carSpeedMark, &style_text_align_center_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_DVR_label_carSpeedMark,  &lv_font_harmonyOS_45, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->screen_DVR_label_carSpeedMarkUnit= lv_label_create(ui->screen_DVR);
	lv_label_set_text(ui->screen_DVR_label_carSpeedMarkUnit, (g_sys_Data.SpeedUnit ? "mi/h" : "km/h"));
	lv_label_set_long_mode(ui->screen_DVR_label_carSpeedMarkUnit, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_pos(ui->screen_DVR_label_carSpeedMarkUnit, 0, 400);
	lv_obj_set_size(ui->screen_DVR_label_carSpeedMarkUnit, 300, 60);
	lv_obj_add_style(ui->screen_DVR_label_carSpeedMarkUnit, &style_text_align_center_mian_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_DVR_label_carSpeedMarkUnit,  &lv_font_harmonyOS_30, LV_PART_MAIN|LV_STATE_DEFAULT);		

	lv_obj_add_flag(ui->screen_DVR_label_Rear1, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(ui->screen_DVR_label_Front, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(ui->screen_DVR_label_Front1, LV_OBJ_FLAG_HIDDEN);	
	lv_obj_add_flag(ui->screen_DVR_label_Rear, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(ui->screen_DVR_label_Rear1, LV_OBJ_FLAG_HIDDEN);
	
	lv_obj_move_foreground(ui->screen_DVR_btn_return);


	startPreview(&g_sys_Data.vipp0_config,720,720,0,720);
	startPreview(&g_sys_Data.vipp8_config,720,720,0,0);
	g_sys_Data.previewMode = PREVIEW_FRONT_REAR;
	prviewModeLabelDash(g_sys_Data.previewMode);

	HideBtnContTimer = lv_timer_create(HideBtnCont, 5000, NULL);
	lv_timer_set_repeat_count(HideBtnContTimer, 1);




	
	//Update current screen layout.
	lv_obj_update_layout(ui->screen_DVR);


	//Init events for screen.
	events_init_screen_DVR(ui);


#if 1
	if(g_sys_Data.ScreenPlayBackOut){
		g_sys_Data.ScreenPlayBackOut = false;
		if(g_sys_Data.recorderMode == RECORDER_NONE && (g_sys_Data.frontCamera || g_sys_Data.rearCamera) && g_sys_Data.TFmounted){
			g_sys_Data.recorderMode = RECORDER_NORMAL;
			printf("start to recording!!!!!\n");
			
			#if 1
			if(!TFFreeMemDetection() && (queue_mpp_size(F_videoFile) < 10) && (queue_mpp_size(R_videoFile) < 10)){
				if(is_popup_visible_v8) close_popup_v8();
				show_popup_simple_v8(get_string_for_language(g_sys_Data.current_language,"main_txt_TFNotFreeMem"));
				return;
			}	
			deletFileInRecorderPath(REC_PATH);

			recording(&g_sys_Data.vipp0_config);	
			recording(&g_sys_Data.vipp8_config);

			dashTimeMark(&g_sys_Data.vipp0_config, g_sys_Data.TimeMark);
			dashTimeMark(&g_sys_Data.vipp8_config, g_sys_Data.TimeMark);


			SoundRecording(&g_sys_Data.vipp0_config, g_sys_Data.SoundRecorder);
			SoundRecording(&g_sys_Data.vipp8_config, g_sys_Data.SoundRecorder);
			#endif
			if(DVRstaTimer == NULL) DVRstaTimer = lv_timer_create(recorder_status_timer, 1000, NULL); 
		}
	}
#endif

	if(g_sys_Data.recorderMode == RECORDER_NORMAL || g_sys_Data.recorderMode == RECORDER_URGENT){
		lv_obj_add_state(ui->screen_DVR_btn_recorder, LV_STATE_CHECKED);
	}
	if(g_sys_Data.recorderMode == RECORDER_URGENT){
		lv_obj_add_state(ui->screen_DVR_btn_lock, LV_STATE_CHECKED);
	}

	if(g_sys_Data.SpeedMark){
		lv_obj_clear_flag(ui->screen_DVR_label_carSpeedMark, LV_OBJ_FLAG_HIDDEN);
		lv_obj_clear_flag(ui->screen_DVR_label_carSpeedMarkUnit, LV_OBJ_FLAG_HIDDEN);
	}else{
		lv_obj_add_flag(ui->screen_DVR_label_carSpeedMark, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(ui->screen_DVR_label_carSpeedMarkUnit, LV_OBJ_FLAG_HIDDEN);
	}

}

