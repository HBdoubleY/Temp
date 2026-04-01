/**
 * @file MainInterface.h
 * @brief OTA主界面头文件
 * 
 * 定义主界面结构体和接口函数
 */

#pragma once

#include "lvgl/lvgl.h"
#include "ota_theme.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 字体和图片资源声明 */
LV_FONT_DECLARE(v853Font_OTA_42)
LV_IMG_DECLARE(Wifi_table)
LV_IMG_DECLARE(OTA_table)

/*============================================================================
 * 主界面结构体定义
 *============================================================================*/

/**
 * @brief 主界面UI元素结构体
 */
typedef struct {
    lv_obj_t *wifi_ota_screen;              // 主屏幕对象
    
    /* 左侧标签栏 */
    lv_obj_t *leftTable;                    // 左侧标签栏容器
    lv_obj_t *left_btn_container;           // 左侧标签按钮容器
    lv_obj_t *exit_btn;                     // 退出按钮
    lv_obj_t *exit_icon;                    // 退出按钮图标
    lv_obj_t *left_tab_buttons[2];          // 左侧两个标签按钮（WiFi/OTA）
    lv_obj_t *tab_contents[2];              // 对应的两个内容页
    lv_obj_t *tab_icons[2];                 // 标签页图标
    
    /* 初始化状态标记 */
    bool is_initialized;                    // 是否已初始化
    int current_tab;                        // 当前选中的标签页索引
    
} wifi_ota_t;

/* 全局主界面实例声明（定义在MainInterface.c中） */
extern wifi_ota_t g_main_ui;

/*============================================================================
 * 函数接口声明
 *============================================================================*/

/**
 * @brief 初始化主界面
 * @param parent 父对象（通常是屏幕）
 * 
 * 创建左侧标签栏和右侧内容区域，初始化WiFi和OTA子页面
 */
void MainInterface_init(lv_obj_t *parent);

/**
 * @brief 销毁主界面
 * 
 * 释放所有UI资源，清理子模块
 */
void MainInterface_deinit(void);

/**
 * @brief 刷新主界面主题
 * 
 * 当主题切换时调用，更新所有UI元素的颜色
 */
void MainInterface_refresh_theme(void);

/**
 * @brief 设置退出按钮回调
 * @param cb 回调函数
 * @param user_data 用户数据
 */
void MainInterface_set_exit_cb(lv_event_cb_t cb, void *user_data);

#ifdef __cplusplus
}
#endif
