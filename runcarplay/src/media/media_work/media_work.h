#ifndef MEDIA_WORK_H
#define MEDIA_WORK_H

#include "media_work_result.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MEDIA_TF_MOUNT_PATH           "/mnt/extsd"
#define MEDIA_VIDEO_DIR_FRONT         "/mnt/extsd/recorder/frontCamera"
#define MEDIA_VIDEO_DIR_REAR          "/mnt/extsd/recorder/rearCamera"
#define MEDIA_VIDEO_DIR_URGENT_FRONT  "/mnt/extsd/recorderUrgent/frontCamera"
#define MEDIA_VIDEO_DIR_URGENT_REAR   "/mnt/extsd/recorderUrgent/rearCamera"
#define MEDIA_PHOTO_DIR_FRONT         "/mnt/extsd/DVRpic/frontPic"
#define MEDIA_PHOTO_DIR_REAR          "/mnt/extsd/DVRpic/rearPic"
#define MEDIA_MAX_FILES               100
#define MEDIA_MAX_PATH_LEN            256
#define MEDIA_TF_DEVICE                "/dev/mmcblk0"
#define MEDIA_TF_PARTITION             "/dev/mmcblk0p1"

typedef void (*media_work_stop_preview_fn)(void *userdata);

typedef void (*media_work_result_notify_fn)(void *userdata);

int media_work_init(void);

void media_work_deinit(void);

void media_work_set_stop_preview_callback(media_work_stop_preview_fn fn, void *userdata);

void media_work_submit_stop_preview(void);

int media_work_poll_done(void);

void media_work_submit_storage_query(void);

void media_work_submit_format_sd(void);

void media_work_submit_scan_video(int tab_index);

void media_work_submit_scan_photo(int tab_index);

void media_work_set_result_notify_callback(media_work_result_notify_fn fn, void *userdata);

int media_work_poll_results(media_result_t *out);

#ifdef __cplusplus
}
#endif

#endif /* MEDIA_WORK_H */
