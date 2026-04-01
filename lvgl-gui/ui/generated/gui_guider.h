/*
* Copyright 2025 NXP
* NXP Confidential and Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef GUI_GUIDER_H
#define GUI_GUIDER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "lvgl_system.h"
extern char *get_bt_name();

typedef struct
{ 
	lv_obj_t *screen;
	bool screen_del;
	lv_obj_t *screen_MediaLibrary;
	bool screen_MediaLibrary_del;
	lv_obj_t *screen_OTA;
	bool screen_OTA_del;
	lv_obj_t *screen_btn_cartrip;
	lv_obj_t *screen_btn_cartrip_label_fTyre;
	lv_obj_t *screen_btn_cartrip_label_bTyre;
	lv_obj_t *screen_btn_cartrip_label_fTyre_data;
	lv_obj_t *screen_btn_cartrip_label_bTyre_data;	
	lv_obj_t *screen_btn_DVR;
	lv_obj_t *screen_btn_DVR_label;
	lv_obj_t *screen_btn_DVR_label_status;
	lv_obj_t *screen_btn_set;
	lv_obj_t *screen_btn_set_label;
	lv_obj_t *screen_btn_androidauto;
	lv_obj_t *screen_btn_androidauto_label;
	lv_obj_t *screen_btn_androidauto_label_statu;
	lv_obj_t *screen_btn_carplay;
	lv_obj_t *screen_btn_carplay_label;
	lv_obj_t *screen_btn_carplay_label_statu;
	lv_obj_t *screen_datetext;
	lv_obj_t *screen_cont_languageChoose;
	lv_obj_t *screen_label_textChoose;
	lv_obj_t *screen_list_language;
	lv_obj_t *screen_list_language_itemEN;
	lv_obj_t *screen_list_language_itemCN;
	lv_obj_t *screen_list_language_itemCN_FT;
	lv_obj_t *screen_list_language_itemFR;
	lv_obj_t *screen_list_language_itemGE;
	lv_obj_t *screen_list_language_itemIT;
	lv_obj_t *screen_list_language_itemJA;
	lv_obj_t *screen_list_language_itemPOR;
	lv_obj_t *screen_list_language_itemRU;
	lv_obj_t *screen_list_language_itemSP;
	lv_obj_t *screen_list_language_itemPOL;
	lv_obj_t *screen_list_language_itemTU;
	lv_obj_t *screen_list_language_itemKO;
	lv_obj_t *screen_list_language_itemDU;
	lv_obj_t *screen_btn_confirm;
	lv_obj_t *screen_btn_confirm_label;
	lv_obj_t *screen_btn_lockScreen;
	lv_obj_t *screen_label_Popup;
	// lv_obj_t *screen_digital_clock_1;
	// lv_obj_t *screen_weekdaytext_1;
	lv_obj_t *screen_cont_sta_icon;
	lv_obj_t *screen_img_bt;
	lv_obj_t *screen_img_wifi;
	lv_obj_t *screen_img_TF;
	lv_obj_t *screen_img_carPLay;
	lv_obj_t *screen_img_androiAuto;
	lv_obj_t *screen_img_rec;
	lv_obj_t *screen_img_GPS;
	lv_obj_t *screen_DVR;
	bool screen_DVR_del;
	lv_obj_t *screen_DVR_btn_return;
	lv_obj_t *screen_DVR_img_rec;
	lv_obj_t *screen_DVR_cont_bottom;
	lv_obj_t *screen_DVR_btn_playback;
	lv_obj_t *screen_DVR_btn_recorder;
	lv_obj_t *screen_DVR_btn_takepic;
	lv_obj_t *screen_DVR_btn_lock;
	lv_obj_t *screen_DVR_btn_preview_mode;
	lv_obj_t *screen_DVR_btn_set;
	lv_obj_t *screen_DVR_label_Popup;
	lv_obj_t *screen_DVR_label_Front;
	lv_obj_t *screen_DVR_label_Front1;
	lv_obj_t *screen_DVR_label_Rear;
	lv_obj_t *screen_DVR_label_Rear1;
	lv_obj_t *screen_DVR_label_carSpeedMark;
	lv_obj_t *screen_DVR_label_carSpeedMarkUnit;
	lv_obj_t *screen_SET;
	bool screen_SET_del;
	lv_obj_t *screen_SET_btn_return;
	lv_obj_t *screen_SET_list_set;
	lv_obj_t *screen_SET_list_set_item0;
	lv_obj_t *screen_SET_list_set_item1;
	lv_obj_t *screen_SET_list_set_item2;
	lv_obj_t *screen_SET_img_lineY;
	lv_obj_t *screen_SET_cont_tongyong;
	lv_obj_t *screen_SET_cont_tongyong_img_lineX0;
	lv_obj_t *screen_SET_cont_tongyong_img_lineX1;
	lv_obj_t *screen_SET_cont_tongyong_img_lineX2;
	lv_obj_t *screen_SET_cont_tongyong_img_lineX3;
	lv_obj_t *screen_SET_cont_tongyong_img_lineX4;
	lv_obj_t *screen_SET_btn_taiya;
	lv_obj_t *screen_SET_btn_taiya_label;
	lv_obj_t *screen_SET_btn_taiya_sw_label;
	lv_obj_t *screen_SET_btn_taiya_img;
	lv_obj_t *screen_SET_label_beiguang;
	lv_obj_t *screen_SET_btn_shijian;
	lv_obj_t *screen_SET_btn_shijian_label;
	lv_obj_t *screen_SET_btn_shijian_img;
	lv_obj_t *screen_SET_btn_yuyan;
	lv_obj_t *screen_SET_btn_yuyan_label;
	lv_obj_t *screen_SET_btn_yuyan_label1;
	lv_obj_t *screen_SET_btn_yuyan_img;
	lv_obj_t *screen_SET_label_zhuti;
	lv_obj_t *screen_SET_label_zhuti1;
	lv_obj_t *screen_SET_btn_zhuti_left;
	lv_obj_t *screen_SET_btn_zhuti_right;
	lv_obj_t *screen_SET_slider_backlight;
	lv_obj_t *screen_SET_label_beiguang1;
	lv_obj_t *screen_SET_cont_lanya;
	lv_obj_t *screen_SET_label_connect;
	lv_obj_t *screen_SET_label_pairdev;
	lv_obj_t *screen_SET_label_btname1;
	lv_obj_t *screen_SET_label_btname;
	lv_obj_t *screen_SET_cont_lanya_img_lineX0;
	lv_obj_t *screen_SET_cont_lanya_img_lineX1;
	lv_obj_t *screen_SET_cont_sys;
	lv_obj_t *screen_SET_cont_sys_img_lineX0;
	lv_obj_t *screen_SET_cont_sys_img_lineX1;
	lv_obj_t *screen_SET_cont_sys_img_lineX2;
	lv_obj_t *screen_SET_cont_sys_img_lineX3;	
	lv_obj_t *screen_SET_btn_refactory;
	lv_obj_t *screen_SET_btn_refactory_label;
	lv_obj_t *screen_SET_btn_refactory_img;
	lv_obj_t *screen_SET_btn_reboot;
	lv_obj_t *screen_SET_btn_reboot_label;
	lv_obj_t *screen_SET_btn_reboot_img;
	lv_obj_t *screen_SET_btn_logo;
	lv_obj_t *screen_SET_btn_logo_label;
	lv_obj_t *screen_SET_btn_logo_img;
	lv_obj_t *screen_SET_btn_about;
	lv_obj_t *screen_SET_btn_about_label;
	lv_obj_t *screen_SET_btn_about_img;
	lv_obj_t *screen_SET_cont_guanyu;
	lv_obj_t *screen_SET_btn_devModel;
	lv_obj_t *screen_SET_label_devModel;
	lv_obj_t *screen_SET_label_devModel1;
	lv_obj_t *screen_SET_cont_guanyu_img_lineX0;
	lv_obj_t *screen_SET_btn_devNum;
	lv_obj_t *screen_SET_label_devNum;
	lv_obj_t *screen_SET_label_devNum1;
	lv_obj_t *screen_SET_cont_guanyu_img_lineX1;
	lv_obj_t *screen_SET_btn_appVersion;
	lv_obj_t *screen_SET_label_appVersion;
	lv_obj_t *screen_SET_btn_appVersion_label;
	lv_obj_t *screen_SET_cont_guanyu_img_lineX2;
	lv_obj_t *screen_SET_btn_hwVersion;
	lv_obj_t *screen_SET_label_hwVersion;
	lv_obj_t *screen_SET_label_hwVersion1;
	lv_obj_t *screen_SET_cont_guanyu_img_lineX3;
	lv_obj_t *screen_SET_btn_otaUpdata;
	lv_obj_t *screen_SET_btn_otaUpdata_label;
	lv_obj_t *screen_SET_btn_otaUpdata_img;
	lv_obj_t *screen_SET_cont_guanyu_img_lineX4;
	lv_obj_t *screen_SET_cont_setlanguage;
	lv_obj_t *screen_SET_btn_ZH;
	lv_obj_t *screen_SET_btn_ZH_label;
	lv_obj_t *screen_SET_btn_ZH_img;
	lv_obj_t *screen_SET_cont_setlanguage_img_lineX0;	
	lv_obj_t *screen_SET_btn_EN;
	lv_obj_t *screen_SET_btn_EN_label;
	lv_obj_t *screen_SET_btn_EN_img;
	lv_obj_t *screen_SET_cont_setlanguage_img_lineX1;
	lv_obj_t *screen_SET_btn_ZH_FT;
	lv_obj_t *screen_SET_btn_ZH_FT_label;	
	lv_obj_t *screen_SET_btn_ZH_FT_img;
	lv_obj_t *screen_SET_cont_setlanguage_img_lineX2;
	lv_obj_t *screen_SET_btn_FR;
	lv_obj_t *screen_SET_btn_FR_label;
	lv_obj_t *screen_SET_btn_FR_img;
	lv_obj_t *screen_SET_cont_setlanguage_img_lineX3;
	lv_obj_t *screen_SET_btn_GE;
	lv_obj_t *screen_SET_btn_GE_label;	
	lv_obj_t *screen_SET_btn_GE_img;
	lv_obj_t *screen_SET_cont_setlanguage_img_lineX4;
	lv_obj_t *screen_SET_btn_IT;
	lv_obj_t *screen_SET_btn_IT_label;	
	lv_obj_t *screen_SET_btn_IT_img;
	lv_obj_t *screen_SET_cont_setlanguage_img_lineX5;
	lv_obj_t *screen_SET_btn_JP;
	lv_obj_t *screen_SET_btn_JP_label;
	lv_obj_t *screen_SET_btn_JP_img;
	lv_obj_t *screen_SET_cont_setlanguage_img_lineX6;
	lv_obj_t *screen_SET_btn_POR;
	lv_obj_t *screen_SET_btn_POR_label;	
	lv_obj_t *screen_SET_btn_POR_img;
	lv_obj_t *screen_SET_cont_setlanguage_img_lineX7;
	lv_obj_t *screen_SET_btn_RU;
	lv_obj_t *screen_SET_btn_RU_label;
	lv_obj_t *screen_SET_btn_RU_img;
	lv_obj_t *screen_SET_cont_setlanguage_img_lineX8;
	lv_obj_t *screen_SET_btn_SP;
	lv_obj_t *screen_SET_btn_SP_label;
	lv_obj_t *screen_SET_btn_SP_img;
	lv_obj_t *screen_SET_cont_setlanguage_img_lineX9;
	lv_obj_t *screen_SET_btn_POL;
	lv_obj_t *screen_SET_btn_POL_label;
	lv_obj_t *screen_SET_btn_POL_img;
	lv_obj_t *screen_SET_cont_setlanguage_img_lineX10;
	lv_obj_t *screen_SET_btn_TU;
	lv_obj_t *screen_SET_btn_TU_label;
	lv_obj_t *screen_SET_btn_TU_img;
	lv_obj_t *screen_SET_cont_setlanguage_img_lineX11;
	lv_obj_t *screen_SET_btn_KO;
	lv_obj_t *screen_SET_btn_KO_label;
	lv_obj_t *screen_SET_btn_KO_img;		
	lv_obj_t *screen_SET_cont_setlanguage_img_lineX12;
	lv_obj_t *screen_SET_btn_DU;
	lv_obj_t *screen_SET_btn_DU_label;
	lv_obj_t *screen_SET_btn_DU_img;
	lv_obj_t *screen_SET_cont_setlanguage_img_lineX13;
	lv_obj_t *screen_SET_cont_settime;
	lv_obj_t *screen_SET_cont_settime_img_lineX0;
	lv_obj_t *screen_SET_cont_settime_img_lineX1;
	lv_obj_t *screen_SET_cont_settime_img_lineX2;
	lv_obj_t *screen_SET_cont_settime_img_lineX3;
	lv_obj_t *screen_SET_label_year;
	lv_obj_t *screen_SET_label_month;
	lv_obj_t *screen_SET_label_day;
	lv_obj_t *screen_SET_label_hour;
	lv_obj_t *screen_SET_label_minute;
	lv_obj_t *screen_SET_cont_settime_img_left;
	lv_obj_t *screen_SET_cont_settime_img_right;
	lv_obj_t *screen_SET_roller_year;
	lv_obj_t *screen_SET_roller_month;
	lv_obj_t *screen_SET_roller_day;
	lv_obj_t *screen_SET_roller_hour;
	lv_obj_t *screen_SET_roller_minute;
	lv_obj_t *screen_SET_cont_bootlogo;
	lv_obj_t *screen_SET_btn_bootlogo_path;
	lv_obj_t *screen_SET_btn_bootlogo_path_label;
	lv_obj_t *screen_SET_btn_bootlogo_path_label1;
	lv_obj_t *screen_SET_btn_bootlogo_path_img;
	lv_obj_t *screen_SET_cont_bootlogo_img_lineX0;
	lv_obj_t *screen_SET_label_bootlogo_choose;
	lv_obj_t *screen_SET_cont_externalBootlogo;
	lv_obj_t *screen_SET_btn_externalBootlogo;
	lv_obj_t *screen_SET_cont_internalBootlogo;
	lv_obj_t *screen_SET_cont_reboot;
	lv_obj_t *screen_SET_label_reboot_text;
	lv_obj_t *screen_SET_btn_reboot_confirm;
	lv_obj_t *screen_SET_btn_reboot_confirm_label;
	lv_obj_t *screen_SET_btn_reboot_cancel;
	lv_obj_t *screen_SET_btn_reboot_cancel_label;
	lv_obj_t *screen_SET_cont_resetFactory;
	lv_obj_t *screen_SET_label_resetFactory_text;
	lv_obj_t *screen_SET_btn_resetFactory_confirm;
	lv_obj_t *screen_SET_btn_resetFactory_confirm_label;
	lv_obj_t *screen_SET_btn_resetFactory_cancel;
	lv_obj_t *screen_SET_btn_resetFactory_cancel_label;
	lv_obj_t *screen_SET_cont_agingMode;
	lv_obj_t *screen_SET_btn_agingMode_memInfo;
	lv_obj_t *screen_SET_btn_agingMode_memInfo_label;
	lv_obj_t *screen_SET_btn_agingMode_memInfo_img;
	lv_obj_t *screen_SET_btn_agingMode_lineX0;
	lv_obj_t *screen_SET_btn_agingMode_netBandWidth;
	lv_obj_t *screen_SET_btn_agingMode_netBandWidth_label;
	lv_obj_t *screen_SET_btn_agingMode_netBandWidth_img;
	lv_obj_t *screen_SET_btn_agingMode_lineX1;
	lv_obj_t *screen_SET_btn_agingMode_touchPoint;
	lv_obj_t *screen_SET_btn_agingMode_touchPoint_label;
	lv_obj_t *screen_SET_btn_agingMode_touchPoint_labelSwitch;
	lv_obj_t *screen_SET_btn_agingMode_touchPoint_img;
	lv_obj_t *screen_SET_btn_agingMode_lineX2;
	lv_obj_t *screen_SET_btn_agingMode_screenSave;
	lv_obj_t *screen_SET_btn_agingMode_screenSave_label;
	lv_obj_t *screen_SET_btn_agingMode_screenSave_labelSwitch;
	lv_obj_t *screen_SET_btn_agingMode_screenSave_img;
	lv_obj_t *screen_SET_btn_agingMode_lineX3;
	lv_obj_t *screen_SET_btn_agingMode_screenSaveTime;
	lv_obj_t *screen_SET_btn_agingMode_screenSaveTime_label;
	lv_obj_t *screen_SET_btn_agingMode_screenSaveTime_labelTime;
	lv_obj_t *screen_SET_btn_agingMode_screenSaveTime_img;
	lv_obj_t *screen_SET_btn_agingMode_lineX4;
	lv_obj_t *screen_SET_btn_agingMode_scrennShot;
	lv_obj_t *screen_SET_btn_agingMode_scrennShot_label;
	lv_obj_t *screen_SET_btn_agingMode_scrennShot_labelSW;
	lv_obj_t *screen_SET_btn_agingMode_scrennShot_img;
	lv_obj_t *screen_SET_btn_agingMode_lineX5;
	lv_obj_t *screen_SET_label_popup;
	lv_obj_t *screen_Tire;
	lv_obj_t *screen_Tire_btn_return;
	lv_obj_t *screen_Tire_cont_behind;
	lv_obj_t *screen_Tire_label_bTemp;
	lv_obj_t *screen_Tire_label_bPressure;
	lv_obj_t *screen_Tire_label_bLable;
	lv_obj_t *screen_Tire_btn_bPair;
	lv_obj_t *screen_Tire_btn_bPair_label;
	lv_obj_t *screen_Tire_cont_front;
	lv_obj_t *screen_Tire_label_fLabel;
	lv_obj_t *screen_Tire_label_fTemp;
	lv_obj_t *screen_Tire_label_fPressure;
	lv_obj_t *screen_Tire_btn_fPair;
	lv_obj_t *screen_Tire_btn_fPair_label;
	lv_obj_t *screen_Tire_cont_flexLabel;
	lv_obj_t *screen_Tire_label_clearPair;
	lv_obj_t *screen_Tire_label_paraReset;
	lv_obj_t *screen_Tire_label_tempUnit;
	lv_obj_t *screen_Tire_label_pressureUnit;
	lv_obj_t *screen_Tire_label_pressureMin;
	lv_obj_t *screen_Tire_label_pressureMax;
	lv_obj_t *screen_Tire_label_tempMax;
	lv_obj_t *screen_Tire_cont_flexBtn;
	lv_obj_t *screen_Tire_cont_pressureMin;
	lv_obj_t *screen_Tire_btn_lPressMax;
	lv_obj_t *screen_Tire_btn_rPressMax;
	lv_obj_t *screen_Tire_label_pressMax1;	
	lv_obj_t *screen_Tire_btn_rPressMin;
	lv_obj_t *screen_Tire_label_pressMin1;
	lv_obj_t *screen_Tire_cont_pressureMax;
	lv_obj_t *screen_Tire_btn_lPressMin;
	lv_obj_t *screen_Tire_cont_tempMax;
	lv_obj_t *screen_Tire_btn_lTemp;
	lv_obj_t *screen_Tire_btn_rTemp;
	lv_obj_t *screen_Tire_label_tempMax1;
	lv_obj_t *screen_Tire_cont_pressBtn;
	lv_obj_t *screen_Tire_btn_pressBar;
	lv_obj_t *screen_Tire_btn_pressBar_label;
	lv_obj_t *screen_Tire_img_pressBar;
	lv_obj_t *screen_Tire_btn_pressPsj;
	lv_obj_t *screen_Tire_btn_pressPsj_label;
	lv_obj_t *screen_Tire_img_pressPsj;
	lv_obj_t *screen_Tire_cont_tempBtn;
	lv_obj_t *screen_Tire_btn_tempC;
	lv_obj_t *screen_Tire_btn_tempC_label;
	lv_obj_t *screen_Tire_img_tempC;
	lv_obj_t *screen_Tire_btn_tempF;
	lv_obj_t *screen_Tire_btn_tempF_label;
	lv_obj_t *screen_Tire_img_tempF;
	lv_obj_t *screen_Tire_btn_paraReset;
	lv_obj_t *screen_Tire_btn_paraReset_label;
	lv_obj_t *screen_Tire_btn_clearPair;
	lv_obj_t *screen_Tire_btn_clearPair_label;
	bool screen_Tire_del;
	lv_obj_t *screen_carPlay;
	lv_obj_t *screen_carPlay_btn_return;
	lv_obj_t *screen_carPlay_btn_return_label;
	lv_obj_t *screen_carPlay_cont_prompt;
	lv_obj_t *screen_carPlay_label1_prompt;
	lv_obj_t *screen_carPlay_label2_prompt;
	lv_obj_t *screen_carPlay_label3_prompt;
	lv_obj_t *screen_carPlay_label4_prompt;
	lv_obj_t *screen_carPlay_label5_prompt;
	lv_obj_t *screen_carPlay_label6_prompt;
	bool screen_carPlay_del;
	lv_obj_t *screen_androidAuto;
	lv_obj_t *screen_androidAuto_btn_return;
	lv_obj_t *screen_androidAuto_btn_return_label;
	lv_obj_t *screen_androidAuto_cont_prompt;
	lv_obj_t *screen_androidAuto_label1_prompt;
	lv_obj_t *screen_androidAuto_label2_prompt;
	lv_obj_t *screen_androidAuto_label3_prompt;
	lv_obj_t *screen_androidAuto_label4_prompt;
	lv_obj_t *screen_androidAuto_label5_prompt;
	lv_obj_t *screen_androidAuto_label6_prompt;
	bool screen_androidAuto_del;
	lv_obj_t *screen_DVR_SET;
	bool screen_DVR_SET_del;
	lv_obj_t *screen_DVR_SET_btn_return;
	lv_obj_t *screen_DVR_SET_cont_parent;
	lv_obj_t *screen_DVR_SET_cont_rearCameraImage;
	lv_obj_t *screen_DVR_SET_label_rearCameraImage;
	lv_obj_t *screen_DVR_SET_btn_rearCameraImage;
	lv_obj_t *screen_DVR_SET_cont_recorderTime;
	lv_obj_t *screen_DVR_SET_label_recorderTime;
	lv_obj_t *screen_DVR_SET_btn_recorder_5min;
	lv_obj_t *screen_DVR_SET_btn_recorder_5min_label;
	lv_obj_t *screen_DVR_SET_btn_recorder_1min;
	lv_obj_t *screen_DVR_SET_btn_recorder_1min_label;
	lv_obj_t *screen_DVR_SET_btn_recorder_3min;
	lv_obj_t *screen_DVR_SET_btn_recorder_3min_label;
	lv_obj_t *screen_DVR_SET_cont_recorderUrgentTime;
	lv_obj_t *screen_DVR_SET_label_recorderUrgentTime;
	lv_obj_t *screen_DVR_SET_btn_urgent_5min;
	lv_obj_t *screen_DVR_SET_btn_urgent_5min_label;
	lv_obj_t *screen_DVR_SET_btn_urgent_1min;
	lv_obj_t *screen_DVR_SET_btn_urgent_1min_label;
	lv_obj_t *screen_DVR_SET_btn_urgent_3min;
	lv_obj_t *screen_DVR_SET_btn_urgent_3min_label;
	lv_obj_t *screen_DVR_SET_cont_powerOnRecording;
	lv_obj_t *screen_DVR_SET_label_powerOnRecording;
	lv_obj_t *screen_DVR_SET_btn_powerOnRecording;
	lv_obj_t *screen_DVR_SET_cont_about;
	lv_obj_t *screen_DVR_SET_label_about;
	lv_obj_t *screen_DVR_SET_label_version;
	lv_obj_t *screen_DVR_SET_cont_recorderSoundSet;
	lv_obj_t *screen_DVR_SET_label_recorderSoundSet;
	lv_obj_t *screen_DVR_SET_btn_recorderSoundSet;
	lv_obj_t *screen_DVR_SET_label_topLabel;
	lv_obj_t *screen_DVR_SET_cont_recorderTimeMark;
	lv_obj_t *screen_DVR_SET_label_recorderTimeMark;
	lv_obj_t *screen_DVR_SET_btn_recorderTimeMark;
	lv_obj_t *screen_DVR_SET_cont_carSpeedMark;
	lv_obj_t *screen_DVR_SET_label_carSpeedMark;
	lv_obj_t *screen_DVR_SET_btn_carSpeedMark;
	lv_obj_t *screen_DVR_SET_cont_carSpeedUnit;
	lv_obj_t *screen_DVR_SET_label_carSpeedUnit;
	lv_obj_t *screen_DVR_SET_btn_carSpeedUnit;	
	lv_obj_t *screen_messageBox;
	bool screen_messageBox_del;
	lv_obj_t *screen_messageBox_label_popup;
}lv_ui;

lv_color_t *buf_screen_carplay_canvas_1;

typedef void (*ui_setup_scr_t)(lv_ui * ui);

void ui_init_style(lv_style_t * style);

void ui_load_scr_animation(lv_ui *ui, lv_obj_t ** new_scr, bool new_scr_del, bool * old_scr_del, ui_setup_scr_t setup_scr,
                           lv_scr_load_anim_t anim_type, uint32_t time, uint32_t delay, bool is_clean, bool auto_del);

void ui_animation(void * var, int32_t duration, int32_t delay, int32_t start_value, int32_t end_value, lv_anim_path_cb_t path_cb,
                       uint16_t repeat_cnt, uint32_t repeat_delay, uint32_t playback_time, uint32_t playback_delay,
                       lv_anim_exec_xcb_t exec_cb, lv_anim_start_cb_t start_cb, lv_anim_ready_cb_t ready_cb, lv_anim_deleted_cb_t deleted_cb);


void init_scr_del_flag(lv_ui *ui);

void setup_ui(lv_ui *ui);


extern lv_ui guider_ui;

void create_analog_clock(void);
void setup_scr_screen(lv_ui *ui);
void setup_scr_screen_DVR(lv_ui *ui);
void setup_scr_screen_SET(lv_ui *ui);
void setup_scr_screen_Tire(lv_ui *ui);
void setup_scr_screen_carPlay(lv_ui *ui);
void setup_scr_screen_androidAuto(lv_ui *ui);
void setup_scr_screen_DVR_SET(lv_ui *ui);
void setup_scr_media_library(lv_ui *ui);
void cleanup_media_library_ui(lv_ui *ui);
void setup_scr_ota(lv_ui *ui);
void cleanup_ota_ui(lv_ui *ui);
void setup_scr_screen_messageBox(lv_ui *ui);

void show_popup_simple_v8(const char* message);
void close_popup_v8(void);
bool is_popup_visible_v8(void);

void ScreenTireBlackNightMode(lv_ui *ui, THEME_MODE mode);
void ScreenSetBlackNightMode(lv_ui *ui, THEME_MODE mode);
void screenDvrSetBlackNightMode(lv_ui *ui, THEME_MODE mode);

LV_IMG_DECLARE(_not_select_dark_40x40);
LV_IMG_DECLARE(_sel_checkbox_off_night_60_60x60);
LV_IMG_DECLARE(_selected_dark_40x40);
LV_IMG_DECLARE(_linx_dark_alpha_1200x5);
LV_IMG_DECLARE(_lineY_dark_5x720);
LV_IMG_DECLARE(_hour_hand_alpha_325x325);
LV_IMG_DECLARE(_min_hand_alpha_325x325);
LV_IMG_DECLARE(_sec_hand_alpha_325x325);
LV_IMG_DECLARE(_dial_alpha_476x476);
LV_IMG_DECLARE(_back_60x60);
LV_IMG_DECLARE(_left_but_normal_night_60x75);
LV_IMG_DECLARE(_ligh_big_30x30);
LV_IMG_DECLARE(_light_30x30);
LV_IMG_DECLARE(_right_but_normal_night_60x75);
LV_IMG_DECLARE(_sun_day_alpha_26x26);
LV_IMG_DECLARE(_rec_sta_40x40);
LV_IMG_DECLARE(_rec_sta_60x60);
LV_IMG_DECLARE(_CarPlay_sta_70x70);
LV_IMG_DECLARE(_tf_sta_70x70);
LV_IMG_DECLARE(_wifi_sta_72x72);
LV_IMG_DECLARE(_AUTO_sta_70x70);
LV_IMG_DECLARE(_CarPlay_sta_70x70);
LV_IMG_DECLARE(_bt_sta_60x60);
LV_IMG_DECLARE(_gps_sta_70x70);
LV_IMG_DECLARE(_menu_btn_198x100);
LV_IMG_DECLARE(_menu_btn_dn_198x100);
LV_IMG_DECLARE(_preview_btn_198x100);
LV_IMG_DECLARE(_preview_btn_dn_198x100);
LV_IMG_DECLARE(_recorder_btn_198x100);
LV_IMG_DECLARE(_recorder_btn_dn_198x100);
LV_IMG_DECLARE(_set_btn_198x100);
LV_IMG_DECLARE(_set_btn_dn_198x100);
LV_IMG_DECLARE(_takepic_btn_198x100);
LV_IMG_DECLARE(_takepic_btn_dn_198x100);
LV_IMG_DECLARE(_dvr_btn_bg_1280x100);
LV_IMG_DECLARE(_lock_btn_198x100);
LV_IMG_DECLARE(_lock_btn_dn_198x100);
LV_IMG_DECLARE(_selected_40x40);
LV_IMG_DECLARE(_not_select_40x40);
LV_IMG_DECLARE(_sel_checkbox_off_day_60_60x60);
LV_IMG_DECLARE(_sel_checkbox_on_60_60x60);
LV_IMG_DECLARE(_left_but_normal_day_60x75);
LV_IMG_DECLARE(_left_but_down_60x75);
LV_IMG_DECLARE(_right_but_normal_day_60x75);
LV_IMG_DECLARE(_right_but_down_60x75);
LV_IMG_DECLARE(_lineX_alpha_1200x5);
LV_IMG_DECLARE(_lineY_alpha_5x720);
LV_IMG_DECLARE(_unlock_select_80x80);
LV_IMG_DECLARE(_lock_select_80x80);
LV_IMG_DECLARE(_back_80x80);
LV_IMG_DECLARE(_back_dark_80x80);
LV_IMG_DECLARE(_back_select_80x80);
LV_IMG_DECLARE(_sw_off_135x61);
LV_IMG_DECLARE(_sw_on_135x61);
LV_IMG_DECLARE(_openTirp_618x304);
LV_IMG_DECLARE(_openTirp_dn_618x304);
LV_IMG_DECLARE(_hm_tire_618x304);
LV_IMG_DECLARE(_hm_set_300x264);
LV_IMG_DECLARE(_hm_set_dn_300x264);
LV_IMG_DECLARE(_hm_andro_dn_360x586);
LV_IMG_DECLARE(_hm_android_360x586);
LV_IMG_DECLARE(_hm_dvr_dn_300x264);
LV_IMG_DECLARE(_hm_dvr_300x264);
LV_IMG_DECLARE(_hm_carplay_360x586);
LV_IMG_DECLARE(_hm_carpaly_dn_360x586);
LV_IMG_DECLARE(_rec_off_alpha_50x50);



LV_FONT_DECLARE(lv_font_montserrat_12)
LV_FONT_DECLARE(lv_font_montserrat_16)
LV_FONT_DECLARE(lv_font_montserrat_25)
LV_FONT_DECLARE(lv_font_montserrat_30)
LV_FONT_DECLARE(lv_font_montserrat_35)

LV_FONT_DECLARE(lv_font_harmonyOS_42)
LV_FONT_DECLARE(lv_font_harmonyOS_40)
LV_FONT_DECLARE(lv_font_harmonyOS_45)
LV_FONT_DECLARE(lv_font_harmonyOS_35)
LV_FONT_DECLARE(lv_font_harmonyOS_30)
#ifdef __cplusplus
}
#endif
#endif
