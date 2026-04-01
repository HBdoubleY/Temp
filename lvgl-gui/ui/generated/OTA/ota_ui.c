#include "gui_guider.h"
#include "ota_main_interface.h"

void setup_scr_ota(lv_ui *ui)
{
    if (ui == NULL) {
        return;
    }

    ui->screen_OTA = lv_obj_create(NULL);
    MainInterface_init(ui->screen_OTA);

    ui->screen_OTA_del = false;
}

void cleanup_ota_ui(lv_ui *ui)
{
    if (ui == NULL) {
        return;
    }

    if (ui->screen_OTA == NULL) {
        ui->screen_OTA_del = true;
        return;
    }

    /*
     * 注意：此时ui->screen_OTA通常仍是lv_scr_act()（active screen）。
     * LVGL不允许直接删除active screen，否则会触发：
     *   lv_obj_del: the active screen was deleted
     * 并导致段错误。
     *
     * 因此这里仅做模块级资源释放（停止timer/回调/子模块等），
     * screen对象的释放统一交给 ui_load_scr_animation(... auto_del=true)。
     */
    MainInterface_deinit();

    /* 标记为等待由界面切换逻辑自动删除 */
    ui->screen_OTA_del = true;
}

