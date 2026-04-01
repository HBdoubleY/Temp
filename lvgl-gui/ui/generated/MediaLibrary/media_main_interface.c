/**
 * @file playbackInterface.c
 * @brief 媒体库播放界面实现
 * 
 * 实现媒体库主界面的创建、事件处理和资源清理
 */

#include "media_main_interface.h"
#include "photoLibrary.h"
#include "videoLibrary.h"
#include "storageLibrary.h"
#include "media_work.h"
#include <malloc.h>

#include "gui_guider.h"
#include "../lvgl_system.h"
/*====================================================================================
 * 全局变量定义
 *====================================================================================*/

/* 播放界面全局实例 */
playbackInterface_t pb = {0};

/*====================================================================================
 * 私有函数声明
 *====================================================================================*/

static void cleanup_playbackInterface(void);
static void pb_init(void);
static theme_type_t map_sys_theme_to_media(THEME_MODE mode);
static void media_sync_from_sys_data(void);

static theme_type_t map_sys_theme_to_media(THEME_MODE mode) {
    switch (mode) {
        case THEME_DARK:
            return THEME_MDARK;
        case THEME_AUTO:
        case THEME_DAY:
        default:
            return THEME_LIGHT;
    }
}

static void media_sync_from_sys_data(void) {
    languageSetting = g_sys_Data.current_language;
    setting_set_theme(map_sys_theme_to_media(g_sys_Data.themeMode));
}

#if MEMTESTRSS
// 添加全局变量记录内存使用历史
static long memory_history[100];
static int memory_history_index = 0;

static void print_detailed_memory_info(const char* label) {
    printf("\n====== %s ======\n", label);

    // 1. 进程内存信息
    FILE* f = fopen("/proc/self/status", "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            if (strstr(line, "Vm") || strstr(line, "Rss")) {
                printf("%s", line);
            }
        }
        fclose(f);
    }

    // 2. malloc统计信息（如果可用）
    #ifdef __GLIBC__
    struct mallinfo mi = mallinfo();
    printf("malloc统计:\n");
    printf("  非映射内存块: %d\n", mi.arena);
    printf("  普通块数量: %d\n", mi.ordblks);
    printf("  快表块数量: %d\n", mi.smblks);
    printf("  快表空间: %d\n", mi.hblks);
    printf("  快表块大小: %d\n", mi.hblkhd);
    printf("  最大总大小: %d\n", mi.usmblks);
    printf("  空闲块空间: %d\n", mi.fsmblks);
    printf("  已释放块空间: %d\n", mi.fordblks);
    printf("  已分配总空间: %d\n", mi.uordblks);
    printf("  空闲总空间: %d\n", mi.keepcost);
    #endif

    // 3. LVGL内存统计（如果有）
    #if LV_USE_MEM_MONITOR
    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);
    printf("LVGL内存:\n");
    printf("  总大小: %ld\n", (long)mon.total_size);
    printf("  空闲大小: %ld\n", (long)mon.free_size);
    printf("  最大空闲块: %ld\n", (long)mon.free_biggest_size);
    printf("  碎片化: %d%%\n", mon.frag_pct);
    printf("  使用量: %d%%\n", mon.used_pct);
    #endif

    printf("====== 结束 %s ======\n", label);
}

static long get_vmrss_kb(void) {
    FILE* f = fopen("/proc/self/status", "r");
    if (!f) return 0;

    char line[128];
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            long rss = 0;
            char* p = line + 6;
            while (*p && (*p < '0' || *p > '9')) p++;
            sscanf(p, "%ld", &rss);
            fclose(f);
            return rss;
        }
    }
    fclose(f);
    return 0;
}

