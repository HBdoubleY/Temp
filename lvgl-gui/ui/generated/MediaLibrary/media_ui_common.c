/**
 * @file ui_common.c
 * @brief 公共UI组件库实现
 * 
 * 实现可复用的UI组件创建和样式设置函数
 */

#include "media_ui_common.h"
#include <stdio.h>
#include <string.h>

/*====================================================================================
 * 私有变量
 *====================================================================================*/

/* 组件库是否已初始化 */
static bool g_ui_common_initialized = false;

/*====================================================================================
 * 公共初始化
 *====================================================================================*/

void ui_common_init(void) {
    if (g_ui_common_initialized) {
        return;
    }
    
    /* 初始化主题管理器 */
    theme_manager_init();
    
    g_ui_common_initialized = true;
    printf("[UI_Common] Initialized\n");
}

/*====================================================================================
 * 列表项组件实现
 *====================================================================================*/

lv_obj_t* ui_create_file_list_item(lv_obj_t *parent, const ui_list_item_cfg_t *cfg) {
    if (!parent || !cfg) {
        printf("[UI_Common] Error: Invalid parameters for list item\n");
        return NULL;
    }
    
    /* 确保已初始化 */
    if (!g_ui_common_initialized) {
        ui_common_init();
    }
    
    /* 创建列表项按钮 */
    lv_obj_t *list_btn = lv_btn_create(parent);
    lv_obj_remove_style_all(list_btn);
    lv_obj_set_width(list_btn, lv_pct(100));
    lv_obj_set_height(list_btn, 80);
    lv_obj_add_flag(list_btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    
    /* 应用主题样式 */
    lv_obj_set_style_bg_color(list_btn, TM_BG_TERTIARY, 0);
    lv_obj_set_style_bg_opa(list_btn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_bg_color(list_btn, TM_BG_HOVER, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(list_btn, LV_OPA_COVER, LV_STATE_PRESSED);
    lv_obj_set_style_border_color(list_btn, TM_BORDER_PRIMARY, 0);
    lv_obj_set_style_border_width(list_btn, 1, 0);
    lv_obj_set_style_border_opa(list_btn, LV_OPA_50, 0);
    lv_obj_set_style_border_side(list_btn, LV_BORDER_SIDE_BOTTOM | LV_BORDER_SIDE_TOP, 0);
    lv_obj_set_style_radius(list_btn, 0, 0);
    lv_obj_set_style_pad_all(list_btn, 0, 0);
    
    /* 创建图标容器 */
    lv_obj_t *img_icon_container = lv_obj_create(list_btn);
    lv_obj_set_size(img_icon_container, 80, 80);
    lv_obj_align(img_icon_container, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_pad_all(img_icon_container, 0, 0);
    lv_obj_set_style_bg_opa(img_icon_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(img_icon_container, 0, 0);
    lv_obj_clear_flag(img_icon_container, LV_OBJ_FLAG_SCROLLABLE);
    
    /* 创建图标 */
    lv_obj_t *icon = lv_img_create(img_icon_container);
    lv_obj_set_style_text_font(icon, &v853Font_Medialibrary_30, 0);
    lv_obj_set_style_text_color(icon, TM_ICON_PRIMARY, 0);
    if (cfg->icon_src) {
        lv_img_set_src(icon, cfg->icon_src);
    } else {
        lv_img_set_src(icon, LV_SYMBOL_FILE);  /* 默认文件图标 */
    }
    lv_obj_center(icon);
    
    /* 创建文本标签 */
    lv_obj_t *label = lv_label_create(list_btn);
    lv_obj_set_style_text_font(label, &v853Font_Medialibrary_30, 0);
    lv_obj_set_style_text_color(label, TM_TEXT_PRIMARY, 0);
    lv_label_set_text(label, cfg->text ? cfg->text : "");
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 80, 0);
    
    /* 存储用户数据并添加点击事件 */
    lv_obj_set_user_data(list_btn, (void*)(intptr_t)cfg->user_data);
    if (cfg->click_cb) {
        lv_obj_add_event_cb(list_btn, cfg->click_cb, LV_EVENT_CLICKED, NULL);
    }
    
    /* 创建删除按钮（可选） */
    if (cfg->show_delete_btn) {
        lv_obj_t *del_btn = lv_btn_create(list_btn);
        lv_obj_set_size(del_btn, 50, 50);
        lv_obj_set_style_bg_opa(del_btn, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(del_btn, 0, 0);
        lv_obj_align(del_btn, LV_ALIGN_RIGHT_MID, -30, 0);
        
        lv_obj_t *del_icon = lv_img_create(del_btn);
        lv_obj_set_style_img_recolor(del_icon, TM_ICON_SECONDARY, 0);
        lv_obj_set_style_img_recolor_opa(del_icon, LV_OPA_COVER, 0);
        lv_obj_set_style_img_recolor(del_icon, TM_TEXT_DISABLED, LV_STATE_PRESSED);
        lv_obj_set_style_img_recolor_opa(del_icon, LV_OPA_COVER, LV_STATE_PRESSED);
        lv_img_set_src(del_icon, &delete);
        lv_obj_center(del_icon);
        
        if (cfg->delete_cb) {
            lv_obj_add_event_cb(del_btn, cfg->delete_cb, LV_EVENT_CLICKED, 
                               (void*)(intptr_t)cfg->user_data);
        }
    }
    
    return list_btn;
}

/*====================================================================================
 * 对话框组件实现
 *====================================================================================*/

bool ui_create_confirm_dialog(lv_obj_t *parent, const ui_dialog_cfg_t *cfg, 
                              ui_dialog_handle_t *handle) {
    if (!parent || !cfg || !handle) {
        printf("[UI_Common] Error: Invalid parameters for dialog\n");
        return false;
    }
    
    /* 确保已初始化 */
    if (!g_ui_common_initialized) {
        ui_common_init();
    }
    
    /* 清空句柄 */
    handle->mask_layer = NULL;
    handle->dialog_bg = NULL;
    
    /* 创建遮罩层 */
    handle->mask_layer = lv_obj_create(parent);
    lv_obj_set_size(handle->mask_layer, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(handle->mask_layer, TM_BG_PRIMARY, 0);
    lv_obj_set_style_bg_opa(handle->mask_layer, LV_OPA_50, 0);
    lv_obj_set_style_radius(handle->mask_layer, 0, 0);
    lv_obj_set_style_border_width(handle->mask_layer, 0, 0);
    lv_obj_clear_flag(handle->mask_layer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_move_foreground(handle->mask_layer);
    
    /* 创建对话框背景 */
    handle->dialog_bg = lv_obj_create(handle->mask_layer);
    lv_obj_set_size(handle->dialog_bg, LV_PCT(40), LV_PCT(40));
    lv_obj_center(handle->dialog_bg);
    lv_obj_set_style_bg_color(handle->dialog_bg, TM_BG_TERTIARY, 0);
    lv_obj_set_style_radius(handle->dialog_bg, 10, 0);
    lv_obj_set_style_pad_all(handle->dialog_bg, 20, 0);
    lv_obj_set_style_border_width(handle->dialog_bg, 2, 0);
    lv_obj_set_style_border_color(handle->dialog_bg, TM_BORDER_SECONDARY, 0);
    lv_obj_clear_flag(handle->dialog_bg, LV_OBJ_FLAG_SCROLLABLE);
    
    /* 创建消息标签 */
    lv_obj_t *msg_label = lv_label_create(handle->dialog_bg);
    lv_label_set_text(msg_label, cfg->message ? cfg->message : "");
    lv_obj_set_style_text_font(msg_label, &v853Font_Medialibrary_30, 0);
    lv_obj_set_style_text_color(msg_label, TM_TEXT_PRIMARY, 0);
    lv_obj_set_width(msg_label, LV_PCT(100));
    lv_obj_set_style_text_align(msg_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(msg_label, LV_ALIGN_TOP_MID, 0, LV_PCT(20));
    
    /* 创建确认按钮 */
    lv_obj_t *confirm_btn = lv_btn_create(handle->dialog_bg);
    lv_obj_set_size(confirm_btn, 100, 40);
    lv_obj_align(confirm_btn, LV_ALIGN_BOTTOM_LEFT, 20, -20);
    lv_obj_set_style_bg_color(confirm_btn, TM_SUCCESS, 0);
    lv_obj_set_style_radius(confirm_btn, 8, 0);
    
    lv_obj_t *confirm_label = lv_label_create(confirm_btn);
    lv_label_set_text(confirm_label, cfg->confirm_text ? cfg->confirm_text : "是");
    lv_obj_set_style_text_font(confirm_label, &v853Font_Medialibrary_30, 0);
    lv_obj_set_style_text_color(confirm_label, TM_TEXT_PRIMARY, 0);
    lv_obj_center(confirm_label);
    
    if (cfg->confirm_cb) {
        lv_obj_add_event_cb(confirm_btn, cfg->confirm_cb, LV_EVENT_CLICKED, cfg->user_data);
    }
    
    /* 创建取消按钮 */
    lv_obj_t *cancel_btn = lv_btn_create(handle->dialog_bg);
    lv_obj_set_size(cancel_btn, 100, 40);
    lv_obj_align(cancel_btn, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    lv_obj_set_style_bg_color(cancel_btn, TM_ERROR, 0);
    lv_obj_set_style_radius(cancel_btn, 8, 0);
    
    lv_obj_t *cancel_label = lv_label_create(cancel_btn);
    lv_label_set_text(cancel_label, cfg->cancel_text ? cfg->cancel_text : "否");
    lv_obj_set_style_text_font(cancel_label, &v853Font_Medialibrary_30, 0);
    lv_obj_set_style_text_color(cancel_label, TM_TEXT_PRIMARY, 0);
    lv_obj_center(cancel_label);
    
    if (cfg->cancel_cb) {
        lv_obj_add_event_cb(cancel_btn, cfg->cancel_cb, LV_EVENT_CLICKED, cfg->user_data);
    }
    
    return true;
}

void ui_close_dialog(ui_dialog_handle_t *handle) {
    if (!handle) return;
    
    if (handle->mask_layer) {
        lv_obj_del(handle->mask_layer);
        handle->mask_layer = NULL;
        handle->dialog_bg = NULL;  /* dialog_bg是mask_layer的子对象，会自动删除 */
    }
}

/* 结果对话框自动关闭定时器回调 */
static void result_dialog_timer_cb(lv_timer_t *timer) {
    lv_obj_t *dialog = (lv_obj_t *)timer->user_data;
    if (UI_OBJ_IS_VALID(dialog)) {
        lv_obj_del(dialog);
    }
    lv_timer_del(timer);
}

lv_obj_t* ui_create_result_dialog(lv_obj_t *parent, const char *message, 
                                  bool is_success, uint32_t auto_close_ms) {
    if (!parent) return NULL;
    
    /* 确保已初始化 */
    if (!g_ui_common_initialized) {
        ui_common_init();
    }
    
    /* 创建对话框背景 */
    lv_obj_t *dialog = lv_obj_create(parent);
    lv_obj_set_size(dialog, 300, 120);
    lv_obj_center(dialog);
    lv_obj_set_style_bg_color(dialog, is_success ? TM_SUCCESS : TM_ERROR, 0);
    lv_obj_set_style_radius(dialog, 10, 0);
    lv_obj_set_style_border_width(dialog, 0, 0);
    lv_obj_clear_flag(dialog, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_move_foreground(dialog);
    
    /* 创建消息标签 */
    lv_obj_t *label = lv_label_create(dialog);
    lv_label_set_text(label, message ? message : "");
    lv_obj_set_style_text_font(label, &v853Font_Medialibrary_30, 0);
    lv_obj_set_style_text_color(label, TM_TEXT_PRIMARY, 0);
    lv_obj_center(label);
    
    /* 设置自动关闭定时器 */
    if (auto_close_ms > 0) {
        lv_timer_t *timer = lv_timer_create(result_dialog_timer_cb, auto_close_ms, dialog);
        lv_timer_set_repeat_count(timer, 1);
    }
    
    return dialog;
}

/*====================================================================================
 * 通用样式设置函数实现
 *====================================================================================*/

void ui_apply_list_style(lv_obj_t *list) {
    if (!list) return;
    
    /* 确保已初始化 */
    if (!g_ui_common_initialized) {
        ui_common_init();
    }
    
    lv_obj_set_style_bg_color(list, TM_BG_PRIMARY, 0);
    lv_obj_set_style_bg_opa(list, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(list, TM_TEXT_PRIMARY, 0);
    lv_obj_set_style_border_width(list, 0, 0);
    lv_obj_set_style_radius(list, 0, 0);
    lv_obj_set_style_pad_bottom(list, 20, 0);
    lv_obj_set_style_pad_left(list, 15, 0);
    lv_obj_set_style_pad_right(list, 15, 0);
}

void ui_apply_title_bar_style(lv_obj_t *bar) {
    if (!bar) return;
    
    /* 确保已初始化 */
    if (!g_ui_common_initialized) {
        ui_common_init();
    }
    
    lv_obj_set_style_bg_color(bar, TM_BG_SECONDARY, 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(bar, 1, 0);
    lv_obj_set_style_border_color(bar, TM_BORDER_PRIMARY, 0);
    lv_obj_set_style_border_side(bar, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_radius(bar, 0, 0);
}

void ui_apply_transparent_btn_style(lv_obj_t *btn) {
    if (!btn) return;
    
    /* 确保已初始化 */
    if (!g_ui_common_initialized) {
        ui_common_init();
    }
    
    lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(btn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_bg_color(btn, TM_ACCENT, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn, LV_OPA_30, LV_STATE_PRESSED);
}

void ui_apply_tab_btn_style(lv_obj_t *btn, bool is_selected) {
    if (!btn) return;
    
    /* 确保已初始化 */
    if (!g_ui_common_initialized) {
        ui_common_init();
    }
    
    if (is_selected) {
        lv_obj_set_style_bg_opa(btn, LV_OPA_30, 0);
        lv_obj_set_style_bg_color(btn, TM_ACCENT, 0);
    } else {
        lv_obj_set_style_bg_opa(btn, LV_OPA_100, 0);
        lv_obj_set_style_bg_color(btn, TM_BG_SECONDARY, 0);
    }
    
    lv_obj_set_style_border_width(btn, 3, 0);
    lv_obj_set_style_border_opa(btn, LV_OPA_100, 0);
    lv_obj_set_style_border_color(btn, TM_BORDER_PRIMARY, 0);
    lv_obj_set_style_border_post(btn, true, 0);
    lv_obj_set_style_radius(btn, 0, 0);
}

void ui_apply_slider_style(lv_obj_t *slider) {
    if (!slider) return;
    
    /* 确保已初始化 */
    if (!g_ui_common_initialized) {
        ui_common_init();
    }
    
    lv_obj_set_style_bg_color(slider, TM_SLIDER_KNOB, LV_PART_KNOB);
    lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_KNOB);
    lv_obj_set_style_bg_color(slider, TM_PROGRESS_FG, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider, TM_PROGRESS_BG, 0);
    lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, 0);
    lv_obj_set_style_border_opa(slider, LV_OPA_0, 0);
    lv_obj_set_style_shadow_opa(slider, LV_OPA_0, 0);
    lv_obj_set_style_outline_opa(slider, LV_OPA_0, 0);
}

void ui_apply_label_style(lv_obj_t *label, bool is_primary) {
    if (!label) return;
    
    /* 确保已初始化 */
    if (!g_ui_common_initialized) {
        ui_common_init();
    }
    
    lv_obj_set_style_text_font(label, &v853Font_Medialibrary_30, 0);
    lv_obj_set_style_text_color(label, is_primary ? TM_TEXT_PRIMARY : TM_TEXT_SECONDARY, 0);
}
