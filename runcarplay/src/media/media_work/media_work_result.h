#ifndef MEDIA_WORK_RESULT_H
#define MEDIA_WORK_RESULT_H

#ifdef __cplusplus
extern "C" {
#endif

#define MEDIA_VIDEO_TAB_FRONT   0
#define MEDIA_VIDEO_TAB_REAR    1
#define MEDIA_VIDEO_TAB_URGENT  2
#define MEDIA_VIDEO_TAB_COUNT   3
#define MEDIA_PHOTO_TAB_FRONT   0
#define MEDIA_PHOTO_TAB_REAR    1
#define MEDIA_PHOTO_TAB_COUNT   2

typedef enum {
    MEDIA_RESULT_NONE = 0,
    MEDIA_RESULT_STOP_PREVIEW,
    MEDIA_RESULT_STORAGE_QUERY,
    MEDIA_RESULT_FORMAT_SD,
    MEDIA_RESULT_SCAN_VIDEO,
    MEDIA_RESULT_SCAN_PHOTO
} media_result_type_t;

typedef struct {
    long long used_mb;   /* -1 表示未挂载/失败 */
    long long total_mb;
} media_result_storage_t;

typedef struct {
    int success;
} media_result_format_t;

typedef struct {
    int tab_index;      /* MEDIA_VIDEO_TAB_* */
    char **paths;       /* 路径数组，主线程负责释放 */
    int count;
} media_result_scan_video_t;

typedef struct {
    int tab_index;      /* MEDIA_PHOTO_TAB_* */
    char **paths;       /* 路径数组，主线程负责释放 */
    int count;
} media_result_scan_photo_t;

typedef struct {
    media_result_type_t type;
    union {
        media_result_storage_t storage;
        media_result_format_t format;
        media_result_scan_video_t scan_video;
        media_result_scan_photo_t scan_photo;
    } u;
} media_result_t;

void media_work_free_result(media_result_t *r);

#ifdef __cplusplus
}
#endif

#endif /* MEDIA_WORK_RESULT_H */
