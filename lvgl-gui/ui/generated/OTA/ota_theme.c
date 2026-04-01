/**
 * @file theme.c
 * @brief 主题系统实现 - 白天/黑夜两套UI风格
 */

#include "ota_theme.h"
#include <stdio.h>

/*============================================================================
 * 白天主题配色方案（浅色主题）
 * 设计风格：清新、明亮、专业
 *============================================================================*/
static const ota_theme_colors_t theme_day = {
    /* 基础背景色 - 浅灰白色系 */
    .bg_main        = 0xF7F9FB,     // 主背景：浅灰蓝
    .bg_card        = 0xFFFFFF,     // 卡片背景：纯白
    .bg_sidebar     = 0xFFFFFF,     // 侧边栏：纯白
    .bg_list        = 0xFFFFFF,     // 列表背景：纯白
    .bg_input       = 0xFFFFFF,     // 输入框背景：纯白
    .bg_modal       = 0xFFFFFF,     // 模态窗口背景：纯白
    .bg_overlay     = 0x000000,     // 遮罩层：黑色（配合透明度）
    
    /* 文字颜色 - 深色系 */
    .text_primary   = 0x0F1724,     // 主要文字：深蓝黑
    .text_secondary = 0x374151,     // 次要文字：中灰
    .text_hint      = 0x94A3B8,     // 提示文字：浅灰
    .text_inverse   = 0xFFFFFF,     // 反色文字：白色
    
    /* 主题色 - 蓝色系 */
    .primary        = 0x2196F3,     // 主色调：Material蓝
    .primary_light  = 0xE3F2FD,     // 浅蓝背景
    .primary_dark   = 0x1E40AF,     // 深蓝（按钮）
    .accent         = 0x4A90D9,     // 强调蓝
    
    /* 状态颜色 */
    .success        = 0x10B981,     // 成功：翠绿
    .success_bg     = 0xE8F5E9,     // 成功背景：浅绿
    .error          = 0xEF4444,     // 错误：亮红
    .error_bg       = 0xFFEBEE,     // 错误背景：浅红
    .warning        = 0xF59E0B,     // 警告：橙色
    .warning_bg     = 0xFFF8E1,     // 警告背景：浅黄
    .info           = 0x3B82F6,     // 信息：蓝色
    
    /* 边框和分隔线 */
    .border         = 0xE6E9F0,     // 普通边框：浅灰
    .border_light   = 0xF1F5F9,     // 浅色边框
    .divider        = 0xE6E6E6,     // 分隔线
    
    /* 按钮颜色 */
    .btn_exit       = 0xDC2626,     // 退出按钮：红色
    .btn_tab_active = 0x2196F3,     // 激活标签：蓝色
    .btn_tab_inactive = 0xDDDDDD,   // 未激活标签：浅灰
    .btn_pressed    = 0xE6F3FF,     // 按下状态：浅蓝
    
    /* 进度条颜色 */
    .progress_bg    = 0xE8EEF5,     // 进度条背景：浅灰
    .progress_fill  = 0x4A90D9,     // 进度条填充：蓝色
    
    /* 阴影 */
    .shadow         = 0x000000,     // 阴影颜色：黑色
    .shadow_opa     = LV_OPA_20,    // 阴影透明度：20%
    
    /* 遮罩透明度 */
    .overlay_opa    = LV_OPA_50,    // 遮罩透明度：50%
};

/*============================================================================
 * 黑夜主题配色方案（深色主题）
 * 设计风格：沉稳、护眼、现代
 *============================================================================*/