// 在playbackInterface.c中添加测试函数
void test_memory_leak_repeatedly(int count) {
    printf("=== 开始内存泄漏压力测试，循环 %d 次 ===\n", count);

    long start_rss = get_vmrss_kb();
    printf("初始内存: %ld KB\n", start_rss);

    for (int i = 0; i < count; i++) {
        printf("\n--- 第 %d 次循环 ---\n", i + 1);

        // 创建界面
        lv_sdcard_widgets();

        // 等待一下
        for (int j = 0; j < 100; j++) {
            lv_timer_handler();
            usleep(10000);
        }

        // 模拟用户操作：切换到每个tab
        for (int tab = 0; tab < 3; tab++) {
            // 模拟点击tab按钮
            if (pb.tab_buttons[tab]) {
                lv_event_send(pb.tab_buttons[tab], LV_EVENT_CLICKED, NULL);
            }
            usleep(50000);
        }

        // 退出界面
        cleanup_playbackInterface();

        // 记录每次循环后的内存
        long current_rss = get_vmrss_kb();
        printf("第 %d 次循环后内存: %ld KB (累计变化: %ld KB)\n", 
               i + 1, current_rss, current_rss - start_rss);

        // 每10次打印详细信息
        if ((i + 1) % 10 == 0) {
            print_detailed_memory_info("压力测试中间检查");
        }
    }

    long end_rss = get_vmrss_kb();
    printf("\n=== 压力测试完成 ===\n");
    printf("初始内存: %ld KB\n", start_rss);
    printf("最终内存: %ld KB\n", end_rss);
    printf("总变化: %ld KB\n", end_rss - start_rss);

    if (end_rss - start_rss > 1024) { // 增长超过1MB
        printf("警告: 可能存在内存泄漏！\n");
    } else if (end_rss - start_rss > 100) { // 增长超过100KB
        printf("注意: 有轻微内存增长\n");
    } else {
        printf("良好: 内存使用稳定\n");
    }
}
#endif

/*====================================================================================
 * 私有函数实现 - 初始化
 *====================================================================================*/

/**
 * @brief 初始化播放界面数据结构
 */
static void pb_init(void) {
    /* 重置所有指针为NULL */
    pb.pb_screen = NULL;
    pb.rightTable = NULL;
    pb.tab_btns = NULL;
    pb.rightTable_content = NULL;
    pb.back_btn = NULL;
    pb.back_icon = NULL;
    pb.temp = NULL;
    pb.temp_icon = NULL;
    pb.btn_table_view_counter = NULL;
    
    for (int i = 0; i < 3; i++) {
        pb.tab_buttons[i] = NULL;
        pb.tab_contents[i] = NULL;
    }
    
    pb.photoPage = NULL;
    pb.videoPage = NULL;
    pb.storagePage = NULL;
    pb.is_initialized = false;
    
    printf("[PlaybackInterface] Initialized data structure\n");
}

/*====================================================================================
 * 私有函数实现 - 事件处理
 *====================================================================================*/

/**
 * @brief 返回按钮事件处理
 */
static void back_btn_event_handler(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (LV_EVENT_CLICKED == code) {
        printf("[PlaybackInterface] Back button clicked, cleaning up...\n");
        cleanup_playbackInterface();
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_DVR, \
                    guider_ui.screen_DVR_del, \
                    &guider_ui.screen_MediaLibrary_del, \
                    setup_scr_screen_DVR, \
                    LV_SCR_LOAD_ANIM_NONE, \
                    0, 0, true, true);
    }
    g_sys_Data.ScreenPlayBackOut = true;
}

/* 延后刷新视频列表，避免进入媒体库时首帧在 UI 线程做扫描导致卡顿 */
static void delayed_video_refresh_timer_cb(lv_timer_t *timer) {
	(void)timer;
	media_work_submit_scan_video(0);
	media_work_submit_scan_video(1);
	media_work_submit_scan_video(2);
	media_work_submit_storage_query();
}

/* 主线程轮询 media_work 结果并分发；每约 1.5s 提交一次存储查询实现拔卡/插卡自动刷新 */
static lv_timer_t *s_media_poll_timer = NULL;
static int s_media_poll_tick = 0;
static void media_poll_timer_cb(lv_timer_t *timer) {
	(void)timer;
	media_result_t result;
	while (media_work_poll_results(&result)) {
		switch (result.type) {
			case MEDIA_RESULT_STORAGE_QUERY:
				update_storage_display(result.u.storage.used_mb, result.u.storage.total_mb);
				if (result.u.storage.used_mb < 0 && result.u.storage.total_mb < 0) {
					/* 无卡：主线程直接清空列表并刷新 UI，不依赖 worker 扫描（避免 opendir 无卡路径阻塞） */
					video_library_clear_all();
					photo_library_clear_all();
				} else {
					/* 有卡：提交一轮扫描，使当前停留的视频/照片列表在约 1.5s 内自动更新，无需切换 tab */
					media_work_submit_scan_video(0);
					media_work_submit_scan_video(1);
					media_work_submit_scan_video(2);
					media_work_submit_scan_photo(0);
					media_work_submit_scan_photo(1);
				}
				break;
			case MEDIA_RESULT_FORMAT_SD:
				storage_on_format_done(result.u.format.success);
				break;
			case MEDIA_RESULT_SCAN_VIDEO:
				video_library_apply_scan_result(result.u.scan_video.tab_index, result.u.scan_video.paths, result.u.scan_video.count);
				media_work_free_result(&result);
				break;
			case MEDIA_RESULT_SCAN_PHOTO:
				photo_library_apply_scan_result(result.u.scan_photo.tab_index, result.u.scan_photo.paths, result.u.scan_photo.count);
				media_work_free_result(&result);
				break;
			default:
				break;
		}
	}
	s_media_poll_tick++;
	if (s_media_poll_tick >= 30) {
		s_media_poll_tick = 0;
		media_work_submit_storage_query();
	}
}

