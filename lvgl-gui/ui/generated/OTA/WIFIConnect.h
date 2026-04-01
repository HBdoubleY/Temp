/**
 * @file WIFIConnect.h
 * @brief WiFi连接模块头文件
 * 
 * 定义WiFi页面UI结构体和接口函数
 */

#pragma once

#include "lvgl/lvgl.h"
#include "ota_theme.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 字体和图片资源声明 */
LV_FONT_DECLARE(v853Font_OTA_16)
LV_FONT_DECLARE(v853Font_OTA_18)
LV_FONT_DECLARE(v853Font_OTA_20)
LV_FONT_DECLARE(v853Font_OTA_24)
LV_FONT_DECLARE(v853Font_OTA_28)
LV_FONT_DECLARE(v853Font_OTA_36)
LV_FONT_DECLARE(v853Font_OTA_42)
LV_IMG_DECLARE(wifi_sw_off)
LV_IMG_DECLARE(wifi_sw_on)

/*============================================================================
 * 结构体定义
 *============================================================================*/

/**
 * @brief WiFi连接参数结构体
 */
struct connect_arg {
    char *ssid;     // WiFi SSID
    char *psk;      // WiFi 密码
};

/**
 * @brief WiFi页面UI元素结构体
 */
typedef struct {
    /* 主卡片容器 */
    lv_obj_t *card;                         // 主卡片容器
    
    /* WiFi 开关和控制按钮 */
    lv_obj_t *connect_btn;                  // WiFi开关按钮
    lv_obj_t *connect_btn_yes;              // 开启状态图标
    lv_obj_t *connect_btn_no;               // 关闭状态图标
    bool currently_connected;               // WiFi“开关”是否为开启状态（控制列表与连接）

    /* 当前连接状态显示 */
    lv_obj_t *current_ssid_label;           // 当前已连接 WiFi 名称（或“未连接”）

    /* 刷新按钮 */
    lv_obj_t *refresh_btn;                  // 刷新网络列表按钮
    lv_obj_t *refresh_img;                  // 刷新按钮图标

    /* WiFi列表 */
    lv_obj_t *list_cont;                    // WiFi列表容器  
    lv_obj_t *list;                         // WiFi网络列表对象

    /* 连接对话框 */
    lv_obj_t *mask_layer_dialog;            // 对话框遮罩层   
    lv_obj_t *dialog_bg;                    // 对话框背景

    /* 密码输入 */
    lv_obj_t *ssid_label;                   // SSID显示标签
    lv_obj_t *password_ta;                  // 密码输入框

    /* 键盘 */
    lv_obj_t *kb;                           // 密码输入键盘
    
    /* 初始化状态 */
    bool is_initialized;                    // 是否已初始化
    
} wifi_t;

/* 全局WiFi UI实例声明（定义在WIFIConnect.c中） */
extern wifi_t g_wifi_ui;

/*============================================================================
 * 函数接口声明
 *============================================================================*/

/**
 * @brief 初始化WiFi连接页面
 * @param parent 父对象
 */
void WIFIConnect_init(lv_obj_t *parent);

/**
 * @brief 销毁WiFi连接页面
 */
void WIFIConnect_deinit(void);

/**
 * @brief 刷新WiFi网络列表
 */
void WIFIConnect_refresh_list(void);

/**
 * @brief 刷新WiFi页面主题
 */
void WIFIConnect_refresh_theme(void);

/**
 * @brief 获取当前 WiFi 连接状态（缓存值，非阻塞，供主界面等使用）
 * @return 已连接返回 true，否则返回 false
 */
bool WIFIConnect_is_connected_cached(void);

/**
 * @brief 启动后台 WiFi 状态轮询线程（在非 UI 线程中轮询，避免阻塞 LVGL）
 * 应在应用启动时调用一次；重复调用会忽略。
 */
void WIFIConnect_start_status_poll(void);

#ifdef __cplusplus
}
#endif
