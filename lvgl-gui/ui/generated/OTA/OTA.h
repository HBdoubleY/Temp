/**
 * @file OTA.h
 * @brief OTA升级模块头文件
 * 
 * 定义OTA页面UI结构体和接口函数
 */

#pragma once

#include "lvgl/lvgl.h"
#include "ota_theme.h"
#include "../i18n/i18n.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 字体资源声明 */
LV_FONT_DECLARE(v853Font_OTA_20)
LV_FONT_DECLARE(v853Font_OTA_26)
LV_FONT_DECLARE(v853Font_OTA_28)
LV_FONT_DECLARE(v853Font_OTA_30)
LV_FONT_DECLARE(v853Font_OTA_32)
LV_FONT_DECLARE(v853Font_OTA_36)
LV_FONT_DECLARE(v853Font_OTA_42)

/*============================================================================
 * 结构体定义
 *============================================================================*/

/**
 * @brief OTA页面UI元素结构体
 */
typedef struct {
    /* 主容器 */
    lv_obj_t *container;                    // 主容器
    lv_obj_t *check_label;                  // 检查更新按钮文字（主题刷新）
    
    /* 定时器 */
    lv_timer_t *unavailble_net_timer;       // 网络不可用提示定时器
    lv_timer_t *download_timer;             // 下载进度更新定时器
    lv_timer_t *upgrade_timer;              // 升级延时定时器
    
    /* 模态窗口 */
    lv_obj_t *modal_overlay;                // 当前模态窗口遮罩层
    lv_obj_t *confirm_yes_btn;              // 确认升级按钮（防重复点击）
    
    /* 下载UI元素 */
    lv_obj_t *download_bar;                 // 下载进度条 (0..100)
    lv_obj_t *download_label;               // 下载进度标签
    
    /* 下载状态（多线程共享） */
    volatile long download_total;           // 总字节数（来自Content-Length），-1表示未知
    volatile long download_done;            // 已下载字节数
    volatile int download_finished;         // 下载线程完成标志
    int download_result;                    // 下载结果: 0=失败, 1=成功

    /* 退出/反初始化控制 */
    volatile int stop_requested;            // 退出请求标志（用于终止后台线程/回调）
    volatile int download_thread_running;   // 下载线程运行中
    
    /* 代际，用于过滤旧timer/回调 */
    volatile uint32_t generation;
    
    /* 初始化状态 */
    bool is_initialized;                    // 是否已初始化
    
} ota_interface_t;

/* 全局OTA UI实例声明（定义在OTA.c中） */
extern ota_interface_t g_ota_ui;

/* OTA 模块语言设置（参考媒体库） */
extern language_t ota_language_setting;

/*============================================================================
 * 函数接口声明
 *============================================================================*/

/**
 * @brief 初始化OTA升级页面
 * @param parent 父对象
 */
void OTA_init(lv_obj_t *parent);

/**
 * @brief 销毁OTA升级页面
 */
void OTA_deinit(void);

/**
 * @brief 刷新OTA页面主题
 */
void OTA_refresh_theme(void);

#ifdef __cplusplus
}
#endif
