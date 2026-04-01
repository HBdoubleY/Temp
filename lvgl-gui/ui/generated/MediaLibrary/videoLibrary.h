/**
 * @file videoLibrary.h
 * @brief 视频库模块 - 管理视频文件的浏览和播放
 * 
 * 提供视频文件列表显示、播放控制、文件删除等功能
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
#include <stdbool.h>
#include "media_library_setting.h"
#include "media_theme_manager.h"
#include "media_ui_common.h"

// 目录路径
#define VIDEO_DIR_FRONT "/mnt/extsd/recorder/frontCamera"
#define VIDEO_DIR_REAR "/mnt/extsd/recorder/rearCamera"
#define VIDEO_DIR_URGENT_FRONT "/mnt/extsd/recorderUrgent/frontCamera"
#define VIDEO_DIR_URGENT_REAR "/mnt/extsd/recorderUrgent/rearCamera"
#define MAX_FILES   100
#define MAX_PATH_LEN    256
#define PROGRESSTIMEWIDTH   150
#define PROGRESSTIMEHEIGHT  30

// 声明字体库
LV_FONT_DECLARE(v853Font_Medialibrary_30)
LV_FONT_DECLARE(v853Font_Medialibrary_40)

// 声明图片
LV_IMG_DECLARE(VideoPlayback_FileLooping)       // 文件目录下循环
LV_IMG_DECLARE(VideoPlayback_notLooped)         // 不循环播放
LV_IMG_DECLARE(VideoPlayback_SingleVideoLoop)   // 单文件循环播放
LV_IMG_DECLARE(prev_white)                      // 上一个
LV_IMG_DECLARE(next_white)                      // 下一个
LV_IMG_DECLARE(pause_white)                     // 暂停
LV_IMG_DECLARE(play_white)                      // 播放
LV_IMG_DECLARE(VideoPlayback_Sound)             // 声音            
LV_IMG_DECLARE(back_white)                      // 返回
LV_IMG_DECLARE(Video_1X)                        // 1倍速
LV_IMG_DECLARE(Video_2X)                        // 2倍速
LV_IMG_DECLARE(delete)                          // 删除

typedef enum {
    TAB_FRONT = 0,
    TAB_REAR,
    TAB_URGENT,
    TAB_COUNT
} tab_type_t;

typedef struct {                    
    char *video_files[MAX_FILES];
    int video_file_count;
    lv_obj_t *video_list;
    lv_obj_t *title_label;
} video_tab_data_t;                                                                 // 标签页数据结构

typedef struct {
    lv_obj_t *cont;                                                                 // 主界面
    lv_obj_t *tabview;                                                              // 标签页
    lv_obj_t *tab_btns;                                                             // 标签按钮栏
    tab_type_t current_tab;                                                         // 当前激活的标签页
    video_tab_data_t tab_data[TAB_COUNT];

    lv_obj_t *mask_layer_del_dialog;                                                // 遮罩层 
} videoLibrary_main_t;

typedef enum {                                                                      // 播放模式枚举
    PLAY_MODE_SEQUENCE = 0,
    PLAY_MODE_SINGLE_LOOP,
    PLAY_MODE_SINGLE_STOP
} play_mode_t;

typedef enum {                                                                      // 播放速率枚举
    PLAY_SPEED_1X = 0,
    PLAY_SPEED_2X
} play_speed_t;

typedef struct {
    lv_timer_t *auto_hide_timer;                                                    // 自动显示/隐藏UI定时器
    lv_obj_t* video_screen;
    lv_obj_t *back_btn;                                                             // 退出播放界面的UI控件
    lv_obj_t *back_icon;
    lv_obj_t *container;                                                            // 播放功能UI控件主容器
    lv_obj_t * row1;
    lv_obj_t * row2;
    char current_buff[9];                                                           // 当前视频时间
    lv_obj_t *current_time_container;                                               // 左侧时间容器
    lv_color_t current_time_canvas_buf[PROGRESSTIMEWIDTH * PROGRESSTIMEHEIGHT];     // 根据文字大小调整
    lv_obj_t *current_time;
    lv_obj_t *total_time_container;                                                 // 右侧时间容器
    lv_color_t total_time_canvas_buf[PROGRESSTIMEWIDTH * PROGRESSTIMEHEIGHT];       // 根据文字大小调整
    lv_obj_t *total_time;
    lv_obj_t *slider1;
    lv_timer_t *progress_timer;                                                     // 更新视频进度的定时器
    bool is_slider_dragging;
    lv_obj_t *play_mode_btn;
    lv_obj_t *play_mode_icon;
    play_mode_t current_play_mode;                                                  // 默认文件循环播放模式
    int current_video_index;                                                        // 默认的视频序号
    lv_obj_t *prev_btn;                                                             // 切换播放文件
    lv_obj_t *prev_icon;
    lv_obj_t *next_btn;
    lv_obj_t *next_icon;
    lv_obj_t *pause_btn;                                                            // 暂停/播放
    lv_obj_t *pause_icon;
    lv_obj_t *play_icon;
    bool playPauseFlag;
    lv_obj_t *play_speed_btn;
    lv_obj_t *play_speed_icon;
    play_speed_t current_play_speed;                                                // 默认播放速率
    lv_obj_t *volume;                                                               // 播放声音
    lv_obj_t *volume_icon;
    lv_obj_t *volume_slider;
    lv_timer_t *auto_hide_volume_slider_timer;                                      // 自动隐藏音量进度条的定时器
    float current_volume;                                                           // 默认初始声音
} videoLibrary_videoViewer_t;

/**
 * @brief 视频库主结构体
 */
typedef struct {
    videoLibrary_main_t vm;                    /* 主界面UI组件 */
    videoLibrary_videoViewer_t vv;             /* 播放界面UI组件 */
    bool is_initialized;                       /* 初始化标志 */
} videoLibrary_t;

/*====================================================================================
 * 全局变量声明
 *====================================================================================*/

/* 视频库全局实例（在videoLibrary.c中定义） */
extern videoLibrary_t vl;

/*====================================================================================
 * 公共API
 *====================================================================================*/

/**
 * @brief 创建视频选择界面
 * @param parent 父容器
 */
void create_video_select_ui(lv_obj_t * parent);

/**
 * @brief 清理视频库资源
 */
void cleanup_video_library(void);

void reflush_video_library(void);

/** 无卡时由主线程调用：清空所有 tab 列表并刷新 UI，不访问磁盘 */
void video_library_clear_all(void);

/** 主线程在 poll 到 MEDIA_RESULT_SCAN_VIDEO 后调用：写入 tab 数据并刷新列表；paths 由调用方在调用后通过 media_work_free_result 释放 */
void video_library_apply_scan_result(int tab_index, char **paths, int count);

#ifdef __cplusplus
}
#endif
