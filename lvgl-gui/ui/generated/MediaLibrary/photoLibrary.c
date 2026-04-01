/**
 * @file photoLibrary.c
 * @brief 照片库模块实现
 * 
 * 实现照片文件列表显示、图片查看、缩放、旋转、删除等功能
 */

#include "photoLibrary.h"
#include "photo_view.h"

/*====================================================================================
 * 全局变量定义
 *====================================================================================*/

/* 照片库全局实例 */
photoLibrary_t pl = {0};

/*====================================================================================
 * 私有函数声明
 *====================================================================================*/

/* UI创建 */
static void create_ui_components(lv_obj_t *parent);
static void pl_init(void);

/* 事件处理 */
static void back_btn_event_handler(lv_event_t *e);
static void tabview_event_handler(lv_event_t *e);
static void refresh_btn_event_handler(lv_event_t *e);
static void prev_btn_event_handler(lv_event_t *e);
static void next_btn_event_handler(lv_event_t *e);
static void on_image_touch(lv_event_t *e);
static void small_btn_event_handler(lv_event_t *e);
static void big_btn_event_handler(lv_event_t *e);
static void rota_btn_event_handler(lv_event_t *e);
static void reset_btn_event_handler(lv_event_t *e);
static void list_btn_event_handler(lv_event_t *e);
static void del_btn_event_handler(lv_event_t *e);

/* 内部工具函数 */
static void create_photo_tab_page(lv_obj_t *parent, int tab_index, const char *tab_name);
static void init_photo_tab(int tab_index);
static void scan_photo_files(int tab_index);
static void populate_photo_list(int tab_index);
static void free_photo_paths_tab(int tab_index);
static void scan_photo_directory_recursive(int tab_index, const char *dir_path);
static int compare_strings(const void *a, const void *b);
static int is_jpg_file(const char *filename);
static void add_photo_path_to_tab(int tab_index, const char *full_path);
/** 从任意子控件向上查找所属的 tab 列表，返回 tab 索引，否则 -1 */
static int get_photo_tab_from_child(lv_obj_t *obj);
static void show_photo_viewer(int index);
static void load_and_display_image(const char *path);
static void update_zoom_label(void);

/* canvas相关 */
static int canvas_display_image(lv_obj_t *canvas, const char *img_path);
static int canvas_zoom(lv_obj_t *canvas, float scale);
static int canvas_move(lv_obj_t *canvas, lv_coord_t x, lv_coord_t y);
static bool rotate_canvas_90_clockwise_argb8888(lv_obj_t *canvas);

/*====================================================================================
 * UI创建函数
 *====================================================================================*/

