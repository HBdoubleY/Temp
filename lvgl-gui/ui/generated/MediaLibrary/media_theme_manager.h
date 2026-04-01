/**
 * @file theme_manager.h
 * @brief 主题管理器 - 支持黑夜/白天主题切换
 * 
 * 该模块提供统一的主题颜色管理，所有UI组件应通过此模块获取颜色配置，
 * 以支持运行时主题切换功能。
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "../lvgl-gui/lvgl/lvgl.h"

/*====================================================================================
 * 主题类型定义
 *====================================================================================*/

/**
 * @brief 主题类型枚举
 */
typedef enum {
    THEME_MDARK = 0,     /* 黑夜模式（默认） */
    THEME_LIGHT,        /* 白天模式 */
    THEME_COUNT         /* 主题总数 */
} theme_type_t;

/*====================================================================================
 * 主题颜色配置结构体
 *====================================================================================*/

/**
 * @brief 主题颜色配置
 * 
 * 定义了界面中所有需要根据主题变化的颜色
 * 注意：使用 uint32_t 存储十六进制颜色值，通过宏转换为 lv_color_t
 */
typedef struct {
    /* 背景颜色系列 */
    uint32_t bg_primary;          /* 主背景色 - 最深层背景 */
    uint32_t bg_secondary;        /* 次要背景色 - 卡片/容器背景 */
    uint32_t bg_tertiary;         /* 第三级背景色 - 列表项/按钮背景 */
    uint32_t bg_hover;            /* 悬停/按下状态背景色 */
    
    /* 文字颜色系列 */
    uint32_t text_primary;        /* 主文字颜色 - 标题/重要内容 */
    uint32_t text_secondary;      /* 次要文字颜色 - 描述/提示 */
    uint32_t text_disabled;       /* 禁用状态文字颜色 */
    
    /* 强调色系列 */
    uint32_t accent;              /* 强调色 - 选中/激活状态 */
    uint32_t accent_pressed;      /* 强调色按下状态 */
    uint32_t accent_light;        /* 浅色强调色 - 用于半透明效果 */
    
    /* 边框颜色系列 */
    uint32_t border_primary;      /* 主边框颜色 */
    uint32_t border_secondary;    /* 次要边框颜色 */
    
    /* 功能性颜色 */
    uint32_t success;             /* 成功状态颜色 - 绿色系 */
    uint32_t warning;             /* 警告状态颜色 - 黄色系 */
    uint32_t error;               /* 错误状态颜色 - 红色系 */
    
    /* 进度条/滑块颜色 */
    uint32_t progress_bg;         /* 进度条背景色 */
    uint32_t progress_fg;         /* 进度条前景色 */
    uint32_t slider_knob;         /* 滑块把手颜色 */
    
    /* 图标颜色 */
    uint32_t icon_primary;        /* 主图标颜色 */
    uint32_t icon_secondary;      /* 次要图标颜色 */
    uint32_t icon_disabled;       /* 禁用状态图标颜色 */
} theme_colors_t;

/*====================================================================================
 * 主题管理API
 *====================================================================================*/

/**
 * @brief 初始化主题管理器
 * 
 * 在程序启动时调用，加载默认主题（黑夜模式）
 */
void theme_manager_init(void);

/**
 * @brief 切换主题
 * 
 * @param theme 目标主题类型
 * @return true 切换成功
 * @return false 切换失败（无效的主题类型）
 */
bool theme_manager_set_theme(theme_type_t theme);

/**
 * @brief 获取当前主题类型
 * 
 * @return theme_type_t 当前激活的主题类型
 */
theme_type_t theme_manager_get_theme(void);

/**
 * @brief 获取当前主题的颜色配置
 * 
 * @return const theme_colors_t* 当前主题颜色配置的指针
 */
const theme_colors_t* theme_manager_get_colors(void);

/**
 * @brief 切换到下一个主题
 * 
 * 用于在主题之间循环切换
 */
void theme_manager_toggle(void);

/*====================================================================================
 * 便捷颜色获取宏
 * 
 * 使用这些宏可以更方便地获取主题颜色
 *====================================================================================*/

#define TM_COLOR(name)              lv_color_hex(theme_manager_get_colors()->name)
#define TM_BG_PRIMARY               TM_COLOR(bg_primary)
#define TM_BG_SECONDARY             TM_COLOR(bg_secondary)
#define TM_BG_TERTIARY              TM_COLOR(bg_tertiary)
#define TM_BG_HOVER                 TM_COLOR(bg_hover)
#define TM_BG_QUATERNARY            TM_COLOR(bg_hover)      /* 第四级背景色，与bg_hover相同 */
#define TM_TEXT_PRIMARY             TM_COLOR(text_primary)
#define TM_TEXT_SECONDARY           TM_COLOR(text_secondary)
#define TM_TEXT_DISABLED            TM_COLOR(text_disabled)
#define TM_ACCENT                   TM_COLOR(accent)
#define TM_ACCENT_PRESSED           TM_COLOR(accent_pressed)
#define TM_ACCENT_LIGHT             TM_COLOR(accent_light)
#define TM_BORDER_PRIMARY           TM_COLOR(border_primary)
#define TM_BORDER_SECONDARY         TM_COLOR(border_secondary)
#define TM_SUCCESS                  TM_COLOR(success)
#define TM_WARNING                  TM_COLOR(warning)
#define TM_ERROR                    TM_COLOR(error)
#define TM_PROGRESS_BG              TM_COLOR(progress_bg)
#define TM_PROGRESS_FG              TM_COLOR(progress_fg)
#define TM_SLIDER_KNOB              TM_COLOR(slider_knob)
#define TM_ICON_PRIMARY             TM_COLOR(icon_primary)
#define TM_ICON_SECONDARY           TM_COLOR(icon_secondary)
#define TM_ICON_DISABLED            TM_COLOR(icon_disabled)

/*====================================================================================
 * 主题是否为深色模式
 *====================================================================================*/

/**
 * @brief 判断当前是否为深色主题
 * 
 * @return true 当前为深色主题
 * @return false 当前为浅色主题
 */
bool theme_manager_is_dark(void);

#ifdef __cplusplus
}
#endif
