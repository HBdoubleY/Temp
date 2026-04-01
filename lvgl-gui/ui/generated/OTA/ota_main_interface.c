/**
 * @file MainInterface.c
 * @brief OTA主界面实现
 * 
 * 实现左侧标签栏和右侧内容区域的创建与管理
 */

#include "ota_main_interface.h"
#include "WIFIConnect.h"
#include "OTA.h"
#include "lvgl/lvgl.h"
#include <stdio.h>

#include "gui_guider.h"
#include "ota_ui.h"
#include "../lvgl_system.h"
/*============================================================================
 * 全局变量定义
 *============================================================================*/

/* 主界面全局实例 */
wifi_ota_t g_main_ui = {0};

/*============================================================================
 * 私有函数声明
 *============================================================================*/

static void tab_btn_event_handler(lv_event_t *e);
static void apply_tab_button_style(lv_obj_t *btn, int index, bool is_active);
static void apply_exit_button_style(lv_obj_t *btn);
static void apply_content_page_style(lv_obj_t *page);
static lv_color_t get_sidebar_icon_text_color(void);
static void apply_tab_icon_style(lv_obj_t *icon);
static ota_theme_type_t map_sys_theme_to_ota(THEME_MODE mode);
static void ota_sync_from_sys_data(void);

/*============================================================================
 * 私有函数实现
 *============================================================================*/

/**
 * @brief 应用标签按钮样式
 * @param btn 按钮对象
 * @param index 按钮索引
 * @param is_active 是否为激活状态
 */
static void apply_tab_button_style(lv_obj_t *btn, int index, bool is_active) {
    if (!btn) return;
    
    if (is_active) {
        /* 激活状态：使用主题色，半透明效果 */
        lv_obj_set_style_bg_color(btn, TH_BTN_TAB_ACTIVE, 0);
        lv_obj_set_style_bg_opa(btn, LV_OPA_30, 0);
    } else {
        /* 非激活状态：使用标签背景色 */
        lv_obj_set_style_bg_color(btn, TH_BTN_TAB_INACTIVE, 0);
        lv_obj_set_style_bg_opa(btn, LV_OPA_100, 0);
    }
    
    /* 边框样式 */
    lv_obj_set_style_border_color(btn, TH_BG_CARD, 0);
    lv_obj_set_style_border_width(btn, BORDER_WIDTH_THICK, 0);
    lv_obj_set_style_border_post(btn, true, 0);
    lv_obj_set_style_radius(btn, 0, 0);
}

/**
 * @brief 应用退出按钮样式
 * @param btn 按钮对象
 */
static void apply_exit_button_style(lv_obj_t *btn) {
    if (!btn) return;
    
    lv_obj_set_style_bg_color(btn, TH_BTN_EXIT, 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_100, 0);
    lv_obj_set_style_border_width(btn, BORDER_WIDTH_THICK, 0);
    lv_obj_set_style_border_opa(btn, LV_OPA_100, 0);
    lv_obj_set_style_border_color(btn, TH_BG_CARD, 0);
    lv_obj_set_style_border_post(btn, true, 0);
    lv_obj_set_style_radius(btn, 0, 0);
    
    /* 按下状态样式 */
    lv_obj_set_style_bg_color(btn, TH_PRIMARY, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn, LV_OPA_30, LV_STATE_PRESSED);
}

/**
 * @brief 应用内容页样式
 * @param page 页面对象
 */
static void apply_content_page_style(lv_obj_t *page) {
    if (!page) return;
    
    lv_obj_set_style_bg_color(page, TH_BG_CARD, 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_100, 0);
    lv_obj_set_style_pad_all(page, PADDING_MD, 0);
    lv_obj_set_style_border_width(page, BORDER_WIDTH_THIN, 0);
    lv_obj_set_style_border_color(page, TH_BORDER, 0);
}

static lv_color_t get_sidebar_icon_text_color(void) {
    return (theme_get_type() == THEME_OTA_NIGHT) ? lv_color_white() : TH_TEXT_INVERSE;
}

static void apply_tab_icon_style(lv_obj_t *icon) {
    if (!icon) return;

    if (theme_get_type() == THEME_OTA_NIGHT) {
        lv_obj_set_style_img_recolor(icon, lv_color_white(), 0);
        lv_obj_set_style_img_recolor_opa(icon, LV_OPA_COVER, 0);
    } else {
        lv_obj_set_style_img_recolor_opa(icon, LV_OPA_TRANSP, 0);
    }
}

