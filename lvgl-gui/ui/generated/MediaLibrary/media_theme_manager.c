/**
 * @file theme_manager.c
 * @brief 主题管理器实现
 * 
 * 实现黑夜/白天两套主题的颜色配置和切换逻辑
 */

#include "media_theme_manager.h"
#include <stdio.h>

/*====================================================================================
 * 私有变量
 *====================================================================================*/

/* 当前激活的主题类型 */
static theme_type_t g_current_theme = THEME_LIGHT;

/* 主题是否已初始化 */
static bool g_theme_initialized = false;

/*====================================================================================
 * 主题颜色配置定义
 *====================================================================================*/

/**
 * @brief 黑夜模式颜色配置
 * 
 * 设计理念：深色背景配合浅色文字，减少眼睛疲劳
 * 颜色层次：从深到浅 0x000000 -> 0x1A1A1A -> 0x2C2C2C -> 0x3C3C3C
 */
static const theme_colors_t g_theme_dark = {
    /* 背景颜色系列 - 从深到浅的灰色渐变 */
    .bg_primary     = 0x000000,   /* 纯黑 - 主背景 */
    .bg_secondary   = 0x1A1A1A,   /* 深灰 - 容器/卡片背景 */
    .bg_tertiary    = 0x2C2C2C,   /* 中灰 - 列表项背景 */
    .bg_hover       = 0x3C3C3C,   /* 浅灰 - 悬停/按下状态 */
    
    /* 文字颜色系列 */
    .text_primary   = 0xFFFFFF,   /* 白色 - 主要文字 */
    // .text_secondary = 0x888888,   /* 灰色 - 次要文字/描述 */
    .text_secondary = 0x000000,   /* 黑色 - 次要文字/描述 */
    .text_disabled  = 0x666666,   /* 暗灰 - 禁用状态 */
    
    /* 强调色系列 - 蓝色系 */
    .accent         = 0x2196F3,   /* Material Design蓝色 */
    .accent_pressed = 0x1976D2,   /* 深蓝 - 按下状态 */
    .accent_light   = 0x64B5F6,   /* 浅蓝 - 选中高亮 */
    
    /* 边框颜色系列 */
    .border_primary   = 0x333333, /* 主边框 */
    .border_secondary = 0x6F7188, /* 次要边框/分隔线 */
    
    /* 功能性颜色 */
    .success        = 0x4CAF50,   /* 绿色 - 成功 */
    .warning        = 0xFFC107,   /* 黄色 - 警告 */
    .error          = 0xF44336,   /* 红色 - 错误 */
    
    /* 进度条/滑块颜色 */
    .progress_bg    = 0x424242,   /* 进度条背景 */
    .progress_fg    = 0xFF5722,   /* 进度条前景 - 橙色 */
    .slider_knob    = 0xFFFFFF,   /* 滑块把手 - 白色 */
    
    /* 图标颜色 */
    .icon_primary   = 0xFFFFFF,   /* 主图标 - 白色 */
    .icon_secondary = 0xCCCCCC,   /* 次要图标 - 浅灰 */
    .icon_disabled  = 0x666666,   /* 禁用图标 - 暗灰 */
};

/**
 * @brief 白天模式颜色配置
 * 
 * 设计理念：浅色背景配合深色文字，适合明亮环境
 * 颜色层次：从浅到深 0xFFFFFF -> 0xF5F5F5 -> 0xE0E0E0 -> 0xBDBDBD
 */
