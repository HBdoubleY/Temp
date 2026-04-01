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

static void screen_DVR_SET_btn_return_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		ui_load_scr_animation(&guider_ui, &guider_ui.screen_DVR, guider_ui.screen_DVR_del, &guider_ui.screen_DVR_SET_del, setup_scr_screen_DVR, LV_SCR_LOAD_ANIM_NONE, 0, 0, true, true);
		break;
	}
    default:
        break;
    }
}

static void screen_DVR_SET_btn_rearCameraImage_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(g_sys_Data.rearCameraImage){
			g_sys_Data.rearCameraImage = false;
		}else{
			g_sys_Data.rearCameraImage = true;
		}
		if(!storageData_setBool("rearCameraImage", g_sys_Data.rearCameraImage)){
			printf("storage data rearCameraImage[%d] fail!\n", g_sys_Data.rearCameraImage);
		}
		break;
	}
    default:
        break;
    }
}
extern void RecorderTimeBtnSta();
static void screen_DVR_SET_btn_recorder_1min_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		g_sys_Data.recorderTime = RECORDER_1;
		RecorderTimeBtnSta();
		if(!storageData_setInt("recorderTime", g_sys_Data.recorderTime)){
			printf("storage data recorderTime[%d] fail!\n", g_sys_Data.recorderTime);
		}
		break;
	}
    default:
        break;
    }
}
static void screen_DVR_SET_btn_recorder_3min_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		g_sys_Data.recorderTime = RECORDER_3;
		RecorderTimeBtnSta();
		if(!storageData_setInt("recorderTime", g_sys_Data.recorderTime)){
			printf("storage data recorderTime[%d] fail!\n", g_sys_Data.recorderTime);
		}
		break;
	}
    default:
        break;
    }
}
static void screen_DVR_SET_btn_recorder_5min_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		g_sys_Data.recorderTime = RECORDER_5;
		RecorderTimeBtnSta();
		if(!storageData_setInt("recorderTime", g_sys_Data.recorderTime)){
			printf("storage data recorderTime[%d] fail!\n", g_sys_Data.recorderTime);
		}
		break;
	}
    default:
        break;
    }
}
static void screen_DVR_SET_btn_urgent_1min_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		g_sys_Data.recorderUrgentTime = RECORDER_1;
		RecorderTimeBtnSta();
		if(!storageData_setInt("recorderUrgentTime", g_sys_Data.recorderUrgentTime)){
			printf("storage data recorderUrgentTime[%d] fail!\n", g_sys_Data.recorderUrgentTime);
		}
		break;
	}
    default:
        break;
    }
}
static void screen_DVR_SET_btn_urgent_3min_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		g_sys_Data.recorderUrgentTime = RECORDER_3;
		RecorderTimeBtnSta();
		if(!storageData_setInt("recorderUrgentTime", g_sys_Data.recorderUrgentTime)){
			printf("storage data recorderUrgentTime[%d] fail!\n", g_sys_Data.recorderUrgentTime);
		}
		break;
	}
    default:
        break;
    }
}
static void screen_DVR_SET_btn_Urgent_5min_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		g_sys_Data.recorderUrgentTime = RECORDER_5;
		RecorderTimeBtnSta();
		if(!storageData_setInt("recorderUrgentTime", g_sys_Data.recorderUrgentTime)){
			printf("storage data recorderUrgentTime[%d] fail!\n", g_sys_Data.recorderUrgentTime);
		}
		break;
	}
    default:
        break;
    }
}
static void screen_DVR_SET_btn_powerOnRecording_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(g_sys_Data.powerOnRecorder){
			g_sys_Data.powerOnRecorder = false;
		}else{
			g_sys_Data.powerOnRecorder = true;
		}
		if(!storageData_setBool("powerOnRecorder", g_sys_Data.powerOnRecorder)){
			printf("storage data powerOnRecorder[%d] fail!\n", g_sys_Data.powerOnRecorder);
		}
		break;
	}
    default:
        break;
    }
}
static void screen_DVR_SET_btn_recorderSoundSet_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(g_sys_Data.SoundRecorder){
			g_sys_Data.SoundRecorder = false;		

		}else{
			g_sys_Data.SoundRecorder = true;

		}

		if(g_sys_Data.recorderMode == RECORDER_NORMAL || g_sys_Data.recorderMode == RECORDER_URGENT){
			SoundRecording(&g_sys_Data.vipp0_config, g_sys_Data.SoundRecorder);
			SoundRecording(&g_sys_Data.vipp8_config, g_sys_Data.SoundRecorder);
		}


		if(!storageData_setBool("SoundRecorder", g_sys_Data.SoundRecorder)){
			printf("storage data SoundRecorder[%d] fail!\n", g_sys_Data.SoundRecorder);
		}
		break;
	}
    default:
        break;
    }
}
static void screen_DVR_SET_btn_recorderTimeMark_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(g_sys_Data.TimeMark){
			g_sys_Data.TimeMark = false;

		}else{
			g_sys_Data.TimeMark = true;

		}
		
		if(g_sys_Data.recorderMode == RECORDER_NORMAL || g_sys_Data.recorderMode == RECORDER_URGENT){
			dashTimeMark(&g_sys_Data.vipp0_config, g_sys_Data.TimeMark);
			dashTimeMark(&g_sys_Data.vipp8_config, g_sys_Data.TimeMark);
		}

		if(!storageData_setBool("TimeMark", g_sys_Data.TimeMark)){
			printf("storage data TimeMark[%d] fail!\n", g_sys_Data.TimeMark);
		}				
		break;
	}
    default:
        break;
    }
}
static void screen_DVR_SET_btn_carSpeedMark_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(g_sys_Data.SpeedMark){
			g_sys_Data.SpeedMark = false;


		}else{
			g_sys_Data.SpeedMark = true;


		}
		if(!storageData_setBool("SpeedMark", g_sys_Data.SpeedMark)){
			printf("storage data SpeedMark[%d] fail!\n", g_sys_Data.SpeedMark);
		}		
		break;
	}
    default:
        break;
    }
}

