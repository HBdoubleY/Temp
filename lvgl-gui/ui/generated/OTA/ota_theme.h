/**
 * @file theme.h
 * @brief 主题系统头文件 - 支持白天/黑夜两套UI风格
 * 
 * 提供统一的颜色、尺寸定义，方便全局主题切换
 */

#pragma once

#include "lvgl/lvgl.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * 主题类型定义
 *============================================================================*/

/**
 * @brief 主题类型枚举
 */
typedef enum {
    THEME_OTA_LIGHT = 0,    // 白天模式（浅色主题）
    THEME_OTA_NIGHT = 1   // 黑夜模式（深色主题）
} ota_theme_type_t;

/*============================================================================
 * 颜色定义结构体
 *============================================================================*/

/**
 * @brief 主题颜色配置结构体
 */
typedef struct {
    /* 基础背景色 */
    uint32_t bg_main;           // 主背景色
    uint32_t bg_card;           // 卡片背景色
    uint32_t bg_sidebar;        // 侧边栏背景色
    uint32_t bg_list;           // 列表背景色
    uint32_t bg_input;          // 输入框背景色
    uint32_t bg_modal;          // 模态窗口背景色
    uint32_t bg_overlay;        // 遮罩层颜色
    
    /* 文字颜色 */
    uint32_t text_primary;      // 主要文字颜色
    uint32_t text_secondary;    // 次要文字颜色
    uint32_t text_hint;         // 提示文字颜色
    uint32_t text_inverse;      // 反色文字（用于深色背景）
    
    /* 主题色/强调色 */
    uint32_t primary;           // 主色调（蓝色系）
    uint32_t primary_light;     // 主色调浅色
    uint32_t primary_dark;      // 主色调深色
    uint32_t accent;            // 强调色
    
    /* 状态颜色 */
    uint32_t success;           // 成功/确认（绿色）
    uint32_t success_bg;        // 成功背景色
    uint32_t error;             // 错误/取消（红色）
    uint32_t error_bg;          // 错误背景色
    uint32_t warning;           // 警告（橙色）
    uint32_t warning_bg;        // 警告背景色
    uint32_t info;              // 信息（蓝色）
    
    /* 边框和分隔线 */
    uint32_t border;            // 普通边框色
    uint32_t border_light;      // 浅色边框
    uint32_t divider;           // 分隔线颜色
    
    /* 按钮特殊颜色 */
    uint32_t btn_exit;          // 退出按钮颜色
    uint32_t btn_tab_active;    // 激活的标签按钮
    uint32_t btn_tab_inactive;  // 未激活的标签按钮
    uint32_t btn_pressed;       // 按钮按下状态
    
    /* 进度条颜色 */
    uint32_t progress_bg;       // 进度条背景
    uint32_t progress_fill;     // 进度条填充
    
    /* 阴影颜色 */
    uint32_t shadow;            // 阴影颜色
    lv_opa_t shadow_opa;        // 阴影透明度
    
    /* 遮罩透明度 */
    lv_opa_t overlay_opa;       // 模态遮罩透明度
    
} ota_theme_colors_t;

/*============================================================================
 * 尺寸定义
 *============================================================================*/

/* 屏幕尺寸 */
#define SCREEN_WIDTH            1440
#define SCREEN_HEIGHT           720

/* 侧边栏尺寸 */
#define SIDEBAR_WIDTH           120
#define SIDEBAR_HEIGHT          SCREEN_HEIGHT
#define EXIT_BTN_HEIGHT         60
#define TAB_BTN_HEIGHT          330

/* 内容区域尺寸 */
#define CONTENT_WIDTH           (SCREEN_WIDTH - SIDEBAR_WIDTH)
#define CONTENT_HEIGHT          SCREEN_HEIGHT

/* 通用间距 */
#define PADDING_XS              4
#define PADDING_SM              8
#define PADDING_MD              16
#define PADDING_LG              24
#define PADDING_XL              32

/* 圆角 */
#define RADIUS_SM               6
#define RADIUS_MD               10
#define RADIUS_LG               16
#define RADIUS_XL               20
#define RADIUS_ROUND            999

/* 边框宽度 */
#define BORDER_WIDTH_THIN       1
#define BORDER_WIDTH_NORMAL     2
#define BORDER_WIDTH_THICK      3
#define BORDER_WIDTH_BOLD       4

/* 阴影宽度 */
#define SHADOW_WIDTH_SM         8
#define SHADOW_WIDTH_MD         16
#define SHADOW_WIDTH_LG         30

/* WiFi列表容器尺寸 */
#define WIFI_LIST_WIDTH         1180
#define WIFI_LIST_HEIGHT        500

