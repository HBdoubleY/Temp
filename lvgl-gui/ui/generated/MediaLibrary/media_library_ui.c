#include "media_main_interface.h"
#include "media_library_ui.h"

void setup_scr_media_library(lv_ui *ui)
{
    if (ui == NULL) {
        return;
    }

    ui->screen_MediaLibrary = lv_obj_create(NULL);
    lv_sdcard_widgets(ui->screen_MediaLibrary);

    ui->screen_MediaLibrary_del = false;
}

void cleanup_media_library_ui(lv_ui *ui) {
    if (ui->screen_MediaLibrary) {
        cleanup_playbackInterface();
        lv_obj_clean(ui->screen_MediaLibrary);
        lv_obj_del(ui->screen_MediaLibrary);
        ui->screen_MediaLibrary = NULL;
        ui->screen_MediaLibrary_del = true;
    }
}