static const theme_colors_t g_theme_light = {
    /*==================== 白天主题颜色配置 ====================*/
    /* 设计理念：高对比度、清晰可读、舒适护眼 */
    
    /* 背景颜色系列 - 从浅到深的灰色渐变 */
    .bg_primary     = 0xECEFF1,   /* 主背景 - 浅蓝灰（页面整体背景） */
    .bg_secondary   = 0xFFFFFF,   /* 次要背景 - 纯白（标题栏、卡片、弹窗背景） */
    .bg_tertiary    = 0xF5F5F5,   /* 第三级背景 - 浅灰（列表项背景） */
    .bg_hover       = 0xE3E3E3,   /* 悬停/按下背景 - 中灰（按钮按下状态） */
    
    /* 文字颜色系列 - 高对比度确保可读性 */
    .text_primary   = 0x1A1A1A,   /* 主文字 - 近似黑色（标题、文件名、重要内容） */
    .text_secondary = 0x616161,   /* 次要文字 - 深灰（描述、提示、已找到X个文件） */
    .text_disabled  = 0x9E9E9E,   /* 禁用文字 - 中灰（不可用状态） */
    
    /* 强调色系列 - 蓝色系（选中、激活状态） */
    .accent         = 0x1976D2,   /* 强调色 - 蓝色（标签选中、按钮按下） */
    .accent_pressed = 0x1565C0,   /* 强调色按下 - 深蓝 */
    .accent_light   = 0xBBDEFB,   /* 浅色强调 - 浅蓝（选中高亮背景） */
    
    /* 边框颜色系列 */
    .border_primary   = 0xBDBDBD, /* 主边框 - 中灰（卡片边框、列表分割线） */
    .border_secondary = 0xE0E0E0, /* 次要边框 - 浅灰（弹窗边框） */
    
    /* 功能性颜色 */
    .success        = 0x388E3C,   /* 成功 - 深绿色（确认按钮、成功提示） */
    .warning        = 0xF57C00,   /* 警告 - 深橙色 */
    .error          = 0xD32F2F,   /* 错误 - 深红色（取消按钮、失败提示） */
    
    /* 进度条/滑块颜色 */
    .progress_bg    = 0xE0E0E0,   /* 进度条背景 - 浅灰 */
    .progress_fg    = 0xFF5722,   /* 进度条前景 - 橙色 */
    .slider_knob    = 0x1976D2,   /* 滑块把手 - 蓝色 */
    
    /* 图标颜色 */
    .icon_primary   = 0x37474F,   /* 主图标 - 深灰蓝（列表图标、刷新按钮图标） */
    .icon_secondary = 0x78909C,   /* 次要图标 - 中灰蓝（删除按钮图标） */
    .icon_disabled  = 0xB0BEC5,   /* 禁用图标 - 浅灰蓝（按下状态） */
};

/* 主题配置数组，索引对应 theme_type_t */
static const theme_colors_t* g_themes[THEME_COUNT] = {
    &g_theme_dark,      /* THEME_MDARK */
    &g_theme_light,     /* THEME_LIGHT */
};

/*====================================================================================
 * 公共API实现
 *====================================================================================*/

/**
 * @brief 初始化主题管理器
 */
void theme_manager_init(void) {
    if (g_theme_initialized) {
        printf("[Theme] Already initialized, skipping\n");
        return;
    }
    
    /* 默认使用白天模式 */
    g_current_theme = THEME_LIGHT;
    g_theme_initialized = true;
    
    printf("[Theme] Theme manager initialized with LIGHT theme\n");
}

/**
 * @brief 切换主题
 */
bool theme_manager_set_theme(theme_type_t theme) {
    if (theme >= THEME_COUNT) {
        printf("[Theme] Error: Invalid theme type %d\n", theme);
        return false;
    }
    
    if (!g_theme_initialized) {
        theme_manager_init();
    }
    
    if (g_current_theme == theme) {
        printf("[Theme] Theme already set to %s\n", 
               theme == THEME_MDARK ? "DARK" : "LIGHT");
        return true;
    }
    
    g_current_theme = theme;
    
    printf("[Theme] Theme changed to %s\n", 
           theme == THEME_MDARK ? "DARK" : "LIGHT");
    
    return true;
}

/**
 * @brief 获取当前主题类型
 */
theme_type_t theme_manager_get_theme(void) {
    if (!g_theme_initialized) {
        theme_manager_init();
    }
    return g_current_theme;
}

/**
 * @brief 获取当前主题的颜色配置
 */
const theme_colors_t* theme_manager_get_colors(void) {
    if (!g_theme_initialized) {
        theme_manager_init();
    }
    return g_themes[g_current_theme];
}

/**
 * @brief 切换到下一个主题
 */
void theme_manager_toggle(void) {
    theme_type_t next_theme = (g_current_theme + 1) % THEME_COUNT;
    theme_manager_set_theme(next_theme);
}

/**
 * @brief 判断当前是否为深色主题
 */
bool theme_manager_is_dark(void) {
    if (!g_theme_initialized) {
        theme_manager_init();
    }
    return (g_current_theme == THEME_MDARK);
}
