#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"
#include <string.h>

#include <string.h>

// 弹窗全局变量
static struct {
    lv_obj_t* container;       // 弹窗容器
    lv_obj_t* label;           // 文本标签
    lv_timer_t* timer;         // 定时器
    bool is_visible;           // 是否可见
} popup_v8 = {0};

// 定时器回调
static void popup_timer_cb(lv_timer_t* timer) {
    if (popup_v8.container) {
        lv_obj_add_flag(popup_v8.container, LV_OBJ_FLAG_HIDDEN);
    }
    popup_v8.is_visible = false;
    
    if (timer) {
        lv_timer_del(timer);
        popup_v8.timer = NULL;
    }
}

// 创建弹窗UI（第一次调用时创建）
static void create_popup(void) {
    if (popup_v8.container != NULL) return;
    
    // 获取顶层layer
    lv_obj_t* layer_top = lv_layer_top();
    
    // 创建弹窗容器
    popup_v8.container = lv_obj_create(layer_top);
    
    // 设置容器样式
    lv_obj_set_size(popup_v8.container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(popup_v8.container, 10, 0);
    lv_obj_set_style_bg_color(popup_v8.container, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(popup_v8.container, LV_OPA_80, 0);
    lv_obj_set_style_pad_all(popup_v8.container, 0, 0);
    lv_obj_set_style_border_width(popup_v8.container, 0, 0);
    
    // 初始隐藏
    lv_obj_add_flag(popup_v8.container, LV_OBJ_FLAG_HIDDEN);
    
    // 创建文本标签
    popup_v8.label = lv_label_create(popup_v8.container);
    lv_obj_set_style_text_color(popup_v8.label, lv_color_white(), 0);
    lv_obj_set_style_text_font(popup_v8.label, &lv_font_harmonyOS_40, 0);
    lv_obj_set_style_text_align(popup_v8.label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(popup_v8.label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    
    
    // 居中标签
    lv_obj_center(popup_v8.label);
    
    popup_v8.is_visible = false;
}


// 显示弹窗
void show_popup_v8(const char* message, uint16_t delay_ms) {
    // 确保弹窗已创建
    create_popup();
    
    // 取消之前的定时器
    if (popup_v8.timer != NULL) {
        lv_timer_del(popup_v8.timer);
        popup_v8.timer = NULL;
    }
    


    // // 计算单行文本的宽高
    // lv_point_t text_size = {0};
    // const lv_font_t* font = &lv_font_harmonyOS_40;

    // // 计算文本尺寸
    // lv_txt_get_size(&text_size,           // 输出尺寸
    //                 message,                 // 文本
    //                 font,                 // 字体
    //                 0,                    // 字间距
    //                 0,                    // 行间距
    //                 0,                    // 最大宽度0=不换行
    //                 LV_TEXT_FLAG_NONE);   // 标志

    // 设置标签大小
    // lv_obj_set_size(popup_v8.label, text_size.y, text_size.x+25);

    // 设置文本
    lv_label_set_text(popup_v8.label, message);
    
    // 更新容器大小（根据文本内容自适应）
    lv_obj_update_layout(popup_v8.label);
    lv_coord_t label_width = lv_obj_get_width(popup_v8.label);
    lv_coord_t label_height = lv_obj_get_height(popup_v8.label);
    
    
    // 设置容器大小（文本宽度+内边距）
    if(label_width > 1000){
        label_width = 1000;
    }
    lv_obj_set_size(popup_v8.container, 
                   label_width + 10,  // 左右各20px内边距
                   label_height + 10); // 上下各15px内边距
    
    // 居中显示
    lv_obj_center(popup_v8.container);

    // lv_obj_set_style_align(popup_v8.container, LV_ALIGN_BOTTOM_MID, 0);
    lv_obj_align_to(popup_v8.container, NULL, LV_ALIGN_BOTTOM_MID, 0, -20);
    
    // 显示弹窗
    lv_obj_clear_flag(popup_v8.container, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(popup_v8.container);
    
    // 设置自动关闭定时器
    if (delay_ms > 0) {
        popup_v8.timer = lv_timer_create(popup_timer_cb, delay_ms, NULL);
        lv_timer_set_repeat_count(popup_v8.timer, 1);
    }
    
    popup_v8.is_visible = true;
}

// 显示弹窗（默认2秒）
void show_popup_simple_v8(const char* message) {
    show_popup_v8(message, 2000);
}

// 立即关闭弹窗
void close_popup_v8(void) {
    if (popup_v8.container) {
        lv_obj_add_flag(popup_v8.container, LV_OBJ_FLAG_HIDDEN);
    }
    
    if (popup_v8.timer) {
        lv_timer_del(popup_v8.timer);
        popup_v8.timer = NULL;
    }
    
    popup_v8.is_visible = false;
}

// 检查弹窗是否显示
bool is_popup_visible_v8(void) {
    return popup_v8.is_visible;
}