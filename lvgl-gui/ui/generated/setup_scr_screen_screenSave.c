#include "lvgl.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include "gui_guider.h"
#include "Rtc2C.h"


#ifndef DEG_TO_RAD
#define DEG_TO_RAD(deg) ((deg) * 3.1415926535 / 180.0)
#endif

lv_timer_t* DashAnalogTimer =NULL;

int screen_bg[4] = {0xFF0000, 0x0000FF, 0x00FF00, 0xFFFF00};
static int cur_bg = 0;
static int auto_updata_bg = 0;

typedef struct {
    const void* bg_image;    
    const void* hour_hand;   
    const void* minute_hand; 
    const void* second_hand; 
    const void* screen_img;  
    lv_coord_t center_x;     
    lv_coord_t center_y;     
    lv_coord_t hour_pivot_y; 
    lv_coord_t minute_pivot_y; 
    lv_coord_t second_pivot_y; 
} analog_clock_config_t;

// 时钟句柄
typedef struct {
    lv_obj_t* overlay;       
    lv_obj_t* bg_img;        
    lv_obj_t* hour_img;      
    lv_obj_t* minute_img;   
    lv_obj_t* second_img;    
    lv_timer_t* timer;       
    analog_clock_config_t config;
} analog_clock_t;

static analog_clock_t* g_clock_instance = NULL;

analog_clock_t* analog_clock_create(const analog_clock_config_t* config);
void analog_clock_delete(analog_clock_t* clock);
void analog_clock_set_time(analog_clock_t* clock, uint8_t hour, uint8_t minute, uint8_t second);
void analog_clock_update(analog_clock_t* clock);
lv_obj_t* analog_clock_get_overlay(analog_clock_t* clock);

void createDashAnalogTimer(int delay_ms);
void destoryDashAnalogTimer();
void resetDashAnalogTimer();


static void clock_timer_cb(lv_timer_t* timer);
static void update_clock_hands(analog_clock_t* clock, uint8_t hour, uint8_t minute, uint8_t second);
static void overlay_click_cb(lv_event_t* e);

