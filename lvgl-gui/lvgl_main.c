#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include "lv_drivers/display/sunxifb.h"
#include "lv_drivers/indev/evdev.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lv_drv_conf.h"
#include "lvgl_main.h"
#include <pthread.h>
#include "ui/generated/gui_guider.h"
#include "ui/generated/events_init.h"
#include "ui/generated/OTA/WIFIConnect.h"
#include "lvgl_system.h"
#include "common.h"
#include "storageDataApi.h"
#ifdef ENABLE_CARPLAY
#include "carplay_display.h"
#include "link_touch_evdev.h"
#include "zlink_client.h"
#endif
#include "myTimer.h"
#include "uart.h"
#include "mppFileManager.h"
#include "bootlogoUp.h"
#include "bt_serial.h"
#include "tire_manager.h"
#include "mpp_camera.h"

static pthread_t threadID;
lv_ui guider_ui;
static int screanWidth = 1440;
static int screanHeight = 720;
static int i2c0_fd = -1;
static int i2c1_fd = -1;

static bool stop_lvgl = false;
/* 切到 CarPlay 全透明界面后若立刻停 lv_task_handler，首帧 flush 可能不发生，底层 VO 会被旧 UI 盖住 */
static int lvgl_post_projection_flush_frames = 0;

lv_timer_t *DVRstaTimer = NULL;


extern sys_data g_sys_Data;
extern lv_timer_t *autoModeTimer;
extern void creatAutoModeTimerCbk(lv_timer_t *timer);
extern void recorder_status_timer(lv_timer_t *timer);

void tire_ui_refresh_now(void);

void PrintTime() {
    time_t t;
    struct tm* tm;
    char buf[64];
    
    time(&t);
    tm = localtime(&t);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm);
    printf("%s\n", buf);
}

#if 1

static bool carplay_connected = false;
static bool androidauto_connected = false;
static unsigned int g_cp_perf_session_seq = 0;
static pthread_t recorder_worker_tid;
static pthread_mutex_t recorder_worker_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t recorder_worker_cond = PTHREAD_COND_INITIALIZER;
static int recorder_worker_req = -1; /* -1: none, 0: stop, 1: start */
static bool recorder_worker_backend_running = false;

static void recorder_worker_request(int req)
{
    pthread_mutex_lock(&recorder_worker_mutex);
    recorder_worker_req = req;
    pthread_cond_signal(&recorder_worker_cond);
    pthread_mutex_unlock(&recorder_worker_mutex);
}

void recorder_request_start_async(void)
{
    recorder_worker_request(1);
}

void recorder_request_stop_async(void)
{
    recorder_worker_request(0);
}

static void *recorder_worker_thread(void *arg)
{
    (void)arg;
    while (1) {
        int req = -1;
        pthread_mutex_lock(&recorder_worker_mutex);
        while (recorder_worker_req < 0) {
            pthread_cond_wait(&recorder_worker_cond, &recorder_worker_mutex);
        }
        req = recorder_worker_req;
        recorder_worker_req = -1;
        pthread_mutex_unlock(&recorder_worker_mutex);

        if (req == 1) {
            if (recorder_worker_backend_running) {
                continue;
            }
            if (g_sys_Data.recorderMode == RECORDER_URGENT) {
                deletFileInRecorderPath(U_REC_PATH);
            } else {
                deletFileInRecorderPath(REC_PATH);
            }
            printf("[recorder_worker] start recording...\n");
            int r0 = recording(&g_sys_Data.vipp0_config);
            int r1 = recording(&g_sys_Data.vipp8_config);
            if (r0 == 0 && r1 == 0) {
                dashTimeMark(&g_sys_Data.vipp0_config, g_sys_Data.TimeMark);
                dashTimeMark(&g_sys_Data.vipp8_config, g_sys_Data.TimeMark);
                SoundRecording(&g_sys_Data.vipp0_config, g_sys_Data.SoundRecorder);
                SoundRecording(&g_sys_Data.vipp8_config, g_sys_Data.SoundRecorder);
                recorder_worker_backend_running = true;
            } else {
                printf("[recorder_worker] start failed: r0=%d, r1=%d\n", r0, r1);
                stopRecording(&g_sys_Data.vipp0_config);
                stopRecording(&g_sys_Data.vipp8_config);
                g_sys_Data.recorderMode = RECORDER_NONE;
                recorder_worker_backend_running = false;
            }
        } else if (req == 0) {
            if (!recorder_worker_backend_running) {
                continue;
            }
            stopRecording(&g_sys_Data.vipp0_config);
            stopRecording(&g_sys_Data.vipp8_config);
            recorder_worker_backend_running = false;
        }
    }
    return NULL;
}

