/**
 * @file videoLibrary.c
 * @brief 视频库模块实现
 * 
 * 实现视频文件列表显示、播放控制、文件删除等功能
 */

#include "videoLibrary.h"
// #include "../src/Player/include/easy_player_capi.h"
#include "../../runcarplay/src/app/SDCardLibrary/easy_player_capi.h"
#include <utils/plat_log.h>
#include <sys/stat.h>
#include <hwdisplay.h>
#include <confparser.h>
#include <mpi_sys.h>
#include <mpi_vo.h>
#include <mpi_vdec.h>
#include <mm_comm_sys.h>
#include <mm_comm_video.h>
#include <mm_comm_vo.h>
#include <tsemaphore.h>

/*====================================================================================
 * 全局变量定义
 *====================================================================================*/

/* 视频库全局实例 */
videoLibrary_t vl = {0};

/* 播放器实例 */
static EasyPlayerHandle P111 = NULL;

/* 标签页名称 */
static const char *tab_names[TAB_COUNT] = {
    "Front Videos",
    "Rear Videos",
    "Urgent Videos"
};

/*====================================================================================
 * 私有函数声明
 *====================================================================================*/

/* UI创建 */
static void create_ui_components(lv_obj_t *parent);
static void create_tab_page(lv_obj_t *parent, tab_type_t tab_type, const char *tab_name);
static void init_tab(tab_type_t tab_type);
static void vl_init(void);

/* 列表项创建（提取的公共函数） */
static void create_video_list_item(lv_obj_t *list, int index, const char *filepath);
static void populate_video_list(tab_type_t tab_type);

/* 事件处理 */
static void screen_click_handler(lv_event_t* e);
static void auto_hide_timer_cb(lv_timer_t* timer);
static void back_btn_event_handler(lv_event_t * e);
static void slider_event_cb(lv_event_t *e);
static void update_progress_timer_cb(lv_timer_t *timer);
static void play_mode_btn_event_handler(lv_event_t * e);
static void prev_btn_event_handler(lv_event_t * e);
static void next_btn_event_handler(lv_event_t * e);
static void pausePlay_btn_event_handler(lv_event_t * e);
static void play_speed_btn_event_handler(lv_event_t * e);
static void volume_event_handler(lv_event_t * e);
static void volume_slider_event_handler(lv_event_t * e);
static void auto_hide_volume_slider_timer_cb(lv_timer_t* timer);
static void tabview_event_handler(lv_event_t *e);
static void refresh_btn_event_handler(lv_event_t *e);
static void del_btn_event_handler(lv_event_t *e);
static void list_btn_event_handler(lv_event_t *e);

/* 内部工具函数 */
static int compare_video_mtime_desc(const void *a, const void *b);
static void scan_video_files(tab_type_t tab_type);
static void play_pause_ui_show(void);
static void check_playback_end(void);
static void draw_text_on_canvas(lv_obj_t *canvas, const char *text);
static void show_playback_controls(bool show);
static void ms_to_hhmmss(int milliseconds, char* buffer, int bufferSize);

/* 播放器相关 */
static void startPlayVideo(const char* filePath, int index, tab_type_t tab_type);
static void closePlayVideo(void);
static void play_video_by_index(int index);
static EasyPlayerHandle getInstanceSDPlayer();
static void setInstanceSDPlayer();

// start play video
static void startPlayVideo(const char* filePath, int index, tab_type_t tab_type) {
    printf("start video\n");
/*==================================================================================*/
#if 0
    // 设置VDEC频率（使用默认值）
    AW_MPI_VDEC_SetVEFreq(MM_INVALID_CHN, 324);

    // 尝试配置VDEC通道0为H265
    VDEC_CHN vdec_chn = 0;
    VDEC_CHN_ATTR_S vdec_attr;
    memset(&vdec_attr, 0, sizeof(VDEC_CHN_ATTR_S));

    // // 先尝试获取现有配置
    // ERRORTYPE ret = AW_MPI_VDEC_GetChnAttr(vdec_chn, &vdec_attr);

    // EyeseePlayer mpPlayer;
    // if (ret == SUCCESS) {
    //     // 通道存在，修改配置
    //     printf("VDEC channel %d exists, modifying for H265...\n", vdec_chn);

    //     // 修改为H265解码器
    //     vdec_attr.mType = PT_H265;  // PT_H265的值是265

    //     // 重新设置配置
    //     mpPlayer.AW_MPI_VDEC_SetChnAttr(vdec_chn, &vdec_attr);
    //     if (ret != SUCCESS) {
    //         printf("Warning: Could not set H265 config to existing channel\n");
    //     }
    // } else {
        // 通道不存在，创建新的
        printf("Creating new H265 VDEC channel %d...\n", vdec_chn);
        
        vdec_attr.mType = PT_H265;
        vdec_attr.mBufSize = 1024 * 1024 * 2;  // 2MB
        vdec_attr.mPicWidth = 1920;
        vdec_attr.mPicHeight = 1080;
        vdec_attr.mOutputPixelFormat = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
        vdec_attr.mInitRotation = 0;
        vdec_attr.mVdecVideoAttr.mRefFrameNum = 4;
        vdec_attr.mVdecVideoAttr.mMode = VIDEO_MODE_STREAM;
        vdec_attr.mVdecVideoAttr.mSupportBFrame = 1;
        vdec_attr.mnFrameBufferNum = 4;
        
        ERRORTYPE re = AW_MPI_VDEC_CreateChn(vdec_chn, &vdec_attr);
        if (re == SUCCESS) {
            printf("H265 VDEC channel created successfully\n");
        } else {
            printf("Failed to create H265 VDEC channel: %d\n", re);
        }
    // }
#endif
/*==================================================================================*/
    // 初始化进度条
    if (vl.vv.slider1) {
        lv_slider_set_value(vl.vv.slider1, 0, LV_ANIM_OFF);
    }
    // 初始化播放器
    P111 = getInstanceSDPlayer();
    EasyPlayerResult result = easy_player_initialize(P111, 720, 1440);
    if (result != EASY_PLAYER_SUCCESS) {
        printf("Initialize failed: %s\n", easy_player_get_error_string(result));
        easy_player_destroy(P111);
        return;
    }
    // 设置播放器显示区域
    result = easy_player_set_display_rect(P111, 0, 0, 720, 1440);
    if (result != EASY_PLAYER_SUCCESS) {
        printf("Set display rect failed: %s\n", easy_player_get_error_string(result));
    }
    // 设置播放器音量
    result = easy_player_set_volume(P111, vl.vv.current_volume, vl.vv.current_volume);
    if (result != EASY_PLAYER_SUCCESS) {
        printf("Set vl.vv.volume failed: %d\n", result);
    }
    // 设置播放视频文件
    result = easy_player_play_file(P111, filePath);
    if (result != EASY_PLAYER_SUCCESS) {
        printf("Play file failed: %s\n", easy_player_get_error_string(result));
    }
    // 获取视频文件总时长
    int duration = 0;
    EasyPlayerResult ret = easy_player_get_duration(P111, &duration);
    if (ret == EASY_PLAYER_SUCCESS && duration > 0) {
        char total_buff[9];
        memset(total_buff, 0, sizeof(total_buff));
        ms_to_hhmmss(duration, total_buff, sizeof(total_buff));
        draw_text_on_canvas(vl.vv.total_time, total_buff);
    } else {
        printf("Video total time receive failed\n");
    }

    printf("Playing video...%s \n", filePath);
    vl.vv.current_video_index = index;

    // 设置点击屏幕隐藏唤醒操作栏
    // lv_obj_clear_flag(vl.vv.video_screen, LV_OBJ_FLAG_HIDDEN);
    // lv_obj_add_event_cb(vl.vv.video_screen, screen_click_handler, LV_EVENT_CLICKED, NULL);
    if (vl.vv.auto_hide_timer) {
        lv_timer_reset(vl.vv.auto_hide_timer);
        lv_timer_resume(vl.vv.auto_hide_timer);
    }
    // 播放进度的定时器
    if (vl.vv.progress_timer) {
        lv_timer_reset(vl.vv.progress_timer);
        lv_timer_resume(vl.vv.progress_timer);
    }

    show_playback_controls(true);
    // vl.vv.progress_timer = lv_timer_create(update_progress_timer_cb, 500, NULL);
}

// close play video
static void closePlayVideo() {
    printf("close video\n");
    // 隐藏播放界面UI控件显示主界面
    show_playback_controls(false);
    // 停止播放
    easy_player_stop(P111);

    if (vl.vv.progress_timer) {
        lv_timer_reset(vl.vv.progress_timer);
        lv_timer_pause(vl.vv.progress_timer);
    }
    if (vl.vv.auto_hide_timer) {
        lv_timer_reset(vl.vv.auto_hide_timer);
        lv_timer_pause(vl.vv.auto_hide_timer);
    }
    if (vl.vv.auto_hide_volume_slider_timer) {
        lv_timer_reset(vl.vv.auto_hide_volume_slider_timer);
        lv_timer_pause(vl.vv.auto_hide_volume_slider_timer);
    }

    // 重置进度条
    if (vl.vv.slider1) {
        lv_slider_set_value(vl.vv.slider1, 0, LV_ANIM_OFF);
    }
    vl.vv.is_slider_dragging = false;

    // 当前视频文件的序号重置
    vl.vv.current_video_index = -1;

    // 恢复播放暂停按钮的初始图片
    vl.vv.playPauseFlag = false;
    play_pause_ui_show();

    // 恢复倍速播放的初始状态
    lv_img_set_src(vl.vv.play_speed_icon, &Video_1X);

    // 释放播放器内存
    setInstanceSDPlayer();
}

static EasyPlayerHandle getInstanceSDPlayer() {
    if (NULL == P111) {
        P111 = easy_player_create();
    }
    return P111;
}

static void setInstanceSDPlayer() {
    if (NULL != P111) {
        easy_player_destroy(P111);
        P111 = NULL;
    }
}