analog_clock_t* analog_clock_create(const analog_clock_config_t* config) {
    analog_clock_t* clock = (analog_clock_t*)lv_mem_alloc(sizeof(analog_clock_t));
    if (!clock) return NULL;
    
    clock->config = *config;

    lv_obj_t* layer_top = lv_layer_top();
    clock->overlay = lv_obj_create(layer_top);
    lv_obj_set_size(clock->overlay, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(clock->overlay, lv_color_hex(screen_bg[cur_bg]), 0);
    lv_obj_set_style_bg_opa(clock->overlay, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(clock->overlay, 0, 0);
    lv_obj_set_style_outline_width(clock->overlay, 0, 0);
    lv_obj_clear_flag(clock->overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(clock->overlay, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(clock->overlay, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_clear_flag(clock->overlay, LV_OBJ_FLAG_CLICK_FOCUSABLE);

    lv_obj_add_event_cb(clock->overlay, overlay_click_cb, LV_EVENT_CLICKED, clock);

    if (config->bg_image) {
        clock->bg_img = lv_img_create(clock->overlay);
        lv_img_set_src(clock->bg_img, config->bg_image);
        lv_obj_align(clock->bg_img, LV_ALIGN_CENTER, 0, 0);
    } else {
        clock->bg_img = lv_obj_create(clock->overlay);
        lv_obj_set_size(clock->bg_img, 200, 200);
        lv_obj_align(clock->bg_img, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_radius(clock->bg_img, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(clock->bg_img, lv_color_white(), 0);
        lv_obj_set_style_border_color(clock->bg_img, lv_color_black(), 0);
        lv_obj_set_style_border_width(clock->bg_img, 2, 0);
    }

    if (config->hour_hand) {
        clock->hour_img = lv_img_create(clock->overlay);
        lv_img_set_src(clock->hour_img, config->hour_hand);

        lv_img_dsc_t *hour_hand = (lv_img_dsc_t *)config->hour_hand;
        lv_coord_t hour_img_w = hour_hand->header.w;
        lv_coord_t hour_img_h = hour_hand->header.h;
        
        lv_obj_align(clock->hour_img, LV_ALIGN_CENTER, 0, 0);

        lv_img_set_pivot(clock->hour_img, hour_img_w / 2, hour_img_h / 2);
    } else {
        clock->hour_img = lv_obj_create(clock->overlay);
        lv_obj_set_size(clock->hour_img, 6, 50);
        lv_obj_set_style_bg_color(clock->hour_img, lv_color_black(), 0);
        lv_obj_set_style_radius(clock->hour_img, 3, 0);
        lv_obj_align(clock->hour_img, LV_ALIGN_CENTER, 0, 0);
        
        lv_obj_set_style_transform_pivot_x(clock->hour_img, 0, 0);
        lv_obj_set_style_transform_pivot_y(clock->hour_img, 25, 0);
    }

    if (config->minute_hand) {
        clock->minute_img = lv_img_create(clock->overlay);
        lv_img_set_src(clock->minute_img, config->minute_hand);

        lv_img_dsc_t *minute_hand = (lv_img_dsc_t *)config->minute_hand;
        lv_coord_t minute_img_w = minute_hand->header.w;
        lv_coord_t minute_img_h = minute_hand->header.h;

        lv_obj_align(clock->minute_img, LV_ALIGN_CENTER, 0, 0);
        
        lv_img_set_pivot(clock->minute_img, minute_img_w / 2, minute_img_h / 2);
    } else {
        clock->minute_img = lv_obj_create(clock->overlay);
        lv_obj_set_size(clock->minute_img, 4, 70);
        lv_obj_set_style_bg_color(clock->minute_img, lv_color_black(), 0);
        lv_obj_set_style_radius(clock->minute_img, 2, 0);
        lv_obj_align(clock->minute_img, LV_ALIGN_CENTER, 0, 0);
        
        lv_obj_set_style_transform_pivot_x(clock->minute_img, 0, 0);
        lv_obj_set_style_transform_pivot_y(clock->minute_img, 35, 0);
    }

    if (config->second_hand) {
        clock->second_img = lv_img_create(clock->overlay);
        lv_img_set_src(clock->second_img, config->second_hand);
        
        lv_img_dsc_t *second_hand = (lv_img_dsc_t *)config->second_hand;
        lv_coord_t second_img_w = second_hand->header.w;
        lv_coord_t second_img_h = second_hand->header.h;

        lv_obj_align(clock->second_img, LV_ALIGN_CENTER, 0, 0);

        lv_img_set_pivot(clock->second_img, second_img_w / 2, second_img_h/2);
    } else {
        clock->second_img = lv_obj_create(clock->overlay);
        lv_obj_set_size(clock->second_img, 2, 80);
        lv_obj_set_style_bg_color(clock->second_img, lv_color_make(255, 0, 0), 0);
        lv_obj_set_style_radius(clock->second_img, 1, 0);
        lv_obj_align(clock->second_img, LV_ALIGN_CENTER, 0, 0);
        
        lv_obj_set_style_transform_pivot_x(clock->second_img, 0, 0);
        lv_obj_set_style_transform_pivot_y(clock->second_img, 40, 0);
    }

    lv_obj_move_foreground(clock->second_img);
    lv_obj_move_foreground(clock->minute_img);
    lv_obj_move_foreground(clock->hour_img);
    
    lv_obj_t* center_dot = lv_obj_create(clock->overlay);
    lv_obj_set_size(center_dot, 10, 10);
    lv_obj_align(center_dot, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_radius(center_dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(center_dot, lv_color_black(), 0);
    lv_obj_set_style_border_color(center_dot, lv_color_white(), 0);
    lv_obj_set_style_border_width(center_dot, 1, 0);
    lv_obj_move_foreground(center_dot); 

    // clock->timer = lv_timer_create(clock_timer_cb, 1000, clock);

    TTime time = {0};
    RtcGetTime2C(&time);
    analog_clock_set_time(clock, time.nHour, time.nMinute, time.nSecond);
    
    return clock;
}

void analog_clock_delete(analog_clock_t* clock) {
    if (!clock) return;
    
    if (clock->timer) {
        lv_timer_del(clock->timer);
    }
    if (clock->overlay) {
        lv_obj_del(clock->overlay);
    }
    lv_mem_free(clock);
    g_clock_instance = NULL;
    lv_refr_now(lv_disp_get_default());
}


void analog_clock_set_time(analog_clock_t* clock, uint8_t hour, uint8_t minute, uint8_t second) {
    if (!clock) return;
    
    update_clock_hands(clock, hour, minute, second);

    lv_obj_invalidate(clock->overlay);
    lv_refr_now(lv_disp_get_default());
}

// 更新时钟指针
static void update_clock_hands(analog_clock_t* clock, uint8_t hour, uint8_t minute, uint8_t second) {
    if (!clock) return;
    
    float hour_deg = (hour % 12) * 30.0f + minute * 0.5f;
    float minute_deg = minute * 6.0f + second * 0.1f;
    float second_deg = second * 6.0f;
    
    if (clock->hour_img && lv_obj_check_type(clock->hour_img, &lv_img_class)) {
        lv_img_set_angle(clock->hour_img, (int16_t)(hour_deg * 10));  // LVGL角度单位是0.1度
        lv_obj_set_style_transform_angle(clock->hour_img, (int32_t)(hour_deg * 10), 0);
    }
    
    if (clock->minute_img && lv_obj_check_type(clock->minute_img, &lv_img_class)) {
        lv_img_set_angle(clock->minute_img, (int16_t)(minute_deg * 10));
    } else if (clock->minute_img) {
        lv_obj_set_style_transform_angle(clock->minute_img, (int32_t)(minute_deg * 10), 0);
    }
    
    if (clock->second_img && lv_obj_check_type(clock->second_img, &lv_img_class)) {
        lv_img_set_angle(clock->second_img, (int16_t)(second_deg * 10));
    } else if (clock->second_img) {
        lv_obj_set_style_transform_angle(clock->second_img, (int32_t)(second_deg * 10), 0);
    }
}

static void clock_timer_cb(lv_timer_t* timer) {
    analog_clock_t* clock = (analog_clock_t*)timer->user_data;
    if (!clock) return;
    
    TTime time = {0};
    RtcGetTime2C(&time);
    
    update_clock_hands(clock, time.nHour, time.nMinute, time.nSecond);
    
    auto_updata_bg ++;
    if((auto_updata_bg % 30) == 0){
        cur_bg++;
        if(cur_bg == 4) cur_bg = 0;
        lv_obj_set_style_bg_color(clock->overlay, lv_color_hex(screen_bg[cur_bg]), 0);
    }
    lv_obj_invalidate(clock->overlay);
}

void analog_clock_update(analog_clock_t* clock) {
    if (!clock || !clock->timer) return;
    
    clock_timer_cb(clock->timer);
    lv_refr_now(lv_disp_get_default());
}

lv_obj_t* analog_clock_get_overlay(analog_clock_t* clock) {
    return clock ? clock->overlay : NULL;
}

static void overlay_click_cb(lv_event_t* e) {
    analog_clock_t* clock = (analog_clock_t*)lv_event_get_user_data(e);
    if (clock) {
        analog_clock_delete(clock);
    }
}

void show_analog_clock(void) {
    if (g_clock_instance) {
        analog_clock_delete(g_clock_instance);
    }
    
    printf("Showing analog clock overlay...\n");

    lv_coord_t screen_w = LV_HOR_RES;
    lv_coord_t screen_h = LV_VER_RES;
    lv_coord_t center_x = screen_w / 2;
    lv_coord_t center_y = screen_h / 2;
    

    analog_clock_config_t config = {
        .bg_image = &_dial_alpha_476x476,        
        .hour_hand = &_hour_hand_alpha_325x325,  
        .minute_hand = &_min_hand_alpha_325x325, 
        .second_hand = &_sec_hand_alpha_325x325, 
        .screen_img = screen_bg,
        .center_x = center_x,                   
        .center_y = center_y,
        .hour_pivot_y = 0,    
        .minute_pivot_y = 0,  
        .second_pivot_y = 0   
    };
    

    g_clock_instance = analog_clock_create(&config);
    auto_updata_bg = 0;
    
    if (g_clock_instance) {
        printf("Clock overlay created successfully\n");
    } else {
        printf("Failed to create clock overlay\n");
    }
    DashAnalogTimer = NULL;
}


void hide_analog_clock(void) {
    if (g_clock_instance) {
        analog_clock_delete(g_clock_instance);
    }
}


bool is_clock_visible(void) {
    return g_clock_instance != NULL;
}

void createDashAnalogTimer(int delay_ms){
    DashAnalogTimer = lv_timer_create(show_analog_clock, delay_ms, NULL);
    lv_timer_set_repeat_count(DashAnalogTimer, 1);
}

void resetDashAnalogTimer(int delay_ms){
    if(DashAnalogTimer){ 
        lv_timer_reset(DashAnalogTimer);
    }else{
        createDashAnalogTimer(delay_ms);
    }
}

void destoryDashAnalogTimer(){
    if(DashAnalogTimer) lv_timer_del(DashAnalogTimer);
    DashAnalogTimer = NULL;
}