static void create_ui_components(lv_obj_t *parent) {
    pl.pm.cont = lv_obj_create(parent);
    lv_obj_remove_style_all(pl.pm.cont);
    lv_obj_set_size(pl.pm.cont, lv_pct(100), lv_pct(100));
    lv_obj_clear_flag(pl.pm.cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(pl.pm.cont, 0, 0);
    lv_obj_set_style_bg_color(pl.pm.cont, TM_BG_PRIMARY, 0);

    /* 标签视图（与视频库一致：LV_DIR_TOP, 80） */
    pl.pm.tabview = lv_tabview_create(pl.pm.cont, LV_DIR_TOP, 80);
    lv_obj_set_size(pl.pm.tabview, LV_PCT(100), LV_PCT(100));
    lv_obj_set_scroll_dir(pl.pm.tabview, LV_DIR_HOR);
    lv_obj_set_scroll_snap_x(pl.pm.tabview, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_scrollbar_mode(pl.pm.tabview, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(pl.pm.tabview, LV_OBJ_FLAG_SCROLL_CHAIN_HOR);
    lv_obj_add_flag(pl.pm.tabview, LV_OBJ_FLAG_SCROLL_ONE);

    pl.pm.tab_btns = lv_tabview_get_tab_btns(pl.pm.tabview);
    if (!pl.pm.tab_btns) {
        pl.pm.tab_btns = lv_obj_get_child(pl.pm.tabview, 0);
    }
    if (pl.pm.tab_btns) {
        lv_obj_set_style_bg_color(pl.pm.tab_btns, TM_BG_SECONDARY, 0);
        lv_obj_set_style_bg_opa(pl.pm.tab_btns, LV_OPA_COVER, 0);
        lv_obj_set_style_text_color(pl.pm.tab_btns, TM_TEXT_PRIMARY, 0);
        lv_obj_set_style_text_font(pl.pm.tab_btns, &v853Font_Medialibrary_40, 0);
        lv_obj_set_style_border_width(pl.pm.tab_btns, 0, 0);
        lv_obj_set_style_radius(pl.pm.tab_btns, 0, 0);
    }

    create_photo_tab_page(pl.pm.tabview, PHOTO_TAB_FRONT, get_string_for_language(languageSetting, "Front_Photo"));
    create_photo_tab_page(pl.pm.tabview, PHOTO_TAB_REAR,  get_string_for_language(languageSetting, "Rear_Photo"));

    lv_tabview_set_act(pl.pm.tabview, 0, LV_ANIM_OFF);
    pl.pm.current_photo_tab = PHOTO_TAB_FRONT;
    lv_obj_add_event_cb(pl.pm.tabview, tabview_event_handler, LV_EVENT_VALUE_CHANGED, NULL);

    lv_style_init(&pl.pm.style_btn);
    lv_style_set_pad_all(&pl.pm.style_btn, 0);
    lv_style_set_pad_right(&pl.pm.style_btn, 40);
    lv_style_set_bg_opa(&pl.pm.style_btn, LV_OPA_0);

    init_photo_tab(PHOTO_TAB_FRONT);
    init_photo_tab(PHOTO_TAB_REAR);

    // 创建查看容器（初始隐藏）
    pl.pv.cont = lv_obj_create(pl.pm.cont);
    lv_obj_set_size(pl.pv.cont, lv_pct(100), lv_pct(100));
    lv_obj_add_flag(pl.pv.cont, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_pad_all(pl.pv.cont, 0, 0);
    lv_obj_set_style_bg_color(pl.pv.cont, TM_BG_PRIMARY, 0);
    lv_obj_clear_flag(pl.pv.cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(pl.pv.cont);
    // lv_obj_move_foreground(pl.pv.cont);

    pl.pv.img_wrapper = lv_obj_create(pl.pv.cont);
    lv_obj_remove_style_all(pl.pv.img_wrapper);
    lv_obj_set_size(pl.pv.img_wrapper, lv_pct(100), lv_pct(100));
    // lv_obj_set_y(pl.pv.img_wrapper, 60);
    // lv_obj_set_height(pl.pv.img_wrapper, lv_pct(100) - 120);              // 减去顶部栏和底部栏高度
    lv_obj_set_style_bg_color(pl.pv.img_wrapper, TM_BG_PRIMARY, 0);
    lv_obj_set_style_border_width(pl.pv.img_wrapper, 0, 0);
    lv_obj_set_style_pad_all(pl.pv.img_wrapper, 0, 0);

    // lv_obj_clear_flag(pl.pv.img_wrapper, LV_OBJ_FLAG_CLICKABLE);
    // lv_obj_clear_flag(pl.pv.img_wrapper, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_clear_flag(pl.pv.img_wrapper, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_clear_flag(pl.pv.img_wrapper, LV_OBJ_FLAG_SCROLL_CHAIN);
    // lv_obj_clear_flag(pl.pv.img_wrapper, LV_OBJ_FLAG_SCROLL_ELASTIC);
    // lv_obj_clear_flag(pl.pv.img_wrapper, LV_OBJ_FLAG_SCROLL_MOMENTUM);

    // lv_obj_set_scrollbar_mode(pl.pv.img_wrapper, LV_SCROLLBAR_MODE_OFF);
    // lv_obj_set_scroll_dir(pl.pv.img_wrapper, LV_DIR_NONE);
    lv_obj_set_style_clip_corner(pl.pv.img_wrapper, false, 0);
    lv_obj_set_style_radius(pl.pv.img_wrapper, 0, 0);

    // 创建图片对象
    pl.pv.img = lv_canvas_create(pl.pv.img_wrapper);
    lv_obj_add_flag(pl.pv.img, LV_OBJ_FLAG_CLICKABLE);
    // lv_obj_add_flag(pl.pv.img, LV_OBJ_FLAG_GESTURE_BUBBLE);
    // 添加触摸事件
    lv_obj_add_event_cb(pl.pv.img, on_image_touch, LV_EVENT_ALL, NULL);

    // 创建查看模式顶部栏
    pl.pv.top_bar = lv_obj_create(pl.pv.cont);
    lv_obj_remove_style_all(pl.pv.top_bar);
    // lv_obj_set_size(pl.pv.top_bar, lv_pct(100), 50);
    lv_obj_set_size(pl.pv.top_bar, lv_pct(100), 80);
    lv_obj_align(pl.pv.top_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(pl.pv.top_bar, TM_BG_SECONDARY, 0);
    lv_obj_set_style_bg_opa(pl.pv.top_bar, LV_OPA_50, 0);
    lv_obj_set_style_border_width(pl.pv.top_bar, 0, 0);
    lv_obj_set_style_pad_all(pl.pv.top_bar, 0, 0);
    lv_obj_set_style_radius(pl.pv.top_bar, 0, 0);
    lv_obj_set_flex_flow(pl.pv.top_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(pl.pv.top_bar, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    // lv_obj_set_flex_align(pl.pv.top_bar, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(pl.pv.top_bar, LV_OBJ_FLAG_SCROLLABLE);

    // 创建返回按钮
    pl.pv.back_btn = lv_btn_create(pl.pv.top_bar);
    lv_obj_set_size(pl.pv.back_btn, 70, 50);
    lv_obj_set_style_bg_opa(pl.pv.back_btn, LV_OPA_TRANSP, 0);        // 设置背景透明
    lv_obj_set_style_border_opa(pl.pv.back_btn, LV_OPA_TRANSP, 0);    // 设置边缘透明
    lv_obj_set_style_bg_color(pl.pv.back_btn, TM_ACCENT, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(pl.pv.back_btn, LV_OPA_30, LV_STATE_PRESSED);  // 50%透明度
    // lv_obj_set_flex_grow(pl.pv.back_btn, 1);
    // 设置返回按钮为左对齐
    lv_obj_set_style_base_dir(pl.pv.back_btn, LV_BASE_DIR_LTR, 0);
    lv_obj_set_style_flex_main_place(pl.pv.back_btn, LV_FLEX_ALIGN_START, 0);
    lv_obj_add_event_cb(pl.pv.back_btn, back_btn_event_handler, LV_EVENT_CLICKED, NULL);
    /* 照片查看器 - 返回按钮图标 */
    pl.pv.back_icon = lv_img_create(pl.pv.back_btn);
    lv_img_set_src(pl.pv.back_icon, &Photo_viewer_back);
    lv_obj_set_style_img_recolor(pl.pv.back_icon, TM_ICON_PRIMARY, 0);    /* 图标颜色 */
    lv_obj_set_style_img_recolor_opa(pl.pv.back_icon, LV_OPA_COVER, 0);
    lv_obj_set_size(pl.pv.back_icon, 70, 50);
    lv_obj_center(pl.pv.back_icon);

    // 创建文件名标签
    pl.pv.file_label = lv_label_create(pl.pv.top_bar);
    lv_obj_set_style_text_font(pl.pv.file_label, &v853Font_Medialibrary_30, 0);
    lv_obj_set_style_text_color(pl.pv.file_label, TM_TEXT_PRIMARY, 0);
    lv_obj_set_flex_grow(pl.pv.file_label, 1);
    // 设置标签居中
    lv_obj_set_style_text_align(pl.pv.file_label, LV_TEXT_ALIGN_CENTER, 0);
    // 设置标签自身居中
    lv_obj_set_style_flex_main_place(pl.pv.file_label, LV_FLEX_ALIGN_CENTER, 0);

    // 创建上一张/下一张按钮容器
    pl.pv.nav_btn_cont = lv_obj_create(pl.pv.top_bar);
    lv_obj_set_size(pl.pv.nav_btn_cont, 120, 50);
    // lv_obj_set_style_bg_color(pl.pv.nav_btn_cont, lv_color_hex(0x1A1A1A), 0);
    // lv_obj_set_style_border_width(pl.pv.nav_btn_cont, 0, 0);
    lv_obj_set_style_bg_opa(pl.pv.nav_btn_cont, LV_OPA_TRANSP, 0);        // 设置背景透明
    lv_obj_set_style_border_opa(pl.pv.nav_btn_cont, LV_OPA_TRANSP, 0);    // 设置边缘透明
    lv_obj_set_style_pad_all(pl.pv.nav_btn_cont, 0, 0);
    // lv_obj_set_flex_grow(pl.pv.nav_btn_cont, 1);
    // 设置导航容器为右对齐
    lv_obj_set_style_flex_main_place(pl.pv.nav_btn_cont, LV_FLEX_ALIGN_END, 0);
    // lv_obj_set_flex_flow(pl.pv.nav_btn_cont, LV_FLEX_FLOW_ROW);
    // lv_obj_set_flex_align(pl.pv.nav_btn_cont, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(pl.pv.nav_btn_cont, LV_OBJ_FLAG_SCROLLABLE);

    // 创建上一张按钮
    pl.pv.prev_btn = lv_btn_create(pl.pv.nav_btn_cont);
    lv_obj_set_pos(pl.pv.prev_btn, 0, 0);
    lv_obj_set_size(pl.pv.prev_btn, 40, 50);
    lv_obj_set_style_bg_opa(pl.pv.prev_btn, LV_OPA_TRANSP, 0);        // 设置背景透明
    lv_obj_set_style_border_opa(pl.pv.prev_btn, LV_OPA_TRANSP, 0);    // 设置边缘透明
    lv_obj_set_style_bg_color(pl.pv.prev_btn, TM_ACCENT, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(pl.pv.prev_btn, LV_OPA_30, LV_STATE_PRESSED);  // 50%透明度
    // lv_obj_set_flex_grow(pl.pv.prev_btn, 1);
    lv_obj_add_event_cb(pl.pv.prev_btn, prev_btn_event_handler, LV_EVENT_CLICKED, NULL);
    /* 照片查看器 - 上一张按钮图标 */
    pl.pv.prev_icon = lv_img_create(pl.pv.prev_btn);
    lv_img_set_src(pl.pv.prev_icon, &Photo_viewer_prev);
    lv_obj_set_style_img_recolor(pl.pv.prev_icon, TM_ICON_PRIMARY, 0);    /* 图标颜色 */
    lv_obj_set_style_img_recolor_opa(pl.pv.prev_icon, LV_OPA_COVER, 0);
    lv_obj_set_size(pl.pv.prev_icon, 40, 50);
    lv_obj_center(pl.pv.prev_icon);

    // 创建下一张按钮
    pl.pv.next_btn = lv_btn_create(pl.pv.nav_btn_cont);
    lv_obj_set_pos(pl.pv.next_btn, 60, 0);
    lv_obj_set_size(pl.pv.next_btn, 40, 50);
    lv_obj_set_style_bg_opa(pl.pv.next_btn, LV_OPA_TRANSP, 0);        // 设置背景透明
    lv_obj_set_style_border_opa(pl.pv.next_btn, LV_OPA_TRANSP, 0);    // 设置边缘透明
    lv_obj_set_style_bg_color(pl.pv.next_btn, TM_ACCENT, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(pl.pv.next_btn, LV_OPA_30, LV_STATE_PRESSED);  // 50%透明度
    // lv_obj_set_flex_grow(pl.pv.next_btn, 1);
    lv_obj_add_event_cb(pl.pv.next_btn, next_btn_event_handler, LV_EVENT_CLICKED, NULL);
    /* 照片查看器 - 下一张按钮图标 */
    pl.pv.next_icon = lv_img_create(pl.pv.next_btn);
    lv_img_set_src(pl.pv.next_icon, &Photo_viewer_next);
    lv_obj_set_style_img_recolor(pl.pv.next_icon, TM_ICON_PRIMARY, 0);    /* 图标颜色 */
    lv_obj_set_style_img_recolor_opa(pl.pv.next_icon, LV_OPA_COVER, 0);
    lv_obj_set_size(pl.pv.next_icon, 40, 50);
    lv_obj_center(pl.pv.next_icon);

    // 创建底部控制栏
    pl.pv.bottom_bar = lv_obj_create(pl.pv.cont);
    lv_obj_remove_style_all(pl.pv.bottom_bar);
    lv_obj_set_size(pl.pv.bottom_bar, lv_pct(100), 50);
    lv_obj_align(pl.pv.bottom_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(pl.pv.bottom_bar, TM_BG_SECONDARY, 0);
    lv_obj_set_style_bg_opa(pl.pv.bottom_bar, LV_OPA_50, 0);
    lv_obj_set_style_border_width(pl.pv.bottom_bar, 0, 0);
    lv_obj_set_style_pad_all(pl.pv.bottom_bar, 5, 0);
    lv_obj_set_style_radius(pl.pv.img_wrapper, 0, 0);
    lv_obj_set_flex_flow(pl.pv.bottom_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(pl.pv.bottom_bar, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(pl.pv.bottom_bar, LV_OBJ_FLAG_SCROLLABLE);

    pl.pv.small_btn = lv_btn_create(pl.pv.bottom_bar);
    lv_obj_set_size(pl.pv.small_btn, 60, 50);
    lv_obj_set_style_bg_opa(pl.pv.small_btn, LV_OPA_TRANSP, 0);        // 设置背景透明
    lv_obj_set_style_border_opa(pl.pv.small_btn, LV_OPA_TRANSP, 0);    // 设置边缘透明
    lv_obj_set_style_bg_color(pl.pv.small_btn, TM_ACCENT, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(pl.pv.small_btn, LV_OPA_30, LV_STATE_PRESSED);  // 50%透明度
    lv_obj_set_flex_grow(pl.pv.small_btn, 1);
    lv_obj_add_event_cb(pl.pv.small_btn, small_btn_event_handler, LV_EVENT_CLICKED, NULL);
    // lv_obj_add_flag(pl.pv.small_btn, LV_OBJ_FLAG_CHECKABLE);
    /* 照片查看器 - 缩小按钮图标 */
    pl.pv.small_btn_icon = lv_img_create(pl.pv.small_btn);
    lv_img_set_src(pl.pv.small_btn_icon, &Photo_viewer_big);
    lv_obj_set_style_img_recolor(pl.pv.small_btn_icon, TM_ICON_PRIMARY, 0);
    lv_obj_set_style_img_recolor_opa(pl.pv.small_btn_icon, LV_OPA_COVER, 0);
    lv_obj_set_size(pl.pv.small_btn_icon, 60, 50);
    lv_obj_center(pl.pv.small_btn_icon);

    pl.pv.big_btn = lv_btn_create(pl.pv.bottom_bar);
    lv_obj_set_size(pl.pv.big_btn, 60, 50);
    lv_obj_set_style_bg_opa(pl.pv.big_btn, LV_OPA_TRANSP, 0);        // 设置背景透明
    lv_obj_set_style_border_opa(pl.pv.big_btn, LV_OPA_TRANSP, 0);    // 设置边缘透明
    lv_obj_set_style_bg_color(pl.pv.big_btn, TM_ACCENT, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(pl.pv.big_btn, LV_OPA_30, LV_STATE_PRESSED);  // 50%透明度
    lv_obj_set_flex_grow(pl.pv.big_btn, 1);
    lv_obj_add_event_cb(pl.pv.big_btn, big_btn_event_handler, LV_EVENT_CLICKED, NULL);
    // lv_obj_add_flag(pl.pv.big_btn, LV_OBJ_FLAG_CHECKABLE);
    /* 照片查看器 - 放大按钮图标 */
    pl.pv.big_btn_icon = lv_img_create(pl.pv.big_btn);
    lv_img_set_src(pl.pv.big_btn_icon, &Photo_viewer_small);
    lv_obj_set_style_img_recolor(pl.pv.big_btn_icon, TM_ICON_PRIMARY, 0);
    lv_obj_set_style_img_recolor_opa(pl.pv.big_btn_icon, LV_OPA_COVER, 0);
    lv_obj_set_size(pl.pv.big_btn_icon, 60, 50);
    lv_obj_center(pl.pv.big_btn_icon);

    pl.pv.rota_btn = lv_btn_create(pl.pv.bottom_bar);
    lv_obj_set_size(pl.pv.rota_btn, 70, 50);
    lv_obj_set_style_bg_opa(pl.pv.rota_btn, LV_OPA_TRANSP, 0);        // 设置背景透明
    lv_obj_set_style_border_opa(pl.pv.rota_btn, LV_OPA_TRANSP, 0);    // 设置边缘透明
    lv_obj_set_style_bg_color(pl.pv.rota_btn, TM_ACCENT, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(pl.pv.rota_btn, LV_OPA_30, LV_STATE_PRESSED);  // 50%透明度
    lv_obj_set_flex_grow(pl.pv.rota_btn, 1);
    lv_obj_add_event_cb(pl.pv.rota_btn, rota_btn_event_handler, LV_EVENT_CLICKED, NULL);
    // lv_obj_add_flag(pl.pv.rota_btn, LV_OBJ_FLAG_CHECKABLE);
    /* 照片查看器 - 旋转按钮图标 */
    pl.pv.rota_btn_icon = lv_img_create(pl.pv.rota_btn);
    lv_img_set_src(pl.pv.rota_btn_icon, &Photo_viewer_rota);
    lv_obj_set_style_img_recolor(pl.pv.rota_btn_icon, TM_ICON_PRIMARY, 0);
    lv_obj_set_style_img_recolor_opa(pl.pv.rota_btn_icon, LV_OPA_COVER, 0);
    lv_obj_set_size(pl.pv.rota_btn_icon, 60, 50);
    lv_obj_center(pl.pv.rota_btn_icon);

    // 创建重置按钮
    pl.pv.reset_btn = lv_btn_create(pl.pv.bottom_bar);
    lv_obj_set_size(pl.pv.reset_btn, 70, 50);
    lv_obj_set_style_bg_opa(pl.pv.reset_btn, LV_OPA_TRANSP, 0);        // 设置背景透明
    lv_obj_set_style_border_opa(pl.pv.reset_btn, LV_OPA_TRANSP, 0);    // 设置边缘透明
    lv_obj_set_style_bg_color(pl.pv.reset_btn, TM_ACCENT, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(pl.pv.reset_btn, LV_OPA_30, LV_STATE_PRESSED);  // 50%透明度
    lv_obj_set_flex_grow(pl.pv.reset_btn, 1);
    lv_obj_add_event_cb(pl.pv.reset_btn, reset_btn_event_handler, LV_EVENT_CLICKED, NULL);
    /* 照片查看器 - 重置按钮图标 */
    pl.pv.reset_btn_icon = lv_img_create(pl.pv.reset_btn);
    lv_img_set_src(pl.pv.reset_btn_icon, &Photo_viewer_reset);
    lv_obj_set_style_img_recolor(pl.pv.reset_btn_icon, TM_ICON_PRIMARY, 0);
    lv_obj_set_style_img_recolor_opa(pl.pv.reset_btn_icon, LV_OPA_COVER, 0);
    lv_obj_set_size(pl.pv.reset_btn_icon, 70, 50);
    lv_obj_center(pl.pv.reset_btn_icon);

    // 创建缩放标签
    pl.pv.zoom_label = lv_label_create(pl.pv.bottom_bar);
    lv_obj_set_size(pl.pv.zoom_label, 80, 50);
    lv_obj_set_style_text_font(pl.pv.zoom_label, &v853Font_Medialibrary_30, 0);
    lv_label_set_text(pl.pv.zoom_label, "zoom: 100%");
    lv_obj_set_style_text_color(pl.pv.zoom_label, TM_TEXT_SECONDARY, 0);
    lv_obj_set_style_pad_top(pl.pv.zoom_label, 10, 0);
    lv_obj_set_flex_grow(pl.pv.zoom_label, 1);

}

static int canvas_move(lv_obj_t *canvas, lv_coord_t x, lv_coord_t y) {
    if (!canvas) {
        return -1;
    }
    
    /* 直接设置Canvas的位置 */
    lv_obj_set_pos(canvas, x, y);
    
    return 0;
}

static int canvas_display_image(lv_obj_t *canvas, const char *img_path) {
    if (!canvas || !img_path) {
        printf("canvas_display_image: invalid parameters\n");
        return -1;
    }

    /* 清理上一张图片的缓存：必须先 invalidate（遍历时需要旧指针有效），再 free */
    if (pl.old_img_path) {
        printf("[PhotoLibrary] Releasing %s\n", pl.old_img_path);
        lv_img_cache_invalidate_src(pl.old_img_path);
        free((void *)pl.old_img_path);
        pl.old_img_path = NULL;
    }

    /*
     * 关键：LVGL 图片缓存会保留传给 lv_canvas_draw_img / lv_img_decoder 的 src 指针，
     * 不做字符串复制。因此必须用堆上长生命周期的字符串，不能用栈上局部变量。
     * 先 strdup 到 pl.old_img_path，后续所有 LVGL 接口均使用该指针。
     */
    pl.old_img_path = strdup(img_path);
    if (!pl.old_img_path) {
        printf("canvas_display_image: strdup failed\n");
        return -1;
    }

    /* 获取图片信息 */
    lv_img_header_t header;
    lv_res_t res = lv_img_decoder_get_info(pl.old_img_path, &header);
    if (res != LV_RES_OK) {
        printf("canvas_display_image: failed to get image info\n");
        free((void *)pl.old_img_path);
        pl.old_img_path = NULL;
        return -1;
    }

    lv_img_cf_t canvas_cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
    uint32_t buf_size = header.w * header.h * 4;

    void *orig_buf = lv_mem_alloc(buf_size);
    if (!orig_buf) {
        printf("canvas_display_image: failed to allocate buffer (%u bytes)\n", buf_size);
        free((void *)pl.old_img_path);
        pl.old_img_path = NULL;
        return -1;
    }

    /* 先设置新缓冲区，再释放旧缓冲区，避免 LVGL 访问已释放内存 */
    void *old_buf = pl.pv.current_canvas_buf;
    lv_canvas_set_buffer(canvas, orig_buf, header.w, header.h, canvas_cf);
    lv_obj_set_size(canvas, header.w, header.h);
    if (old_buf) {
        lv_mem_free(old_buf);
    }
    pl.pv.current_canvas_buf = orig_buf;
    pl.pv.current_buf_size = buf_size;

    lv_canvas_fill_bg(canvas, lv_color_make(0xff, 0, 0), LV_OPA_COVER);

    lv_draw_img_dsc_t draw_dsc;
    lv_draw_img_dsc_init(&draw_dsc);
    draw_dsc.opa = LV_OPA_COVER;
    lv_canvas_draw_img(canvas, 0, 0, pl.old_img_path, &draw_dsc);

    lv_img_dsc_t *canvas_img = lv_canvas_get_img(canvas);
    if (!canvas_img || !canvas_img->data) {
        printf("canvas_display_image: failed to get canvas image data\n");
        return -1;
    }

    printf("Canvas image: w=%d, h=%d, cf=%d(%d)\n", 
                canvas_img->header.w, canvas_img->header.h, 
                canvas_img->header.cf, canvas_img->header.cf);

    const uint32_t target_w = 960;
    const uint32_t target_h = 540;
    uint32_t target_buf_size = target_w * target_h * 4;
    void *target_buf = lv_mem_alloc(target_buf_size);
    if (!target_buf) {
        printf("canvas_display_image: failed to allocate target buffer (%u bytes)\n", target_buf_size);
        return -1;
    }

    photo_view_bilinear_scale_argb8888((const unsigned char *)canvas_img->data,
                                      (unsigned int)canvas_img->header.w, (unsigned int)canvas_img->header.h,
                                      (unsigned char *)target_buf, target_w, target_h);

    /* 先设置新缓冲区，再释放原始缓冲区 */
    lv_canvas_set_buffer(canvas, target_buf, target_w, target_h, LV_IMG_CF_TRUE_COLOR_ALPHA);
    lv_obj_set_size(canvas, target_w, target_h);
    lv_mem_free(orig_buf);
    pl.pv.current_canvas_buf = target_buf;
    pl.pv.current_buf_size = target_buf_size;

    printf("canvas_display_image: success - scaled from %dx%d to %dx%d\n", 
                header.w, header.h, target_w, target_h);

    return 0;
}

static int canvas_zoom(lv_obj_t *canvas, float scale) {
    if (!canvas || scale <= 0) {
        return -1;
    }

    lv_img_dsc_t *canvas_dsc = lv_canvas_get_img(canvas);
    if (!canvas_dsc || !canvas_dsc->data) {
        return -1;
    }

    uint32_t src_w = canvas_dsc->header.w;
    uint32_t src_h = canvas_dsc->header.h;
    uint32_t dst_w = (uint32_t)(src_w * scale);
    uint32_t dst_h = (uint32_t)(src_h * scale);

    if (dst_w == 0 || dst_h == 0) {
        return -1;
    }

    uint32_t dst_buf_size = dst_w * dst_h * 4;
    uint8_t *dst_buf = (uint8_t *)lv_mem_alloc(dst_buf_size);
    if (!dst_buf) {
        return -1;
    }

    photo_view_bilinear_scale_argb8888((const unsigned char *)canvas_dsc->data, src_w, src_h,
                                       dst_buf, dst_w, dst_h);

    void *old_buf = pl.pv.current_canvas_buf;
    lv_canvas_set_buffer(canvas, dst_buf, dst_w, dst_h, LV_IMG_CF_TRUE_COLOR_ALPHA);
    lv_obj_set_size(canvas, dst_w, dst_h);
    if (old_buf) {
        lv_mem_free(old_buf);
    }
    pl.pv.current_canvas_buf = dst_buf;
    pl.pv.current_buf_size = dst_buf_size;

    return 0;
}

static bool rotate_canvas_90_clockwise_argb8888(lv_obj_t *canvas) {
    if (canvas == NULL) return false;
    lv_img_dsc_t *current_img = lv_canvas_get_img(canvas);
    if (current_img == NULL || current_img->data == NULL) return false;
    if (current_img->header.cf != LV_IMG_CF_TRUE_COLOR_ALPHA) return false;
    unsigned int src_w = (unsigned int)current_img->header.w;
    unsigned int src_h = (unsigned int)current_img->header.h;
    uint32_t buf_size = src_h * src_w * 4u;
    unsigned char *rotated_data = (unsigned char *)lv_mem_alloc(buf_size);
    if (!rotated_data) return false;
    photo_view_rotate_90_cw_argb8888((const unsigned char *)current_img->data, src_w, src_h, rotated_data);
    void *old_buf = pl.pv.current_canvas_buf;
    lv_canvas_set_buffer(canvas, rotated_data, (lv_coord_t)src_h, (lv_coord_t)src_w, LV_IMG_CF_TRUE_COLOR_ALPHA);
    lv_obj_set_size(canvas, (lv_coord_t)src_h, (lv_coord_t)src_w);
    if (old_buf) lv_mem_free(old_buf);
    pl.pv.current_canvas_buf = rotated_data;
    pl.pv.current_buf_size = buf_size;
    return true;
}

static int is_jpg_file(const char *filename) {
    if (!filename) return 0;

    size_t len = strlen(filename);
    if (len < 4) return 0;

    const char *ext = filename + len - 4;
    if (strcasecmp(ext, ".jpg") == 0) return 1;

    // 也支持.jpeg扩展名
    if (len >= 5) {
        ext = filename + len - 5;
        if (strcasecmp(ext, ".jpeg") == 0) return 1;
    }

    return 0;
}

static int compare_strings(const void *a, const void *b) {
    const char *str_a = *(const char**)a;
    const char *str_b = *(const char**)b;
    return strcmp(str_b, str_a);
}

static void free_photo_paths_tab(int tab_index) {
    if (tab_index < 0 || tab_index >= PHOTO_TAB_COUNT) return;
    photo_tab_data_t *t = &pl.pm.tab_data[tab_index];
    if (t->photo_paths) {
        for (int i = 0; i < t->photo_count; i++) {
            if (t->photo_paths[i]) free(t->photo_paths[i]);
        }
        free(t->photo_paths);
        t->photo_paths = NULL;
    }
    t->photo_count = 0;
    t->photo_capacity = 0;
}

static void add_photo_path_to_tab(int tab_index, const char *full_path) {
    if (tab_index < 0 || tab_index >= PHOTO_TAB_COUNT) return;
    photo_tab_data_t *t = &pl.pm.tab_data[tab_index];
    if (t->photo_count >= t->photo_capacity) {
        int new_cap = t->photo_capacity == 0 ? 50 : t->photo_capacity * 2;
        char **new_paths = realloc(t->photo_paths, new_cap * sizeof(char*));
        if (!new_paths) return;
        t->photo_paths = new_paths;
        t->photo_capacity = new_cap;
    }
    t->photo_paths[t->photo_count] = strdup(full_path);
    if (t->photo_paths[t->photo_count]) t->photo_count++;
}

static void scan_photo_directory_recursive(int tab_index, const char *dir_path) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    char full_path[512];

    dir = opendir(dir_path);
    if (!dir) {
        printf("Cannot open directory: %s\n", dir_path);
        return;
    }
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        if (stat(full_path, &file_stat) != 0) continue;
        if (S_ISDIR(file_stat.st_mode)) {
            scan_photo_directory_recursive(tab_index, full_path);
        } else if (S_ISREG(file_stat.st_mode) && is_jpg_file(entry->d_name)) {
            add_photo_path_to_tab(tab_index, full_path);
        }
    }
    closedir(dir);
}

static void scan_photo_files(int tab_index) {
    if (tab_index < 0 || tab_index >= PHOTO_TAB_COUNT) return;
    free_photo_paths_tab(tab_index);
    const char *dir = (tab_index == PHOTO_TAB_FRONT) ? PHOTO_DIR_FRONT : PHOTO_DIR_REAR;
    printf("Scanning photo directory: %s (tab %d)\n", dir, tab_index);
    scan_photo_directory_recursive(tab_index, dir);
    photo_tab_data_t *t = &pl.pm.tab_data[tab_index];
    if (t->photo_count > 1) {
        qsort(t->photo_paths, t->photo_count, sizeof(char*), compare_strings);
    }
    printf("Found %d photos in tab %d\n", t->photo_count, tab_index);
}

static void create_photo_tab_page(lv_obj_t *parent, int tab_index, const char *tab_name) {
    lv_obj_t *tab_page = lv_tabview_add_tab(parent, tab_name);
    lv_obj_set_style_pad_all(tab_page, 0, 0);
    lv_obj_clear_flag(tab_page, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title_bar = lv_obj_create(tab_page);
    lv_obj_remove_style_all(title_bar);
    lv_obj_set_size(title_bar, 1340, 60);
    lv_obj_set_style_bg_color(title_bar, TM_BG_PRIMARY, 0);
    lv_obj_set_style_bg_opa(title_bar, LV_OPA_100, 0);
    lv_obj_set_style_border_width(title_bar, 0, 0);
    lv_obj_set_style_radius(title_bar, 0, 0);
    lv_obj_set_style_pad_all(title_bar, 0, 0);
    lv_obj_set_flex_flow(title_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(title_bar, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(title_bar, LV_OBJ_FLAG_SCROLLABLE);

    pl.pm.tab_data[tab_index].title_label = lv_label_create(title_bar);
    lv_obj_set_height(pl.pm.tab_data[tab_index].title_label, 60);
    char title_text[64];
    snprintf(title_text, sizeof(title_text), " %s %d %s",
             get_string_for_language(languageSetting, "Found"),
             pl.pm.tab_data[tab_index].photo_count,
             get_string_for_language(languageSetting, "photos"));
    lv_obj_set_style_text_font(pl.pm.tab_data[tab_index].title_label, &v853Font_Medialibrary_30, 0);
    lv_obj_set_style_text_color(pl.pm.tab_data[tab_index].title_label, TM_TEXT_SECONDARY, 0);
    lv_label_set_text(pl.pm.tab_data[tab_index].title_label, title_text);
    lv_obj_set_style_pad_all(pl.pm.tab_data[tab_index].title_label, 15, 0);

    lv_obj_t *spacer = lv_obj_create(title_bar);
    lv_obj_remove_style_all(spacer);
    lv_obj_set_style_bg_opa(spacer, LV_OPA_0, 0);
    lv_obj_set_flex_grow(spacer, 1);

    lv_obj_t *refresh_btn = lv_btn_create(title_bar);
    lv_obj_set_style_pad_all(refresh_btn, 0, 0);
    lv_obj_set_size(refresh_btn, 100, 60);
    lv_obj_set_style_pad_right(refresh_btn, 40, 0);
    lv_obj_set_style_bg_opa(refresh_btn, LV_OPA_TRANSP, 0);
    lv_obj_add_event_cb(refresh_btn, refresh_btn_event_handler, LV_EVENT_CLICKED, (void*)(intptr_t)tab_index);
    lv_obj_t *refresh_label = lv_label_create(refresh_btn);
    lv_obj_set_style_text_font(refresh_label, &v853Font_Medialibrary_30, 0);
    lv_obj_set_style_text_color(refresh_label, TM_ICON_PRIMARY, 0);
    lv_label_set_text(refresh_label, LV_SYMBOL_REFRESH);
    lv_obj_center(refresh_label);

    lv_obj_t *list_cont = lv_obj_create(tab_page);
    lv_obj_set_size(list_cont, LV_PCT(100), 720 - 60 - 80);
    lv_obj_set_style_pad_all(list_cont, 0, 0);
    lv_obj_set_style_border_width(list_cont, 0, 0);
    lv_obj_set_style_bg_opa(list_cont, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(list_cont, TM_BG_PRIMARY, 0);
    lv_obj_set_style_radius(list_cont, 0, 0);
    lv_obj_set_pos(list_cont, 0, 60);
    lv_obj_clear_flag(list_cont, LV_OBJ_FLAG_SCROLLABLE);

    pl.pm.tab_data[tab_index].list = lv_list_create(list_cont);
    lv_obj_set_size(pl.pm.tab_data[tab_index].list, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_text_color(pl.pm.tab_data[tab_index].list, TM_TEXT_PRIMARY, 0);
    lv_obj_set_style_bg_color(pl.pm.tab_data[tab_index].list, TM_BG_PRIMARY, 0);
    lv_obj_set_style_pad_bottom(pl.pm.tab_data[tab_index].list, 20, 0);
    lv_obj_set_style_pad_left(pl.pm.tab_data[tab_index].list, 15, 0);
    lv_obj_set_style_pad_right(pl.pm.tab_data[tab_index].list, 15, 0);
    lv_obj_set_style_radius(pl.pm.tab_data[tab_index].list, 0, 0);
}

static void populate_photo_list(int tab_index) {
    if (tab_index < 0 || tab_index >= PHOTO_TAB_COUNT) return;
    photo_tab_data_t *t = &pl.pm.tab_data[tab_index];
    if (!t->list) return;

    lv_obj_clean(t->list);

    if (t->title_label) {
        char buf[64];
        snprintf(buf, sizeof(buf), " %s %d %s",
                 get_string_for_language(languageSetting, "Found"),
                 t->photo_count,
                 get_string_for_language(languageSetting, "photos"));
        lv_label_set_text(t->title_label, buf);
    }

    const char *dir = (tab_index == PHOTO_TAB_FRONT) ? PHOTO_DIR_FRONT : PHOTO_DIR_REAR;
    size_t dir_len = strlen(dir);

    if (t->photo_count == 0) {
#if 0
        lv_obj_clear_flag(t->list, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_t *empty_label1 = lv_label_create(t->list);
        lv_obj_set_style_text_font(empty_label1, &v853Font_Medialibrary_30, 0);
        lv_obj_set_style_text_color(empty_label1, TM_TEXT_PRIMARY, 0);
        lv_label_set_text(empty_label1, get_string_for_language(languageSetting, "No_photo_found"));
        lv_obj_center(empty_label1);
        char buff[256];
        snprintf(buff, sizeof(buff), "%s %s\n", get_string_for_language(languageSetting, "check_contents"), dir);
        lv_obj_t *empty_label2 = lv_label_create(t->list);
        lv_obj_set_style_text_font(empty_label2, &v853Font_Medialibrary_30, 0);
        lv_label_set_text(empty_label2, buff);
        lv_obj_set_style_text_color(empty_label2, TM_TEXT_SECONDARY, 0);
        lv_obj_align(empty_label2, LV_ALIGN_BOTTOM_MID, 0, -30);
        lv_obj_t *empty_label3 = lv_label_create(t->list);
        lv_obj_set_style_text_font(empty_label3, &v853Font_Medialibrary_30, 0);
        lv_label_set_text(empty_label3, get_string_for_language(languageSetting, "Supported_formats"));
        lv_obj_set_style_text_color(empty_label3, TM_TEXT_DISABLED, 0);
        lv_obj_align(empty_label3, LV_ALIGN_BOTTOM_MID, 0, 0);
#endif
        return;
    }

    lv_obj_add_flag(t->list, LV_OBJ_FLAG_SCROLLABLE);
    for (int i = 0; i < t->photo_count; i++) {
        const char *full_path = t->photo_paths[i];
        const char *relative_path = full_path + dir_len;
        if (*relative_path == '/') relative_path++;

        lv_obj_t *list_btn = lv_btn_create(t->list);
        lv_obj_remove_style_all(list_btn);
        lv_obj_set_width(list_btn, lv_pct(100));
        lv_obj_set_height(list_btn, 80);
        lv_obj_add_flag(list_btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
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

        lv_obj_t *img_icon = lv_obj_create(list_btn);
        lv_obj_set_size(img_icon, 80, 80);
        lv_obj_align(img_icon, LV_ALIGN_LEFT_MID, 0, 0);
        lv_obj_set_style_pad_all(img_icon, 0, 0);
        lv_obj_set_style_bg_opa(img_icon, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(img_icon, 0, 0);
        lv_obj_t *icon = lv_img_create(img_icon);
        lv_obj_set_style_text_font(icon, &v853Font_Medialibrary_30, 0);
        lv_obj_set_style_text_color(icon, TM_ICON_PRIMARY, 0);
        lv_img_set_src(icon, LV_SYMBOL_IMAGE);
        lv_obj_center(icon);

        lv_obj_t *label = lv_label_create(list_btn);
        lv_obj_set_style_text_font(label, &v853Font_Medialibrary_30, 0);
        lv_obj_set_style_text_color(label, TM_TEXT_PRIMARY, 0);
        lv_label_set_text_fmt(label, "%s", relative_path);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 80, 0);

        lv_obj_set_user_data(list_btn, (void*)(intptr_t)i);
        lv_obj_add_event_cb(list_btn, list_btn_event_handler, LV_EVENT_CLICKED, NULL);

        lv_obj_t *del_btn = lv_btn_create(list_btn);
        lv_obj_set_size(del_btn, 50, 50);
        lv_obj_set_style_bg_opa(del_btn, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(del_btn, 0, 0);
        lv_obj_align(del_btn, LV_ALIGN_RIGHT_MID, -30, 0);
        lv_obj_t *del_icon = lv_img_create(del_btn);
        lv_obj_set_style_img_recolor(del_icon, TM_ICON_SECONDARY, 0);
        lv_obj_set_style_img_recolor_opa(del_icon, LV_OPA_COVER, 0);
        lv_obj_set_style_img_recolor(del_icon, TM_ICON_DISABLED, LV_STATE_PRESSED);
        lv_obj_set_style_img_recolor_opa(del_icon, LV_OPA_COVER, LV_STATE_PRESSED);
        lv_img_set_src(del_icon, &delete);
        lv_obj_center(del_icon);
        lv_obj_add_event_cb(del_btn, del_btn_event_handler, LV_EVENT_CLICKED, (void*)(intptr_t)i);
    }
}

static void init_photo_tab(int tab_index) {
    scan_photo_files(tab_index);
    populate_photo_list(tab_index);
}

/** 从列表项或删除按钮等子控件向上遍历，找到属于哪个 tab 的 list，返回 tab 索引 */
static int get_photo_tab_from_child(lv_obj_t *obj) {
    for (lv_obj_t *p = obj; p != NULL; p = lv_obj_get_parent(p)) {
        for (int i = 0; i < PHOTO_TAB_COUNT; i++) {
            if (pl.pm.tab_data[i].list == p) return i;
        }
    }
    return -1;
}

static void close_format_dialog() {
    if (pl.pm.mask_layer_del_dialog) {
        lv_obj_del(pl.pm.mask_layer_del_dialog);
        pl.pm.mask_layer_del_dialog = NULL;
    }
}

static void ask_dialog_yes_event_cb(lv_event_t *e) {
    intptr_t packed = (intptr_t)lv_event_get_user_data(e);
    int tab_index = (int)((packed >> 16) & 0xFFFF);
    int index = (int)(packed & 0xFFFF);

    if (tab_index < 0 || tab_index >= PHOTO_TAB_COUNT) return;
    photo_tab_data_t *t = &pl.pm.tab_data[tab_index];
    if (index < 0 || index >= t->photo_count) return;

    const char *full_path = t->photo_paths[index];
    if (full_path) {
        if (remove(full_path) == 0) {
            printf("[PhotoLibrary] Deleted file: %s\n", full_path);
        } else {
            printf("[PhotoLibrary] Failed to delete file: %s\n", full_path);
            return;
        }
    }

    scan_photo_files(tab_index);
    populate_photo_list(tab_index);
    close_format_dialog();
}

static void ask_dialog_no_event_cb(lv_event_t *e) {
    (void)e; // 未使用参数
    close_format_dialog();
}

static void create_ask_dialog(lv_obj_t *parent, int tab_index, int index) {
    intptr_t packed = ((intptr_t)tab_index << 16) | (intptr_t)(index & 0xFFFF);
    /* 照片库 - 删除确认对话框背景 */
    lv_obj_t *dialog_bg = lv_obj_create(parent);
    lv_obj_set_size(dialog_bg, LV_PCT(40), LV_PCT(40));
    lv_obj_set_pos(dialog_bg, 382, 216);
    lv_obj_set_style_bg_color(dialog_bg, TM_BG_SECONDARY, 0);             /* 对话框背景 */
    lv_obj_set_style_radius(dialog_bg, 10, 0);
    lv_obj_set_style_pad_all(dialog_bg, 20, 0);
    lv_obj_set_style_border_width(dialog_bg, 2, 0);
    lv_obj_set_style_border_color(dialog_bg, TM_BORDER_SECONDARY, 0);     /* 对话框边框 */
    lv_obj_clear_flag(dialog_bg, LV_OBJ_FLAG_SCROLLABLE);

    /* 照片库 - 删除确认消息 */
    lv_obj_t *msg_label = lv_label_create(dialog_bg);
    lv_label_set_text(msg_label, get_string_for_language(languageSetting, "Delete_photo_confirm"));
    lv_obj_set_style_text_font(msg_label, &v853Font_Medialibrary_30, 0);
    lv_obj_set_width(msg_label, LV_PCT(100));
    lv_obj_set_style_text_align(msg_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(msg_label, TM_TEXT_PRIMARY, 0);           /* 消息文字颜色 */
    lv_obj_align(msg_label, LV_ALIGN_TOP_MID, 0, LV_PCT(20));

    /* 照片库 - 确认按钮 */
    lv_obj_t *yes_btn = lv_btn_create(dialog_bg);
    lv_obj_set_size(yes_btn, 140, 80);
    lv_obj_align(yes_btn, LV_ALIGN_BOTTOM_LEFT, 20, -20);
    lv_obj_set_style_bg_color(yes_btn, TM_SUCCESS, 0);                    /* 确认按钮背景(绿色) */
    lv_obj_set_style_radius(yes_btn, 8, 0);
    lv_obj_add_event_cb(yes_btn, ask_dialog_yes_event_cb, LV_EVENT_CLICKED, (void*)packed);
    lv_obj_t *yes_label = lv_label_create(yes_btn);
    lv_label_set_text(yes_label, get_string_for_language(languageSetting, "Yes"));
    lv_obj_set_style_text_font(yes_label, &v853Font_Medialibrary_30, 0);
    lv_obj_set_style_text_color(yes_label, lv_color_hex(0xFFFFFF), 0);    /* 按钮文字始终白色 */
    lv_obj_center(yes_label);
    
    /* 照片库 - 取消按钮 */
    lv_obj_t *no_btn = lv_btn_create(dialog_bg);
    lv_obj_set_size(no_btn, 140, 80);
    lv_obj_align(no_btn, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    lv_obj_set_style_bg_color(no_btn, TM_ERROR, 0);                       /* 取消按钮背景(红色) */
    lv_obj_set_style_radius(no_btn, 8, 0);
    lv_obj_add_event_cb(no_btn, ask_dialog_no_event_cb, LV_EVENT_CLICKED, NULL);    
    lv_obj_t *no_label = lv_label_create(no_btn);
    lv_label_set_text(no_label, get_string_for_language(languageSetting, "No"));
    lv_obj_set_style_text_font(no_label, &v853Font_Medialibrary_30, 0);
    lv_obj_set_style_text_color(no_label, lv_color_hex(0xFFFFFF), 0);     /* 按钮文字始终白色 */
    lv_obj_center(no_label);
}

static void show_format_sd_dialog(lv_obj_t *parent, int tab_index, int index) {
    close_format_dialog();

    pl.pm.mask_layer_del_dialog = lv_obj_create(parent);
    lv_obj_set_size(pl.pm.mask_layer_del_dialog, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(pl.pm.mask_layer_del_dialog, TM_BG_PRIMARY, 0);
    lv_obj_set_style_bg_opa(pl.pm.mask_layer_del_dialog, LV_OPA_50, 0);
    lv_obj_set_style_radius(pl.pm.mask_layer_del_dialog, 0, 0);
    lv_obj_set_style_border_width(pl.pm.mask_layer_del_dialog, 0, 0);
    lv_obj_clear_flag(pl.pm.mask_layer_del_dialog, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_move_foreground(pl.pm.mask_layer_del_dialog);

    create_ask_dialog(pl.pm.mask_layer_del_dialog, tab_index, index);
}

static void load_and_display_image(const char *path) {
    if (!path || path[0] == '\0' || !pl.pv.img) return;

    char full_path[512];
    memset(full_path, 0, sizeof(full_path));
    snprintf(full_path, sizeof(full_path), "S:%s", path);
    printf("Loading image: %s\n", full_path);

    /* 安全计算相对路径：path 必须以 photo_dir 为前缀，否则避免越界 */
    const char *relative_path = path;
    size_t dir_len = strlen(pl.pm.photo_dir);
    if (dir_len > 0 && strncmp(path, pl.pm.photo_dir, dir_len) == 0) {
        relative_path = path + dir_len;
        if (*relative_path == '/') relative_path++;
    }
    if (*relative_path == '\0') relative_path = path; /* 回退为完整 path 的尾部或 path */

    if (canvas_display_image(pl.pv.img, full_path) != 0) return;

    pl.pv.has_photo_cache = 1;

    if (pl.pv.file_label) {
        lv_label_set_text_fmt(pl.pv.file_label, "%s", relative_path);
    }
}

static void update_zoom_label(void) {
    if (pl.pv.zoom_label) {
        int percent = 100;
        switch (pl.pv.current_zoom_value) {
        case PERCENT50:
            percent = 50;
            break;
        case PERCENT75:
            percent = 75;
            break;
        case PERCENT100:
            percent = 100;
            break;
        case PERCENT150:
            percent = 150;
            break;
        case PERCENT200:
            percent = 200;
            break;
        default:
            break;
        }
        lv_label_set_text_fmt(pl.pv.zoom_label, "zoom: %d%%", percent);
    }
}

static void show_photo_viewer(int index) {
    /* 始终从当前 tab 同步路径指针，避免 media_poll 触发 apply_scan_result 后 pl.pm.photo_paths 悬空 */
    pl.pm.photo_paths = pl.pm.tab_data[pl.pm.current_photo_tab].photo_paths;
    pl.pm.photo_count = pl.pm.tab_data[pl.pm.current_photo_tab].photo_count;

    if (index < 0 || index >= pl.pm.photo_count) return;
    if (!pl.pv.img) return;

    const char *path = pl.pm.photo_paths[index];
    if (!path || path[0] == '\0') return;

    pl.pm.current_index = index;
    strncpy(pl.pm.photo_dir, pl.pm.current_photo_tab == PHOTO_TAB_FRONT ? PHOTO_DIR_FRONT : PHOTO_DIR_REAR,
            sizeof(pl.pm.photo_dir) - 1);
    pl.pm.photo_dir[sizeof(pl.pm.photo_dir) - 1] = '\0';

    if (pl.pv.cont) {
        lv_obj_clear_flag(pl.pv.cont, LV_OBJ_FLAG_HIDDEN);
    }

    pl.pv.offset_x = IMGORIGINX;
    pl.pv.offset_y = IMGORIGINY;
    load_and_display_image(path);
    // 新图片移动到初始位置
    canvas_move(pl.pv.img, IMGORIGINX, IMGORIGINY);
    // 更新底部栏的标签
    pl.pv.current_zoom_value = PERCENT100;  // 重置为100%    
    update_zoom_label();
}

static void list_btn_event_handler(lv_event_t *e) {
    lv_obj_t *btn = lv_event_get_target(e);
    int index = (int)(intptr_t)lv_obj_get_user_data(btn);
    int tab = get_photo_tab_from_child(btn);
    if (tab < 0 || tab >= PHOTO_TAB_COUNT) return;
    pl.pm.current_photo_tab = tab;
    pl.pm.photo_paths = pl.pm.tab_data[tab].photo_paths;
    pl.pm.photo_count = pl.pm.tab_data[tab].photo_count;
    show_photo_viewer(index);
}

static void del_btn_event_handler(lv_event_t *e) {
    lv_event_stop_bubbling(e);
    lv_obj_t *del_btn = lv_event_get_target(e);
    int index = (int)(intptr_t)lv_event_get_user_data(e);
    int tab = get_photo_tab_from_child(del_btn);
    if (tab < 0 || tab >= PHOTO_TAB_COUNT) return;
    pl.pm.current_photo_tab = tab;
    pl.pm.photo_paths = pl.pm.tab_data[tab].photo_paths;
    pl.pm.photo_count = pl.pm.tab_data[tab].photo_count;
    if (index < 0 || index >= pl.pm.photo_count) return;
    show_format_sd_dialog(lv_scr_act(), tab, index);
}

static void back_btn_event_handler(lv_event_t *e) {
    LV_UNUSED(e);

    /* 释放 canvas 缓冲区以回收内存 */
    if (pl.pv.img && pl.pv.current_canvas_buf) {
        static uint8_t dummy_buf[4] = {0};
        lv_canvas_set_buffer(pl.pv.img, dummy_buf, 1, 1, LV_IMG_CF_TRUE_COLOR_ALPHA);
        lv_mem_free(pl.pv.current_canvas_buf);
        pl.pv.current_canvas_buf = NULL;
        pl.pv.current_buf_size = 0;
    }
    if (pl.old_img_path) {
        lv_img_cache_invalidate_src(pl.old_img_path);
        free((void *)pl.old_img_path);
        pl.old_img_path = NULL;
    }
    pl.pv.has_photo_cache = 0;

    if (pl.pv.cont) {
        lv_obj_add_flag(pl.pv.cont, LV_OBJ_FLAG_HIDDEN);
    }
}

static void tabview_event_handler(lv_event_t *e) {
    if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;
    lv_obj_t *tabview = lv_event_get_target(e);
    uint16_t idx = lv_tabview_get_tab_act(tabview);
    if (idx < (uint16_t)PHOTO_TAB_COUNT) {
        pl.pm.current_photo_tab = (int)idx;
    }
}

static void refresh_btn_event_handler(lv_event_t *e) {
    int tab_index = (int)(intptr_t)lv_event_get_user_data(e);
    if (tab_index < 0 || tab_index >= PHOTO_TAB_COUNT) return;
    scan_photo_files(tab_index);
    populate_photo_list(tab_index);
}

static void prev_btn_event_handler(lv_event_t *e) {
    LV_UNUSED(e);
    pl.pm.photo_paths = pl.pm.tab_data[pl.pm.current_photo_tab].photo_paths;
    pl.pm.photo_count = pl.pm.tab_data[pl.pm.current_photo_tab].photo_count;
    if (pl.pm.current_index > 0) {
        show_photo_viewer(pl.pm.current_index - 1);
    }
}

static void next_btn_event_handler(lv_event_t *e) {
    LV_UNUSED(e);
    pl.pm.photo_paths = pl.pm.tab_data[pl.pm.current_photo_tab].photo_paths;
    pl.pm.photo_count = pl.pm.tab_data[pl.pm.current_photo_tab].photo_count;
    if (pl.pm.current_index >= 0 && pl.pm.current_index < pl.pm.photo_count - 1) {
        show_photo_viewer(pl.pm.current_index + 1);
    }
}

static void on_image_touch(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_PRESSED:                              // 记录按下的位置和按下的时间辍
        {
#if GESTURECHANGEENBALE
            lv_indev_t *indev = lv_indev_get_act();
            lv_indev_data_t data;
            _lv_indev_read(indev, &data);

            pl.pv.gesture_start_x = data.point.x;
            pl.pv.gesture_start_y = data.point.y;
            printf("gesture_start_x = %d, gesture_start_y = %d\n", pl.pv.gesture_start_x, pl.pv.gesture_start_y);
            pl.pv.gesture_start_time = lv_tick_get();
            pl.pv.is_gesture_active = 1;
#endif
        }
        break;
    case LV_EVENT_PRESSING:                             // 移动图片
        {
            lv_indev_t *indev = lv_indev_get_act();
            lv_indev_data_t data;
            _lv_indev_read(indev, &data);

            /* 拖动图片逻辑 */
            if (!pl.pv.is_dragging) {
                /* 开始拖动 */
                pl.pv.is_dragging = 1;
                pl.last_x = data.point.x;
                pl.last_y = data.point.y;
            } else {
                /* 计算相对移动 */
                int dx = data.point.x - pl.last_x;
                int dy = data.point.y - pl.last_y;

                int distance_sq = dx*dx + dy*dy;
                if (distance_sq > 49) {  // 7的平方 = 49
                    pl.pv.offset_x = pl.pv.offset_x - dy;
                    pl.pv.offset_y = pl.pv.offset_y + dx;

                    /* 应用变换 */
                    if (pl.pv.img) {
                        canvas_move(pl.pv.img, pl.pv.offset_x, pl.pv.offset_y);
                    }

                    /* 更新最后位置 */
                    pl.last_x = data.point.x;
                    pl.last_y = data.point.y;
                }
            }
        }
        break;    
    case LV_EVENT_RELEASED:                             // 记录释放的位置和释放的时间辍
        {
            pl.pv.is_dragging = 0;
#if GESTURECHANGEENBALE
            // 检测快速滑动（手势切换图片）
            if (pl.pv.is_gesture_active) {
                lv_indev_t *indev = lv_indev_get_act();
                lv_indev_data_t data;
                _lv_indev_read(indev, &data);

                int dx = data.point.x - pl.pv.gesture_start_x;
                int dy = data.point.y - pl.pv.gesture_start_y;
                uint32_t time_diff = lv_tick_elaps(pl.pv.gesture_start_time);

                printf("elaps time = %d\n", time_diff);

                // 如果是水平快速滑动且时间很短，切换图片
                if (time_diff < 180 && abs(dx) > 150 && abs(dx) > abs(dy) * 2) {
                    if (dx > 0) {
                        // 右滑，上一张
                        prev_btn_event_handler(e);
                    } else {
                        // 左滑，下一张
                        next_btn_event_handler(e);
                    }
                }

                pl.pv.is_gesture_active = 0;
            }
#endif
        }
        break;
    }
}

static void small_btn_event_handler(lv_event_t *e) {
    LV_UNUSED(e);
    --pl.pv.current_zoom_value;
    if (pl.pv.current_zoom_value >= 5 || pl.pv.current_zoom_value < 1) {
        printf("Don't zoom this value\n");
        ++pl.pv.current_zoom_value;
        return;
    }

    // 设置缩放
    switch (pl.pv.current_zoom_value)
    {
    case 4:
        canvas_zoom(pl.pv.img, (float)(1.5 / 2.0));
        break;
    case 3:
        canvas_zoom(pl.pv.img, (float)(1.0 / 1.5));
        break;
    case 2:
        canvas_zoom(pl.pv.img, (float)(0.75 / 1.0));
        break;
    case 1:
        canvas_zoom(pl.pv.img, (float)(0.5 / 0.75));
        break;
    default:
        break;
    }
    update_zoom_label();
}

static void big_btn_event_handler(lv_event_t *e) {
    LV_UNUSED(e);
    // set_image_zoom_level(1);  // 100%
    ++pl.pv.current_zoom_value;
    if (pl.pv.current_zoom_value > 5 || pl.pv.current_zoom_value <= 1) {
        printf("Don't zoom this value\n");
        --pl.pv.current_zoom_value;
        return;
    }

    // 设置缩放
    switch (pl.pv.current_zoom_value)
    {
    case 5:
        canvas_zoom(pl.pv.img, (float)(2.0 / 1.5));
        break;
    case 4:
        canvas_zoom(pl.pv.img, (float)(1.5 / 1.0));
        break;
    case 3:
        canvas_zoom(pl.pv.img, (float)(1.0 / 0.75));
        break;
    case 2:
        canvas_zoom(pl.pv.img, (float)(0.75 / 0.5));
        break;
    default:
        break;
    }
    // apply_image_transform();
    // lv_obj_invalidate(pl.pv.img);
    // if (pl.pv.img->parent) {
    //     lv_obj_invalidate(pl.pv.img->parent);  // 也使父容器失效
    // }
    update_zoom_label();
}

static void rota_btn_event_handler(lv_event_t *e) {
    LV_UNUSED(e);
    rotate_canvas_90_clockwise_argb8888(pl.pv.img);
}

static void reset_btn_event_handler(lv_event_t *e) {
    LV_UNUSED(e);
    // 边界检查
    if (!pl.pm.photo_paths || pl.pm.current_index < 0 || pl.pm.current_index >= pl.pm.photo_count) {
        return;
    }
    if (!pl.pv.img) {
        return;
    }
    int index = pl.pm.current_index;
    // 先退出查看界面：与返回按钮相同的清理逻辑，释放 canvas 缓冲区和图片缓存并隐藏容器
    if (pl.pv.img && pl.pv.current_canvas_buf) {
        static uint8_t dummy_buf[4] = {0};
        lv_canvas_set_buffer(pl.pv.img, dummy_buf, 1, 1, LV_IMG_CF_TRUE_COLOR_ALPHA);
        lv_mem_free(pl.pv.current_canvas_buf);
        pl.pv.current_canvas_buf = NULL;
        pl.pv.current_buf_size = 0;
    }
    if (pl.old_img_path) {
        lv_img_cache_invalidate_src(pl.old_img_path);
        free((void *)pl.old_img_path);
        pl.old_img_path = NULL;
    }
    pl.pv.has_photo_cache = 0;
    if (pl.pv.cont) {
        lv_obj_add_flag(pl.pv.cont, LV_OBJ_FLAG_HIDDEN);
    }
    // 再重新进入当前图片的查看界面，实现重置（初始位置、100% 缩放、重新加载）
    show_photo_viewer(index);
}

/**
 * @brief 初始化照片库数据结构
 */
static void pl_init(void) {
    printf("[PhotoLibrary] Initializing data structure...\n");
    
    /* 重置主界面数据 */
    pl.pm.photo_paths = NULL;
    pl.pm.photo_count = 0;
    pl.pm.current_index = -1;
    pl.pm.current_photo_tab = 0;
    strncpy(pl.pm.photo_dir, PHOTO_DIR_FRONT, sizeof(pl.pm.photo_dir) - 1);
    pl.pm.photo_dir[sizeof(pl.pm.photo_dir) - 1] = '\0';
    pl.pm.cont = NULL;
    pl.pm.tabview = NULL;
    pl.pm.tab_btns = NULL;
    pl.pm.mask_layer_del_dialog = NULL;
    for (int i = 0; i < PHOTO_TAB_COUNT; i++) {
        pl.pm.tab_data[i].photo_paths = NULL;
        pl.pm.tab_data[i].photo_count = 0;
        pl.pm.tab_data[i].photo_capacity = 0;
        pl.pm.tab_data[i].list = NULL;
        pl.pm.tab_data[i].title_label = NULL;
    }

    /* 重置图片查看器数据 */
    pl.pv.has_photo_cache = 0;
    pl.pv.current_zoom_value = PERCENT100;
    pl.pv.offset_x = 0;
    pl.pv.offset_y = 0;
    pl.pv.drag_start_x = 0;
    pl.pv.drag_start_y = 0;
    pl.pv.img_start_x = 0;
    pl.pv.img_start_y = 0;
    pl.pv.is_dragging = 0;

    pl.pv.gesture_start_x = 0;
    pl.pv.gesture_start_y = 0;
    pl.pv.is_gesture_active = 0;
    pl.pv.gesture_start_time = 0;
    pl.pv.cont = NULL;
    pl.pv.top_bar = NULL;
    pl.pv.back_btn = NULL;
    pl.pv.back_icon = NULL;       
    pl.pv.file_label = NULL;
    pl.pv.nav_btn_cont = NULL;
    pl.pv.prev_btn = NULL;
    pl.pv.prev_icon = NULL;
    pl.pv.next_btn = NULL;
    pl.pv.next_icon = NULL;
    pl.pv.img_wrapper = NULL;

    pl.pv.img = NULL;
    pl.pv.current_canvas_buf = NULL;
    pl.pv.current_buf_size = 0;
    pl.pv.cached_img_path = NULL;

    pl.pv.bottom_bar = NULL;
    pl.pv.small_btn = NULL;
    pl.pv.small_btn_icon = NULL;
    pl.pv.big_btn = NULL;
    pl.pv.big_btn_icon = NULL;
    pl.pv.rota_btn = NULL;
    pl.pv.rota_btn_icon = NULL;
    pl.pv.reset_btn = NULL;
    pl.pv.reset_btn_icon = NULL;
    pl.pv.zoom_label = NULL;
    
    /* 重置拖动状态变量 */
    pl.last_x = 0;
    pl.last_y = 0;
    pl.old_img_path = NULL;
    
    /* 重置初始化标志 */
    pl.is_initialized = false;
    
    printf("[PhotoLibrary] Data structure initialized\n");
}

/*====================================================================================
 * 公共API实现
 *====================================================================================*/

/**
 * @brief 创建照片选择界面
 */
void create_photo_select_ui(lv_obj_t *parent) {
    if (!parent) return;

    /* 初始化数据结构 */
    pl_init();

    /* 创建UI组件（含 tabview 与两个 tab，init_photo_tab 已在 create 中调用） */
    create_ui_components(parent);

    /* 设置初始化标志 */
    pl.is_initialized = true;
    printf("[PhotoLibrary] UI created successfully\n");
}

/**
 * @brief 清理照片库资源
 */
void cleanup_photo_library(void) {
    printf("[PhotoLibrary] Starting cleanup...\n");

    /* 1. 清理图片缓存：仅使用 pl.old_img_path 这一条权威记录，避免访问已被扫描线程更新的 photo_paths */
    if (pl.old_img_path) {
        printf("[PhotoLibrary] Invalidating old image cache: %s\n", pl.old_img_path);
        lv_img_cache_invalidate_src(pl.old_img_path);
        free((void *)pl.old_img_path);
        pl.old_img_path = NULL;
    }
    pl.pv.has_photo_cache = 0;

    /* 3. 安全释放canvas缓冲区 */
    if (pl.pv.img) {
        if (pl.pv.current_canvas_buf) {
            static uint8_t dummy_buf[4] = {0};
            lv_canvas_set_buffer(pl.pv.img, dummy_buf, 1, 1, LV_IMG_CF_TRUE_COLOR_ALPHA);
            lv_mem_free(pl.pv.current_canvas_buf);
            pl.pv.current_canvas_buf = NULL;
            pl.pv.current_buf_size = 0;
            printf("[PhotoLibrary] Freed canvas buffer\n");
        }
        lv_obj_remove_event_cb(pl.pv.img, on_image_touch);
    }

    if (pl.pm.tabview) {
        lv_obj_remove_event_cb(pl.pm.tabview, tabview_event_handler);
    }
    if (pl.pv.back_btn) {
        lv_obj_remove_event_cb(pl.pv.back_btn, back_btn_event_handler);
    }
    if (pl.pv.prev_btn) {
        lv_obj_remove_event_cb(pl.pv.prev_btn, prev_btn_event_handler);
    }
    if (pl.pv.next_btn) {
        lv_obj_remove_event_cb(pl.pv.next_btn, next_btn_event_handler);
    }
    if (pl.pv.small_btn) {
        lv_obj_remove_event_cb(pl.pv.small_btn, small_btn_event_handler);
    }
    if (pl.pv.big_btn) {
        lv_obj_remove_event_cb(pl.pv.big_btn, big_btn_event_handler);
    }
    if (pl.pv.rota_btn) {
        lv_obj_remove_event_cb(pl.pv.rota_btn, rota_btn_event_handler);
    }
    if (pl.pv.reset_btn) {
        lv_obj_remove_event_cb(pl.pv.reset_btn, reset_btn_event_handler);
    }

    for (int i = 0; i < PHOTO_TAB_COUNT; i++) {
        free_photo_paths_tab(i);
    }

    /* 释放样式资源 */
    lv_style_reset(&pl.pm.style_btn);

    /* 不在此处删除 pl.pm.cont —— 它是 photoPage 的子对象，
     * 由 cleanup_playbackInterface 统一通过 UI_SAFE_DELETE(pb.tab_contents[1]) 删除，
     * 避免对象树重复析构导致 use-after-free。 */

    pl_init();

    printf("[PhotoLibrary] Cleanup completed.\n");
}

void reflush_photo_library(void) {
    init_photo_tab(PHOTO_TAB_FRONT);
    init_photo_tab(PHOTO_TAB_REAR);
}

/** 无卡时由主线程调用：清空所有 tab 列表并刷新 UI，不访问磁盘 */
void photo_library_clear_all(void) {
    int i;
    for (i = 0; i < PHOTO_TAB_COUNT; i++) {
        free_photo_paths_tab(i);
        populate_photo_list(i);
    }
}

void photo_library_apply_scan_result(int tab_index, char **paths, int count) {
    if (tab_index < 0 || tab_index >= PHOTO_TAB_COUNT) return;
    photo_tab_data_t *t = &pl.pm.tab_data[tab_index];
    free_photo_paths_tab(tab_index);
    if (count <= 0 || !paths) {
        populate_photo_list(tab_index);
        return;
    }
    t->photo_paths = (char**)malloc((size_t)count * sizeof(char*));
    if (!t->photo_paths) return;
    t->photo_capacity = count;
    t->photo_count = 0;
    for (int i = 0; i < count; i++) {
        if (!paths[i]) continue;
        t->photo_paths[t->photo_count] = strdup(paths[i]);
        if (t->photo_paths[t->photo_count]) t->photo_count++;
    }
    populate_photo_list(tab_index);
}