static void create_ui_components(lv_obj_t *parent) {
    // 透明主界面
    vl.vv.video_screen = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(vl.vv.video_screen);
    lv_obj_set_size(vl.vv.video_screen, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(vl.vv.video_screen, LV_OPA_TRANSP, 0);        // 设置背景透明
    lv_obj_set_style_border_opa(vl.vv.video_screen, LV_OPA_TRANSP, 0);    // 设置边缘透明
    lv_obj_set_pos(vl.vv.video_screen, 0, 0);
    lv_obj_add_flag(vl.vv.video_screen, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(vl.vv.video_screen, screen_click_handler, LV_EVENT_CLICKED, NULL);
    vl.vv.auto_hide_timer = lv_timer_create(auto_hide_timer_cb, 5000, NULL);
    if (vl.vv.auto_hide_timer) {
        lv_timer_pause(vl.vv.auto_hide_timer);                            // 初始暂停，等待第一次点击
    }

    // 返回按钮
    vl.vv.back_btn = lv_btn_create(vl.vv.video_screen);
    lv_obj_set_size(vl.vv.back_btn, 60, 60);
    lv_obj_align(vl.vv.back_btn, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_set_style_bg_opa(vl.vv.back_btn, LV_OPA_TRANSP, 0);        // 设置背景透明
    lv_obj_set_style_border_opa(vl.vv.back_btn, LV_OPA_TRANSP, 0);    // 设置边缘透明
    lv_obj_add_flag(vl.vv.back_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(vl.vv.back_btn, back_btn_event_handler, LV_EVENT_ALL, NULL);
    vl.vv.back_icon = lv_img_create(vl.vv.back_btn);
    lv_img_set_src(vl.vv.back_icon, &back_white);
    lv_obj_set_size(vl.vv.back_icon, 60, 60);
    lv_obj_center(vl.vv.back_icon);

    // 主容器 - 垂直Flex
    vl.vv.container = lv_obj_create(vl.vv.video_screen);
    lv_obj_remove_style_all(vl.vv.container);
    lv_obj_set_size(vl.vv.container, 1440, 170);
    lv_obj_set_flex_flow(vl.vv.container, LV_FLEX_FLOW_COLUMN);           // 垂直排列
    lv_obj_set_flex_align(vl.vv.container, LV_FLEX_ALIGN_START, 
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_color(vl.vv.container, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(vl.vv.container, LV_OPA_0, 0);
    lv_obj_set_style_border_opa(vl.vv.container, LV_OPA_0, 0);
    lv_obj_set_style_radius(vl.vv.container, 0, 0);
    lv_obj_set_style_pad_top(vl.vv.container, 5, 0);
    lv_obj_set_style_pad_bottom(vl.vv.container, 5, 0);
    lv_obj_set_style_pad_row(vl.vv.container, 0, 0);                      // 设置内部间距
    lv_obj_align(vl.vv.container, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(vl.vv.container, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(vl.vv.container, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_clear_flag(vl.vv.container, LV_OBJ_FLAG_SCROLLABLE);

    // 第一行 - 水平Flex
    vl.vv.row1 = lv_obj_create(vl.vv.container);
    lv_obj_remove_style_all(vl.vv.row1);
    lv_obj_set_size(vl.vv.row1, 1440, 50);
    lv_obj_set_flex_flow(vl.vv.row1, LV_FLEX_FLOW_ROW);                   // 水平排列
    lv_obj_set_flex_align(vl.vv.row1, LV_FLEX_ALIGN_CENTER, 
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(vl.vv.row1, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(vl.vv.row1, LV_OPA_TRANSP, 0);
    lv_obj_set_style_outline_opa(vl.vv.row1, LV_OPA_TRANSP, 0);
    lv_obj_set_style_shadow_opa(vl.vv.row1, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(vl.vv.row1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(vl.vv.row1, LV_OBJ_FLAG_EVENT_BUBBLE);

    // 视频当前时间
    vl.vv.current_time_container = lv_obj_create(vl.vv.row1);
    lv_obj_remove_style_all(vl.vv.current_time_container);
    lv_obj_set_flex_grow(vl.vv.current_time_container, 1);                    // 权重
    lv_obj_set_size(vl.vv.current_time_container, 150, 30);
    lv_obj_add_flag(vl.vv.current_time_container, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_set_style_bg_opa(vl.vv.current_time_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(vl.vv.current_time_container, LV_OPA_TRANSP, 0);
    vl.vv.current_time = lv_canvas_create(vl.vv.current_time_container);
    lv_obj_remove_style_all(vl.vv.current_time);
    lv_obj_set_style_bg_opa(vl.vv.current_time, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(vl.vv.current_time, LV_OPA_TRANSP, 0);
    lv_canvas_set_buffer(vl.vv.current_time, \
                        vl.vv.current_time_canvas_buf, \
                        150, 30, \
                        LV_IMG_CF_TRUE_COLOR_ALPHA);
    draw_text_on_canvas(vl.vv.current_time, "00:00:00");                      // 初始值
    lv_obj_center(vl.vv.current_time);
    // 注册事件处理函数
    // lv_obj_add_event_cb(vl.vv.current_time, current_time_refresh_event_handler, LV_EVENT_REFRESH, NULL);

    // 播放进度条
    vl.vv.slider1 = lv_slider_create(vl.vv.row1);
    lv_obj_remove_style_all(vl.vv.slider1);
    lv_obj_set_style_bg_color(vl.vv.slider1, TM_SLIDER_KNOB, LV_PART_KNOB);
    lv_obj_set_style_bg_opa(vl.vv.slider1, LV_OPA_COVER, LV_PART_KNOB);
    lv_obj_set_style_bg_color(vl.vv.slider1, TM_PROGRESS_FG, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(vl.vv.slider1, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(vl.vv.slider1, TM_PROGRESS_BG, 0);
    lv_obj_set_style_bg_opa(vl.vv.slider1, LV_OPA_COVER, 0);
    lv_obj_set_style_border_opa(vl.vv.slider1, LV_OPA_0, 0);
    lv_obj_set_style_shadow_opa(vl.vv.slider1, LV_OPA_0, 0);
    lv_obj_set_style_outline_opa(vl.vv.slider1, LV_OPA_0, 0);
    lv_obj_add_flag(vl.vv.slider1, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_event_cb(vl.vv.slider1, slider_event_cb, LV_EVENT_ALL, NULL);
    vl.vv.is_slider_dragging = false;
    lv_obj_set_flex_grow(vl.vv.slider1, 3);                               // 权重
    vl.vv.progress_timer = lv_timer_create(update_progress_timer_cb, 500, NULL);              // 播放视频的进度定时器
    if (vl.vv.progress_timer) {
        lv_timer_pause(vl.vv.progress_timer);
    }

    // 视频总时间
    vl.vv.total_time_container = lv_obj_create(vl.vv.row1);
    lv_obj_remove_style_all(vl.vv.total_time_container);
    lv_obj_set_flex_grow(vl.vv.total_time_container, 1);                  // 权重
    lv_obj_set_size(vl.vv.total_time_container, 150, 30);
    lv_obj_add_flag(vl.vv.total_time_container, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_set_style_bg_opa(vl.vv.total_time_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(vl.vv.total_time_container, LV_OPA_TRANSP, 0);
    vl.vv.total_time = lv_canvas_create(vl.vv.total_time_container);
    lv_canvas_set_buffer(vl.vv.total_time, \
                        vl.vv.total_time_canvas_buf, \
                        120, 30, \
                        LV_IMG_CF_TRUE_COLOR_ALPHA);
    draw_text_on_canvas(vl.vv.total_time, "99:99:99");                    // 初始值
    lv_obj_center(vl.vv.total_time);

    // 第二行 - 水平Flex
    vl.vv.row2 = lv_obj_create(vl.vv.container);
    lv_obj_set_size(vl.vv.row2, 1440, 120);
    lv_obj_set_flex_flow(vl.vv.row2, LV_FLEX_FLOW_ROW);                   // 水平排列
    lv_obj_set_flex_align(vl.vv.row2, LV_FLEX_ALIGN_SPACE_EVENLY,         // 均匀分布
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_color(vl.vv.row2, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(vl.vv.row2, LV_OPA_30, 0);
    lv_obj_set_style_border_color(vl.vv.row2, lv_color_black(), 0);
    lv_obj_set_style_border_width(vl.vv.row2, 3, 0);
    lv_obj_set_style_border_opa(vl.vv.row2, LV_OPA_100, 0);
    lv_obj_set_style_radius(vl.vv.row2, 0, 0);
    lv_obj_clear_flag(vl.vv.row2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(vl.vv.row2, LV_OBJ_FLAG_EVENT_BUBBLE);

    // 播放模式
    vl.vv.play_mode_btn = lv_btn_create(vl.vv.row2);
    lv_obj_set_size(vl.vv.play_mode_btn, 150, 100);
    lv_obj_set_style_bg_opa(vl.vv.play_mode_btn, LV_OPA_0, 0);
    lv_obj_set_style_border_opa(vl.vv.play_mode_btn, LV_OPA_TRANSP, 0);
    // lv_obj_set_style_bg_color(vl.vv.play_mode_btn, lv_color_hex(0x2196F3), LV_STATE_PRESSED);
    // lv_obj_set_style_bg_opa(vl.vv.play_mode_btn, LV_OPA_30, LV_STATE_PRESSED);  // 50%透明度
    lv_obj_set_flex_grow(vl.vv.play_mode_btn, 1);                         // 权重
    lv_obj_add_flag(vl.vv.play_mode_btn, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_event_cb(vl.vv.play_mode_btn, play_mode_btn_event_handler, LV_EVENT_CLICKED, NULL);
    vl.vv.play_mode_icon = lv_img_create(vl.vv.play_mode_btn);
    lv_obj_set_size(vl.vv.play_mode_icon, 110, 65);
    lv_img_set_src(vl.vv.play_mode_icon, &VideoPlayback_FileLooping);
    lv_obj_center(vl.vv.play_mode_icon);

    // 上一个
    vl.vv.prev_btn = lv_btn_create(vl.vv.row2);
    lv_obj_set_size(vl.vv.prev_btn, 150, 100);
    lv_obj_set_style_bg_opa(vl.vv.prev_btn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(vl.vv.prev_btn, LV_OPA_TRANSP, 0);
    lv_obj_set_flex_grow(vl.vv.prev_btn, 1);                              // 权重
    lv_obj_add_flag(vl.vv.prev_btn, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_event_cb(vl.vv.prev_btn, prev_btn_event_handler, LV_EVENT_CLICKED, NULL);
    vl.vv.prev_icon = lv_img_create(vl.vv.prev_btn);
    lv_img_set_src(vl.vv.prev_icon, &prev_white);
    lv_obj_set_size(vl.vv.prev_icon, 110, 65);
    lv_obj_center(vl.vv.prev_icon);

    // 暂停/播放
    vl.vv.pause_btn = lv_btn_create(vl.vv.row2);
    lv_obj_set_size(vl.vv.pause_btn, 150, 100);
    lv_obj_set_style_bg_opa(vl.vv.pause_btn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(vl.vv.pause_btn, LV_OPA_TRANSP, 0);
    lv_obj_set_flex_grow(vl.vv.pause_btn, 1);                             // 权重
    lv_obj_add_flag(vl.vv.pause_btn, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_event_cb(vl.vv.pause_btn, pausePlay_btn_event_handler, LV_EVENT_ALL, NULL);
    // 暂停按钮
    vl.vv.pause_icon = lv_img_create(vl.vv.pause_btn);
    lv_obj_set_style_img_opa(vl.vv.pause_icon, LV_OPA_COVER, 0);
    lv_img_set_src(vl.vv.pause_icon, &pause_white);
    lv_obj_set_size(vl.vv.pause_icon, 110, 65);
    lv_obj_center(vl.vv.pause_icon);
    // 播放按钮
    vl.vv.play_icon = lv_img_create(vl.vv.pause_btn);
    lv_obj_set_style_img_opa(vl.vv.play_icon, LV_OPA_COVER, 0);
    lv_img_set_src(vl.vv.play_icon, &play_white);
    lv_obj_set_size(vl.vv.play_icon, 150, 90);
    lv_obj_center(vl.vv.play_icon);
    vl.vv.playPauseFlag = false;
    play_pause_ui_show();

    // 下一个
    vl.vv.next_btn = lv_btn_create(vl.vv.row2);
    lv_obj_set_size(vl.vv.next_btn, 150, 100);
    lv_obj_set_style_bg_opa(vl.vv.next_btn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(vl.vv.next_btn, LV_OPA_TRANSP, 0);
    lv_obj_set_flex_grow(vl.vv.next_btn, 1);                              // 权重
    lv_obj_add_flag(vl.vv.next_btn, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_event_cb(vl.vv.next_btn, next_btn_event_handler, LV_EVENT_CLICKED, NULL);
    vl.vv.next_icon = lv_img_create(vl.vv.next_btn);
    lv_img_set_src(vl.vv.next_icon, &next_white);
    lv_obj_set_size(vl.vv.next_icon, 110, 65);
    lv_obj_center(vl.vv.next_icon);

    // 播放速度
    vl.vv.play_speed_btn = lv_btn_create(vl.vv.row2);
    lv_obj_remove_style_all(vl.vv.play_speed_btn);
    lv_obj_set_size(vl.vv.play_speed_btn, 150, 100);
    lv_obj_set_style_bg_opa(vl.vv.play_speed_btn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(vl.vv.play_speed_btn, LV_OPA_TRANSP, 0);
    lv_obj_set_flex_grow(vl.vv.play_speed_btn, 1);                        // 权重
    lv_obj_add_flag(vl.vv.play_speed_btn, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_event_cb(vl.vv.play_speed_btn, play_speed_btn_event_handler, LV_EVENT_CLICKED, NULL);
    // 播放速率显示
    vl.vv.play_speed_icon = lv_img_create(vl.vv.play_speed_btn);
    lv_img_set_src(vl.vv.play_speed_icon, &Video_1X);
    lv_obj_set_size(vl.vv.play_speed_icon, 115, 100);
    lv_obj_center(vl.vv.play_speed_icon);

    // 播放声音
    vl.vv.volume = lv_btn_create(vl.vv.row2);
    lv_obj_remove_style_all(vl.vv.volume);
    lv_obj_set_size(vl.vv.volume, 150, 100);
    lv_obj_set_style_bg_opa(vl.vv.volume, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(vl.vv.volume, LV_OPA_TRANSP, 0);
    lv_obj_set_flex_grow(vl.vv.volume, 1);                                // 权重
    lv_obj_add_flag(vl.vv.volume, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_event_cb(vl.vv.volume, volume_event_handler, LV_EVENT_CLICKED, NULL);
    vl.vv.volume_icon = lv_img_create(vl.vv.volume);
    lv_img_set_src(vl.vv.volume_icon, &VideoPlayback_Sound);
    lv_obj_set_size(vl.vv.volume_icon, 84, 50);
    lv_obj_center(vl.vv.volume_icon);

    // 声音控制
    vl.vv.volume_slider = lv_slider_create(lv_layer_top());
    lv_obj_set_size(vl.vv.volume_slider, 10, 250);
    lv_obj_align(vl.vv.volume_slider, LV_ALIGN_RIGHT_MID, -40, 0);
    lv_obj_set_style_bg_color(vl.vv.volume_slider, TM_PROGRESS_BG, 0);
    lv_obj_set_style_bg_color(vl.vv.volume_slider, TM_SUCCESS, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(vl.vv.volume_slider, TM_SLIDER_KNOB, LV_PART_KNOB);
    lv_obj_set_style_radius(vl.vv.volume_slider, 25, LV_PART_KNOB);
    lv_obj_set_style_radius(vl.vv.volume_slider, 0, LV_PART_INDICATOR);
    lv_obj_set_style_radius(vl.vv.volume_slider, 0, 0);
    lv_obj_set_style_pad_all(vl.vv.volume_slider, 0, 0|LV_PART_KNOB|LV_PART_INDICATOR);
    lv_slider_set_range(vl.vv.volume_slider, 0, 100);
    vl.vv.current_volume = 0.5f;
    lv_slider_set_value(vl.vv.volume_slider, vl.vv.current_volume * 100, LV_ANIM_OFF);
    lv_obj_add_event_cb(vl.vv.volume_slider, volume_slider_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_flag(vl.vv.volume_slider, LV_OBJ_FLAG_HIDDEN);
    vl.vv.auto_hide_volume_slider_timer = lv_timer_create(auto_hide_volume_slider_timer_cb, 5000, NULL);
    if (vl.vv.auto_hide_volume_slider_timer) {
        lv_timer_pause(vl.vv.auto_hide_volume_slider_timer);                                // 初始暂停，等待第一次点击
    }

    vl.vm.cont = lv_obj_create(parent);
    lv_obj_set_size(vl.vm.cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_pad_all(vl.vm.cont, 0, 0);
    lv_obj_set_style_bg_color(vl.vm.cont, TM_BG_PRIMARY, 0);
    lv_obj_set_style_border_width(vl.vm.cont, 0, 0);

    // 创建标签视图
    vl.vm.tabview = lv_tabview_create(vl.vm.cont, LV_DIR_TOP, 80);
    lv_obj_set_size(vl.vm.tabview, LV_PCT(100), LV_PCT(100));

    // 2. 滚动行为优化
    lv_obj_set_scroll_dir(vl.vm.tabview, LV_DIR_HOR);  // 明确水平滚动
    lv_obj_set_scroll_snap_x(vl.vm.tabview, LV_SCROLL_SNAP_CENTER);  // 中心吸附
    lv_obj_set_scrollbar_mode(vl.vm.tabview, LV_SCROLLBAR_MODE_OFF);  // 隐藏滚动条

    // 5. 事件传播控制（重要！防止列表滚动干扰）
    lv_obj_clear_flag(vl.vm.tabview, LV_OBJ_FLAG_SCROLL_CHAIN_HOR);
    lv_obj_add_flag(vl.vm.tabview, LV_OBJ_FLAG_SCROLL_ONE);

    // 标签栏样式
    vl.vm.tab_btns = lv_tabview_get_tab_btns(vl.vm.tabview);
    if (!vl.vm.tab_btns) {
        vl.vm.tab_btns = lv_obj_get_child(vl.vm.tabview, 0);
    }
    /* 视频库 - 顶部标签栏样式（与照片库标题栏保持一致） */
    if (vl.vm.tab_btns) {
        lv_obj_set_style_bg_color(vl.vm.tab_btns, TM_BG_SECONDARY, 0);
        lv_obj_set_style_bg_opa(vl.vm.tab_btns, LV_OPA_COVER, 0);
        lv_obj_set_style_text_color(vl.vm.tab_btns, TM_TEXT_PRIMARY, 0);
        lv_obj_set_style_text_font(vl.vm.tab_btns, &v853Font_Medialibrary_40, 0);
        lv_obj_set_style_border_width(vl.vm.tab_btns, 0, 0);              /* 移除边框，与照片库标题栏一致 */
        lv_obj_set_style_radius(vl.vm.tab_btns, 0, 0);
    }

    // 创建三个标签页
    create_tab_page(vl.vm.tabview, TAB_FRONT, get_string_for_language(languageSetting, "Front"));
    create_tab_page(vl.vm.tabview, TAB_REAR, get_string_for_language(languageSetting, "Rear"));
    create_tab_page(vl.vm.tabview, TAB_URGENT, get_string_for_language(languageSetting, "Urgent"));

    lv_tabview_set_act(vl.vm.tabview, 0, LV_ANIM_OFF);
    vl.vm.current_tab = TAB_FRONT;
    lv_obj_add_event_cb(vl.vm.tabview, tabview_event_handler, LV_EVENT_VALUE_CHANGED, NULL);

    // 初始化所有标签页
    init_tab(TAB_FRONT);
    init_tab(TAB_REAR);
    init_tab(TAB_URGENT);
}

static void auto_hide_timer_cb(lv_timer_t* timer) {
    // 5秒无操作后隐藏UI控件
    if (!lv_obj_has_flag(vl.vv.back_btn, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_add_flag(vl.vv.back_btn, LV_OBJ_FLAG_HIDDEN);
        printf("auto hide vl.vv.back_btn\n");
    }
    if (!lv_obj_has_flag(vl.vv.container, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_add_flag(vl.vv.container, LV_OBJ_FLAG_HIDDEN);
        printf("auto hide vl.vv.container\n");
    }
}

static void auto_hide_volume_slider_timer_cb(lv_timer_t* timer) {
    // 5秒无操作后隐藏UI控件
    if (!lv_obj_has_flag(vl.vv.volume_slider, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_add_flag(vl.vv.volume_slider, LV_OBJ_FLAG_HIDDEN);
        printf("auto hide vl.vv.volume_slider\n");
    }
}

static void screen_click_handler(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        printf("screen point\n");
        // 显示UI控件
        if (lv_obj_has_flag(vl.vv.back_btn, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_clear_flag(vl.vv.back_btn, LV_OBJ_FLAG_HIDDEN);
        }
        if (lv_obj_has_flag(vl.vv.container, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_clear_flag(vl.vv.container, LV_OBJ_FLAG_HIDDEN);
        }
        // 重置定时器（重新开始5秒计时）
        if (vl.vv.auto_hide_timer) {
            printf("reset hide timer\n");
            lv_timer_reset(vl.vv.auto_hide_timer);
        }
    }
}

// 3. 事件处理函数（在UI线程中执行）
static void current_time_refresh_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_REFRESH) {
        draw_text_on_canvas(vl.vv.current_time, vl.vv.current_buff);
    }
}

// 定义事件处理函数
static void pausePlay_btn_event_handler(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    switch(code) {
        case LV_EVENT_CLICKED:
            if (vl.vv.playPauseFlag) {
                vl.vv.playPauseFlag = false;
                play_pause_ui_show();
                printf("show pause\n");
                easy_player_resume(P111);
                
                if (vl.vv.progress_timer) {
                    lv_timer_resume(vl.vv.progress_timer);
                }
            } else {
                vl.vv.playPauseFlag = true;
                play_pause_ui_show();
                printf("show play\n");
                easy_player_pause(P111);
                
                if (vl.vv.progress_timer) {
                    lv_timer_pause(vl.vv.progress_timer);
                }
            }
            break;
    }
}

// 上一个视频按钮事件处理
static void prev_btn_event_handler(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    
    if(code == LV_EVENT_CLICKED) {
        if (vl.vv.current_video_index > 0) {
            play_video_by_index(vl.vv.current_video_index - 1);
            printf("Play previous video, index: %d\n", vl.vv.current_video_index - 1);
        } else {
            printf("Already at the first video, cannot play previous\n");
        }
    }
}

// 下一个视频按钮事件处理
static void next_btn_event_handler(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    
    if(code == LV_EVENT_CLICKED) {
        int max_index = vl.vm.tab_data[vl.vm.current_tab].video_file_count - 1;
        
        if (vl.vv.current_video_index < max_index) {
            play_video_by_index(vl.vv.current_video_index + 1);
            printf("Play next video, index: %d\n", vl.vv.current_video_index + 1);
        } else {
            printf("Already at the last video, cannot play next\n");
        }
    }
}

static void volume_event_handler(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        if (lv_obj_has_flag(vl.vv.volume_slider, LV_OBJ_FLAG_HIDDEN)) {
            if (vl.vv.auto_hide_volume_slider_timer) {
                lv_timer_reset(vl.vv.auto_hide_volume_slider_timer);
                lv_timer_resume(vl.vv.auto_hide_volume_slider_timer);
            }
            lv_obj_clear_flag(vl.vv.volume_slider, LV_OBJ_FLAG_HIDDEN);
        } else {
            if (vl.vv.auto_hide_volume_slider_timer) {
                lv_timer_reset(vl.vv.auto_hide_volume_slider_timer);
                lv_timer_pause(vl.vv.auto_hide_volume_slider_timer);
            }
            lv_obj_add_flag(vl.vv.volume_slider, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

// 播放模式按钮事件处理
static void play_mode_btn_event_handler(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        vl.vv.current_play_mode = (vl.vv.current_play_mode + 1) % 3;
        switch(vl.vv.current_play_mode) {
            case PLAY_MODE_SEQUENCE:
                lv_img_set_src(vl.vv.play_mode_icon, &VideoPlayback_FileLooping);
                printf("Play mode: Sequence (auto play next)\n");
                break;
            case PLAY_MODE_SINGLE_LOOP:
                lv_img_set_src(vl.vv.play_mode_icon, &VideoPlayback_SingleVideoLoop);
                printf("Play mode: Single Loop\n");
                break;
            case PLAY_MODE_SINGLE_STOP:
                lv_img_set_src(vl.vv.play_mode_icon, &VideoPlayback_notLooped);
                printf("Play mode: Single Stop\n");
                break;
        }
    }
}

// 播放速度按钮事件处理
static void play_speed_btn_event_handler(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    
    if(code == LV_EVENT_CLICKED) {
        vl.vv.current_play_speed = (vl.vv.current_play_speed + 1) % 2;
        if (vl.vv.current_play_speed == PLAY_SPEED_1X) {
            lv_img_set_src(vl.vv.play_speed_icon, &Video_1X);
            printf("Play speed: 1X\n");
            easy_player_set_video_speed(P111, 1.0f);
        } else {
            lv_img_set_src(vl.vv.play_speed_icon, &Video_2X);
            printf("Play speed: 2X\n");
            easy_player_set_video_speed(P111, 2.0f);
        }
    }
}

// 音量滑块事件处理
static void volume_slider_event_handler(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        if (vl.vv.auto_hide_volume_slider_timer) {
            lv_timer_reset(vl.vv.auto_hide_volume_slider_timer);
        }
        int value = lv_slider_get_value(vl.vv.volume_slider);
        vl.vv.current_volume = value / 100.0f;
        if (P111) {
            EasyPlayerResult result = easy_player_set_volume(P111, vl.vv.current_volume, vl.vv.current_volume);
            if (result != EASY_PLAYER_SUCCESS) {
                printf("Set vl.vv.volume failed: %d\n", result);
            } else {
                printf("vl.vv.volume set to: %d%%\n", value);
            }
        }
    }
}

static void draw_text_on_canvas(lv_obj_t *canvas, const char *text) {
    if (!canvas || !text) return;
    // 1. 先获取Canvas的当前尺寸
    lv_img_dsc_t *canvas_img = lv_canvas_get_img(canvas);
    if (!canvas_img) return;
    int canvas_width = canvas_img->header.w;
    int canvas_height = canvas_img->header.h;

    // 2. 清空Canvas为透明
    lv_canvas_fill_bg(canvas, lv_color_make(55, 55, 55), LV_OPA_0);

    // 3. 创建文本绘制描述符
    lv_draw_label_dsc_t dsc;
    lv_draw_label_dsc_init(&dsc);

    // 4. 设置文本属性
    dsc.color = lv_color_white();       // 白色文字
    dsc.font = &v853Font_Medialibrary_30;          // 字体
    dsc.align = LV_TEXT_ALIGN_CENTER;   // 居中对齐
    dsc.letter_space = 0;               // 字符间距设为0
    dsc.line_space = 0;                 // 行间距设为0
    dsc.opa = LV_OPA_100;

    // 5. 计算文本实际大小
    lv_point_t txt_size;
    lv_txt_get_size(&txt_size, text, dsc.font, 
                   dsc.letter_space, dsc.line_space, 
                   LV_COORD_MAX, LV_TEXT_FLAG_NONE);

//    printf("Text '%s' size: %d x %d, Canvas: %d x %d\n", \
           text, txt_size.x, txt_size.y, canvas_width, canvas_height);

    // 6. 居中绘制文本
    int x = 0;  // 从Canvas左侧开始
    int y = (canvas_height - txt_size.y) / 2;  // 垂直居中

    // 使用文本的实际宽度作为绘制宽度
    lv_canvas_draw_text(canvas, x, y, txt_size.x, &dsc, text);
    // 7. 刷新Canvas
    lv_obj_invalidate(canvas);
}

static void play_pause_ui_show() {
    if (vl.vv.playPauseFlag) {
        lv_obj_add_flag(vl.vv.pause_icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(vl.vv.play_icon, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(vl.vv.pause_icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(vl.vv.play_icon, LV_OBJ_FLAG_HIDDEN);
    }
}

static void get_screen_absolute_coords(lv_obj_t *obj, lv_area_t *screen_coords) {
    lv_area_t obj_coords;
    lv_obj_get_coords(obj, &obj_coords); // 获取对象相对于其父对象的坐标[5](@ref)

    // 初始化绝对坐标为对象自身的相对坐标
    screen_coords->x1 = obj_coords.x1;
    screen_coords->y1 = obj_coords.y1;
    screen_coords->x2 = obj_coords.x2;
    screen_coords->y2 = obj_coords.y2;
    printf("hybym%d, %d\n", screen_coords->x1, screen_coords->y1);

    lv_obj_t *parent = lv_obj_get_parent(obj);
    
    // 从父对象开始，逐级向上累加偏移量，直到屏幕对象（父对象为NULL）
    while (parent != NULL) {
        lv_area_t parent_coords;
        lv_obj_get_coords(parent, &parent_coords);

        // 将当前区域的坐标加上父对象的坐标
        screen_coords->x1 += parent_coords.x1;
        screen_coords->y1 += parent_coords.y1;
        screen_coords->x2 += parent_coords.x1; // 注意：x2 和 y2 同样只加父对象的 x1/y1
        screen_coords->y2 += parent_coords.y1;
        printf("hybym%d, %d\n", screen_coords->x1, screen_coords->y1);
        parent = lv_obj_get_parent(parent);
    }
}

// 显示或隐藏播放控制UI
static void show_playback_controls(bool show) {
    if (show) {
        lv_obj_clear_flag(vl.vv.video_screen, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(lv_scr_act(), LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(vl.vv.video_screen, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_HIDDEN);
    }
}

// 根据索引播放视频
static void play_video_by_index(int index) {
    if (vl.vm.current_tab < TAB_COUNT && index >= 0 && index < vl.vm.tab_data[vl.vm.current_tab].video_file_count) {
        printf("Playing video by index: %d, path: %s\n", index, vl.vm.tab_data[vl.vm.current_tab].video_files[index]);
        closePlayVideo();
        startPlayVideo(vl.vm.tab_data[vl.vm.current_tab].video_files[index], index, vl.vm.current_tab);
        printf("Playing video %d/%d from tab %d\n", index + 1, vl.vm.tab_data[vl.vm.current_tab].video_file_count, vl.vm.current_tab);
    }
}

// 检查播放结束并处理
static void check_playback_end(void) {
    int current_position = 0;
    int duration = 0;

    EasyPlayerResult ret1 = easy_player_get_current_position(P111, &current_position);
    EasyPlayerResult ret2 = easy_player_get_duration(P111, &duration);

    if (ret1 == EASY_PLAYER_SUCCESS && ret2 == EASY_PLAYER_SUCCESS && duration > 0) {
        if (duration - current_position < 100) {
            printf("Video playback ended\n");

            switch(vl.vv.current_play_mode) {
                case PLAY_MODE_SEQUENCE:
                {
                    int max_index = vl.vm.tab_data[vl.vm.current_tab].video_file_count - 1;
                    
                    if (vl.vv.current_video_index < max_index) {
                        play_video_by_index(vl.vv.current_video_index + 1);
                    } else {
                        printf("Last video, stopping playback\n");
                    }
                    break;
                }
                case PLAY_MODE_SINGLE_LOOP:
                    easy_player_pause(P111);
                    if (vl.vv.progress_timer) {
                        lv_timer_pause(vl.vv.progress_timer);
                    }
                    easy_player_resume(P111);
                    if (vl.vv.progress_timer) {
                        lv_timer_resume(vl.vv.progress_timer);
                    }
                    printf("Single video loop playback\n");
                    break;
                case PLAY_MODE_SINGLE_STOP:
                    printf("Single video playback ended\n");
                    break;
            }
        }
    }
}

static void ms_to_hhmmss(int milliseconds, char* buffer, int bufferSize) {
    // 参数检查
    if (buffer == NULL || bufferSize < 9) { // HH:MM:SS 最少需要 9 字节（包含结束符）
        return;
    }

    // 处理负值
    int isNegative = 0;
    if (milliseconds < 0) {
        isNegative = 1;
        milliseconds = -milliseconds;
    }

    // 计算时分秒
    int totalSeconds = milliseconds / 1000;
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;

    // 格式化字符串
    if (isNegative) {
        // 如果是负数，格式化为 -HH:MM:SS
        snprintf(buffer, bufferSize, "-%02d:%02d:%02d", hours, minutes, seconds);
    } else {
        snprintf(buffer, bufferSize, "%02d:%02d:%02d", hours, minutes, seconds);
    }
}

// 修改更新进度条回调函数，添加播放结束检查
static void update_progress_timer_cb(lv_timer_t *timer) {
    // if (vl.vv.is_slider_dragging) {
    //     return;
    // }
    
    if (!P111 || !vl.vv.slider1) {
        return;
    }

    int current_position = 0;
    int duration = 0;

    EasyPlayerResult ret1 = easy_player_get_current_position(P111, &current_position);
    EasyPlayerResult ret2 = easy_player_get_duration(P111, &duration);

    if (ret1 == EASY_PLAYER_SUCCESS && ret2 == EASY_PLAYER_SUCCESS && duration > 0) {
        int value = (int)((double)current_position / duration * 100);

        if (vl.vv.is_slider_dragging) {
            vl.vv.is_slider_dragging = false;
            return;
        }

        if (value >= 99) {
            check_playback_end();
        }

        lv_slider_set_value(vl.vv.slider1, value, LV_ANIM_OFF);

        vl.vv.current_buff[9];
        memset(vl.vv.current_buff, 0, sizeof(vl.vv.current_buff));
        ms_to_hhmmss(current_position, vl.vv.current_buff, sizeof(vl.vv.current_buff));
        draw_text_on_canvas(vl.vv.current_time, vl.vv.current_buff);
        
        printf("Progress: %d%%, Position: %d ms, Duration: %d ms\n", value, current_position, duration);
        // static int last_percent = -1;
        // if (value != last_percent) {
        //     printf("Progress: %d%%, Position: %d ms, Duration: %d ms\n", value, current_position, duration);
        //     last_percent = value;

        //     if (value >= 99) {
        //         check_playback_end();
        //     }
        // }
    } else {
        if (ret1 != EASY_PLAYER_SUCCESS) {
            printf("Get current position failed: %d\n", ret1);
        }
        if (ret2 != EASY_PLAYER_SUCCESS) {
            printf("Get duration failed: %d\n", ret2);
        }
    }
}

// 定义事件处理函数
static void back_btn_event_handler(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED) {
        printf("The back button was clicked\n");
        closePlayVideo();
        if (!lv_obj_has_flag(vl.vv.volume_slider, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_add_flag(vl.vv.volume_slider, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

// 进度条拖动开始事件
static void slider_drag_start_event(lv_event_t *e) {
    vl.vv.is_slider_dragging = true;
    printf("Slider drag started\n");
}

// 进度条拖动结束事件
static void slider_drag_end_event(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        vl.vv.is_slider_dragging = false;

        if (P111 && vl.vv.slider1) {
            int value = lv_slider_get_value(vl.vv.slider1);
            int duration = 0;

            EasyPlayerResult ret = easy_player_get_duration(P111, &duration);
            if (ret == EASY_PLAYER_SUCCESS && duration > 0) {
                int seek_position = (int)((double)(value) / 100.0 * duration);
                printf("Seeking to: %d ms (%.1f%%)\n", seek_position, (float)value);

                EasyPlayerResult seek_ret = easy_player_seek_to(P111, seek_position);
                if (seek_ret != EASY_PLAYER_SUCCESS) {
                    printf("Seek failed: %d\n", seek_ret);
                }
            }
        }

        printf("Slider drag ended\n");
    }
}

// 修改后的进度条事件回调函数
static void slider_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
#if 0
    if(code == LV_EVENT_REFR_EXT_DRAW_SIZE) {
        lv_coord_t * s = lv_event_get_param(e);
        *s = LV_MAX(*s, 60);                            // 为滑块上方显示进度数值申请空间
    }
    else if(code == LV_EVENT_DRAW_PART_END) {
        lv_obj_draw_part_dsc_t * dsc = lv_event_get_param(e);
        if(dsc->part == LV_PART_KNOB && lv_obj_has_state(obj, LV_STATE_PRESSED)) {
            char buf[8];
            lv_snprintf(buf, sizeof(buf), "%"LV_PRId32, lv_slider_get_value(obj));

            lv_point_t text_size;
            lv_txt_get_size(&text_size, buf, &lv_font_montserrat_24, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

            lv_area_t txt_area;
            txt_area.x1 = dsc->draw_area->x1 + lv_area_get_width(dsc->draw_area) / 2 - text_size.x / 2;
            txt_area.x2 = txt_area.x1 + text_size.x;
            txt_area.y2 = dsc->draw_area->y1 - 10;
            txt_area.y1 = txt_area.y2 - text_size.y;

            lv_area_t bg_area;
            bg_area.x1 = txt_area.x1 - LV_DPX(8);
            bg_area.x2 = txt_area.x2 + LV_DPX(8);
            bg_area.y1 = txt_area.y1 - LV_DPX(8);
            bg_area.y2 = txt_area.y2 + LV_DPX(8);

            lv_draw_rect_dsc_t rect_dsc;
            lv_draw_rect_dsc_init(&rect_dsc);
            rect_dsc.bg_color = lv_palette_darken(LV_PALETTE_GREY, 3);
            rect_dsc.radius = LV_DPX(5);
            lv_draw_rect(dsc->draw_ctx, &rect_dsc, &bg_area);

            lv_draw_label_dsc_t label_dsc;
            lv_draw_label_dsc_init(&label_dsc);
            label_dsc.color = lv_color_white();
            label_dsc.font = &lv_font_montserrat_24;
            lv_draw_label(dsc->draw_ctx, &label_dsc, &txt_area, buf, NULL);
        }
    }
    else if(code == LV_EVENT_PRESSING) {
        if (!vl.vv.is_slider_dragging) {
            slider_drag_start_event(e);
        }
    }
    else if(code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        slider_drag_end_event(e);
    }
#else
    if(code == LV_EVENT_PRESSED) {
        // 播放按钮图标显示为暂停
        vl.vv.playPauseFlag = true;
        play_pause_ui_show();
        printf("show play\n");

        // 停止进度条更新定时器
        if (vl.vv.progress_timer) {
            lv_timer_reset(vl.vv.progress_timer);
            lv_timer_pause(vl.vv.progress_timer);
        }

        // 暂停当前视频的播放
        easy_player_pause(P111);
    }
    if(code == LV_EVENT_PRESSING) {
        if (!vl.vv.is_slider_dragging) {
            vl.vv.is_slider_dragging = true;    // 进度条开始拖动的标志位
        }
    }
    else if(code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        // 进度条设在拖动结束时跳转视频位置
        // vl.vv.is_slider_dragging = false;       // 进度条结束拖动的标志位

        if (P111 && vl.vv.slider1) {
            int value = lv_slider_get_value(vl.vv.slider1);     // 获取当前滑块值
            int duration = 0;

            EasyPlayerResult ret = easy_player_get_duration(P111, &duration);   // 获取视频总时长
            if (ret == EASY_PLAYER_SUCCESS && duration > 0) {
                int seek_position = (int)((double)(value) / 100.0 * duration);  // 计算跳转位置
                printf("Seeking to: %d ms (%.1f%%)\n", seek_position, (float)value);
                seek_position += 1500;                                          // 修正跳转误差
                

                EasyPlayerResult seek_ret = easy_player_seek_to(P111, seek_position);   // 跳转视频
                if (seek_ret != EASY_PLAYER_SUCCESS) {
                    printf("Seek failed: %d\n", seek_ret);
                }
            }
        }

        printf("Slider drag ended\n");

        // 恢复进度条更新定时器
        if (vl.vv.progress_timer) {
            lv_timer_resume(vl.vv.progress_timer);
        }

        // 播放按钮图标显示为播放
        vl.vv.playPauseFlag = false;
        play_pause_ui_show();
        printf("show pause\n");

        // 恢复当前视频的播放
        easy_player_resume(P111);
    }
#endif
}

static int compare_video_mtime_desc(const void *a, const void *b) {
    const char *path_a = *(const char *const *)a;
    const char *path_b = *(const char *const *)b;
    if (!path_a || !path_b) return 0;
    struct stat sa, sb;
    if (stat(path_a, &sa) != 0 || stat(path_b, &sb) != 0) return 0;
    if (sa.st_mtime > sb.st_mtime) return -1;
    if (sa.st_mtime < sb.st_mtime) return 1;
    return 0;
}

// 扫描视频文件函数
static void scan_video_files(tab_type_t tab_type) {
    DIR *dir;
    struct dirent *entry;

    // 清空之前的文件列表
    for (int i = 0; i < vl.vm.tab_data[tab_type].video_file_count; i++) {
        free(vl.vm.tab_data[tab_type].video_files[i]);
        vl.vm.tab_data[tab_type].video_files[i] = NULL;
    }
    vl.vm.tab_data[tab_type].video_file_count = 0;

    if (tab_type == TAB_FRONT) {
        dir = opendir(VIDEO_DIR_FRONT);
        if (dir == NULL) {
            printf("open err: %s\n", VIDEO_DIR_FRONT);
        } else {
            while ((entry = readdir(dir)) != NULL && vl.vm.tab_data[tab_type].video_file_count < MAX_FILES) {
                char *filename = entry->d_name;
                int len = strlen(filename);
                
                if (len > 4 && strcmp(filename + len - 4, ".mp4") == 0) {
                    char full_path[MAX_PATH_LEN];
                    snprintf(full_path, sizeof(full_path), "%s/%s", VIDEO_DIR_FRONT, filename);
                    
                    vl.vm.tab_data[tab_type].video_files[vl.vm.tab_data[tab_type].video_file_count] = malloc(strlen(full_path) + 1);
                    if (vl.vm.tab_data[tab_type].video_files[vl.vm.tab_data[tab_type].video_file_count]) {
                        strcpy(vl.vm.tab_data[tab_type].video_files[vl.vm.tab_data[tab_type].video_file_count], full_path);
                        printf("front file = %s\n", full_path);
                        vl.vm.tab_data[tab_type].video_file_count++;
                    }
                }
            }

            closedir(dir);
        }
    } 
    else if (tab_type == TAB_REAR) {
        dir = opendir(VIDEO_DIR_REAR);
        if (dir == NULL) {
            printf("open err: %s\n", VIDEO_DIR_REAR);
        } else {
            while ((entry = readdir(dir)) != NULL && vl.vm.tab_data[tab_type].video_file_count < MAX_FILES) {
                char *filename = entry->d_name;
                int len = strlen(filename);
                
                if (len > 4 && strcmp(filename + len - 4, ".mp4") == 0) {
                    char full_path[MAX_PATH_LEN];
                    snprintf(full_path, sizeof(full_path), "%s/%s", VIDEO_DIR_REAR, filename);
                    
                    vl.vm.tab_data[tab_type].video_files[vl.vm.tab_data[tab_type].video_file_count] = malloc(strlen(full_path) + 1);
                    if (vl.vm.tab_data[tab_type].video_files[vl.vm.tab_data[tab_type].video_file_count]) {
                        strcpy(vl.vm.tab_data[tab_type].video_files[vl.vm.tab_data[tab_type].video_file_count], full_path);
                        printf("rear file = %s\n", full_path);
                        vl.vm.tab_data[tab_type].video_file_count++;
                    }
                }
            }
            
            closedir(dir);
        }
    } 
    else if (tab_type == TAB_URGENT) {
        // 扫描前置紧急录制
        dir = opendir(VIDEO_DIR_URGENT_FRONT);
        if (dir != NULL) {
            while ((entry = readdir(dir)) != NULL && vl.vm.tab_data[tab_type].video_file_count < MAX_FILES) {
                char *filename = entry->d_name;
                int len = strlen(filename);
                
                if (len > 4 && strcmp(filename + len - 4, ".mp4") == 0) {
                    char full_path[MAX_PATH_LEN];
                    snprintf(full_path, sizeof(full_path), "%s/%s", VIDEO_DIR_URGENT_FRONT, filename);
                    
                    vl.vm.tab_data[tab_type].video_files[vl.vm.tab_data[tab_type].video_file_count] = malloc(strlen(full_path) + 1);
                    if (vl.vm.tab_data[tab_type].video_files[vl.vm.tab_data[tab_type].video_file_count]) {
                        strcpy(vl.vm.tab_data[tab_type].video_files[vl.vm.tab_data[tab_type].video_file_count], full_path);
                        printf("urgant front file = %s\n", full_path);
                        vl.vm.tab_data[tab_type].video_file_count++;
                    }
                }
            }
            closedir(dir);
        } else {
            printf("open err: %s\n", VIDEO_DIR_URGENT_FRONT);
        }
        
        // 扫描后置紧急录制
        dir = opendir(VIDEO_DIR_URGENT_REAR);
        if (dir != NULL) {
            while ((entry = readdir(dir)) != NULL && vl.vm.tab_data[tab_type].video_file_count < MAX_FILES) {
                char *filename = entry->d_name;
                int len = strlen(filename);
                
                if (len > 4 && strcmp(filename + len - 4, ".mp4") == 0) {
                    char full_path[MAX_PATH_LEN];
                    snprintf(full_path, sizeof(full_path), "%s/%s", VIDEO_DIR_URGENT_REAR, filename);
                    
                    vl.vm.tab_data[tab_type].video_files[vl.vm.tab_data[tab_type].video_file_count] = malloc(strlen(full_path) + 1);
                    if (vl.vm.tab_data[tab_type].video_files[vl.vm.tab_data[tab_type].video_file_count]) {
                        strcpy(vl.vm.tab_data[tab_type].video_files[vl.vm.tab_data[tab_type].video_file_count], full_path);
                        printf("urgant rear file = %s\n", full_path);
                        vl.vm.tab_data[tab_type].video_file_count++;
                    }
                }
            }
            closedir(dir);
        } else {
            printf("open err: %s\n", VIDEO_DIR_URGENT_REAR);
        }
    }

    /* 按修改时间降序排序，最新录制的视频排在最上面 */
    if (vl.vm.tab_data[tab_type].video_file_count > 1) {
        qsort(vl.vm.tab_data[tab_type].video_files,
              (size_t)vl.vm.tab_data[tab_type].video_file_count,
              sizeof(char *),
              compare_video_mtime_desc);
    }

    char title_text[50];
    // snprintf(title_text, sizeof(title_text), "%s (%d)", tab_names[tab_type], vl.vm.tab_data[tab_type].video_file_count);
    snprintf(title_text, sizeof(title_text), "%s %d %s", \
                                    get_string_for_language(languageSetting, "Found"), \
                                    vl.vm.tab_data[tab_type].video_file_count, \
                                    get_string_for_language(languageSetting, "Record"));
    LV_FONT_DECLARE(v853Font_Medialibrary_30)
    lv_obj_set_style_text_font(vl.vm.tab_data[tab_type].title_label, &v853Font_Medialibrary_30, 0);
    lv_obj_set_style_text_color(vl.vm.tab_data[tab_type].title_label, TM_TEXT_SECONDARY, 0);
    lv_label_set_text(vl.vm.tab_data[tab_type].title_label, title_text);
}

/*====================================================================================
 * 公共函数 - 视频列表项创建
 *====================================================================================*/

/**
 * @brief 创建单个视频列表项
 * @param list 父列表对象
 * @param index 视频索引
 * @param filepath 视频文件路径
 */
static void create_video_list_item(lv_obj_t *list, int index, const char *filepath) {
    if (!list || !filepath) return;
    
    /* 提取纯文件名 */
    const char *filename = strrchr(filepath, '/');
    if (filename) filename++;
    else filename = filepath;

    /* 创建列表项按钮 */
    lv_obj_t *list_btn = lv_btn_create(list);
    lv_obj_remove_style_all(list_btn);
    lv_obj_set_width(list_btn, lv_pct(100));
    /* TODO: 字库就绪后改为 96 或 100 */
    lv_obj_set_height(list_btn, 80);
    lv_obj_add_flag(list_btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    
    /* 应用主题样式 */
    lv_obj_set_style_bg_color(list_btn, TM_BG_TERTIARY, 0);
    lv_obj_set_style_bg_opa(list_btn, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(list_btn, TM_BG_HOVER, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(list_btn, LV_OPA_COVER, LV_STATE_PRESSED);
    lv_obj_set_style_border_color(list_btn, TM_BORDER_SECONDARY, 0);
    lv_obj_set_style_border_width(list_btn, 1, 0);
    lv_obj_set_style_border_opa(list_btn, LV_OPA_50, 0);
    lv_obj_set_style_border_side(list_btn, LV_BORDER_SIDE_BOTTOM | LV_BORDER_SIDE_TOP, 0);
    lv_obj_set_style_radius(list_btn, 0, 0);
    lv_obj_set_style_pad_all(list_btn, 0, 0);

    /* 创建图标容器 */
    lv_obj_t *img_icon = lv_obj_create(list_btn);
    /* TODO: 字库就绪后改为 96x96 或 100x100 */
    lv_obj_set_size(img_icon, 80, 80);
    lv_obj_align(img_icon, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_pad_all(img_icon, 0, 0);
    lv_obj_set_style_bg_opa(img_icon, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(img_icon, 0, 0);
    
    lv_obj_t *icon = lv_img_create(img_icon);
    lv_obj_set_style_text_font(icon, &v853Font_Medialibrary_30, 0);
    lv_obj_set_style_text_color(icon, TM_TEXT_PRIMARY, 0);
    lv_img_set_src(icon, LV_SYMBOL_VIDEO);
    lv_obj_center(icon);

    /* 创建文件名标签 */
    lv_obj_t *label = lv_label_create(list_btn);
    /* TODO: 字库就绪后改为 v853Font_Medialibrary_40，并相应调整 align 的 x 偏移与图标宽度一致 */
    lv_obj_set_style_text_font(label, &v853Font_Medialibrary_30, 0);
    lv_obj_set_style_text_color(label, TM_TEXT_PRIMARY, 0);
    lv_label_set_text(label, filename);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 80, 0);

    /* 存储索引并添加点击事件 */
    lv_obj_set_user_data(list_btn, (void*)(intptr_t)index);
    lv_obj_add_event_cb(list_btn, list_btn_event_handler, LV_EVENT_CLICKED, NULL);

    /* 创建删除按钮 */
    lv_obj_t *del_btn = lv_btn_create(list_btn);
    lv_obj_set_size(del_btn, 50, 50);
    lv_obj_set_style_bg_opa(del_btn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(del_btn, 0, 0);
    /* TODO: 若列表项尺寸变更，可微调 -30 为 -35 等 */
    lv_obj_align(del_btn, LV_ALIGN_RIGHT_MID, -30, 0);
    
    /* 视频列表项 - 删除按钮图标（与照片库保持一致使用TM_ICON_SECONDARY） */
    lv_obj_t *del_icon = lv_img_create(del_btn);
    lv_obj_set_style_img_recolor(del_icon, TM_ICON_SECONDARY, 0);       /* 删除图标颜色 */
    lv_obj_set_style_img_recolor_opa(del_icon, LV_OPA_COVER, 0);
    lv_obj_set_style_img_recolor(del_icon, TM_ICON_DISABLED, LV_STATE_PRESSED); /* 按下状态 */
    lv_obj_set_style_img_recolor_opa(del_icon, LV_OPA_COVER, LV_STATE_PRESSED);
    lv_img_set_src(del_icon, &delete);
    lv_obj_center(del_icon);
    lv_obj_add_event_cb(del_btn, del_btn_event_handler, LV_EVENT_CLICKED, (void*)(intptr_t)index);
}

/**
 * @brief 填充视频列表
 * @param tab_type 标签页类型
 */
static void populate_video_list(tab_type_t tab_type) {
    if (tab_type >= TAB_COUNT || !vl.vm.tab_data[tab_type].video_list) {
        return;
    }
    /* 先清空列表容器，避免重复添加导致子项重复；并保证拔卡清空后界面立即更新 */
    lv_obj_clean(vl.vm.tab_data[tab_type].video_list);
    /* 更新标题为当前数量，与 scan_video_files 一致，避免仅切换 tab 时标题仍显示 0 */
    if (vl.vm.tab_data[tab_type].title_label) {
        char title_text[50];
        snprintf(title_text, sizeof(title_text), " %s %d %s",
                 get_string_for_language(languageSetting, "Found"),
                 vl.vm.tab_data[tab_type].video_file_count,
                 get_string_for_language(languageSetting, "Record"));
        lv_obj_set_style_text_font(vl.vm.tab_data[tab_type].title_label, &v853Font_Medialibrary_30, 0);
        lv_obj_set_style_text_color(vl.vm.tab_data[tab_type].title_label, TM_TEXT_SECONDARY, 0);
        lv_label_set_text(vl.vm.tab_data[tab_type].title_label, title_text);
    }
    for (int i = 0; i < vl.vm.tab_data[tab_type].video_file_count; i++) {
        create_video_list_item(vl.vm.tab_data[tab_type].video_list,
                               i,
                               vl.vm.tab_data[tab_type].video_files[i]);
    }
    printf("[VideoLibrary] Populated %d items in tab %d\n",
           vl.vm.tab_data[tab_type].video_file_count, tab_type);
}

/*====================================================================================
 * 事件处理函数
 *====================================================================================*/

/**
 * @brief 列表按钮点击事件处理
 */
static void list_btn_event_handler(lv_event_t *e) {
    lv_obj_t *btn = lv_event_get_target(e);
    int index = (int)(intptr_t)lv_obj_get_user_data(btn);

    printf("[VideoLibrary] Selected video from tab %d, index: %d\n", vl.vm.current_tab, index);

    if (vl.vm.current_tab < TAB_COUNT && index >= 0 && index < vl.vm.tab_data[vl.vm.current_tab].video_file_count) {
        printf("[VideoLibrary] Playing: %s\n", vl.vm.tab_data[vl.vm.current_tab].video_files[index]);
        startPlayVideo(vl.vm.tab_data[vl.vm.current_tab].video_files[index], index, vl.vm.current_tab);
    } else {
        printf("[VideoLibrary] Error: Invalid index or tab data\n");
    }
}

static void close_format_dialog() {
    if (vl.vm.mask_layer_del_dialog) {
        lv_obj_del(vl.vm.mask_layer_del_dialog);
        vl.vm.mask_layer_del_dialog = NULL;
    }
}

/**
 * @brief 刷新当前页面的视频列表
 * @param tab_type 标签页类型
 */
static void reflush_current_page(tab_type_t tab_type) {
    /* 清空列表 */
    if (UI_OBJ_IS_VALID(vl.vm.tab_data[tab_type].video_list)) {
        lv_obj_clean(vl.vm.tab_data[tab_type].video_list);
    }
    
    /* 重新扫描文件并填充列表 */
    scan_video_files(tab_type);
    populate_video_list(tab_type);

    printf("[VideoLibrary] Refresh complete, found %d files in tab %d\n", 
           vl.vm.tab_data[tab_type].video_file_count, tab_type);
}

static void ask_dialog_yes_event_cb(lv_event_t *e) {
    int index = (int)(intptr_t)lv_event_get_user_data(e);

    if (index < 0 || index >= vl.vm.tab_data[vl.vm.current_tab].video_file_count) {
        return;
    }

    const char *full_path = vl.vm.tab_data[vl.vm.current_tab].video_files[index];
    if (full_path) {
        // 删除文件
        if (remove(full_path) == 0) {
            printf("Deleted file: %s\n", full_path);
        } else {
            printf("Failed to delete file: %s\n", full_path);
            return;
        }
    }

    // 重新扫描目录并刷新列表
    reflush_current_page(vl.vm.current_tab);

    close_format_dialog();
}

static void ask_dialog_no_event_cb(lv_event_t *e) {
    (void)e; // 未使用参数
    close_format_dialog();
}

static void create_ask_dialog(lv_obj_t *parent, int index) {
    /* 视频库 - 删除确认对话框背景 */
    lv_obj_t *dialog_bg = lv_obj_create(parent);
    lv_obj_set_size(dialog_bg, LV_PCT(40), LV_PCT(40));
    lv_obj_set_pos(dialog_bg, 382, 216);
    lv_obj_set_style_bg_color(dialog_bg, TM_BG_SECONDARY, 0);
    lv_obj_set_style_radius(dialog_bg, 10, 0);
    lv_obj_set_style_pad_all(dialog_bg, 20, 0);
    lv_obj_set_style_border_width(dialog_bg, 2, 0);
    lv_obj_set_style_border_color(dialog_bg, TM_BORDER_SECONDARY, 0);
    lv_obj_clear_flag(dialog_bg, LV_OBJ_FLAG_SCROLLABLE);

    /* 视频库 - 删除确认消息 */
    lv_obj_t *msg_label = lv_label_create(dialog_bg);
    lv_label_set_text(msg_label, get_string_for_language(languageSetting, "Delete_video_confirm"));
    lv_obj_set_style_text_font(msg_label, &v853Font_Medialibrary_30, 0);
    lv_obj_set_width(msg_label, LV_PCT(100));
    lv_obj_set_style_text_align(msg_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(msg_label, TM_TEXT_PRIMARY, 0);
    lv_obj_align(msg_label, LV_ALIGN_TOP_MID, 0, LV_PCT(20));

    /* 视频库 - 确认按钮 */
    lv_obj_t *yes_btn = lv_btn_create(dialog_bg);
    lv_obj_set_size(yes_btn, 140, 80);
    lv_obj_align(yes_btn, LV_ALIGN_BOTTOM_LEFT, 20, -20);
    lv_obj_set_style_bg_color(yes_btn, TM_SUCCESS, 0);
    lv_obj_set_style_radius(yes_btn, 8, 0);
    lv_obj_add_event_cb(yes_btn, ask_dialog_yes_event_cb, LV_EVENT_CLICKED, (void*)(intptr_t)index);
    lv_obj_t *yes_label = lv_label_create(yes_btn);
    lv_label_set_text(yes_label, get_string_for_language(languageSetting, "Yes"));
    lv_obj_set_style_text_font(yes_label, &v853Font_Medialibrary_30, 0);
    lv_obj_set_style_text_color(yes_label, lv_color_hex(0xFFFFFF), 0);  /* 按钮文字始终白色 */
    lv_obj_center(yes_label);
    
    /* 视频库 - 取消按钮 */
    lv_obj_t *no_btn = lv_btn_create(dialog_bg);
    lv_obj_set_size(no_btn, 140, 80);
    lv_obj_align(no_btn, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    lv_obj_set_style_bg_color(no_btn, TM_ERROR, 0);
    lv_obj_set_style_radius(no_btn, 8, 0);
    lv_obj_add_event_cb(no_btn, ask_dialog_no_event_cb, LV_EVENT_CLICKED, NULL);    
    lv_obj_t *no_label = lv_label_create(no_btn);
    lv_label_set_text(no_label, get_string_for_language(languageSetting, "No"));
    lv_obj_set_style_text_font(no_label, &v853Font_Medialibrary_30, 0);
    lv_obj_set_style_text_color(no_label, lv_color_hex(0xFFFFFF), 0);   /* 按钮文字始终白色 */
    lv_obj_center(no_label);
}

static void show_format_sd_dialog(lv_obj_t *parent, int index) {
    // 如果已经存在对话框，先关闭
    close_format_dialog();

    // 创建遮罩层
    vl.vm.mask_layer_del_dialog = lv_obj_create(parent);
    lv_obj_set_size(vl.vm.mask_layer_del_dialog, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(vl.vm.mask_layer_del_dialog, TM_BG_PRIMARY, 0);
    lv_obj_set_style_bg_opa(vl.vm.mask_layer_del_dialog, LV_OPA_50, 0);
    lv_obj_set_style_radius(vl.vm.mask_layer_del_dialog, 0, 0);
    lv_obj_set_style_border_width(vl.vm.mask_layer_del_dialog, 0, 0);
    lv_obj_clear_flag(vl.vm.mask_layer_del_dialog, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_move_foreground(vl.vm.mask_layer_del_dialog);

    // 创建询问对话框
    create_ask_dialog(vl.vm.mask_layer_del_dialog, index);
}

static void del_btn_event_handler(lv_event_t *e) {
    lv_event_stop_bubbling(e); // 阻止事件冒泡到父对象
    lv_obj_t *btn = lv_event_get_target(e);
    int index = (int)(intptr_t)lv_event_get_user_data(e);

    if (index < 0 || index >=  vl.vm.tab_data[vl.vm.current_tab].video_file_count) {
        return;
    }

    show_format_sd_dialog(lv_scr_act(), index);
}

/**
 * @brief 刷新按钮事件处理
 */
static void refresh_btn_event_handler(lv_event_t *e) {
    tab_type_t tab_type = (tab_type_t)(intptr_t)lv_event_get_user_data(e);
    printf("[VideoLibrary] Refreshing tab %d...\n", tab_type);
    reflush_current_page(tab_type);
}

/**
 * @brief 标签切换事件处理
 */
static void tabview_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_VALUE_CHANGED) {
        vl.vm.current_tab = (tab_type_t)lv_tabview_get_tab_act(vl.vm.tabview);
        printf("[VideoLibrary] Switched to tab %d\n", vl.vm.current_tab);
    }
}

/**
 * @brief 初始化标签页
 * @param tab_type 标签页类型
 */
static void init_tab(tab_type_t tab_type) {
    scan_video_files(tab_type);
    populate_video_list(tab_type);
    printf("[VideoLibrary] Initialized tab %d with %d files\n", 
           tab_type, vl.vm.tab_data[tab_type].video_file_count);
}

// 创建单个标签页
static void create_tab_page(lv_obj_t *parent, tab_type_t tab_type, const char *tab_name) {
    lv_obj_t *tab_page = lv_tabview_add_tab(parent, tab_name);
    lv_obj_set_style_pad_all(tab_page, 0, 0);
    lv_obj_clear_flag(tab_page, LV_OBJ_FLAG_SCROLLABLE);

    // 标题栏
    lv_obj_t *title_bar = lv_obj_create(tab_page);
    lv_obj_remove_style_all(title_bar);
    lv_obj_set_size(title_bar, 1340, 60);
    lv_obj_set_style_bg_color(title_bar, TM_BG_PRIMARY, 0);
    lv_obj_set_style_bg_opa(title_bar, LV_OPA_100, 0);
    lv_obj_set_style_border_width(title_bar, 0, 0);
    lv_obj_set_style_radius(title_bar, 0, 0);
    // lv_obj_set_style_border_width(title_bar, 0, 0);
    // lv_obj_set_style_border_opa(title_bar, LV_OPA_100, 0);
    // lv_obj_set_style_pad_all(title_bar, 5, 0);
    // lv_obj_set_flex_flow(title_bar, LV_FLEX_FLOW_ROW);
    // lv_obj_set_flex_align(title_bar, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    // lv_obj_clear_flag(title_bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(title_bar, 0, 0);
    lv_obj_set_flex_flow(title_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(title_bar, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(title_bar, LV_OBJ_FLAG_SCROLLABLE);

    // 标题标签
    vl.vm.tab_data[tab_type].title_label = lv_label_create(title_bar);
    lv_obj_set_height(vl.vm.tab_data[tab_type].title_label, 60);
    char title_text[50];
    snprintf(title_text, sizeof(title_text), " %s %d %s", \
                                    get_string_for_language(languageSetting, "Found"), \
                                    vl.vm.tab_data[tab_type].video_file_count, \
                                    get_string_for_language(languageSetting, "Record"));
    lv_obj_set_style_text_font(vl.vm.tab_data[tab_type].title_label, &v853Font_Medialibrary_30, 0);
    lv_obj_set_style_text_color(vl.vm.tab_data[tab_type].title_label, TM_TEXT_SECONDARY, 0);
    lv_label_set_text(vl.vm.tab_data[tab_type].title_label, title_text);
    lv_obj_set_style_pad_all(vl.vm.tab_data[tab_type].title_label, 15, 0);

    // 【关键】创建中间占位符，将按钮推到右边
    lv_obj_t *spacer = lv_obj_create(title_bar);
    lv_obj_remove_style_all(spacer);
    lv_obj_set_style_bg_opa(spacer, LV_OPA_0, 0);
    lv_obj_set_flex_grow(spacer, 1);  // 占据所有剩余空间

    /* 视频库 - 刷新按钮 */
    lv_obj_t *refresh_btn = lv_btn_create(title_bar);
    lv_obj_set_style_pad_all(refresh_btn, 0, 0);
    lv_obj_set_size(refresh_btn, 100, 60);
    lv_obj_set_style_pad_right(refresh_btn, 40, 0);
    lv_obj_set_style_bg_opa(refresh_btn, LV_OPA_TRANSP, 0);
    lv_obj_add_event_cb(refresh_btn, refresh_btn_event_handler, LV_EVENT_CLICKED, (void*)(intptr_t)tab_type);
    /* 视频库 - 刷新按钮图标 */
    lv_obj_t *refresh_label = lv_label_create(refresh_btn);
    lv_obj_set_style_text_font(refresh_label, &v853Font_Medialibrary_30, 0);
    lv_obj_set_style_text_color(refresh_label, TM_ICON_PRIMARY, 0);       /* 刷新图标颜色 */
    lv_label_set_text(refresh_label, LV_SYMBOL_REFRESH);
    lv_obj_center(refresh_label);

    // 文件列表
    lv_obj_t *list_cont = lv_obj_create(tab_page);
    // lv_obj_set_size(list_cont, LV_PCT(100), LV_PCT(100) - 10);
    lv_obj_set_size(list_cont, LV_PCT(100), 720 - 60 - 80);
    lv_obj_set_style_pad_all(list_cont, 0, 0);
    lv_obj_set_style_border_width(list_cont, 0, 0);
    lv_obj_set_style_bg_opa(list_cont, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(list_cont, TM_BG_PRIMARY, 0);
    lv_obj_set_style_radius(list_cont, 0, 0);
    // lv_obj_align_to(list_cont, title_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_obj_set_pos(list_cont, 0, 60);
    lv_obj_clear_flag(list_cont, LV_OBJ_FLAG_SCROLLABLE);

    vl.vm.tab_data[tab_type].video_list = lv_list_create(list_cont);
    lv_obj_set_size(vl.vm.tab_data[tab_type].video_list, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_text_color(vl.vm.tab_data[tab_type].video_list, TM_TEXT_PRIMARY, 0);
    lv_obj_set_style_bg_color(vl.vm.tab_data[tab_type].video_list, TM_BG_PRIMARY, 0);
    // 为列表添加底部边距
    // lv_obj_set_style_pad_all(vl.vm.tab_data[tab_type].video_list, 10, 0);
    lv_obj_set_style_pad_bottom(vl.vm.tab_data[tab_type].video_list, 20, 0);
    lv_obj_set_style_pad_left(vl.vm.tab_data[tab_type].video_list, 15, 0);
    lv_obj_set_style_pad_right(vl.vm.tab_data[tab_type].video_list, 15, 0);
    lv_obj_set_style_radius(vl.vm.tab_data[tab_type].video_list, 0, 0);
}

/**
 * @brief 初始化视频库数据结构
 */
static void vl_init(void) {
    printf("[VideoLibrary] Initializing data structure...\n");
    
    /* 首先确保播放器被释放 */
    if (P111 != NULL) {
        printf("[VideoLibrary] Warning: Player instance not properly cleaned up\n");
        setInstanceSDPlayer();
    }
    
    /* 释放所有视频文件路径的内存 */
    for (int t = 0; t < TAB_COUNT; t++) {
        for (int i = 0; i < vl.vm.tab_data[t].video_file_count; i++) {
            if (vl.vm.tab_data[t].video_files[i]) {
                free(vl.vm.tab_data[t].video_files[i]);
                vl.vm.tab_data[t].video_files[i] = NULL;
            }
        }
        vl.vm.tab_data[t].video_file_count = 0;
    }
    
    /* 重置播放器界面UI指针 */
    vl.vv.auto_hide_timer = NULL;
    vl.vv.video_screen = NULL;
    vl.vv.back_btn = NULL;
    vl.vv.back_icon = NULL;
    vl.vv.container = NULL;
    vl.vv.row1 = NULL;
    vl.vv.row2 = NULL;
    memset(vl.vv.current_buff, 0, sizeof(vl.vv.current_buff));
    vl.vv.current_time_container = NULL;
    memset(vl.vv.current_time_canvas_buf, 0, sizeof(vl.vv.current_time_canvas_buf));
    vl.vv.current_time = NULL;
    vl.vv.total_time_container = NULL;
    memset(vl.vv.total_time_canvas_buf, 0, sizeof(vl.vv.total_time_canvas_buf));
    vl.vv.total_time = NULL;
    vl.vv.slider1 = NULL;
    vl.vv.progress_timer = NULL;
    vl.vv.is_slider_dragging = false;
    vl.vv.play_mode_btn = NULL;
    vl.vv.play_mode_icon = NULL;
    vl.vv.current_play_mode = PLAY_MODE_SEQUENCE;
    vl.vv.current_video_index = -1;
    vl.vv.prev_btn = NULL;
    vl.vv.prev_icon = NULL;
    vl.vv.next_btn = NULL;
    vl.vv.next_icon = NULL;
    vl.vv.pause_btn = NULL;
    vl.vv.pause_icon = NULL;
    vl.vv.play_icon = NULL;
    vl.vv.playPauseFlag = false;
    vl.vv.play_speed_btn = NULL;
    vl.vv.play_speed_icon = NULL;
    vl.vv.current_play_speed = PLAY_SPEED_1X;
    vl.vv.volume = NULL;
    vl.vv.volume_icon = NULL;
    vl.vv.volume_slider = NULL;
    vl.vv.auto_hide_volume_slider_timer = NULL;
    vl.vv.current_volume = 0.5f;
    
    /* 重置主界面UI指针 */
    vl.vm.cont = NULL;
    vl.vm.tabview = NULL;
    vl.vm.tab_btns = NULL;
    vl.vm.current_tab = TAB_FRONT;
    vl.vm.mask_layer_del_dialog = NULL;
    
    /* 重置标签页数据 */
    for (int t = 0; t < TAB_COUNT; t++) {
        vl.vm.tab_data[t].video_list = NULL;
        vl.vm.tab_data[t].title_label = NULL;
        vl.vm.tab_data[t].video_file_count = 0;
    }
    
    /* 重置初始化标志 */
    vl.is_initialized = false;
    
    printf("[VideoLibrary] Data structure initialized\n");
}

/*====================================================================================
 * 公共API实现
 *====================================================================================*/

/**
 * @brief 创建视频选择界面
 */
void create_video_select_ui(lv_obj_t * parent) {
    if (!parent) return;

    /* 初始化数据结构 */
    vl_init();

    /* 创建UI组件 */
    create_ui_components(parent);
    
    /* 设置初始化标志 */
    vl.is_initialized = true;
    printf("[VideoLibrary] UI created successfully\n");
}

/**
 * @brief 清理视频库资源
 * 
 * 清理顺序：
 * 1. 停止并删除所有定时器
 * 2. 移除事件回调
 * 3. 释放播放器资源
 * 4. 释放文件路径内存
 * 5. 删除UI对象
 * 6. 重置状态
 */
void cleanup_video_library(void) {
    printf("[VideoLibrary] Starting cleanup...\n");
    
    /* 1. 停止并删除所有定时器 */
    UI_SAFE_DELETE_TIMER(vl.vv.progress_timer);
    UI_SAFE_DELETE_TIMER(vl.vv.auto_hide_timer);
    UI_SAFE_DELETE_TIMER(vl.vv.auto_hide_volume_slider_timer);
    printf("[VideoLibrary] Timers deleted\n");
    
    /* 2. 移除对象上的事件回调 */
    UI_SAFE_REMOVE_EVENT(vl.vv.video_screen, screen_click_handler);
    UI_SAFE_REMOVE_EVENT(vl.vv.slider1, slider_event_cb);
    UI_SAFE_REMOVE_EVENT(vl.vv.back_btn, back_btn_event_handler);
    UI_SAFE_REMOVE_EVENT(vl.vv.pause_btn, pausePlay_btn_event_handler);
    UI_SAFE_REMOVE_EVENT(vl.vv.prev_btn, prev_btn_event_handler);
    UI_SAFE_REMOVE_EVENT(vl.vv.next_btn, next_btn_event_handler);
    UI_SAFE_REMOVE_EVENT(vl.vv.play_mode_btn, play_mode_btn_event_handler);
    UI_SAFE_REMOVE_EVENT(vl.vv.play_speed_btn, play_speed_btn_event_handler);
    UI_SAFE_REMOVE_EVENT(vl.vv.volume, volume_event_handler);
    UI_SAFE_REMOVE_EVENT(vl.vv.volume_slider, volume_slider_event_handler);
    UI_SAFE_REMOVE_EVENT(vl.vm.tabview, tabview_event_handler);
    printf("[VideoLibrary] Event callbacks removed\n");

    /* 3. 释放播放器资源 */
    closePlayVideo();
    
    /* 4. 释放所有视频文件路径的内存（在删除UI之前） */
    for (int t = 0; t < TAB_COUNT; t++) {
        for (int i = 0; i < vl.vm.tab_data[t].video_file_count; i++) {
            if (vl.vm.tab_data[t].video_files[i]) {
                free(vl.vm.tab_data[t].video_files[i]);
                vl.vm.tab_data[t].video_files[i] = NULL;
            }
        }
        vl.vm.tab_data[t].video_file_count = 0;
    }
    printf("[VideoLibrary] Video file paths freed\n");
    
    /* 5. 删除UI对象（注意顺序：先删除父对象会自动删除所有子对象） */
    
    /* 删除对话框（如果存在） */
    UI_SAFE_DELETE(vl.vm.mask_layer_del_dialog);
    
    /* 删除独立的音量滑块 */
    UI_SAFE_DELETE(vl.vv.volume_slider);
    
    /* 删除播放界面的顶层容器（将自动删除所有子控件） */
    UI_SAFE_DELETE(vl.vv.video_screen);
    printf("[VideoLibrary] Video screen deleted\n");
    
    /* 删除主界面容器（将自动删除tabview和所有tab_data中的UI对象） */
    UI_SAFE_DELETE(vl.vm.cont);
    printf("[VideoLibrary] Main container deleted\n");
    
    /* 重要：在删除vm.cont之后，不要再访问tab_data中的UI对象！ */
    /* 它们已经被自动删除了，只需要将指针置空 */
    for (int t = 0; t < TAB_COUNT; t++) {
        vl.vm.tab_data[t].video_list = NULL;
        vl.vm.tab_data[t].title_label = NULL;
    }
    
    /* 6. 确保静态播放器实例被释放 */
    if (P111 != NULL) {
        easy_player_destroy(P111);
        P111 = NULL;
        printf("[VideoLibrary] Static player instance destroyed\n");
    }
    
    /* 7. 重置初始化标志 */
    vl.is_initialized = false;
    
    printf("[VideoLibrary] Cleanup completed\n");
}

void reflush_video_library(void) {
    for (tab_type_t i = 0; i < TAB_COUNT; ++i) {
        reflush_current_page(i);
    }
}

/** 无卡时由主线程调用：清空所有 tab 列表并刷新 UI，不访问磁盘 */
void video_library_clear_all(void) {
    for (tab_type_t tab = TAB_FRONT; tab < TAB_COUNT; tab++) {
        for (int i = 0; i < vl.vm.tab_data[tab].video_file_count; i++) {
            free(vl.vm.tab_data[tab].video_files[i]);
            vl.vm.tab_data[tab].video_files[i] = NULL;
        }
        vl.vm.tab_data[tab].video_file_count = 0;
        populate_video_list(tab);
    }
}

void video_library_apply_scan_result(int tab_index, char **paths, int count) {
    if (tab_index < 0 || tab_index >= TAB_COUNT) return;
    /* 允许 paths==NULL 且 count==0，表示清空该 tab */
    if (count <= 0 && !paths) {
        tab_type_t tab = (tab_type_t)tab_index;
        for (int i = 0; i < vl.vm.tab_data[tab].video_file_count; i++) {
            free(vl.vm.tab_data[tab].video_files[i]);
            vl.vm.tab_data[tab].video_files[i] = NULL;
        }
        vl.vm.tab_data[tab].video_file_count = 0;
        populate_video_list(tab);
        return;
    }
    if (!paths) return;
    tab_type_t tab = (tab_type_t)tab_index;
    for (int i = 0; i < vl.vm.tab_data[tab].video_file_count; i++) {
        free(vl.vm.tab_data[tab].video_files[i]);
        vl.vm.tab_data[tab].video_files[i] = NULL;
    }
    vl.vm.tab_data[tab].video_file_count = 0;
    int n = count;
    if (n > MAX_FILES) n = MAX_FILES;
    for (int i = 0; i < n; i++) {
        if (!paths[i]) continue;
        vl.vm.tab_data[tab].video_files[i] = strdup(paths[i]);
        if (vl.vm.tab_data[tab].video_files[i])
            vl.vm.tab_data[tab].video_file_count++;
    }
    populate_video_list(tab);
}