static void screen_DVR_SET_btn_carSpeedUnit_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(g_sys_Data.SpeedUnit){
			g_sys_Data.SpeedUnit = false;
		}else{
			g_sys_Data.SpeedUnit = true;
		}
		if(!storageData_setBool("SpeedUnit", g_sys_Data.SpeedUnit)){
			printf("storage data SpeedUnit[%d] fail!\n", g_sys_Data.SpeedUnit);
		}		
		break;
	}
    default:
        break;
    }
}

void events_init_screen_DVR_SET (lv_ui *ui)
{
	lv_obj_add_event_cb(ui->screen_DVR_SET_btn_return, screen_DVR_SET_btn_return_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_DVR_SET_btn_rearCameraImage, screen_DVR_SET_btn_rearCameraImage_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_DVR_SET_btn_recorder_1min, screen_DVR_SET_btn_recorder_1min_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_DVR_SET_btn_recorder_3min, screen_DVR_SET_btn_recorder_3min_event_handler,LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_DVR_SET_btn_recorder_5min , screen_DVR_SET_btn_recorder_5min_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_DVR_SET_btn_urgent_1min, screen_DVR_SET_btn_urgent_1min_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_DVR_SET_btn_urgent_3min, screen_DVR_SET_btn_urgent_3min_event_handler,LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_DVR_SET_btn_urgent_5min , screen_DVR_SET_btn_Urgent_5min_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_DVR_SET_btn_powerOnRecording , screen_DVR_SET_btn_powerOnRecording_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_DVR_SET_btn_recorderSoundSet , screen_DVR_SET_btn_recorderSoundSet_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_DVR_SET_btn_recorderTimeMark , screen_DVR_SET_btn_recorderTimeMark_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_DVR_SET_btn_carSpeedMark, screen_DVR_SET_btn_carSpeedMark_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_DVR_SET_btn_carSpeedUnit, screen_DVR_SET_btn_carSpeedUnit_event_handler, LV_EVENT_ALL, ui);
}