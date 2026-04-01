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
#include "mppFileManager.h"

extern lv_timer_t *DVRstaTimer;
extern lv_timer_t *HideBtnContTimer;
extern void recorder_status_timer(lv_timer_t *timer);
extern void HideBtnCont(void);
extern void clearRecorderStatu(void);
extern void prviewModeLabelDash(preview_mode mode);
static recorder_mode lastRecorderMode = RECORDER_NORMAL;

static void screen_DVR_btn_return_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		stop_label_timer(guider_ui.screen_DVR_label_Popup);
		ui_load_scr_animation(&guider_ui, &guider_ui.screen, guider_ui.screen_del, &guider_ui.screen_DVR_del, setup_scr_screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, true, true);

		stopPreview(&g_sys_Data.vipp0_config);
		stopPreview(&g_sys_Data.vipp8_config);		
		g_sys_Data.previewMode = PREVIEW_NONE;

		if(HideBtnContTimer != NULL){
			lv_timer_del(HideBtnContTimer);
			HideBtnContTimer = NULL;
		}

		break;
	}
    default:
        break;
    }
}

static void screen_DVR_btn_preview_mode_event_handler (lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);
	m_para_conf para;
    memset(&para, 0 , sizeof(m_para_conf));
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(!lv_obj_has_flag(guider_ui.screen_DVR_label_Popup, LV_OBJ_FLAG_HIDDEN)) break;

		switch (g_sys_Data.previewMode)
		{
		case PREVIEW_NONE:
			startPreview(&g_sys_Data.vipp0_config,720,1440,0,0);
			g_sys_Data.previewMode = PREVIEW_FRONT;
			break;
		case PREVIEW_FRONT:
			stopPreview(&g_sys_Data.vipp0_config);
			startPreview(&g_sys_Data.vipp8_config,720,1440,0,0);	
			g_sys_Data.previewMode = PREVIEW_REAR;
			break;
		case PREVIEW_REAR:
			stopPreview(&g_sys_Data.vipp8_config);
			startPreview(&g_sys_Data.vipp0_config,720,720,0,720);

			startPreview(&g_sys_Data.vipp8_config,720,720,0,0);
			g_sys_Data.previewMode = PREVIEW_FRONT_REAR;
			break;
		case PREVIEW_FRONT_REAR:
			stopPreview(&g_sys_Data.vipp0_config);
			stopPreview(&g_sys_Data.vipp8_config);	
			startPreview(&g_sys_Data.vipp0_config,720,1440,0,0);
			g_sys_Data.previewMode = PREVIEW_FRONT;
			break;	
		default:
			break;
		}
		prviewModeLabelDash(g_sys_Data.previewMode);
		break;
	}
    default:
        break;
    }	
}


static void screen_DVR_btn_recorder_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		/* code */
		if(!lv_obj_has_flag(guider_ui.screen_DVR_label_Popup, LV_OBJ_FLAG_HIDDEN)) break;

		if(!checkTFCardMountProc()){
			lv_obj_clear_state(guider_ui.screen_DVR_btn_recorder, LV_STATE_CHECKED);
			show_label_with_timer(guider_ui.screen_DVR_label_Popup, "dvr_set_txt_tfDetected", 1000);
			break;
		}

		switch (g_sys_Data.recorderMode)
		{
		case RECORDER_NONE:
			
			g_sys_Data.recorderMode = RECORDER_NORMAL;
			printf("start to recording!!!!!\n");
			show_label_with_timer(guider_ui.screen_DVR_label_Popup, "dvr_txt_startRecorder", 1000);
#if 1
			if(!TFFreeMemDetection() && (queue_mpp_size(F_videoFile) < 10) && (queue_mpp_size(R_videoFile) < 10)) {
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
			break;
		case RECORDER_NORMAL:
		case RECORDER_URGENT:
			g_sys_Data.recorderMode = RECORDER_NONE;
			printf("stop to recording!!!!!\n");
			show_label_with_timer(guider_ui.screen_DVR_label_Popup, "dvr_txt_stopRecorder", 1000);
#if 1
			stopRecording(&g_sys_Data.vipp0_config);
			stopRecording(&g_sys_Data.vipp8_config);
#endif
            if(DVRstaTimer != NULL){
                lv_timer_del(DVRstaTimer);
                DVRstaTimer = NULL;	
            }		
			clearRecorderStatu();
			lv_obj_clear_state(guider_ui.screen_DVR_btn_lock, LV_STATE_CHECKED);
			break;
			/* code */
			break;		
		default:
			break;
		}
		break;
	
	default:
		break;
	}
}