/**
 * @brief 标签页按钮事件处理
 */
static void tab_btn_event_handler(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_t * btn = lv_event_get_target(e);
        int tab_index = (int)(intptr_t)lv_obj_get_user_data(btn);

        /* 隐藏所有内容页面，重置标签样式 */
        for (int i = 0; i < 3; i++) {
            if (UI_OBJ_IS_VALID(pb.videoPage)) {
                lv_obj_add_flag(pb.videoPage, LV_OBJ_FLAG_HIDDEN);
            }
            if (UI_OBJ_IS_VALID(pb.photoPage)) {
                lv_obj_add_flag(pb.photoPage, LV_OBJ_FLAG_HIDDEN);
            }
            if (UI_OBJ_IS_VALID(pb.storagePage)) {
                lv_obj_add_flag(pb.storagePage, LV_OBJ_FLAG_HIDDEN);
            }
            /* 应用非选中状态样式（使用主题颜色） */
            if (UI_OBJ_IS_VALID(pb.tab_buttons[i])) {
                ui_apply_tab_btn_style(pb.tab_buttons[i], false);
            }
        }

        /* 应用选中状态样式 */
        if (UI_OBJ_IS_VALID(pb.tab_buttons[tab_index])) {
            ui_apply_tab_btn_style(pb.tab_buttons[tab_index], true);
        }
        
        /* 显示选中的内容页面；刷新改为提交 media_work 任务，结果由 poll 定时器分发 */
        switch (tab_index) {
            case 0:
                media_work_submit_scan_video(0);
                media_work_submit_scan_video(1);
                media_work_submit_scan_video(2);
                if (UI_OBJ_IS_VALID(pb.videoPage)) {
                    lv_obj_clear_flag(pb.videoPage, LV_OBJ_FLAG_HIDDEN);
                }
                break;
            case 1:
                media_work_submit_scan_photo(0);
                media_work_submit_scan_photo(1);
                if (UI_OBJ_IS_VALID(pb.photoPage)) {
                    lv_obj_clear_flag(pb.photoPage, LV_OBJ_FLAG_HIDDEN);
                }
                break;
            case 2:
                reflush_storage_display();
                if (UI_OBJ_IS_VALID(pb.storagePage)) {
                    lv_obj_clear_flag(pb.storagePage, LV_OBJ_FLAG_HIDDEN);
                }
                break;
        }
    }
}

/*====================================================================================
 * 公共API实现
 *====================================================================================*/

