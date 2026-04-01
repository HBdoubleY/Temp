/**
 * @file storageLibrary.h
 * @brief 存储空间模块 - 管理SD卡存储空间显示和格式化
 * 
 * 提供SD卡容量显示、格式化等功能
 */

#pragma once

#include "../lvgl-gui/lvgl/lvgl.h"
#include "media_library_setting.h"
#include "media_theme_manager.h"
#include "media_ui_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 容量/挂载见 tf_card.h；FAT32+MBR 格式化见 runcarplay tf_card_format.c */

/*====================================================================================
 * 字体资源声明
 *====================================================================================*/

LV_FONT_DECLARE(v853Font_Medialibrary_30)
LV_FONT_DECLARE(v853Font_Medialibrary_40)

/*====================================================================================
 * 类型定义
 *====================================================================================*/

typedef struct {
    lv_obj_t *pb;               // 图库和视频库界面
    lv_obj_t *container;        // 主界面容器
    lv_obj_t *back_btn;
    lv_obj_t *back_icon;
    lv_obj_t *title_label;
    lv_obj_t *progress_bar;
    lv_obj_t *percent_label;
    lv_obj_t *details_label;
    lv_obj_t *format_btn;
    lv_obj_t *format_btn_icon;
} storageLibrary_t;

/**
 * @brief 格式化对话框结构体
 */
typedef struct {
    lv_obj_t *mask_layer;           /* 遮罩层 */
    lv_obj_t *dialog_bg;            /* 对话框背景 */
    lv_obj_t *msg_label;            /* 消息标签 */
    lv_obj_t *yes_btn;              /* 确认按钮 */
    lv_obj_t *no_btn;               /* 取消按钮 */
    lv_obj_t *result_dialog;        /* 结果对话框 */
    lv_timer_t *close_timer;        /* 自动关闭定时器 */
} storageLibrary_format_window_t;

/**
 * @brief 存储库主结构体
 */
typedef struct {
    storageLibrary_t sl;                    /* 主界面UI组件 */
    storageLibrary_format_window_t sf;      /* 格式化对话框UI组件 */
    bool is_initialized;                    /* 初始化标志 */
} storageLibrary_ctx_t;

/*====================================================================================
 * 全局变量声明
 *====================================================================================*/

/* 存储库全局实例（在storageLibrary.c中定义） */
extern storageLibrary_ctx_t g_storage;

// 入口函数
void create_storage_select_ui(lv_obj_t *parent);
// 回收资源
void cleanup_storage_library(void);

/** 仅更新 UI，由主线程在 poll 到 MEDIA_RESULT_STORAGE 后调用；used_mb/total_mb 为 -1 表示未挂载 */
void update_storage_display(int64_t used_mb, int64_t total_mb);
/** 请求刷新存储显示（提交 TASK_STORAGE_QUERY，结果通过 poll 后调用 update_storage_display） */
void reflush_storage_display(void);
/** 格式化完成时由主线程在 poll 到 MEDIA_RESULT_FORMAT 后调用：关对话框、显示结果并触发存储刷新；success 1=成功 0=失败 */
void storage_on_format_done(int success);

#ifdef __cplusplus
}
#endif