static void bt_status_check_timer(lv_timer_t *timer) { 
    lv_obj_t* current_screen = lv_scr_act();
    static bool recorderFlag = false;
    static int last_bt_connected = -1;
    static char last_bt_label[64] = {0};
    bool frontCamera = tp2804_check_camera_connected(i2c0_fd);
    bool rearCamera = tp2804_check_camera_connected(i2c1_fd);
    if (mpp_camera_take_storage_fault()) {
        if (g_sys_Data.recorderMode == RECORDER_NORMAL || g_sys_Data.recorderMode == RECORDER_URGENT) {
            printf("[lvgl] storage fault detected, force stop recording.\n");
            recorder_request_stop_async();
            g_sys_Data.recorderMode = RECORDER_NONE;
            if (DVRstaTimer != NULL) {
                lv_timer_del(DVRstaTimer);
                DVRstaTimer = NULL;
            }
            clearRecorderStatu();
            if (is_popup_visible_v8) close_popup_v8();
            show_popup_simple_v8(get_string_for_language(g_sys_Data.current_language, "main_txt_TFNotFreeMem"));
        }
    }

    if (current_screen == guider_ui.screen && lv_obj_is_valid(guider_ui.screen_img_wifi)) {
        bool want_show = WIFIConnect_is_connected_cached();
        bool is_hidden = lv_obj_has_flag(guider_ui.screen_img_wifi, LV_OBJ_FLAG_HIDDEN);
        if (want_show && is_hidden) {
            lv_obj_clear_flag(guider_ui.screen_img_wifi, LV_OBJ_FLAG_HIDDEN);
        } else if (!want_show && !is_hidden) {
            lv_obj_add_flag(guider_ui.screen_img_wifi, LV_OBJ_FLAG_HIDDEN);
        }
    }

    int bt_connected = get_BT_connect_state();
    if (current_screen == guider_ui.screen && lv_obj_is_valid(guider_ui.screen_img_bt)) {
        bool want_show = (bt_connected != 0);
        bool is_hidden = lv_obj_has_flag(guider_ui.screen_img_bt, LV_OBJ_FLAG_HIDDEN);
        if (want_show && is_hidden) {
            lv_obj_clear_flag(guider_ui.screen_img_bt, LV_OBJ_FLAG_HIDDEN);
        } else if (!want_show && !is_hidden) {
            lv_obj_add_flag(guider_ui.screen_img_bt, LV_OBJ_FLAG_HIDDEN);
        }
    }

    tire_ui_refresh_now();

    if (current_screen == guider_ui.screen_SET &&
        lv_obj_is_valid(guider_ui.screen_SET_label_connect)) {

        const char *target_text = NULL;
        if (bt_connected) {
            const char *name = get_BT_connected_name();
            if (name && name[0] != '\0') {
                target_text = name;
            } else {
                target_text = get_string_for_language(g_sys_Data.current_language,
                                                      "main_txt_nConnect");
            }
        } else {
            target_text = get_string_for_language(g_sys_Data.current_language,
                                                  "main_txt_nConnect");
        }

        if (!target_text) {
            target_text = "";
        }

        if (bt_connected != last_bt_connected ||
            strncmp(last_bt_label, target_text, sizeof(last_bt_label) - 1) != 0) {
            lv_label_set_text(guider_ui.screen_SET_label_connect, target_text);
            strncpy(last_bt_label, target_text, sizeof(last_bt_label) - 1);
            last_bt_label[sizeof(last_bt_label) - 1] = '\0';
            last_bt_connected = bt_connected;
        }
    }
#ifdef ENABLE_CARPLAY
    if (current_screen == guider_ui.screen && lv_obj_is_valid(guider_ui.screen_img_carPLay)) {
        bool is_hidden = lv_obj_has_flag(guider_ui.screen_img_carPLay, LV_OBJ_FLAG_HIDDEN);
        if (carplay_connected && is_hidden) {
            lv_obj_clear_flag(guider_ui.screen_img_carPLay, LV_OBJ_FLAG_HIDDEN);
        } else if (!carplay_connected && !is_hidden) {
            lv_obj_add_flag(guider_ui.screen_img_carPLay, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (current_screen == guider_ui.screen && lv_obj_is_valid(guider_ui.screen_img_androiAuto)) {
        bool is_hidden = lv_obj_has_flag(guider_ui.screen_img_androiAuto, LV_OBJ_FLAG_HIDDEN);
        if (androidauto_connected && is_hidden) {
            lv_obj_clear_flag(guider_ui.screen_img_androiAuto, LV_OBJ_FLAG_HIDDEN);
        } else if (!androidauto_connected && !is_hidden) {
            lv_obj_add_flag(guider_ui.screen_img_androiAuto, LV_OBJ_FLAG_HIDDEN);
        }
    }
#endif

    if(g_sys_Data.frontCamera != frontCamera){
        g_sys_Data.frontCamera = frontCamera;
        if(!g_sys_Data.frontCamera){
            if(is_popup_visible_v8) close_popup_v8();
            show_popup_simple_v8(get_string_for_language(g_sys_Data.current_language,"main_txt_FrontCameraMove"));
        }
    }
    if(g_sys_Data.rearCamera != rearCamera){
        g_sys_Data.rearCamera = rearCamera;
        if(!g_sys_Data.rearCamera){
            if(is_popup_visible_v8) close_popup_v8();
            show_popup_simple_v8(get_string_for_language(g_sys_Data.current_language,"main_txt_RearCameraMove"));
        }
    }

    if(g_sys_Data.TFmounted != checkTFCardMountProc()){
        g_sys_Data.TFmounted = checkTFCardMountProc();
        if(g_sys_Data.TFmounted){
            // if(is_popup_visible_v8) close_popup_v8();
            // show_popup_simple_v8(get_string_for_language(g_sys_Data.current_language,"dvr_txt_mountTF"));
        }else{
            if(is_popup_visible_v8) close_popup_v8();
            show_popup_simple_v8(get_string_for_language(g_sys_Data.current_language,"dvr_txt_moveTF"));
        }
        
        recorderFlag = true;
        if(current_screen == guider_ui.screen && lv_obj_is_valid(guider_ui.screen_img_TF)){
            g_sys_Data.TFmounted ? lv_obj_clear_flag(guider_ui.screen_img_TF, LV_OBJ_FLAG_HIDDEN) : lv_obj_add_flag(guider_ui.screen_img_TF, LV_OBJ_FLAG_HIDDEN);
        }
    }   

    if(!g_sys_Data.TFmounted){
        recorderFlag = true;
        if(g_sys_Data.recorderMode == RECORDER_NORMAL || g_sys_Data.recorderMode == RECORDER_URGENT){

            if(current_screen == guider_ui.screen_DVR && lv_obj_is_valid(guider_ui.screen_DVR_btn_recorder)){
                lv_obj_clear_state(guider_ui.screen_DVR_btn_recorder, LV_STATE_CHECKED);
                show_label_with_timer(guider_ui.screen_DVR_label_Popup, "dvr_txt_stopRecorder", 1000);
            }
            #if 1
            recorder_request_stop_async();
            #endif
            g_sys_Data.recorderMode = RECORDER_NONE;
            if(DVRstaTimer != NULL){
                lv_timer_del(DVRstaTimer);
                DVRstaTimer = NULL;	
            }
		
            clearRecorderStatu();
        } 

    }else{
        if(g_sys_Data.frontCamera || g_sys_Data.rearCamera) {
            if(recorderFlag){
                recorderFlag = false;
                if(g_sys_Data.powerOnRecorder && g_sys_Data.recorderMode == RECORDER_NONE){
                    g_sys_Data.recorderMode = RECORDER_NORMAL;

                    if(lv_obj_is_valid(guider_ui.screen_DVR_btn_recorder)){
                        lv_obj_add_state(guider_ui.screen_DVR_btn_recorder, LV_STATE_CHECKED);
                        show_label_with_timer(guider_ui.screen_DVR_label_Popup, "dvr_txt_startRecorder", 1000);
                    }
                    #if 1
                    if(!TFFreeMemDetection() && (queue_mpp_size(F_videoFile) < 10) && (queue_mpp_size(R_videoFile) < 10)) {
                        if(is_popup_visible_v8) close_popup_v8();
                        show_popup_simple_v8(get_string_for_language(g_sys_Data.current_language,"main_txt_TFNotFreeMem"));
                        return;
                    }
                    recorder_request_start_async();
                    #endif
                    if(DVRstaTimer == NULL) DVRstaTimer = lv_timer_create(recorder_status_timer, 1000, NULL);    
                }
            }       
        }else{
            recorderFlag = true;
            if(g_sys_Data.recorderMode == RECORDER_NORMAL || g_sys_Data.recorderMode == RECORDER_URGENT){

                if(current_screen == guider_ui.screen_DVR && lv_obj_is_valid(guider_ui.screen_DVR_btn_recorder)){
                    lv_obj_clear_state(guider_ui.screen_DVR_btn_recorder, LV_STATE_CHECKED);
                    show_label_with_timer(guider_ui.screen_DVR_label_Popup, "dvr_txt_stopRecorder", 1000);

                }
                #if 1
                recorder_request_stop_async();
                #endif
                g_sys_Data.recorderMode = RECORDER_NONE;
                if(DVRstaTimer != NULL){
                    lv_timer_del(DVRstaTimer);
                    DVRstaTimer = NULL;	
                }			
                clearRecorderStatu();
            }
            // if(current_screen == guider_ui.screen_DVR && lv_obj_is_valid(guider_ui.screen_DVR_img_rec)){
            //     ui_load_scr_animation(&guider_ui, &guider_ui.screen, guider_ui.screen_del, &guider_ui.screen_DVR_del, setup_scr_screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, true, true);
            //     stopPreview(&g_sys_Data.vipp0_config);
            //     stopPreview(&g_sys_Data.vipp8_config);		
            //     g_sys_Data.previewMode = PREVIEW_NONE; 
            // }             
        }
    }

    if(g_sys_Data.recorderMode == RECORDER_NONE){
        if(g_sys_Data.frontCamera || g_sys_Data.rearCamera){
            if(lv_obj_is_valid(guider_ui.screen_img_rec)){
                if(lv_obj_has_flag(guider_ui.screen_img_rec, LV_OBJ_FLAG_HIDDEN)) lv_obj_clear_flag(guider_ui.screen_img_rec, LV_OBJ_FLAG_HIDDEN);

            }
        }else{
            if(lv_obj_is_valid(guider_ui.screen_img_rec)){
                if(!lv_obj_has_flag(guider_ui.screen_img_rec, LV_OBJ_FLAG_HIDDEN)) lv_obj_add_flag(guider_ui.screen_img_rec, LV_OBJ_FLAG_HIDDEN);           
            }

        }
    }

    //home 显示dvr录像状态
    if(lv_obj_is_valid(guider_ui.screen_btn_DVR_label_status)){
        if(g_sys_Data.recorderMode == RECORDER_NORMAL || g_sys_Data.recorderMode == RECORDER_URGENT){
            if(lv_obj_has_flag(guider_ui.screen_btn_DVR_label_status, LV_OBJ_FLAG_HIDDEN)) lv_obj_clear_flag(guider_ui.screen_btn_DVR_label_status, LV_OBJ_FLAG_HIDDEN);
        }else{
            if(!lv_obj_has_flag(guider_ui.screen_btn_DVR_label_status, LV_OBJ_FLAG_HIDDEN)) lv_obj_add_flag(guider_ui.screen_btn_DVR_label_status, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void tire_ui_refresh_now(void) {
    int f_kpa = 0, f_c = 0;
    int b_kpa = 0, b_c = 0;
    bool f_ok = tire_front_get_kpa_temp(&f_kpa, &f_c);
    bool b_ok = tire_rear_get_kpa_temp(&b_kpa, &b_c);

    // 1 bar = 14.5037738 psi
    const float PSI_PER_BAR = 14.5037738f;

    const bool pressure_unit = g_sys_Data.pressureUnit;
    const bool temp_unit = g_sys_Data.tempUnit;
    const language_t tire_lang = g_sys_Data.current_language;

    if (lv_obj_is_valid(guider_ui.screen_Tire_btn_bPair_label)) {
        char suffix6[7] = {0};
        if (tire_rear_get_suffix6(suffix6)) {
            lv_label_set_text(guider_ui.screen_Tire_btn_bPair_label, suffix6);
        } else {
            lv_label_set_text(guider_ui.screen_Tire_btn_bPair_label,
                               get_string_for_language(tire_lang, "tire_txt_nPair"));
        }
        lv_obj_invalidate(guider_ui.screen_Tire_btn_bPair_label);
    }

    if (lv_obj_is_valid(guider_ui.screen_Tire_btn_fPair_label)) {
        char suffix6[7] = {0};
        if (tire_front_get_suffix6(suffix6)) {
            lv_label_set_text(guider_ui.screen_Tire_btn_fPair_label, suffix6);
        } else {
            lv_label_set_text(guider_ui.screen_Tire_btn_fPair_label,
                               get_string_for_language(tire_lang, "tire_txt_nPair"));
        }
        lv_obj_invalidate(guider_ui.screen_Tire_btn_fPair_label);
    }

    if (f_ok) {
        if (!pressure_unit) {
            g_sys_Data.fPressure = (float)f_kpa / 100.0f;
        } else {
            g_sys_Data.fPressure = ((float)f_kpa / 100.0f) * PSI_PER_BAR;
        }

        if (!temp_unit) {
            g_sys_Data.fTemp = f_c;
        } else {
            g_sys_Data.fTemp = (int)(((float)f_c * 9.0f / 5.0f) + 32.0f);
        }

        char buf[32] = {0};
        if (!pressure_unit) {
            snprintf(buf, sizeof(buf), "%.1f Bar", g_sys_Data.fPressure);
        } else {
            snprintf(buf, sizeof(buf), "%.0f Psi", g_sys_Data.fPressure);
        }
        if (lv_obj_is_valid(guider_ui.screen_Tire_label_fPressure)) {
            lv_label_set_text(guider_ui.screen_Tire_label_fPressure, buf);
            lv_obj_invalidate(guider_ui.screen_Tire_label_fPressure);
        }

        if (!temp_unit) {
            snprintf(buf, sizeof(buf), "%d ℃", g_sys_Data.fTemp);
        } else {
            snprintf(buf, sizeof(buf), "%d ℉", g_sys_Data.fTemp);
        }
        if (lv_obj_is_valid(guider_ui.screen_Tire_label_fTemp)) {
            lv_label_set_text(guider_ui.screen_Tire_label_fTemp, buf);
            lv_obj_invalidate(guider_ui.screen_Tire_label_fTemp);
        }
    } else {
        if (lv_obj_is_valid(guider_ui.screen_Tire_label_fPressure)) {
            lv_label_set_text(guider_ui.screen_Tire_label_fPressure, !pressure_unit ? "--Bar" : "--Psi");
            lv_obj_invalidate(guider_ui.screen_Tire_label_fPressure);
        }
        if (lv_obj_is_valid(guider_ui.screen_Tire_label_fTemp)) {
            lv_label_set_text(guider_ui.screen_Tire_label_fTemp, !temp_unit ? "--℃" : "--℉");
            lv_obj_invalidate(guider_ui.screen_Tire_label_fTemp);
        }
    }

    if (b_ok) {
        if (!pressure_unit) {
            g_sys_Data.bPressure = (float)b_kpa / 100.0f;
        } else {
            g_sys_Data.bPressure = ((float)b_kpa / 100.0f) * PSI_PER_BAR;
        }

        if (!temp_unit) {
            g_sys_Data.bTemp = b_c;
        } else {
            g_sys_Data.bTemp = (int)(((float)b_c * 9.0f / 5.0f) + 32.0f);
        }

        char buf[32] = {0};
        if (!pressure_unit) {
            snprintf(buf, sizeof(buf), "%.1f Bar", g_sys_Data.bPressure);
        } else {
            snprintf(buf, sizeof(buf), "%.0f Psi", g_sys_Data.bPressure);
        }
        if (lv_obj_is_valid(guider_ui.screen_Tire_label_bPressure)) {
            lv_label_set_text(guider_ui.screen_Tire_label_bPressure, buf);
            lv_obj_invalidate(guider_ui.screen_Tire_label_bPressure);
        }

        if (!temp_unit) {
            snprintf(buf, sizeof(buf), "%d ℃", g_sys_Data.bTemp);
        } else {
            snprintf(buf, sizeof(buf), "%d ℉", g_sys_Data.bTemp);
        }
        if (lv_obj_is_valid(guider_ui.screen_Tire_label_bTemp)) {
            lv_label_set_text(guider_ui.screen_Tire_label_bTemp, buf);
            lv_obj_invalidate(guider_ui.screen_Tire_label_bTemp);
        }
    } else {
        if (lv_obj_is_valid(guider_ui.screen_Tire_label_bPressure)) {
            lv_label_set_text(guider_ui.screen_Tire_label_bPressure, !pressure_unit ? "--Bar" : "--Psi");
            lv_obj_invalidate(guider_ui.screen_Tire_label_bPressure);
        }
        if (lv_obj_is_valid(guider_ui.screen_Tire_label_bTemp)) {
            lv_label_set_text(guider_ui.screen_Tire_label_bTemp, !temp_unit ? "--℃" : "--℉");
            lv_obj_invalidate(guider_ui.screen_Tire_label_bTemp);
        }
    }

    if (lv_obj_is_valid(guider_ui.screen_btn_cartrip_label_fTyre_data)) {
        char buf[40] = {0};
        if (f_ok) {
            snprintf(buf, sizeof(buf), "%dkpa %d℃", f_kpa, f_c);
        } else {
            snprintf(buf, sizeof(buf), "---kpa --℃");
        }
        lv_label_set_text(guider_ui.screen_btn_cartrip_label_fTyre_data, buf);
        lv_obj_invalidate(guider_ui.screen_btn_cartrip_label_fTyre_data);
    }

    if (lv_obj_is_valid(guider_ui.screen_btn_cartrip_label_bTyre_data)) {
        char buf[40] = {0};
        if (b_ok) {
            snprintf(buf, sizeof(buf), "%dkpa %d℃", b_kpa, b_c);
        } else {
            snprintf(buf, sizeof(buf), "---kpa --℃");
        }
        lv_label_set_text(guider_ui.screen_btn_cartrip_label_bTyre_data, buf);
        lv_obj_invalidate(guider_ui.screen_btn_cartrip_label_bTyre_data);
    }
}


static void tire_ui_refresh_timer_cb(lv_timer_t *timer) {
    (void)timer;
    tire_ui_refresh_now();
}
#endif

#ifdef ENABLE_CARPLAY
extern void createDashAnalogTimer(int delay_ms);
extern void destoryDashAnalogTimer(void);

void link_ui_on_projection_entered(void)
{
	destoryDashAnalogTimer();
	carplay_link_touch_set_active(1);
}

void link_ui_on_projection_exited(void)
{
	carplay_link_touch_set_active(0);
	if (g_sys_Data.agingMode.screenSaveSw)
		createDashAnalogTimer(g_sys_Data.agingMode.screenSaveTime * 1000);
}
#endif

#ifdef ENABLE_CARPLAY
int request_link_touchevent(LinkType type, bool isPressed, int x, int y)
{
	if (type == LINK_TYPE_CARPLAY || type == LINK_TYPE_ANDROIDAUTO)
		carplay_touch_send_xy(x, y, isPressed ? 1 : 0);
	return 0;
}
#else
int request_link_touchevent(LinkType type, bool isPressed, int x, int y)
{
	(void)type;
	(void)isPressed;
	(void)x;
	(void)y;
	return 0;
}
#endif

#ifdef ENABLE_CARPLAY
static void lvgl_refresh_main_link_labels(void)
{
    lv_obj_t *cur = lv_scr_act();
    if (!(cur == guider_ui.screen || cur == guider_ui.screen_carPlay || cur == guider_ui.screen_androidAuto))
        return;
    if (!lv_obj_is_valid(guider_ui.screen_btn_carplay_label_statu) ||
        !lv_obj_is_valid(guider_ui.screen_btn_androidauto_label_statu))
        return;

    static int last_session = -1;
    static int last_linktype = -1;
    static language_t last_language = (language_t)-1;
    int session_started = zlink_client_is_session_started();
    int linktype = (int)g_sys_Data.linktype;
    language_t lang = g_sys_Data.current_language;

    if (last_session == session_started && last_linktype == linktype && last_language == lang)
        return;
    last_session = session_started;
    last_linktype = linktype;
    last_language = lang;

    if (session_started && linktype == LINK_TYPE_CARPLAY) {
        lv_label_set_text(guider_ui.screen_btn_carplay_label_statu,
            get_string_for_language(lang, "main_txt_Connect"));
        carplay_connected = true;
    } else {
        lv_label_set_text(guider_ui.screen_btn_carplay_label_statu,
            get_string_for_language(lang, "main_txt_nConnect"));
        carplay_connected = false;
    }

    if (session_started && linktype == LINK_TYPE_ANDROIDAUTO) {
        lv_label_set_text(guider_ui.screen_btn_androidauto_label_statu,
            get_string_for_language(lang, "main_txt_Connect"));
        androidauto_connected = true;
    } else {
        lv_label_set_text(guider_ui.screen_btn_androidauto_label_statu,
            get_string_for_language(lang, "main_txt_nConnect"));
        androidauto_connected = false;
    }
}

static void lvgl_handle_zlink_ui_requests(void)
{
    lv_obj_t *cur = lv_scr_act();
    int session_started = zlink_client_is_session_started();
    int linktype = (int)g_sys_Data.linktype;
    bool on_target_screen = (cur == guider_ui.screen || cur == guider_ui.screen_carPlay || cur == guider_ui.screen_androidAuto);

    static int last_session_started = -1;
    static bool pending_carplay_projection = false;
    static bool pending_androidauto_projection = false;
    static int last_video_ctrl_state_cp = -1;
    static int last_video_ctrl_state_aa = -1;
    static uint64_t last_video_ctrl_ts_cp = 0;
    static uint64_t last_video_ctrl_ts_aa = 0;
    const uint64_t focus_debounce_ms = 800;

#define REQUEST_VIDEO_CTRL_DEBOUNCED(lt, state) do { \
        uint64_t now = (uint64_t)custom_tick_get(); \
        int *last_state = ((lt) == LINK_TYPE_CARPLAY) ? &last_video_ctrl_state_cp : &last_video_ctrl_state_aa; \
        uint64_t *last_ts = ((lt) == LINK_TYPE_CARPLAY) ? &last_video_ctrl_ts_cp : &last_video_ctrl_ts_aa; \
        if (!(*last_state == (state) && (now - *last_ts) < focus_debounce_ms)) { \
            *last_state = (state); \
            *last_ts = now; \
            request_link_action((lt), LINK_ACTION_VIDEO_CTRL, (state), NULL); \
        } else { \
            printf("[lvgl] video ctrl debounced: link=%d state=%d\n", (lt), (state)); \
        } \
    } while (0)
    bool session_rising = (session_started == 1 && last_session_started != 1);
    last_session_started = session_started;

    if (session_rising) {
        bt_serial_send("CD");
        /* session_rising 只一帧：若当时不在主页/互联入口屏，自动投屏整段被跳过；记下来回到主页再进 */
        if (!on_target_screen) {
            if (linktype == LINK_TYPE_CARPLAY)
                pending_carplay_projection = true;
            else if (linktype == LINK_TYPE_ANDROIDAUTO)
                pending_androidauto_projection = true;
        }
    }
    if (!session_started) {
        pending_carplay_projection = false;
        pending_androidauto_projection = false;
    }

    bool enter_carplay = (session_started && on_target_screen && linktype == LINK_TYPE_CARPLAY) &&
        (session_rising || pending_carplay_projection);
    bool enter_androidauto = (session_started && on_target_screen && linktype == LINK_TYPE_ANDROIDAUTO) &&
        (session_rising || pending_androidauto_projection);

    if (enter_carplay) {
        long long t0_us = 0;
        long long t1_us = 0;
        long long t2_us = 0;
        long long t3_us = 0;
        long long t4_us = 0;
        long long t5_us = 0;
        unsigned int sid = ++g_cp_perf_session_seq;
        if (zlink_client_perf_is_enabled()) {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            t0_us = (long long)tv.tv_sec * 1000000LL + (long long)tv.tv_usec;
            printf("[cp_perf] ts_us=%lld stage=enter_carplay_begin sid=%u recorderMode=%d rec0_flag=%d rec8_flag=%d\n",
                   t0_us, sid, g_sys_Data.recorderMode,
                   g_sys_Data.vipp0_config.mRecorderFlag, g_sys_Data.vipp8_config.mRecorderFlag);
        }
        zlink_client_perf_set_session_id(sid);
        pending_carplay_projection = false;
        zlink_client_reset_video_prebuffer();
        if (zlink_client_perf_is_enabled()) {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            t1_us = (long long)tv.tv_sec * 1000000LL + (long long)tv.tv_usec;
            printf("[cp_perf] ts_us=%lld stage=enter_carplay_step sid=%u name=reset_prebuffer cost_us=%lld\n",
                   t1_us, sid, t1_us - t0_us);
        }
        zlink_client_request_video_focus(1);
        REQUEST_VIDEO_CTRL_DEBOUNCED(LINK_TYPE_CARPLAY, 0);
        if (zlink_client_perf_is_enabled()) {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            t2_us = (long long)tv.tv_sec * 1000000LL + (long long)tv.tv_usec;
            printf("[cp_perf] ts_us=%lld stage=enter_carplay_step sid=%u name=focus_hu_and_ctrl_off cost_us=%lld\n",
                   t2_us, sid, t2_us - t1_us);
        }
        int disp_w = 720;
        int disp_h = 1440;
        int cr = carplay_display_create(0, 0, disp_w, disp_h, 1440, 720);
        if (zlink_client_perf_is_enabled()) {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            t3_us = (long long)tv.tv_sec * 1000000LL + (long long)tv.tv_usec;
            printf("[cp_perf] ts_us=%lld stage=enter_carplay_step sid=%u name=carplay_display_create ret=%d cost_us=%lld\n",
                   t3_us, sid, cr, t3_us - t2_us);
        }
        zlink_client_set_video_active(1);
        zlink_client_request_video_focus(0);
        REQUEST_VIDEO_CTRL_DEBOUNCED(LINK_TYPE_CARPLAY, 1);
        if (zlink_client_perf_is_enabled()) {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            t4_us = (long long)tv.tv_sec * 1000000LL + (long long)tv.tv_usec;
            printf("[cp_perf] ts_us=%lld stage=enter_carplay_step sid=%u name=active_and_focus_phone cost_us=%lld\n",
                   t4_us, sid, t4_us - t3_us);
        }
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_carPlay, guider_ui.screen_carPlay_del,
                              &guider_ui.screen_del, setup_scr_screen_carPlay,
                              LV_SCR_LOAD_ANIM_NONE, 0, 0, true, true);
        if (zlink_client_perf_is_enabled()) {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            t5_us = (long long)tv.tv_sec * 1000000LL + (long long)tv.tv_usec;
            printf("[cp_perf] ts_us=%lld stage=enter_carplay_step sid=%u name=ui_load_scr_animation cost_us=%lld\n",
                   t5_us, sid, t5_us - t4_us);
        }
        if (cr == 0) {
            link_ui_on_projection_entered();
            stop_lvgl = true;
            lvgl_post_projection_flush_frames = 12;
            if (zlink_client_perf_is_enabled()) {
                struct timeval tv;
                long long t6_us;
                gettimeofday(&tv, NULL);
                t6_us = (long long)tv.tv_sec * 1000000LL + (long long)tv.tv_usec;
                printf("[cp_perf] ts_us=%lld stage=enter_carplay_done sid=%u total_us=%lld\n",
                       t6_us, sid, t6_us - t0_us);
            }
        } else {
            printf("[lvgl] carplay_display_create failed, ret=%d\n", cr);
        }
    } else if (enter_androidauto) {
        pending_androidauto_projection = false;
        zlink_client_reset_video_prebuffer();
        zlink_client_request_video_focus(1);
        REQUEST_VIDEO_CTRL_DEBOUNCED(LINK_TYPE_ANDROIDAUTO, 0);
        int disp_w = 720;
        int disp_h = 1440;
        int cr = carplay_display_create(0, 0, disp_w, disp_h, 1440, 720);
        zlink_client_set_video_active(1);
        zlink_client_request_video_focus(0);
        REQUEST_VIDEO_CTRL_DEBOUNCED(LINK_TYPE_ANDROIDAUTO, 1);
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_androidAuto, guider_ui.screen_androidAuto_del,
                              &guider_ui.screen_del, setup_scr_screen_androidAuto,
                              LV_SCR_LOAD_ANIM_NONE, 0, 0, true, true);
        if (cr == 0)
            link_ui_on_projection_entered();
        else
            printf("[lvgl] carplay_display_create (AA) failed, ret=%d\n", cr);
    }

    int link_type = zlink_client_take_pending_home_request();
    if (link_type != 0) {
        if (link_type == LINK_TYPE_CARPLAY) {
            if (cur == guider_ui.screen_carPlay) {
                REQUEST_VIDEO_CTRL_DEBOUNCED(LINK_TYPE_CARPLAY, 0);
                ui_load_scr_animation(&guider_ui, &guider_ui.screen, guider_ui.screen_del,
                                      &guider_ui.screen_carPlay_del, setup_scr_screen,
                                      LV_SCR_LOAD_ANIM_NONE, 0, 0, true, true);
                link_ui_on_projection_exited();
                stop_lvgl = false;
                lvgl_post_projection_flush_frames = 0;
            }
        } else if (link_type == LINK_TYPE_ANDROIDAUTO) {
            if (cur == guider_ui.screen_androidAuto) {
                REQUEST_VIDEO_CTRL_DEBOUNCED(LINK_TYPE_ANDROIDAUTO, 0);
                ui_load_scr_animation(&guider_ui, &guider_ui.screen, guider_ui.screen_del,
                                      &guider_ui.screen_androidAuto_del, setup_scr_screen,
                                      LV_SCR_LOAD_ANIM_NONE, 0, 0, true, true);
                link_ui_on_projection_exited();
            }
        }
    }
    lvgl_refresh_main_link_labels();
#undef REQUEST_VIDEO_CTRL_DEBOUNCED
}
#endif

int lvgl_main(int w, int h)
{
    printf("-------%s:%d-----------------\n",__func__,__LINE__);
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    // uint32_t rotated = LV_DISP_ROT_NONE;
    uint32_t rotated = LV_DISP_ROT_90;


    /*LittlevGL init*/
    lv_init();

    /*Linux frame buffer device init*/
    sunxifb_init(rotated);

    /*A buffer for LittlevGL to draw the screen's content*/
    static uint32_t width, height;
    sunxifb_get_sizes(&width, &height);

    static lv_color_t *buf;
    buf = (lv_color_t*) sunxifb_alloc(width * height * sizeof(lv_color_t),
            "lv_examples");

    if (buf == NULL) {
        sunxifb_exit();
        printf("malloc draw buffer fail\n");
        return 0;
    }

    /*Initialize a descriptor for the buffer*/
    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, width * height);

    /*Initialize and register a display driver*/
    disp_drv.draw_buf   = &disp_buf;
    disp_drv.flush_cb   = sunxifb_flush;
    disp_drv.hor_res    = width;
    disp_drv.ver_res    = height;
    disp_drv.rotated    = rotated;
    disp_drv.antialiasing =1;
    disp_drv.screen_transp = 1;
#ifndef USE_SUNXIFB_G2D_ROTATE
    if (rotated != LV_DISP_ROT_NONE)
        disp_drv.sw_rotate = 1;
#endif
    lv_disp_drv_register(&disp_drv);

#ifdef ENABLE_CARPLAY
    /* Same hor/ver/rot as disp_drv: matches lv_indev pointer rotation for zlink coords */
    carplay_link_touch_configure((int)width, (int)height, (int)rotated);
#endif

    evdev_init();
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);                /*Basic initialization*/
    indev_drv.type =LV_INDEV_TYPE_POINTER;        /*See below.*/
    indev_drv.read_cb = evdev_read;               /*See below.*/
    /*Register the driver in LVGL and save the created input device object*/
    lv_indev_t * evdev_indev = lv_indev_drv_register(&indev_drv);

    // lv_demo_widgets();
//    lv_demo_music();
//    lv_demo_benchmark();
//    lv_demo_benchmark();
//    lv_demo_keypad_encoder();
//    lv_demo_keypad_encoder();
//    lv_demo_stress();
//------------------------------------------------


    setup_ui(&guider_ui);
	lv_task_handler();
    custom_init(&guider_ui);

    if(g_sys_Data.themeMode == THEME_AUTO){
        createLightPerceptionThread();
        autoModeTimer = lv_timer_create(creatAutoModeTimerCbk, 1000, NULL);
    }

    pthread_create(&recorder_worker_tid, NULL, recorder_worker_thread, NULL);
    pthread_detach(recorder_worker_tid);
    lv_timer_create(bt_status_check_timer, 500, NULL);
	tire_ui_refresh_now();
//--------------------------------------------------------------
    /*Handle LitlevGL tasks (tickless mode)*/
    while(1) {
        /* CarPlay 投屏时只停 UI 刷新/动画/定时器，不退出线程；切全透明屏后多刷几帧再停，否则底层 VO 可能被旧 UI 盖住 */
        if (!stop_lvgl || lvgl_post_projection_flush_frames > 0) {
            lv_task_handler();
            if (lvgl_post_projection_flush_frames > 0)
                lvgl_post_projection_flush_frames--;
        }
#ifdef ENABLE_CARPLAY
        lvgl_handle_zlink_ui_requests();
#endif
        usleep(5000);
    }


    tp2804_i2c_deinit(&i2c0_fd);
    tp2804_i2c_deinit(&i2c1_fd);

    sunxifb_free((void**) &buf, "lv_examples");
    sunxifb_exit();
    return 0;
}

/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if(start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}
int the_tick_to_extern(void){
    return custom_tick_get();
}

void* LvglMain(void *arg) {
   
    lvgl_main(screanWidth, screanHeight);  //while(1)

    return NULL;
}

int LvglService(int w, int h) {
	screanWidth = w;
	screanHeight = h;
    createQueueGetMppFileThread();
//---------------------------上电获取储存数据------------------
    storageData_init(STORAGE_DATA_PATH, true);
    g_sys_Data.backlight = storageData_getInt("backlight", 80);
    g_sys_Data.themeMode = storageData_getInt("themeMode", THEME_DAY);
    g_sys_Data.splitScreenDisp = storageData_getInt("splitScreenDisp", DISPLAY_NORMAL);
    g_sys_Data.current_language = storageData_getInt("current_language", LANG_ENGLISH);
    g_sys_Data.carTripSwitch = storageData_getBool("carTripSwitch", true);
    g_sys_Data.resetFactory = storageData_getBool("resetFactory", true);
    g_sys_Data.carTripSwitch = storageData_getBool("carTripSwitch", true);
    g_sys_Data.recorderTime = storageData_getInt("recorderTime", RECORDER_3);
    g_sys_Data.recorderUrgentTime = storageData_getInt("recorderUrgentTime", RECORDER_1);
    g_sys_Data.rearCameraImage = storageData_getBool("rearCameraImage", true);
    g_sys_Data.powerOnRecorder = storageData_getBool("powerOnRecorder", true);
    g_sys_Data.SoundRecorder = storageData_getBool("SoundRecorder", true);
    g_sys_Data.TimeMark = storageData_getBool("TimeMark", true);
    g_sys_Data.SpeedMark = storageData_getBool("SpeedMark", true);
    g_sys_Data.SpeedUnit = storageData_getBool("SpeedUnit", true);
    g_sys_Data.agingMode.screenSaveTime = storageData_getInt("screenSaveTime", T_10S);
    g_sys_Data.themeMode = storageData_getInt("themeMode", THEME_DAY);
    printf("storage data: backlight:%d,\n \
    splitScreenDisp:%d,\n,\
    language:%d,\n\
    recorderTime:%d\n,\
    recorderUrgentTime:%d\n,\
    carTripSwitch:%d\n"\
    , g_sys_Data.backlight,g_sys_Data.splitScreenDisp,g_sys_Data.current_language,g_sys_Data.recorderTime, g_sys_Data.recorderUrgentTime, g_sys_Data.carTripSwitch);

#if 1
    SetBackLight(g_sys_Data.backlight);
    createGpsThread();
    tp2804_i2c_init(&i2c0_fd, I2C_BUS0, TP2804_I2C_ADDR);
    tp2804_i2c_init(&i2c1_fd, I2C_BUS1, TP2804_I2C_ADDR);

    MPI_init();
    AW_MPI_VDEC_SetVEFreq(MM_INVALID_CHN, 0);

    initVi(&g_sys_Data.vipp0_config, 0, 0);
    initVi(&g_sys_Data.vipp8_config, 8, 0);
 #endif
//------------------------------------------------------------
	int err = pthread_create(&threadID, NULL, LvglMain, NULL);
	if(err != 0)
	{
	    printf("LvglService error!\n");
	    return 0;
	}

	return 0;
}