static ota_theme_type_t map_sys_theme_to_ota(THEME_MODE mode) {
    switch (mode) {
        case THEME_DARK:
            return THEME_OTA_NIGHT;
        // case THEME_AUTO:
        case THEME_DAY:
        default:
            return THEME_OTA_LIGHT;
    }
}

static void ota_sync_from_sys_data(void) {
    ota_language_setting = g_sys_Data.current_language;
    if(g_sys_Data.themeMode == THEME_DAY || (g_sys_Data.themeMode == THEME_AUTO && g_sys_Data.gpadcVol < 1300)){
        theme_set(map_sys_theme_to_ota(THEME_DAY));
	}else{
        theme_set(map_sys_theme_to_ota(THEME_DARK));
	}
}

/**
 * @brief 标签按钮点击事件处理
 * @param e 事件对象
 */
static void tab_btn_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *btn = lv_event_get_target(e);
        int tab_index = (int)(intptr_t)lv_obj_get_user_data(btn);

        /* 如果点击的是当前选中的标签，不做任何操作 */
        if (tab_index == g_main_ui.current_tab) {
            return;
        }

        printf("[MainInterface] Tab switched to index %d\n", tab_index);

        /* 隐藏所有内容页面，重置所有标签按钮样式 */
        for (int i = 0; i < 2; i++) {
            lv_obj_add_flag(g_main_ui.tab_contents[i], LV_OBJ_FLAG_HIDDEN);
            apply_tab_button_style(g_main_ui.left_tab_buttons[i], i, false);
        }

        /* 设置当前激活的标签 */
        g_main_ui.current_tab = tab_index;
        apply_tab_button_style(g_main_ui.left_tab_buttons[tab_index], tab_index, true);
        
        /* 显示选中的内容页面 */
        lv_obj_clear_flag(g_main_ui.tab_contents[tab_index], LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief 返回按钮事件处理
 */
static void back_btn_event_handler(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (LV_EVENT_CLICKED == code) {
        printf("[PlaybackInterface] Back button clicked, cleaning up...\n");
        cleanup_ota_ui(&guider_ui);
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_SET, \
                    guider_ui.screen_SET_del, &guider_ui.screen_OTA_del, \
                    setup_scr_screen_SET, LV_SCR_LOAD_ANIM_NONE, \
                    0, 0, true, true);
        lv_obj_add_state(guider_ui.screen_SET_list_set_item0,LV_STATE_DEFAULT);
		lv_obj_add_state(guider_ui.screen_SET_list_set_item1,LV_STATE_DEFAULT);
		lv_obj_add_state(guider_ui.screen_SET_list_set_item2,LV_STATE_PRESSED);
		lv_obj_add_flag(guider_ui.screen_SET_cont_tongyong,LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(guider_ui.screen_SET_cont_guanyu, LV_OBJ_FLAG_HIDDEN);
    }
}

/*============================================================================
 * 公共函数实现
 *============================================================================*/

/**
 * @brief 初始化主界面
 * @param parent 父对象（通常是屏幕）
 */
void MainInterface_init(lv_obj_t *parent) {
    printf("[MainInterface] Initializing main interface...\n");
    
    /* 如果已经初始化过，先销毁旧的资源 */
    if (g_main_ui.is_initialized) {
        printf("[MainInterface] Already initialized, cleaning up first...\n");
        MainInterface_deinit();
    }
    
    /* 同步系统层主题/语言配置到 OTA 模块 */
    ota_sync_from_sys_data();

    /*========================================================================
     * 创建主屏幕容器
     *========================================================================*/
    g_main_ui.wifi_ota_screen = lv_obj_create(parent);
    lv_obj_remove_style_all(g_main_ui.wifi_ota_screen);
    lv_obj_set_size(g_main_ui.wifi_ota_screen, LV_PCT(100), LV_PCT(100));
    lv_obj_clear_flag(g_main_ui.wifi_ota_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(g_main_ui.wifi_ota_screen, 0, 0);
    lv_obj_set_style_bg_color(g_main_ui.wifi_ota_screen, TH_BG_MAIN, 0);
    lv_obj_set_style_bg_opa(g_main_ui.wifi_ota_screen, LV_OPA_100, 0);

    /*========================================================================
     * 创建左侧标签栏
     *========================================================================*/
    g_main_ui.leftTable = lv_obj_create(g_main_ui.wifi_ota_screen);
    lv_obj_remove_style_all(g_main_ui.leftTable);
    lv_obj_set_size(g_main_ui.leftTable, SIDEBAR_WIDTH, SIDEBAR_HEIGHT);
    lv_obj_align(g_main_ui.leftTable, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_bg_color(g_main_ui.leftTable, TH_BG_SIDEBAR, 0);
    lv_obj_set_style_bg_opa(g_main_ui.leftTable, LV_OPA_100, 0);
    lv_obj_clear_flag(g_main_ui.leftTable, LV_OBJ_FLAG_SCROLLABLE);

    /*========================================================================
     * 创建退出按钮
     *========================================================================*/
    g_main_ui.exit_btn = lv_btn_create(g_main_ui.leftTable);
    lv_obj_set_size(g_main_ui.exit_btn, LV_PCT(100), EXIT_BTN_HEIGHT);
    apply_exit_button_style(g_main_ui.exit_btn);
    
    /* 退出按钮的事件回调由外部设置 */
    lv_obj_add_event_cb(g_main_ui.exit_btn, back_btn_event_handler, LV_EVENT_CLICKED, NULL);
    
    /* 退出按钮图标 */
    g_main_ui.exit_icon = lv_label_create(g_main_ui.exit_btn);
    lv_obj_set_style_text_font(g_main_ui.exit_icon, &v853Font_OTA_42, 0);
    lv_label_set_text(g_main_ui.exit_icon, LV_SYMBOL_CLOSE);
    lv_obj_set_style_text_color(g_main_ui.exit_icon, get_sidebar_icon_text_color(), 0);
    lv_obj_center(g_main_ui.exit_icon);

    /*========================================================================
     * 创建左侧标签按钮容器
     *========================================================================*/
    g_main_ui.left_btn_container = lv_obj_create(g_main_ui.leftTable);
    lv_obj_remove_style_all(g_main_ui.left_btn_container);
    lv_obj_set_size(g_main_ui.left_btn_container, LV_PCT(100), 
                    SIDEBAR_HEIGHT - EXIT_BTN_HEIGHT);
    lv_obj_set_pos(g_main_ui.left_btn_container, 0, EXIT_BTN_HEIGHT);
    lv_obj_set_flex_flow(g_main_ui.left_btn_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(g_main_ui.left_btn_container, 
                          LV_FLEX_ALIGN_START, 
                          LV_FLEX_ALIGN_SPACE_EVENLY, 
                          LV_FLEX_ALIGN_CENTER);

    /*========================================================================
     * 创建两个标签按钮（WiFi和OTA）
     *========================================================================*/
    const lv_img_dsc_t *tab_icons[] = {&Wifi_table, &OTA_table};
    
    for (int i = 0; i < 2; i++) {
        g_main_ui.left_tab_buttons[i] = lv_btn_create(g_main_ui.left_btn_container);
        lv_obj_set_size(g_main_ui.left_tab_buttons[i], LV_PCT(100), TAB_BTN_HEIGHT);
        lv_obj_set_flex_grow(g_main_ui.left_tab_buttons[i], 1);
        
        /* 应用初始样式（第一个默认激活） */
        apply_tab_button_style(g_main_ui.left_tab_buttons[i], i, (i == 0));

        /* 创建标签图标 */
        g_main_ui.tab_icons[i] = lv_img_create(g_main_ui.left_tab_buttons[i]);
        lv_img_set_src(g_main_ui.tab_icons[i], tab_icons[i]);
        lv_obj_set_size(g_main_ui.tab_icons[i], 100, 100);
        apply_tab_icon_style(g_main_ui.tab_icons[i]);
        lv_obj_center(g_main_ui.tab_icons[i]);

        /* 设置用户数据和事件回调 */
        lv_obj_set_user_data(g_main_ui.left_tab_buttons[i], (void*)(intptr_t)i);
        lv_obj_add_event_cb(g_main_ui.left_tab_buttons[i], 
                           tab_btn_event_handler, 
                           LV_EVENT_CLICKED, NULL);
    }

    /*========================================================================
     * 创建内容页面
     *========================================================================*/
    for (int i = 0; i < 2; i++) {
        g_main_ui.tab_contents[i] = lv_obj_create(g_main_ui.wifi_ota_screen);
        lv_obj_set_size(g_main_ui.tab_contents[i], CONTENT_WIDTH, CONTENT_HEIGHT);
        lv_obj_align(g_main_ui.tab_contents[i], LV_ALIGN_RIGHT_MID, 0, 0);
        
        /* 应用内容页样式 */
        apply_content_page_style(g_main_ui.tab_contents[i]);
        
        /* 除了第一个页面外，其他都隐藏 */
        if (i > 0) {
            lv_obj_add_flag(g_main_ui.tab_contents[i], LV_OBJ_FLAG_HIDDEN);
        }
    }

    /* 设置当前选中的标签为第一个 */
    g_main_ui.current_tab = 0;

    /*========================================================================
     * 初始化子页面
     *========================================================================*/
    printf("[MainInterface] Initializing WiFi sub-page...\n");
    WIFIConnect_init(g_main_ui.tab_contents[0]);
    
    printf("[MainInterface] Initializing OTA sub-page...\n");
    OTA_init(g_main_ui.tab_contents[1]);

    /* 标记初始化完成 */
    g_main_ui.is_initialized = true;
    
    printf("[MainInterface] Initialization completed\n");
}

/**
 * @brief 销毁主界面
 */
void MainInterface_deinit(void) {
    printf("[MainInterface] Deinitializing main interface...\n");
    
    if (!g_main_ui.is_initialized) {
        printf("[MainInterface] Not initialized, skip deinit\n");
        return;
    }
    
    /* 先销毁子模块 */
    printf("[MainInterface] Deinitializing sub-modules...\n");
    WIFIConnect_deinit();
    OTA_deinit();

    /* 删除主屏幕对象（会递归删除所有子对象） */
    if (g_main_ui.wifi_ota_screen) {
        lv_obj_del(g_main_ui.wifi_ota_screen);
        g_main_ui.wifi_ota_screen = NULL;
    }

    /* 重置所有指针 */
    g_main_ui.leftTable = NULL;
    g_main_ui.left_btn_container = NULL;
    g_main_ui.exit_btn = NULL;
    g_main_ui.exit_icon = NULL;
    
    for (int i = 0; i < 2; i++) {
        g_main_ui.left_tab_buttons[i] = NULL;
        g_main_ui.tab_contents[i] = NULL;
        g_main_ui.tab_icons[i] = NULL;
    }
    
    g_main_ui.current_tab = 0;
    g_main_ui.is_initialized = false;
    
    printf("[MainInterface] Deinitialization completed\n");
}

/**
 * @brief 刷新主界面主题
 */
void MainInterface_refresh_theme(void) {
    if (!g_main_ui.is_initialized) {
        return;
    }
    
    printf("[MainInterface] Refreshing theme...\n");
    
    /* 刷新主屏幕背景 */
    if (g_main_ui.wifi_ota_screen) {
        lv_obj_set_style_bg_color(g_main_ui.wifi_ota_screen, TH_BG_MAIN, 0);
    }
    
    /* 刷新侧边栏 */
    if (g_main_ui.leftTable) {
        lv_obj_set_style_bg_color(g_main_ui.leftTable, TH_BG_SIDEBAR, 0);
    }
    
    /* 刷新退出按钮 */
    if (g_main_ui.exit_btn) {
        apply_exit_button_style(g_main_ui.exit_btn);
    }
    if (g_main_ui.exit_icon) {
        lv_obj_set_style_text_color(g_main_ui.exit_icon, get_sidebar_icon_text_color(), 0);
    }
    
    /* 刷新标签按钮 */
    for (int i = 0; i < 2; i++) {
        if (g_main_ui.left_tab_buttons[i]) {
            apply_tab_button_style(g_main_ui.left_tab_buttons[i], i, 
                                  (i == g_main_ui.current_tab));
        }

        if (g_main_ui.tab_icons[i]) {
            apply_tab_icon_style(g_main_ui.tab_icons[i]);
        }
        
        /* 刷新内容页 */
        if (g_main_ui.tab_contents[i]) {
            apply_content_page_style(g_main_ui.tab_contents[i]);
        }
    }
    
    printf("[MainInterface] Theme refresh completed\n");
}

/**
 * @brief 设置退出按钮回调
 * @param cb 回调函数
 * @param user_data 用户数据
 */
void MainInterface_set_exit_cb(lv_event_cb_t cb, void *user_data) {
    if (g_main_ui.exit_btn && cb) {
        /* 移除旧的回调（如果有的话），添加新的回调 */
        lv_obj_remove_event_cb(g_main_ui.exit_btn, NULL);
        lv_obj_add_event_cb(g_main_ui.exit_btn, cb, LV_EVENT_CLICKED, user_data);
    }
}