/* 弹窗尺寸 */
#define MODAL_WIDTH_SM          400
#define MODAL_WIDTH_MD          480
#define MODAL_WIDTH_LG          920
#define MODAL_HEIGHT_SM         240
#define MODAL_HEIGHT_MD         280
#define MODAL_HEIGHT_LG         300

/* 按钮尺寸 */
#define BTN_WIDTH_SM            120
#define BTN_WIDTH_MD            160
#define BTN_WIDTH_LG            200
#define BTN_WIDTH_XL            420
#define BTN_HEIGHT_SM           60
#define BTN_HEIGHT_MD           50
#define BTN_HEIGHT_LG           64
#define BTN_HEIGHT_XL           110

/* 键盘尺寸 */
#define KEYBOARD_WIDTH_PCT      80
#define KEYBOARD_HEIGHT_PCT     50

/*============================================================================
 * 全局主题状态（外部访问）
 *============================================================================*/

extern ota_theme_type_t g_current_theme;
extern const ota_theme_colors_t *g_theme;

/*============================================================================
 * 函数接口声明
 *============================================================================*/

/**
 * @brief 初始化主题系统
 * @param type 初始主题类型
 */
void theme_init(ota_theme_type_t type);

/**
 * @brief 设置/切换主题
 * @param type 目标主题类型
 */
void theme_set(ota_theme_type_t type);

/**
 * @brief 切换到另一个主题（白天<->黑夜）
 */
void theme_toggle(void);

/**
 * @brief 获取当前主题类型
 * @return 当前主题类型
 */
ota_theme_type_t theme_get_type(void);

/**
 * @brief 获取当前主题颜色配置
 * @return 当前主题颜色结构体指针
 */
const ota_theme_colors_t* theme_get_colors(void);

/**
 * @brief 获取lv_color格式的颜色
 * @param hex_color 十六进制颜色值
 * @return lv_color_t格式颜色
 */
static inline lv_color_t theme_color(uint32_t hex_color) {
    return lv_color_hex(hex_color);
}

/*============================================================================
 * 便捷宏定义 - 快速获取当前主题颜色
 *============================================================================*/

/* 背景色 */
#define TH_BG_MAIN          theme_color(g_theme->bg_main)
#define TH_BG_CARD          theme_color(g_theme->bg_card)
#define TH_BG_SIDEBAR       theme_color(g_theme->bg_sidebar)
#define TH_BG_LIST          theme_color(g_theme->bg_list)
#define TH_BG_INPUT         theme_color(g_theme->bg_input)
#define TH_BG_MODAL         theme_color(g_theme->bg_modal)
#define TH_BG_OVERLAY       theme_color(g_theme->bg_overlay)

/* 文字颜色 */
#define TH_TEXT_PRIMARY     theme_color(g_theme->text_primary)
#define TH_TEXT_SECONDARY   theme_color(g_theme->text_secondary)
#define TH_TEXT_HINT        theme_color(g_theme->text_hint)
#define TH_TEXT_INVERSE     theme_color(g_theme->text_inverse)

/* 主题色 */
#define TH_PRIMARY          theme_color(g_theme->primary)
#define TH_PRIMARY_LIGHT    theme_color(g_theme->primary_light)
#define TH_PRIMARY_DARK     theme_color(g_theme->primary_dark)
#define TH_ACCENT           theme_color(g_theme->accent)

/* 状态颜色 */
#define TH_SUCCESS          theme_color(g_theme->success)
#define TH_SUCCESS_BG       theme_color(g_theme->success_bg)
#define TH_ERROR            theme_color(g_theme->error)
#define TH_ERROR_BG         theme_color(g_theme->error_bg)
#define TH_WARNING          theme_color(g_theme->warning)
#define TH_WARNING_BG       theme_color(g_theme->warning_bg)
#define TH_INFO             theme_color(g_theme->info)

/* 边框颜色 */
#define TH_BORDER           theme_color(g_theme->border)
#define TH_BORDER_LIGHT     theme_color(g_theme->border_light)
#define TH_DIVIDER          theme_color(g_theme->divider)

/* 按钮颜色 */
#define TH_BTN_EXIT         theme_color(g_theme->btn_exit)
#define TH_BTN_TAB_ACTIVE   theme_color(g_theme->btn_tab_active)
#define TH_BTN_TAB_INACTIVE theme_color(g_theme->btn_tab_inactive)
#define TH_BTN_PRESSED      theme_color(g_theme->btn_pressed)

/* 进度条颜色 */
#define TH_PROGRESS_BG      theme_color(g_theme->progress_bg)
#define TH_PROGRESS_FILL    theme_color(g_theme->progress_fill)

/* 阴影颜色 */
#define TH_SHADOW           theme_color(g_theme->shadow)
#define TH_SHADOW_OPA       g_theme->shadow_opa
#define TH_OVERLAY_OPA      g_theme->overlay_opa

#ifdef __cplusplus
}
#endif