void lv_sdcard_widgets(lv_obj_t *parent)
{
    /* 防止重复初始化 */
    if (pb.is_initialized) {
        printf("[PlaybackInterface] Already initialized, skipping...\n");
        return;
    }
    /* 确保 media_work 工作线程已启动，否则 poll 永远收不到存储/扫描结果 */
    if (media_work_init() != 0)
        printf("[PlaybackInterface] media_work_init failed\n");
    /* 初始化数据结构 */
    pb_init();

    /* 同步系统层主题/语言配置到 MediaLibrary 模块 */
    media_sync_from_sys_data();
    
    /* 初始化UI公共组件库（包括主题管理器） */
    ui_common_init();

    // setting_set_theme(THEME_MDARK);

    /* 创建主屏幕容器 */
    if (parent == NULL) {
        parent = lv_scr_act();
    }
    pb.pb_screen = lv_obj_create(parent);
    lv_obj_remove_style_all(pb.pb_screen);
    lv_obj_set_size(pb.pb_screen, LV_PCT(100), LV_PCT(100));
    lv_obj_center(pb.pb_screen);
    lv_obj_set_style_radius(pb.pb_screen, 0, 0);
    lv_obj_set_style_bg_opa(pb.pb_screen, LV_OPA_0, 0);
    lv_obj_set_style_border_opa(pb.pb_screen, LV_OPA_0, 0);

    /* 创建右侧标签栏容器 */
    pb.rightTable = lv_obj_create(pb.pb_screen);
    lv_obj_remove_style_all(pb.rightTable);
    lv_obj_set_size(pb.rightTable, 100, 720);
    lv_obj_align(pb.rightTable, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_bg_color(pb.rightTable, TM_BG_PRIMARY, 0);
    lv_obj_set_style_bg_opa(pb.rightTable, LV_OPA_100, 0);
    lv_obj_set_style_radius(pb.rightTable, 0, 0);
    lv_obj_clear_flag(pb.rightTable, LV_OBJ_FLAG_SCROLLABLE);

    /* 创建返回按钮 */
    pb.back_btn = lv_btn_create(pb.rightTable);
    lv_obj_set_size(pb.back_btn, LV_PCT(100), 60);
    lv_obj_set_style_bg_color(pb.back_btn, TM_ERROR, 0);
    lv_obj_set_style_bg_opa(pb.back_btn, LV_OPA_100, 0);
    lv_obj_set_style_border_width(pb.back_btn, 3, 0);
    lv_obj_set_style_border_opa(pb.back_btn, LV_OPA_100, 0);
    lv_obj_set_style_border_color(pb.back_btn, TM_BORDER_PRIMARY, 0);
    lv_obj_set_style_border_post(pb.back_btn, true, 0);
    lv_obj_set_style_bg_color(pb.back_btn, TM_ACCENT, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(pb.back_btn, LV_OPA_30, LV_STATE_PRESSED);
    lv_obj_set_style_radius(pb.back_btn, 0, 0);
    lv_obj_add_event_cb(pb.back_btn, back_btn_event_handler, LV_EVENT_ALL, NULL);

    pb.back_icon = lv_label_create(pb.back_btn);
    lv_obj_set_style_text_font(pb.back_icon, &v853Font_Medialibrary_30, 0);
    lv_label_set_text(pb.back_icon, LV_SYMBOL_CLOSE);
    lv_obj_set_style_text_color(pb.back_icon, TM_TEXT_PRIMARY, 0);
    lv_obj_center(pb.back_icon);

    /* 创建标签按钮容器 */
    pb.btn_table_view_counter = lv_obj_create(pb.rightTable);
    lv_obj_remove_style_all(pb.btn_table_view_counter);
    lv_obj_set_size(pb.btn_table_view_counter, LV_PCT(100), 660);
    lv_obj_set_pos(pb.btn_table_view_counter, 0, 60);
    lv_obj_set_flex_flow(pb.btn_table_view_counter, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(pb.btn_table_view_counter, LV_FLEX_ALIGN_START, 
                          LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER);

    /* 创建三个标签页按钮和内容页面 */
    for (int i = 0; i < 3; i++) {
        /* 创建标签按钮 */
        pb.tab_buttons[i] = lv_btn_create(pb.btn_table_view_counter);
        lv_obj_set_size(pb.tab_buttons[i], LV_PCT(100), 220);
        lv_obj_set_flex_grow(pb.tab_buttons[i], 1);

        /* 设置按钮图标 */
        lv_obj_t *icon = lv_img_create(pb.tab_buttons[i]);
        lv_obj_set_size(icon, 100, 100);
        lv_obj_center(icon);
        
        /* 右侧栏标签图标 - 设置图标源和主题颜色 */
        if (i == 0) {
            lv_img_set_src(icon, &video_tableview);
        } else if (i == 1) {
            lv_img_set_src(icon, &photo_tableview);
        } else {
            lv_img_set_src(icon, &file_tableview);
        }
        /* 图标颜色随主题变化 */
        lv_obj_set_style_img_recolor(icon, TM_ICON_PRIMARY, 0);
        lv_obj_set_style_img_recolor_opa(icon, LV_OPA_COVER, 0);

        /* 应用标签按钮样式（第一个默认选中） */
        ui_apply_tab_btn_style(pb.tab_buttons[i], (i == 0));

        /* 创建对应的内容页面 */
        pb.tab_contents[i] = lv_obj_create(pb.pb_screen);
        lv_obj_set_size(pb.tab_contents[i], 1340, 720);
        lv_obj_align(pb.tab_contents[i], LV_ALIGN_LEFT_MID, 0, 0);
        lv_obj_set_style_bg_color(pb.tab_contents[i], TM_BG_PRIMARY, 0);
        lv_obj_set_style_bg_opa(pb.tab_contents[i], LV_OPA_100, 0);
        lv_obj_set_style_radius(pb.tab_contents[i], 0, 0);
        lv_obj_set_style_border_opa(pb.tab_contents[i], LV_OPA_100, 0);
        lv_obj_set_style_border_width(pb.tab_contents[i], 1, 0);
        lv_obj_set_style_border_color(pb.tab_contents[i], TM_BORDER_SECONDARY, 0);
        lv_obj_set_style_border_post(pb.tab_contents[i], true, 0);
        lv_obj_set_style_pad_all(pb.tab_contents[i], 0, 0);

        /* 默认隐藏非第一页 */
        if (i > 0) {
            lv_obj_add_flag(pb.tab_contents[i], LV_OBJ_FLAG_HIDDEN);
        }

        /* 为按钮添加点击事件 */
        lv_obj_set_user_data(pb.tab_buttons[i], (void*)(intptr_t)i);
        lv_obj_add_event_cb(pb.tab_buttons[i], tab_btn_event_handler, LV_EVENT_CLICKED, NULL);
    }

    /* 设置页面快捷引用 */
    pb.videoPage = pb.tab_contents[0];
    pb.photoPage = pb.tab_contents[1];
    pb.storagePage = pb.tab_contents[2];

    /* 初始化各个内容页面 */
    create_video_select_ui(pb.videoPage);
    create_photo_select_ui(pb.photoPage);
    create_storage_select_ui(pb.storagePage);

    pb.is_initialized = true;
    /* 首屏延后 150ms 提交 scan_video(0) + storage_query，结果由 poll 定时器分发 */
    lv_timer_t *refresh_timer = lv_timer_create(delayed_video_refresh_timer_cb, 150, NULL);
    if (refresh_timer)
        lv_timer_set_repeat_count(refresh_timer, 1);
    /* 主线程轮询 media_work 结果（50ms），并每 1.5s 提交一次 storage_query */
    s_media_poll_tick = 0;
    s_media_poll_timer = lv_timer_create(media_poll_timer_cb, 50, NULL);
    printf("[PlaybackInterface] Interface initialized successfully\n");
}

/*====================================================================================
 * 私有函数实现 - 资源清理
 *====================================================================================*/

/**
 * @brief 清理播放界面资源
 * 
 * 清理顺序：
 * 1. 先清理子模块资源
 * 2. 移除事件回调
 * 3. 删除UI对象（只删除顶层容器，子对象自动释放）
 * 4. 重置状态
 */
static void cleanup_playbackInterface(void) {
    printf("[PlaybackInterface] Starting cleanup...\n");

    /* 检查是否已初始化 */
    if (!pb.is_initialized) {
        printf("[PlaybackInterface] Not initialized, skipping cleanup\n");
        return;
    }

    /* 0. 停止 media_work 轮询定时器 */
    if (s_media_poll_timer) {
        lv_timer_del(s_media_poll_timer);
        s_media_poll_timer = NULL;
    }
    /* 1. 先清理子模块资源 */
    cleanup_video_library();
    cleanup_photo_library();
    cleanup_storage_library();

    /* 2. 移除事件回调 */
    UI_SAFE_REMOVE_EVENT(pb.back_btn, back_btn_event_handler);
    
    for (int i = 0; i < 3; i++) {
        UI_SAFE_REMOVE_EVENT(pb.tab_buttons[i], tab_btn_event_handler);
    }

    /* 3. 删除临时对象 */
    UI_SAFE_DELETE(pb.temp);

    /* 4. 删除内容页面（先于父容器删除） */
    for (int i = 0; i < 3; i++) {
        UI_SAFE_DELETE(pb.tab_contents[i]);
        pb.tab_buttons[i] = NULL;  /* tab_buttons是btn_table_view_counter的子对象，将随父对象删除 */
    }

    /* 5. 删除右侧栏容器（将自动删除btn_table_view_counter和back_btn） */
    UI_SAFE_DELETE(pb.rightTable);
    pb.btn_table_view_counter = NULL;
    pb.back_btn = NULL;
    pb.back_icon = NULL;

    /* 6. 删除主屏幕容器 */
    UI_SAFE_DELETE(pb.pb_screen);

    /* 7. 重置所有指针和状态 */
    pb.tab_btns = NULL;
    pb.rightTable_content = NULL;
    pb.temp_icon = NULL;
    pb.photoPage = NULL;
    pb.videoPage = NULL;
    pb.storagePage = NULL;
    pb.is_initialized = false;

    printf("[PlaybackInterface] Cleanup completed\n");
}
