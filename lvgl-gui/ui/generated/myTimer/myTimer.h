#ifndef MYTIMER_H
#define MYTIMER_H

#include "lvgl.h"
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif


bool show_label_with_timer(lv_obj_t* label, const char* text, uint32_t delay_ms);

void stop_label_timer(lv_obj_t* label);

bool has_label_timer(lv_obj_t* label);

bool show_label_immediate(lv_obj_t* label, const char* text, uint32_t delay_ms);

#ifdef __cplusplus
}
#endif

#endif