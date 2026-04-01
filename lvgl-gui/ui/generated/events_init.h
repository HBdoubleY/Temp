/*
* Copyright 2025 NXP
* NXP Confidential and Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/


#ifndef EVENTS_INIT_H_
#define EVENTS_INIT_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"
#include "common.h"
#include "linksupport.h"
// #include "carplay_service.h"

void events_init(lv_ui *ui);
void events_init_screen(lv_ui *ui);
void events_init_screen_DVR(lv_ui *ui);
void events_init_screen_SET(lv_ui *ui);
void events_init_screen_Tire(lv_ui *ui);
void events_init_screen_carPlay(lv_ui *ui);
void event_init_screen_androidAuto(lv_ui *ui);
void events_init_screen_DVR_SET(lv_ui *ui);
#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */
