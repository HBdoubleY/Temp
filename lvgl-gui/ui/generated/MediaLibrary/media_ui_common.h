/**
 * @file ui_common.h
 * @brief 公共UI组件库 - 提供可复用的UI组件创建函数
 * 
 * 该模块封装了媒体库中常用的UI组件创建逻辑，包括：
 * - 文件列表项
 * - 确认对话框
 * - 通用按钮样式
 * 
 * 所有组件默认支持主题切换
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "../lvgl-gui/lvgl/lvgl.h"
#include "media_theme_manager.h"

/*====================================================================================
 * 字体声明
 *====================================================================================*/

LV_FONT_DECLARE(v853Font_Medialibrary_30)
LV_FONT_DECLARE(v853Font_Medialibrary_40)

/*====================================================================================
 * 图片资源声明
 *====================================================================================*/

LV_IMG_DECLARE(delete)              /* 删除图标 */

/*====================================================================================
 * 对象有效性检查宏
 *====================================================================================*/

/**
 * @brief 检查LVGL对象是否有效
 * 
 * 用于在清理函数中安全检查对象指针
 */
#define UI_OBJ_IS_VALID(obj)        ((obj) != NULL && lv_obj_is_valid(obj))

/**
 * @brief 安全删除LVGL对象
 * 
 * 检查对象有效性后删除，并将指针置NULL
 */
#define UI_SAFE_DELETE(obj) do { \
    if (UI_OBJ_IS_VALID(obj)) { \
        lv_obj_del(obj); \
    } \
    (obj) = NULL; \
} while(0)

/**
 * @brief 安全删除定时器
 */
#define UI_SAFE_DELETE_TIMER(timer) do { \
    if ((timer) != NULL) { \
        lv_timer_set_cb(timer, NULL); \
        lv_timer_pause(timer); \
        lv_timer_del(timer); \
        (timer) = NULL; \
    } \
} while(0)

/**
 * @brief 安全移除事件回调
 */
#define UI_SAFE_REMOVE_EVENT(obj, cb) do { \
    if (UI_OBJ_IS_VALID(obj)) { \
        lv_obj_remove_event_cb(obj, cb); \
    } \
} while(0)

/*====================================================================================
 * 列表项组件
 *====================================================================================*/

/**
 * @brief 列表项配置结构体
 */
typedef struct {
    const char *text;               /* 显示文本 */
    const void *icon_src;           /* 图标源（LV_SYMBOL_* 或图片描述符） */
    bool show_delete_btn;           /* 是否显示删除按钮 */
    int user_data;                  /* 用户数据（通常为索引） */
    lv_event_cb_t click_cb;         /* 点击事件回调 */
    lv_event_cb_t delete_cb;        /* 删除按钮回调 */
} ui_list_item_cfg_t;

/**
 * @brief 创建带主题的文件列表项
 * 
 * @param parent 父容器
 * @param cfg 列表项配置
 * @return lv_obj_t* 创建的列表项按钮对象，失败返回NULL
 */
lv_obj_t* ui_create_file_list_item(lv_obj_t *parent, const ui_list_item_cfg_t *cfg);

/*====================================================================================
 * 对话框组件
 *====================================================================================*/

/**
 * @brief 确认对话框配置结构体
 */
typedef struct {
    const char *title;              /* 对话框标题（可选） */
    const char *message;            /* 提示消息 */
    const char *confirm_text;       /* 确认按钮文本 */
    const char *cancel_text;        /* 取消按钮文本 */
    void *user_data;                /* 用户数据（传递给回调） */
    lv_event_cb_t confirm_cb;       /* 确认按钮回调 */
    lv_event_cb_t cancel_cb;        /* 取消按钮回调 */
} ui_dialog_cfg_t;

/**
 * @brief 对话框句柄结构体
 * 
 * 用于管理对话框生命周期
 */
typedef struct {
    lv_obj_t *mask_layer;           /* 遮罩层 */
    lv_obj_t *dialog_bg;            /* 对话框背景 */
} ui_dialog_handle_t;

/**
 * @brief 创建确认对话框
 * 
 * @param parent 父容器（通常为 lv_scr_act()）
 * @param cfg 对话框配置
 * @param handle 输出参数，对话框句柄
 * @return true 创建成功
 * @return false 创建失败
 */
bool ui_create_confirm_dialog(lv_obj_t *parent, const ui_dialog_cfg_t *cfg, 
                              ui_dialog_handle_t *handle);

/**
 * @brief 关闭并销毁对话框
 * 
 * @param handle 对话框句柄
 */
void ui_close_dialog(ui_dialog_handle_t *handle);

/**
 * @brief 创建结果提示对话框（自动关闭）
 * 
 * @param parent 父容器
 * @param message 提示消息
 * @param is_success 是否为成功状态（影响颜色）
 * @param auto_close_ms 自动关闭时间（毫秒），0表示不自动关闭
 * @return lv_obj_t* 对话框对象
 */
lv_obj_t* ui_create_result_dialog(lv_obj_t *parent, const char *message, 
                                  bool is_success, uint32_t auto_close_ms);

/*====================================================================================
 * 通用样式设置函数
 *====================================================================================*/

/**
 * @brief 应用列表容器样式
 * 
 * 设置列表容器的背景、边框、内边距等样式
 * 
 * @param list 列表对象
 */
void ui_apply_list_style(lv_obj_t *list);

/**
 * @brief 应用标题栏样式
 * 
 * @param bar 标题栏容器
 */
void ui_apply_title_bar_style(lv_obj_t *bar);

/**
 * @brief 应用透明按钮样式
 * 
 * 设置按钮为透明背景，带按下效果
 * 
 * @param btn 按钮对象
 */
void ui_apply_transparent_btn_style(lv_obj_t *btn);

/**
 * @brief 应用标签页按钮样式
 * 
 * @param btn 标签页按钮
 * @param is_selected 是否为选中状态
 */
void ui_apply_tab_btn_style(lv_obj_t *btn, bool is_selected);

/**
 * @brief 应用滑块/进度条样式
 * 
 * @param slider 滑块对象
 */
void ui_apply_slider_style(lv_obj_t *slider);

/**
 * @brief 应用文本标签样式
 * 
 * @param label 标签对象
 * @param is_primary 是否为主要文本（影响颜色）
 */
void ui_apply_label_style(lv_obj_t *label, bool is_primary);

/*====================================================================================
 * 公共初始化函数
 *====================================================================================*/

/**
 * @brief 初始化公共UI组件库
 * 
 * 在使用UI组件前调用，初始化主题管理器等依赖
 */
void ui_common_init(void);

#ifdef __cplusplus
}
#endif