static void screen_DVR_btn_playback_event_handler(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
		/* code */
		if(!lv_obj_has_flag(guider_ui.screen_DVR_label_Popup, LV_OBJ_FLAG_HIDDEN)) break;

		if(!checkTFCardMountProc()){
			show_label_with_timer(guider_ui.screen_DVR_label_Popup, "dvr_set_txt_tfDetected", 1000);
			break;
		} else {
			/* 先切屏再停预览：立即加载媒体库画面，避免在回调内同步 stopPreview 导致黑屏 */
			
#if 1
			if(g_sys_Data.recorderMode == RECORDER_NORMAL || g_sys_Data.recorderMode == RECORDER_URGENT){
				g_sys_Data.recorderMode = RECORDER_NONE;
				show_label_with_timer(guider_ui.screen_DVR_label_Popup, "dvr_txt_stopRecorder", 1000);
				#if 1
				stopRecording(&g_sys_Data.vipp0_config);
				stopRecording(&g_sys_Data.vipp8_config);
				#endif
				if(DVRstaTimer != NULL){
					lv_timer_del(DVRstaTimer);
					DVRstaTimer = NULL;	
				}	
				clearRecorderStatu();
			}
#endif
			ui_load_scr_animation(&guider_ui, &guider_ui.screen_MediaLibrary,guider_ui.screen_MediaLibrary_del, &guider_ui.screen_DVR_del,setup_scr_media_library, LV_SCR_LOAD_ANIM_NONE,0, 0, true, true);

			g_sys_Data.previewMode = PREVIEW_NONE;
			stopPreview(&g_sys_Data.vipp0_config);
			stopPreview(&g_sys_Data.vipp8_config);

		}
		break;
	
	default:
		break;
	}
}

static void screen_DVR_btn_set_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(!lv_obj_has_flag(guider_ui.screen_DVR_label_Popup, LV_OBJ_FLAG_HIDDEN)) break;

		stopPreview(&g_sys_Data.vipp0_config);
		stopPreview(&g_sys_Data.vipp8_config);		
		g_sys_Data.previewMode = PREVIEW_NONE;

		ui_load_scr_animation(&guider_ui, &guider_ui.screen_DVR_SET, guider_ui.screen_DVR_SET_del, &guider_ui.screen_DVR_del, setup_scr_screen_DVR_SET, LV_SCR_LOAD_ANIM_NONE, 0, 0, true, true);
		break;
	}
    default:
        break;
    }
}

static void screen_DVR_btn_takepic_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(!lv_obj_has_flag(guider_ui.screen_DVR_label_Popup, LV_OBJ_FLAG_HIDDEN)) break;

		if(!checkTFCardMountProc()){
			show_label_with_timer(guider_ui.screen_DVR_label_Popup, "dvr_set_txt_tfDetected", 1000);
			break;
		}

		if(!TFFreeMemDetection() && (queue_mpp_size(F_picFile) < 2) && (queue_mpp_size(R_picFile) < 2)) {
			if(is_popup_visible_v8) close_popup_v8();
			show_popup_simple_v8(get_string_for_language(g_sys_Data.current_language,"main_txt_TFNotFreeMem"));
			return;
		}			
		deletFileInRecorderPath(PIC_PATH);
		startTakePic(&g_sys_Data.vipp0_config);
		startTakePic(&g_sys_Data.vipp8_config);
		if(is_popup_visible_v8) close_popup_v8();
        show_popup_simple_v8(get_string_for_language(g_sys_Data.current_language,"dvr_txt_tackpic"));
		break;
	}
    default:
        break;
    }
}


