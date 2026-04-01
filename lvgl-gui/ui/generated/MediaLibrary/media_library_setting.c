/**
 * @file setting.c
 * @brief 全局设置模块实现
 */

#include "media_library_setting.h"
#include <stdio.h>

/*====================================================================================
 * 全局变量定义
 *====================================================================================*/

/* 语言设置（默认中文） */
language_t languageSetting = LANG_CHINESE;

/* 主题设置（默认黑暗主题） */
theme_type_t themeSetting = THEME_LIGHT;

/*====================================================================================
 * 公共API实现
 *====================================================================================*/

/**
 * @brief 切换主题
 */
void setting_toggle_theme(void) {
    if (themeSetting == THEME_MDARK) {
        setting_set_theme(THEME_LIGHT);
    } else {
        setting_set_theme(THEME_MDARK);
    }
}

/**
 * @brief 设置主题
 */
void setting_set_theme(theme_type_t theme) {
    themeSetting = theme;
    theme_manager_set_theme(theme);
    printf("[Setting] Theme changed to %s\n", 
           theme == THEME_MDARK ? "Dark" : "Light");
}
