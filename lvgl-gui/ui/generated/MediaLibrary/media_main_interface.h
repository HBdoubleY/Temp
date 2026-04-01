/**
 * @file playbackInterface.h
 * @brief 媒体库播放界面入口 - 管理视频库、照片库、存储空间三个子模块
 * 
 * 该模块是媒体库的主入口，通过标签页方式组织三个子功能模块。
 * 支持主题切换，所有颜色通过主题管理器获取。
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// #include "../lvgl-gui/lvgl/demos/lv_demos.h"
#include "../lvgl-gui/lvgl/lvgl.h"
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include "media_library_setting.h"
#include "media_theme_manager.h"
#include "media_ui_common.h"

/*====================================================================================
 * 配置宏定义
 *====================================================================================*/

#define PRESSINGBUGTEST 0       /* 按压bug测试开关 */
#define MEMTESTRSS 0            /* 内存测试开关 */

/*====================================================================================
 * 图片资源声明
 *====================================================================================*/

LV_FONT_DECLARE(v853Font_Medialibrary_30)
LV_IMG_DECLARE(file_tableview)              /* 存储标签图标 */
LV_IMG_DECLARE(photo_tableview)             /* 照片标签图标 */
LV_IMG_DECLARE(video_tableview)             /* 视频标签图标 */

/*====================================================================================
 * 类型定义
 *====================================================================================*/

/**
 * @brief 显示尺寸枚举（预留扩展）
 */
typedef enum {
    DISP_SMALL,
    DISP_MEDIUM,
    DISP_LARGE,
} disp_size_t;

/**
 * @brief 播放界面UI结构体
 * 
 * 管理主界面的所有UI组件指针
 */
typedef struct {
    lv_obj_t *pb_screen;                       /* 主屏幕容器 */

    lv_obj_t *rightTable;                      /* 右侧标签栏容器 */
    lv_obj_t *tab_btns;                        /* 标签栏按钮（保留兼容） */
    lv_obj_t *rightTable_content;              /* 标签页内容区（保留兼容） */
    lv_obj_t *back_btn;                        /* 退出按钮 */
    lv_obj_t *back_icon;                       /* 退出按钮图标 */
    lv_obj_t *temp;                            /* 对齐flex占位（保留兼容） */
    lv_obj_t *temp_icon;                       /* 占位图标（保留兼容） */

    lv_obj_t *btn_table_view_counter;          /* 右侧菜单栏容器 */
    lv_obj_t *tab_buttons[3];                  /* 三个标签按钮 */
    lv_obj_t *tab_contents[3];                 /* 三个内容页面 */

    lv_obj_t *photoPage;                       /* 照片库页面 */
    lv_obj_t *videoPage;                       /* 视频库页面 */
    lv_obj_t *storagePage;                     /* 存储空间页面 */
    
    bool is_initialized;                       /* 初始化标志 */
} playbackInterface_t;

/*====================================================================================
 * 全局变量声明
 *====================================================================================*/

/* 播放界面全局实例（在playbackInterface.c中定义） */
extern playbackInterface_t pb;

/*====================================================================================
 * 公共API
 *====================================================================================*/

/**
 * @brief 创建媒体库界面
 * 
 * 入口函数，创建并显示媒体库的主界面
 */
void lv_sdcard_widgets(lv_obj_t *parent);

/**
 * @brief 内存泄漏测试函数
 * 
 * @param count 测试循环次数
 */
void test_memory_leak_repeatedly(int count);

#ifdef __cplusplus
} /* extern "C" */
#endif