static const ota_theme_colors_t theme_night = {
    /* 基础背景色 - 深灰蓝色系 */
    .bg_main        = 0x121827,     // 主背景：深蓝黑
    .bg_card        = 0x1E293B,     // 卡片背景：深灰蓝
    .bg_sidebar     = 0x1E293B,     // 侧边栏：深灰蓝
    .bg_list        = 0x1E293B,     // 列表背景：深灰蓝
    .bg_input       = 0x334155,     // 输入框背景：中灰蓝
    .bg_modal       = 0x1E293B,     // 模态窗口背景：深灰蓝
    .bg_overlay     = 0x000000,     // 遮罩层：黑色
    
    /* 文字颜色 - 浅色系 */
    .text_primary   = 0xF1F5F9,     // 主要文字：亮灰白
    .text_secondary = 0xCBD5E1,     // 次要文字：浅灰
    .text_hint      = 0x64748B,     // 提示文字：中灰
    .text_inverse   = 0x0F1724,     // 反色文字：深色
    
    /* 主题色 - 蓝色系（稍微调亮以在深色背景上更明显） */
    .primary        = 0x3B82F6,     // 主色调：亮蓝
    .primary_light  = 0x1E3A5F,     // 浅蓝背景（深色版本）
    .primary_dark   = 0x2563EB,     // 深蓝
    .accent         = 0x60A5FA,     // 强调蓝（更亮）
    
    /* 状态颜色（稍微调亮） */
    .success        = 0x22C55E,     // 成功：亮绿
    .success_bg     = 0x14532D,     // 成功背景：深绿
    .error          = 0xF87171,     // 错误：亮红
    .error_bg       = 0x7F1D1D,     // 错误背景：深红
    .warning        = 0xFBBF24,     // 警告：亮橙
    .warning_bg     = 0x78350F,     // 警告背景：深橙
    .info           = 0x60A5FA,     // 信息：亮蓝
    
    /* 边框和分隔线 */
    .border         = 0x334155,     // 普通边框：中灰
    .border_light   = 0x475569,     // 浅色边框（相对深色背景）
    .divider        = 0x334155,     // 分隔线
    
    /* 按钮颜色 */
    .btn_exit       = 0xDC2626,     // 退出按钮：红色（保持不变）
    .btn_tab_active = 0x3B82F6,     // 激活标签：亮蓝
    .btn_tab_inactive = 0x334155,   // 未激活标签：中灰
    .btn_pressed    = 0x1E3A5F,     // 按下状态：深蓝
    
    /* 进度条颜色 */
    .progress_bg    = 0x334155,     // 进度条背景：中灰
    .progress_fill  = 0x3B82F6,     // 进度条填充：亮蓝
    
    /* 阴影 */
    .shadow         = 0x000000,     // 阴影颜色：黑色
    .shadow_opa     = LV_OPA_40,    // 阴影透明度：40%（深色背景需要更重的阴影）
    
    /* 遮罩透明度 */
    .overlay_opa    = LV_OPA_60,    // 遮罩透明度：60%
};

/*============================================================================
 * 全局变量定义
 *============================================================================*/

/* 当前主题类型 */
ota_theme_type_t g_current_theme = THEME_OTA_LIGHT;

/* 当前主题颜色指针 */
const ota_theme_colors_t *g_theme = &theme_day;

/*============================================================================
 * 函数实现
 *============================================================================*/

/**
 * @brief 初始化主题系统
 * @param type 初始主题类型
 */
void theme_init(ota_theme_type_t type) {
    printf("[theme] Initializing theme system, type=%d\n", type);
    theme_set(type);
}

/**
 * @brief 设置/切换主题
 * @param type 目标主题类型
 */
void theme_set(ota_theme_type_t type) {
    g_current_theme = type;

    switch (type) {
        case THEME_OTA_NIGHT:
            g_theme = &theme_night;
            printf("[theme] Theme set to NIGHT mode\n");
            break;
        case THEME_OTA_LIGHT:
        default:
            g_theme = &theme_day;
            printf("[theme] Theme set to DAY mode\n");
            break;
    }
}

/**
 * @brief 切换到另一个主题（白天<->黑夜）
 */
void theme_toggle(void) {
    if (g_current_theme == THEME_OTA_LIGHT) {
        theme_set(THEME_OTA_NIGHT);
    } else {
        theme_set(THEME_OTA_LIGHT);
    }
    printf("[theme] Theme toggled to %s mode\n", 
           g_current_theme == THEME_OTA_LIGHT ? "DAY" : "NIGHT");
}

/**
 * @brief 获取当前主题类型
 * @return 当前主题类型
 */
ota_theme_type_t theme_get_type(void) {
    return g_current_theme;
}

/**
 * @brief 获取当前主题颜色配置
 * @return 当前主题颜色结构体指针
 */
const ota_theme_colors_t* theme_get_colors(void) {
    return g_theme;
}
