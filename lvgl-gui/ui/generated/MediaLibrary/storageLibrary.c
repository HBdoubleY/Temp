/**
 * @file storageLibrary.c
 * @brief 存储空间模块实现
 * 
 * 实现SD卡容量显示、格式化等功能
 */

#include "storageLibrary.h"
#include "media_work.h"
#include "tf_card.h"
#include <stdbool.h>
#include <sys/statvfs.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

/*====================================================================================
 * 全局变量定义
 *====================================================================================*/

/* 存储库全局实例 */
storageLibrary_ctx_t g_storage = {0};

/* 快捷引用宏，简化代码 */
#define sl  (g_storage.sl)
#define sf  (g_storage.sf)

/*====================================================================================
 * 私有函数声明
 *====================================================================================*/

static void create_ui_components(lv_obj_t *parent);
static void sl_init(void);
// 回调接口
static void back_btn_event_handler(lv_event_t * e);
static void format_btn_event_handler(lv_event_t * e);
static void ask_dialog_no_event_cb(lv_event_t *e);
static void ask_dialog_yes_event_cb(lv_event_t *e);
static void close_timer_callback(lv_timer_t *timer);
// 内部函数声明（update_storage_display 带参版本在 .h 中声明，供 poll 调用）
static int64_t get_tf_card_used_capacity_mb(const char *path);
static int64_t get_tf_card_total_capacity_mb(const char *path);
static void update_progress_bar_color_by_percentage(int percentage);
static void show_format_sd_dialog(lv_obj_t *parent);
static void close_format_dialog(void);
static void create_ask_dialog(lv_obj_t *parent);
static void create_result_dialog(lv_obj_t *parent, const char *text, lv_color_t color);

/* 格式化实现见 runcarplay tf_card_format.c，经 media_work 工作线程执行 */

