/**
 * @file photoLibrary.h
 * @brief 照片库模块 - 管理照片文件的浏览和查看
 * 
 * 提供照片文件列表显示、图片查看、缩放、旋转、删除等功能
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

#define DEFAULT_PHOTO_DIR "/mnt/extsd/DVRpic"    /* 默认照片目录（兼容） */
#define PHOTO_DIR_FRONT   "/mnt/extsd/DVRpic/frontPic"  /* 前置照片 */
#define PHOTO_DIR_REAR    "/mnt/extsd/DVRpic/rearPic"   /* 后置照片 */
#define GESTURECHANGEENBALE 0                      /* 手势切换开关 */
#define IMGORIGINX  115                            /* 图片原点X */
#define IMGORIGINY  50                             /* 图片原点Y */

/*====================================================================================
 * 字体和图片资源声明
 *====================================================================================*/

LV_FONT_DECLARE(v853Font_Medialibrary_30)
LV_FONT_DECLARE(v853Font_Medialibrary_40)

LV_IMG_DECLARE(Photo_viewer_back)               /* 返回 */
LV_IMG_DECLARE(Photo_viewer_small)              /* 缩小 */
LV_IMG_DECLARE(Photo_viewer_big)                /* 放大 */
LV_IMG_DECLARE(Photo_viewer_prev)               /* 上一张 */
LV_IMG_DECLARE(Photo_viewer_next)               /* 下一张 */
LV_IMG_DECLARE(Photo_viewer_rota)               /* 旋转 */
LV_IMG_DECLARE(Photo_viewer_reset)              /* 重置 */
LV_IMG_DECLARE(delete)                          /* 删除 */

/*====================================================================================
 * 类型定义
 *====================================================================================*/

#define PHOTO_TAB_FRONT  0
#define PHOTO_TAB_REAR   1
#define PHOTO_TAB_COUNT  2

/** 单个标签页数据（与视频库一致：每 tab 独立列表与标题） */
typedef struct {
    char **photo_paths;
    int photo_count;
    int photo_capacity;
    lv_obj_t *list;
    lv_obj_t *title_label;
} photo_tab_data_t;

typedef struct  {
    lv_obj_t *cont;                     // 主容器（全屏）
    lv_obj_t *tabview;                   // 标签页（与视频库一致）
    lv_obj_t *tab_btns;                  // 标签按钮栏
    int current_photo_tab;               // 当前标签 0=前置 1=后置
    photo_tab_data_t tab_data[PHOTO_TAB_COUNT];

    lv_style_t style_btn;                // 按钮样式（刷新等）

    /* 查看器用：当前 tab 的路径/数量/索引，与 photo_dir 一致 */
    char **photo_paths;                  // 指向当前 tab 的 photo_paths（查看器用）
    int current_index;                   // 当前查看的照片索引
    int photo_count;                     // 当前 tab 照片数量（查看器用）
    char photo_dir[256];                 // 当前照片目录（查看器用相对路径）

    lv_obj_t *mask_layer_del_dialog;     // 遮罩层
} photoLibrary_main_t;

typedef enum {
    PERCENT50 = 1,
    PERCENT75,
    PERCENT100,
    PERCENT150,
    PERCENT200
} CANVAS_PERCENT;

typedef struct {
    lv_obj_t *cont;                     // 查看容器
    lv_obj_t *top_bar;                  // 创建查看模式顶部栏
    lv_obj_t *back_btn;                 // 返回按钮
    lv_obj_t *back_icon;        
    lv_obj_t *file_label;               // 文件名标签
    lv_obj_t *nav_btn_cont;             // 创建上一张/下一张按钮容器
    lv_obj_t *prev_btn;                 // 上一张按钮
    lv_obj_t *prev_icon;
    lv_obj_t *next_btn;                 // 下一张按钮
    lv_obj_t *next_icon;
    lv_obj_t *img_wrapper;              // 图片容器

    lv_obj_t *img;                      // 图片显示对象Canvas
    // 缓冲区管理
    void *current_canvas_buf;           // 当前canvas缓冲区
    size_t current_buf_size;            // 当前缓冲区大小
    const char *cached_img_path;        // 当前缓存的图片路径

    lv_obj_t *bottom_bar;               // 创建底部控制栏
    lv_obj_t *small_btn;                // 缩小按钮
    lv_obj_t *small_btn_icon;
    lv_obj_t *big_btn;                  // 放大按钮
    lv_obj_t *big_btn_icon;
    lv_obj_t *rota_btn;                 // 旋转按钮
    lv_obj_t *rota_btn_icon;
    lv_obj_t *reset_btn;                // 重置按钮
    lv_obj_t *reset_btn_icon;           // 重置按钮
    lv_obj_t *zoom_label;               // 缩放标签

    int has_photo_cache;                // 有图片缓存
    CANVAS_PERCENT current_zoom_value;  // 当前缩放数值

    int offset_x;                       // X轴偏移
    int offset_y;                       // Y轴偏移
    int is_dragging;                    // 是否正在拖动
    int drag_start_x;                   // 拖动起始X
    int drag_start_y;                   // 拖动起始Y
    int img_start_x;                    // 图片起始X
    int img_start_y;                    // 图片起始Y

    // 手势相关
    int gesture_start_x;                // 手势起始X
    int gesture_start_y;                // 手势起始Y
    uint32_t gesture_start_time;        // 手势开始时间
    int is_gesture_active;              // 手势是否激活
} photoLibrary_photoViewer_t;

/**
 * @brief 照片库主结构体
 */
typedef struct {
    photoLibrary_main_t pm;             /* 主界面UI组件 */
    photoLibrary_photoViewer_t pv;      /* 图片查看界面UI组件 */
    bool is_initialized;                /* 初始化标志 */
    
    /* 图片拖动状态（从static变量移动过来） */
    int last_x;                         /* 上一次结束移动时的x */
    int last_y;                         /* 上一次结束移动时的y */
    const char *old_img_path;           /* 上一张图片路径(用于缓存清理) */
} photoLibrary_t;

/*====================================================================================
 * 全局变量声明
 *====================================================================================*/

/* 照片库全局实例（在photoLibrary.c中定义） */
extern photoLibrary_t pl;

/*====================================================================================
 * 公共API
 *====================================================================================*/

/**
 * @brief 创建照片选择界面
 * @param parent 父容器
 */
void create_photo_select_ui(lv_obj_t *parent);

/**
 * @brief 清理照片库资源
 */
void cleanup_photo_library(void);

void reflush_photo_library(void);

/** 无卡时由主线程调用：清空所有 tab 列表并刷新 UI，不访问磁盘 */
void photo_library_clear_all(void);

/** 主线程在 poll 到 MEDIA_RESULT_SCAN_PHOTO 后调用：写入 tab 数据并刷新列表；paths 由调用方在调用后通过 media_work_free_result 释放 */
void photo_library_apply_scan_result(int tab_index, char **paths, int count);

#ifdef __cplusplus
} /* extern "C" */
#endif