static void screen_DVR_btn_lock_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(!lv_obj_has_flag(guider_ui.screen_DVR_label_Popup, LV_OBJ_FLAG_HIDDEN)) break;

		if(!checkTFCardMountProc()){
			lv_obj_clear_state(guider_ui.screen_DVR_btn_lock, LV_STATE_CHECKED);
			show_label_with_timer(guider_ui.screen_DVR_label_Popup, "dvr_set_txt_tfDetected", 1000);
			break;
		}
		switch (g_sys_Data.recorderMode)
		{
		case RECORDER_NONE:
			g_sys_Data.recorderMode = RECORDER_URGENT;
			printf("start to recording!!!!!\n");

			if(is_popup_visible_v8) close_popup_v8();
            show_popup_simple_v8(get_string_for_language(g_sys_Data.current_language,"dvr_txt_startUrgentRecorder"));
#if 1
			if(!TFFreeMemDetection() && (queue_mpp_size(F_U_videoFile) < 10) && (queue_mpp_size(R_U_videoFile) < 10)){
				if(is_popup_visible_v8) close_popup_v8();
				show_popup_simple_v8(get_string_for_language(g_sys_Data.current_language,"main_txt_TFNotFreeMem"));
				return;
			}	
			deletFileInRecorderPath(U_REC_PATH);

			recording(&g_sys_Data.vipp0_config);
			recording(&g_sys_Data.vipp8_config);

			dashTimeMark(&g_sys_Data.vipp0_config, g_sys_Data.TimeMark);
			dashTimeMark(&g_sys_Data.vipp8_config, g_sys_Data.TimeMark);			

			SoundRecording(&g_sys_Data.vipp0_config, g_sys_Data.SoundRecorder);
			SoundRecording(&g_sys_Data.vipp8_config, g_sys_Data.SoundRecorder);
#endif
			if(DVRstaTimer == NULL) DVRstaTimer = lv_timer_create(recorder_status_timer, 1000, NULL); 

			lv_obj_add_state(guider_ui.screen_DVR_btn_recorder, LV_STATE_CHECKED);
			break;
		case RECORDER_NORMAL:
			g_sys_Data.recorderMode = RECORDER_URGENT;
            if(DVRstaTimer != NULL){
                lv_timer_del(DVRstaTimer);
                DVRstaTimer = NULL;	
            }		
			clearRecorderStatu();

			if(DVRstaTimer == NULL) DVRstaTimer = lv_timer_create(recorder_status_timer, 1000, NULL);
			if(!TFFreeMemDetection() && (queue_mpp_size(F_U_videoFile) < 10) && (queue_mpp_size(R_U_videoFile) < 10)){
				if(is_popup_visible_v8) close_popup_v8();
				show_popup_simple_v8(get_string_for_language(g_sys_Data.current_language,"main_txt_TFNotFreeMem"));
				return;
			}

			changeVideoRecordingMode(&g_sys_Data.vipp0_config);
			changeVideoRecordingMode(&g_sys_Data.vipp8_config);

			if(is_popup_visible_v8) close_popup_v8();
            show_popup_simple_v8(get_string_for_language(g_sys_Data.current_language,"dvr_txt_startUrgentRecorder"));
			break;
		case RECORDER_URGENT:
			g_sys_Data.recorderMode = RECORDER_NORMAL;
			if(DVRstaTimer != NULL){
                lv_timer_del(DVRstaTimer);
                DVRstaTimer = NULL;	
            }		
			clearRecorderStatu();

			if(DVRstaTimer == NULL) DVRstaTimer = lv_timer_create(recorder_status_timer, 1000, NULL);
			changeVideoRecordingMode(&g_sys_Data.vipp0_config);
			changeVideoRecordingMode(&g_sys_Data.vipp8_config);
			/* code */

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



static void screen_DVR_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{

		if(!lv_obj_has_flag(guider_ui.screen_DVR_label_Popup, LV_OBJ_FLAG_HIDDEN)) break;

		if(lv_obj_has_flag(guider_ui.screen_DVR_cont_bottom, LV_OBJ_FLAG_HIDDEN)){
			lv_obj_clear_flag(guider_ui.screen_DVR_cont_bottom, LV_OBJ_FLAG_HIDDEN);
		}
		if(HideBtnContTimer){
			lv_timer_reset(HideBtnContTimer);
		}else{
			HideBtnContTimer = lv_timer_create(HideBtnCont, 5000, NULL);
			lv_timer_set_repeat_count(HideBtnContTimer, 1);
		}
		break;
	}
    default:
        break;
    }
}

void events_init_screen_DVR (lv_ui *ui)
{

	lv_obj_add_event_cb(ui->screen_DVR, screen_DVR_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_DVR_btn_return, screen_DVR_btn_return_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_DVR_btn_preview_mode, screen_DVR_btn_preview_mode_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_DVR_btn_recorder, screen_DVR_btn_recorder_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_DVR_btn_playback, screen_DVR_btn_playback_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_DVR_btn_set, screen_DVR_btn_set_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_DVR_btn_takepic, screen_DVR_btn_takepic_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_DVR_btn_lock, screen_DVR_btn_lock_event_handler, LV_EVENT_ALL, ui);
}