static void show_format_sd_dialog(lv_obj_t *parent) {
    // 如果已经存在对话框，先关闭
    close_format_dialog();
    
    // 创建遮罩层
    sf.mask_layer = lv_obj_create(parent);
    lv_obj_set_size(sf.mask_layer, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(sf.mask_layer, TM_BG_PRIMARY, 0);
    lv_obj_set_style_bg_opa(sf.mask_layer, LV_OPA_50, 0);
    lv_obj_set_style_radius(sf.mask_layer, 0, 0);
    lv_obj_set_style_border_width(sf.mask_layer, 0, 0);
    lv_obj_clear_flag(sf.mask_layer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_move_foreground(sf.mask_layer);
    
    // 创建询问对话框
    create_ask_dialog(sf.mask_layer);
}

static void close_format_dialog(void) {
    // 关闭定时器
    if (sf.close_timer) {
        lv_timer_del(sf.close_timer);
        sf.close_timer = NULL;
    }
    // 销毁结果对话框
    if (sf.result_dialog) {
        lv_obj_del(sf.result_dialog);
        sf.result_dialog = NULL;
    }
    // 销毁询问对话框和遮罩层
    if (sf.mask_layer) {
        lv_obj_del(sf.mask_layer);
        sf.mask_layer = NULL;
        sf.dialog_bg = NULL;
        sf.msg_label = NULL;
        sf.yes_btn = NULL;
        sf.no_btn = NULL;
    }
}

static void create_ask_dialog(lv_obj_t *parent) {
    /* 存储空间 - 格式化确认对话框背景 */
    sf.dialog_bg = lv_obj_create(parent);
    lv_obj_set_size(sf.dialog_bg, LV_PCT(40), LV_PCT(40));
    lv_obj_set_pos(sf.dialog_bg, 382, 216);
    lv_obj_set_style_bg_color(sf.dialog_bg, TM_BG_SECONDARY, 0);
    lv_obj_set_style_radius(sf.dialog_bg, 10, 0);
    lv_obj_set_style_pad_all(sf.dialog_bg, 20, 0);
    lv_obj_set_style_border_width(sf.dialog_bg, 2, 0);
    lv_obj_set_style_border_color(sf.dialog_bg, TM_BORDER_SECONDARY, 0);
    lv_obj_clear_flag(sf.dialog_bg, LV_OBJ_FLAG_SCROLLABLE);

    /* 存储空间 - 格式化确认消息 */
    sf.msg_label = lv_label_create(sf.dialog_bg);
    lv_label_set_text(sf.msg_label, get_string_for_language(languageSetting, "Format_confirm"));
    lv_obj_set_style_text_font(sf.msg_label, &v853Font_Medialibrary_30, 0);
    lv_obj_set_width(sf.msg_label, LV_PCT(100));
    lv_obj_set_style_text_align(sf.msg_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(sf.msg_label, TM_TEXT_PRIMARY, 0);
    lv_obj_align(sf.msg_label, LV_ALIGN_TOP_MID, 0, LV_PCT(20));
    
    /* 存储空间 - 确认按钮 */
    sf.yes_btn = lv_btn_create(sf.dialog_bg);
    lv_obj_set_size(sf.yes_btn, 140, 80);
    lv_obj_align(sf.yes_btn, LV_ALIGN_BOTTOM_LEFT, 20, -20);
    lv_obj_set_style_bg_color(sf.yes_btn, TM_SUCCESS, 0);
    lv_obj_set_style_radius(sf.yes_btn, 8, 0);
    lv_obj_add_event_cb(sf.yes_btn, ask_dialog_yes_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *yes_label = lv_label_create(sf.yes_btn);
    lv_label_set_text(yes_label, get_string_for_language(languageSetting, "Yes"));
    lv_obj_set_style_text_font(yes_label, &v853Font_Medialibrary_30, 0);
    lv_obj_set_style_text_color(yes_label, lv_color_hex(0xFFFFFF), 0);  /* 按钮文字始终白色 */
    lv_obj_center(yes_label);
    
    /* 存储空间 - 取消按钮 */
    sf.no_btn = lv_btn_create(sf.dialog_bg);
    lv_obj_set_size(sf.no_btn, 140, 80);
    lv_obj_align(sf.no_btn, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    lv_obj_set_style_bg_color(sf.no_btn, TM_ERROR, 0);
    lv_obj_set_style_radius(sf.no_btn, 8, 0);
    lv_obj_add_event_cb(sf.no_btn, ask_dialog_no_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *no_label = lv_label_create(sf.no_btn);
    lv_label_set_text(no_label, get_string_for_language(languageSetting, "No"));
    lv_obj_set_style_text_font(no_label, &v853Font_Medialibrary_30, 0);
    lv_obj_set_style_text_color(no_label, lv_color_hex(0xFFFFFF), 0);   /* 按钮文字始终白色 */
    lv_obj_center(no_label);
}

static void create_result_dialog(lv_obj_t *parent, const char *text, lv_color_t color) {
    sf.result_dialog = lv_obj_create(parent);
    lv_obj_set_size(sf.result_dialog, LV_PCT(40), LV_PCT(40));
    lv_obj_set_pos(sf.result_dialog, 382, 216);
    // lv_obj_center(sf.result_dialog);
    lv_obj_set_style_bg_color(sf.result_dialog, TM_BG_SECONDARY, 0);
    lv_obj_set_style_radius(sf.result_dialog, 10, 0);
    lv_obj_set_style_pad_all(sf.result_dialog, 20, 0);
    lv_obj_set_style_border_width(sf.result_dialog, 2, 0);
    lv_obj_set_style_border_color(sf.result_dialog, color, 0);
    lv_obj_clear_flag(sf.result_dialog, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *result_label = lv_label_create(sf.result_dialog);
    lv_label_set_text(result_label, text);
    lv_obj_set_style_text_color(result_label, color, 0);
    lv_obj_set_style_text_font(result_label, &v853Font_Medialibrary_30, 0);
    lv_obj_set_style_text_align(result_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(result_label, 210);
    lv_obj_center(result_label);
}

static void close_timer_callback(lv_timer_t *timer) {
    (void)timer;
    close_format_dialog();
    media_work_submit_storage_query();
}

/* 用户点“是”：仅提交格式化任务，结果由主线程 poll 到 MEDIA_RESULT_FORMAT_SD 后调用 storage_on_format_done(success) 处理 */
static void ask_dialog_yes_event_cb(lv_event_t *e) {
    (void)e;
    printf("[Storage] Submit format SD task (worker will run tf_card_format)\n");
    media_work_submit_format_sd();
    /* 关闭询问对话框，结果对话框与关对话框在 storage_on_format_done 中处理 */
    if (sf.dialog_bg) {
        lv_obj_del(sf.dialog_bg);
        sf.dialog_bg = NULL;
        sf.msg_label = NULL;
        sf.yes_btn = NULL;
        sf.no_btn = NULL;
    }
}

static void ask_dialog_no_event_cb(lv_event_t *e) {
    (void)e; // 未使用参数
    close_format_dialog();
}

/**
 * @brief 检查TF卡是否已挂载
 * 
 * 通过读取/proc/mounts检查指定路径是否已挂载
 * 
 * @param mount_point 挂载点路径，如 "/mnt/extsd"
 * @return true TF卡已挂载
 * @return false TF卡未挂载
 */
static bool is_tf_card_mounted(const char *mount_point) {
    FILE *fp = fopen("/proc/mounts", "r");
    if (!fp) {
        printf("[Storage] Failed to open /proc/mounts: %s\n", strerror(errno));
        return false;
    }
    
    char line[512];
    bool mounted = false;
    
    while (fgets(line, sizeof(line), fp)) {
        /* /proc/mounts 格式: device mount_point fs_type options ... */
        /* 检查该行是否包含挂载点 */
        if (strstr(line, mount_point) != NULL) {
            /* 进一步验证是否真正是挂载点（而不是设备名的一部分） */
            char device[128], mnt[128];
            if (sscanf(line, "%127s %127s", device, mnt) == 2) {
                if (strcmp(mnt, mount_point) == 0) {
                    mounted = true;
                    printf("[Storage] TF card mounted: %s -> %s\n", device, mnt);
                    break;
                }
            }
        }
    }
    
    fclose(fp);
    
    if (!mounted) {
        printf("[Storage] TF card not mounted at %s\n", mount_point);
    }
    
    return mounted;
}

static int64_t get_tf_card_total_capacity_mb(const char *path)
{
    /* 先检查TF卡是否挂载 */
    if (!is_tf_card_mounted(TF_CARD_MOUNT_PATH)) {
        printf("[Storage] TF card not mounted, cannot get total capacity\n");
        return -1;
    }
    
    struct statvfs fs_info;
    
    if (statvfs(path, &fs_info) == -1) {
        printf("Failed to get filesystem info: %s\n", strerror(errno));
        return -1;
    }
    
    // 总容量 = 块数 * 块大小
    uint64_t total_bytes = (uint64_t)fs_info.f_blocks * fs_info.f_frsize;
    int64_t total_mb = (int64_t)(total_bytes / (1024 * 1024));
    
    return total_mb;
}

static int64_t get_tf_card_used_capacity_mb(const char *path)
{
    /* 先检查TF卡是否挂载 */
    if (!is_tf_card_mounted(TF_CARD_MOUNT_PATH)) {
        printf("[Storage] TF card not mounted, cannot get used capacity\n");
        return -1;
    }
    
    struct statvfs fs_info;
    
    if (statvfs(path, &fs_info) == -1) {
        printf("Failed to get filesystem info: %s\n", strerror(errno));
        return -1;
    }
    
    // 总容量
    uint64_t total_bytes = (uint64_t)fs_info.f_blocks * fs_info.f_frsize;
    
    // 可用容量（给普通用户的）
    uint64_t avail_bytes = (uint64_t)fs_info.f_bavail * fs_info.f_frsize;
    
    // 已使用容量
    uint64_t used_bytes = total_bytes - avail_bytes;
    int64_t used_mb = (int64_t)(used_bytes / (1024 * 1024));
    
    return used_mb;
}

static void update_progress_bar_color_by_percentage(int percentage)
{
    lv_color_t indicator_color = (percentage > 80) ? TM_ERROR : TM_SUCCESS;
    lv_obj_set_style_bg_color(sl.progress_bar, indicator_color, LV_PART_INDICATOR);
}

/* 仅更新 UI；used_mb/total_mb 为 -1 或 total_mb<=0 表示未挂载，由主线程在 poll 到 MEDIA_RESULT_STORAGE_QUERY 后调用 */
void update_storage_display(int64_t used_mb, int64_t total_mb) {
    if (total_mb <= 0 || used_mb < 0) {
        lv_bar_set_value(sl.progress_bar, 0, LV_ANIM_OFF);
        update_progress_bar_color_by_percentage(0);
        lv_label_set_text(sl.percent_label, "--");
        lv_label_set_text_fmt(sl.details_label, "%s\n%s",
                            get_string_for_language(languageSetting, "SDCardNotFound"),
                            get_string_for_language(languageSetting, "SDCardInsertTip"));
        return;
    }
    int percentage = (int)((used_mb * 100) / total_mb);
    update_progress_bar_color_by_percentage(percentage);
    lv_bar_set_value(sl.progress_bar, percentage, LV_ANIM_ON);
    lv_label_set_text_fmt(sl.percent_label, "%d%%", percentage);
    int used_gb_int = (int)(used_mb / 1024);
    int used_gb_frac = (int)((used_mb % 1024) * 100 / 1024);
    int total_gb_int = (int)(total_mb / 1024);
    int total_gb_frac = (int)((total_mb % 1024) * 100 / 1024);
    int64_t free_mem = total_mb - used_mb;
    if (free_mem < 0) free_mem = 0;
    int free_gb_int = (int)(free_mem / 1024);
    int free_gb_frac = (int)((free_mem % 1024) * 100 / 1024);
    lv_label_set_text_fmt(sl.details_label,
                         "%s: %d.%02d GB / %d.%02d GB\n"
                         "%s: %d.%02d GB",
                         get_string_for_language(languageSetting, "SDCardUsed"), used_gb_int, used_gb_frac, total_gb_int, total_gb_frac,
                         get_string_for_language(languageSetting, "SDCardFree"), free_gb_int, free_gb_frac);
}

static void format_btn_event_handler(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    if (LV_EVENT_CLICKED == code) {
        show_format_sd_dialog(lv_scr_act());
    }
}

static void back_btn_event_handler(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    if (LV_EVENT_CLICKED == code) {
        lv_obj_del(sl.container);
        // lv_obj_clear_flag(sl.pb, LV_OBJ_FLAG_HIDDEN);
        lv_sdcard_widgets();
    }
}

static void create_ui_components(lv_obj_t *parent) {
    /*==================== 存储空间界面 - 主容器 ====================*/
    sl.container = lv_obj_create(parent);
    lv_obj_set_size(sl.container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(sl.container, TM_BG_PRIMARY, 0);            /* 页面背景 */
    lv_obj_set_style_bg_opa(sl.container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_opa(sl.container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_radius(sl.container, 0, 0);
    lv_obj_center(sl.container);
    lv_obj_clear_flag(sl.container, LV_OBJ_FLAG_SCROLLABLE);

    /*==================== 存储空间界面 - 标题 ====================*/
    sl.title_label = lv_label_create(sl.container);
    lv_obj_set_style_text_color(sl.title_label, TM_TEXT_PRIMARY, 0);      /* 标题文字颜色 */
    lv_obj_set_style_text_font(sl.title_label, &v853Font_Medialibrary_40, 0);
    lv_label_set_text(sl.title_label, get_string_for_language(languageSetting, "SDCardTitle"));
    lv_obj_align(sl.title_label, LV_ALIGN_TOP_MID, 0, 25);

    /*==================== 存储空间界面 - 进度条 ====================*/
    sl.progress_bar = lv_bar_create(sl.container);
    lv_obj_set_size(sl.progress_bar, 1000, 50);
    lv_obj_align(sl.progress_bar, LV_ALIGN_TOP_MID, 0, 275);
    lv_obj_set_style_bg_color(sl.progress_bar, TM_PROGRESS_BG, LV_PART_MAIN);  /* 进度条背景 */
    lv_obj_set_style_bg_opa(sl.progress_bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(sl.progress_bar, 10, LV_PART_MAIN);
    lv_obj_set_style_bg_color(sl.progress_bar, TM_SUCCESS, LV_PART_INDICATOR); /* 进度条前景(绿色) */
    lv_obj_set_style_bg_opa(sl.progress_bar, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_radius(sl.progress_bar, 10, LV_PART_INDICATOR);
    lv_bar_set_range(sl.progress_bar, 0, 100);

    /*==================== 存储空间界面 - SD卡图标 ====================*/
    lv_obj_t *label_left = lv_label_create(sl.container);
    lv_obj_set_style_text_font(label_left, &v853Font_Medialibrary_30, 0);
    lv_obj_set_style_text_color(label_left, TM_ICON_PRIMARY, 0);          /* SD卡图标颜色 */
    lv_obj_set_style_bg_opa(label_left, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_label_set_text(label_left, LV_SYMBOL_SD_CARD);
    lv_obj_align_to(label_left, sl.progress_bar, LV_ALIGN_OUT_TOP_LEFT, 10, -15);

    /*==================== 存储空间界面 - 百分比标签 ====================*/
    sl.percent_label = lv_label_create(sl.container);
    lv_obj_set_style_text_font(sl.percent_label, &v853Font_Medialibrary_30, 0);
    lv_obj_set_style_text_color(sl.percent_label, TM_ACCENT, 0);          /* 百分比文字(强调色) */
    lv_obj_align_to(sl.percent_label, sl.progress_bar, LV_ALIGN_OUT_TOP_RIGHT, 0, -15);
    lv_label_set_text(sl.percent_label, "--");  /* 初始值，poll 到结果后由 update_storage_display 更新 */

    /*==================== 存储空间界面 - 使用情况详情 ====================*/
    sl.details_label = lv_label_create(sl.container);
    lv_obj_set_style_text_font(sl.details_label, &v853Font_Medialibrary_30, 0);
    lv_obj_set_style_text_color(sl.details_label, TM_TEXT_SECONDARY, 0);  /* 详情文字颜色 */
    lv_obj_set_style_text_align(sl.details_label, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_style_text_line_space(sl.details_label, 8, 0);
    lv_obj_align_to(sl.details_label, sl.progress_bar, LV_ALIGN_OUT_RIGHT_MID, -350, 100);
    /* 初始显示“未检测到”或占位，避免显示默认 "Text"；poll 到结果后由 update_storage_display 更新 */
    {
        const char *s1 = get_string_for_language(languageSetting, "SDCardNotFound");
        const char *s2 = get_string_for_language(languageSetting, "SDCardInsertTip");
        if (s1 && s2)
            lv_label_set_text_fmt(sl.details_label, "%s\n%s", s1, s2);
        else
            lv_label_set_text(sl.details_label, "--");
    }

    /*==================== 存储空间界面 - 格式化按钮 ====================*/
    sl.format_btn = lv_btn_create(sl.container);
    /* TODO: 字库就绪后改为更大尺寸，如 480x60 */
    lv_obj_set_size(sl.format_btn, 400, 80);
    lv_obj_set_style_bg_color(sl.format_btn, TM_BG_TERTIARY, 0);          /* 按钮背景 */
    lv_obj_set_style_bg_opa(sl.format_btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(sl.format_btn, TM_BORDER_PRIMARY, 0);   /* 按钮边框 */
    lv_obj_set_style_border_width(sl.format_btn, 1, 0);
    lv_obj_set_style_border_opa(sl.format_btn, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(sl.format_btn, TM_ACCENT, LV_STATE_PRESSED); /* 按下状态 */
    lv_obj_set_style_bg_opa(sl.format_btn, LV_OPA_COVER, LV_STATE_PRESSED);
    lv_obj_set_style_radius(sl.format_btn, 8, 0);
    lv_obj_align(sl.format_btn, LV_ALIGN_TOP_MID, 0, 550);
    lv_obj_add_event_cb(sl.format_btn, format_btn_event_handler, LV_EVENT_CLICKED, NULL);
    
    /*==================== 存储空间界面 - 格式化按钮文字 ====================*/
    sl.format_btn_icon = lv_label_create(sl.format_btn);
    /* TODO: 字库就绪后替换为 v853Font_Medialibrary_40 或更大字号 */
    lv_obj_set_style_text_font(sl.format_btn_icon, &v853Font_Medialibrary_30, 0);
    lv_label_set_text(sl.format_btn_icon, get_string_for_language(languageSetting, "SDCardFormat"));
    lv_obj_set_style_text_color(sl.format_btn_icon, TM_ACCENT, 0);        /* 按钮文字(强调色) */
    lv_obj_center(sl.format_btn_icon);

    /* 初始显示由 poll 收到 MEDIA_RESULT_STORAGE_QUERY 后调用 update_storage_display */
    media_work_submit_storage_query();
}
#if 1
static void sl_init() {
    sl.pb = NULL;
    sl.container = NULL;
    sl.back_btn = NULL;
    sl.back_icon = NULL;
    sl.title_label = NULL;
    sl.progress_bar = NULL;
    sl.percent_label = NULL;
    sl.details_label = NULL;
    sl.format_btn = NULL;
    sl.format_btn_icon = NULL;
    sf.mask_layer = NULL;
    sf.dialog_bg = NULL;
    sf.msg_label = NULL;
    sf.yes_btn = NULL;
    sf.no_btn = NULL;
    sf.result_dialog = NULL;
    sf.close_timer = NULL;

}
#else

#endif
void create_storage_select_ui(lv_obj_t *parent) {
    if (!parent) return;

    sl.pb = parent;

    // 初始化数据（不创建UI）
    sl_init();

    // 创建UI组件
    create_ui_components(parent);
}

// 完整的 cleanup_storage_library 函数
void cleanup_storage_library(void) {
    printf("Cleaning up storage library resources\n");
    
    // 1. 先关闭格式化对话框和相关资源
    // 关闭定时器
    if (sf.close_timer) {
        lv_timer_del(sf.close_timer);
        sf.close_timer = NULL;
    }

    // 销毁结果对话框
    if (sf.result_dialog) {
        lv_obj_del(sf.result_dialog);
        sf.result_dialog = NULL;
    }
    
    // 销毁询问对话框和遮罩层
    if (sf.mask_layer) {
        lv_obj_del(sf.mask_layer);
        sf.mask_layer = NULL;
        sf.dialog_bg = NULL;    // 这些是子对象，会被自动删除，但指针需要置NULL
        sf.msg_label = NULL;
        sf.yes_btn = NULL;
        sf.no_btn = NULL;
    }
    
    // 2. 销毁主界面组件
    // 注意：销毁容器会自动销毁其所有子对象
    if (sl.container) {
        lv_obj_del(sl.container);
        sl.container = NULL;
        
        // 子对象会被自动删除，只需将指针置NULL
        sl.back_btn = NULL;
        sl.back_icon = NULL;
        sl.title_label = NULL;
        sl.progress_bar = NULL;
        sl.percent_label = NULL;
        sl.details_label = NULL;
        sl.format_btn = NULL;
        sl.format_btn_icon = NULL;
    }
    
    // 3. 最后重置父对象指针
    sl.pb = NULL;

    
    printf("Storage library cleanup completed\n");
}

void reflush_storage_display(void) {
    media_work_submit_storage_query();
}

void storage_on_format_done(int success) {
    close_format_dialog();
    /* 结果对话框父对象用当前屏幕（close_format_dialog 已删掉 mask_layer） */
    lv_obj_t *parent = lv_scr_act();
    if (success)
        create_result_dialog(parent, get_string_for_language(languageSetting, "Format_success"), TM_SUCCESS);
    else
        create_result_dialog(parent, get_string_for_language(languageSetting, "Format_failed"), TM_ERROR);
    sf.close_timer = lv_timer_create(close_timer_callback, 1000, NULL);
    media_work_submit_storage_query();
}