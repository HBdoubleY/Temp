#include "myTimer.h"
#include "lv_obj.h"
#include "i18n.h"
#include "lvgl_system.h"
#include <stdlib.h>
#include <string.h>
#include "i18n.h"

typedef struct {
    lv_timer_t* timer;      
    lv_obj_t* label;       
    bool is_running;        
} LabelTimer;

#define MAX_TIMERS 16
static LabelTimer timers[MAX_TIMERS] = {0};

static void timer_callback(lv_timer_t* timer) {
    if (!timer || !timer->user_data) {
        return;
    }
    
    LabelTimer* timer_info = (LabelTimer*)timer->user_data;

    if (timer_info->label && lv_obj_is_valid(timer_info->label)) {
        lv_obj_add_flag(timer_info->label, LV_OBJ_FLAG_HIDDEN);
    }

    timer_info->is_running = false;
    timer_info->timer = NULL;

}

static LabelTimer* find_label_timer(lv_obj_t* label) {
    if (!label) return NULL;
    
    for (int i = 0; i < MAX_TIMERS; i++) {
        if (timers[i].label == label && timers[i].is_running) {
            return &timers[i];
        }
    }
    return NULL;
}

static LabelTimer* find_free_timer_slot(void) {
    for (int i = 0; i < MAX_TIMERS; i++) {
        if (!timers[i].is_running) {
            return &timers[i];
        }
    }
    return NULL;
}

bool show_label_with_timer(lv_obj_t* label, const char* text, uint32_t delay_ms) {
    if (!label) {
        return false;
    }

    LabelTimer* timer_info = find_label_timer(label);
    
    if (timer_info) {
        if (timer_info->timer) {
            lv_timer_del(timer_info->timer);
            timer_info->timer = NULL;
        }

        if (text) {
            lv_label_set_text(label, get_string_for_language(g_sys_Data.current_language,text));
        }

        timer_info->timer = lv_timer_create(timer_callback, delay_ms, timer_info);
        if (!timer_info->timer) {
            timer_info->is_running = false;
            return false;
        }

        lv_timer_set_repeat_count(timer_info->timer, 1);

        lv_obj_clear_flag(label, LV_OBJ_FLAG_HIDDEN);
        // lv_obj_invalidate(label);
        lv_refr_now(NULL);
        return true;
    }
    
    timer_info = find_free_timer_slot();
    if (!timer_info) {
        return false;  
    }
    
    timer_info->label = label;
    timer_info->is_running = true;
    
    if (text) {
        lv_label_set_text(label, get_string_for_language(g_sys_Data.current_language,text));
    }

    timer_info->timer = lv_timer_create(timer_callback, delay_ms, timer_info);
    if (!timer_info->timer) {
        timer_info->is_running = false;
        return false;
    }

    lv_timer_set_repeat_count(timer_info->timer, 1);

    lv_obj_clear_flag(label, LV_OBJ_FLAG_HIDDEN);
    // lv_obj_invalidate(label);
    lv_refr_now(NULL);
    return true;
}

void stop_label_timer(lv_obj_t* label) {
    if (!label) return;
    
    LabelTimer* timer_info = find_label_timer(label);
    if (!timer_info) return;

    if (timer_info->timer) {
        lv_timer_del(timer_info->timer);
        timer_info->timer = NULL;
    }
    
    lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);

    timer_info->is_running = false;
}

bool has_label_timer(lv_obj_t* label) {
    if (!label) return false;
    
    LabelTimer* timer_info = find_label_timer(label);
    return timer_info != NULL && timer_info->is_running;
}


bool show_label_immediate(lv_obj_t* label, const char* text, uint32_t delay_ms) {
    if (!label) return false;

    stop_label_timer(label);
    
    return show_label_with_timer(label, text, delay_ms);
}