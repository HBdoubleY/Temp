#include "media_work.h"
#include "tf_card.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_TASKS 24
#define RESULT_QUEUE_MAX 32

typedef enum {
    TASK_NONE = 0,
    TASK_STOP_PREVIEW,
    TASK_STORAGE_QUERY,
    TASK_FORMAT_SD,
    TASK_SCAN_VIDEO,
    TASK_SCAN_PHOTO
} task_type_t;

typedef struct {
    task_type_t type;
    int tab_index;
} task_item_t;

typedef struct result_node {
    media_result_t result;
    struct result_node *next;
} result_node_t;

static pthread_t s_worker;
static pthread_mutex_t s_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_cond = PTHREAD_COND_INITIALIZER;
static volatile int s_quit = 0;

static task_item_t s_tasks[MAX_TASKS];
static int s_task_head = 0;
static int s_task_tail = 0;
static int s_task_count = 0;

static result_node_t *s_result_head = NULL;
static result_node_t *s_result_tail = NULL;
static int s_result_count = 0;

static volatile int s_stop_preview_done = 0;
static media_work_stop_preview_fn s_stop_preview_fn = NULL;
static void *s_stop_preview_userdata = NULL;

static media_work_result_notify_fn s_result_notify_fn = NULL;
static void *s_result_notify_userdata = NULL;

/* ---------- 视频扫描（按 mtime 降序） ---------- */
static int compare_video_mtime_desc(const void *a, const void *b)
{
    const char *path_a = *(const char *const *)a;
    const char *path_b = *(const char *const *)b;
    if (!path_a || !path_b) return 0;
    struct stat sa, sb;
    if (stat(path_a, &sa) != 0 || stat(path_b, &sb) != 0) return 0;
    if (sa.st_mtime > sb.st_mtime) return -1;
    if (sa.st_mtime < sb.st_mtime) return 1;
    return 0;
}

static void do_scan_video(int tab_index, media_result_t *out)
{
    out->type = MEDIA_RESULT_SCAN_VIDEO;
    out->u.scan_video.tab_index = tab_index;
    out->u.scan_video.paths = NULL;
    out->u.scan_video.count = 0;

    const char *dir_path = NULL;
    const char *dir_path2 = NULL;
    if (tab_index == MEDIA_VIDEO_TAB_FRONT) {
        dir_path = MEDIA_VIDEO_DIR_FRONT;
    } else if (tab_index == MEDIA_VIDEO_TAB_REAR) {
        dir_path = MEDIA_VIDEO_DIR_REAR;
    } else if (tab_index == MEDIA_VIDEO_TAB_URGENT) {
        dir_path = MEDIA_VIDEO_DIR_URGENT_FRONT;
        dir_path2 = MEDIA_VIDEO_DIR_URGENT_REAR;
    } else {
        return;
    }

    char **list = (char **)malloc(sizeof(char *) * MEDIA_MAX_FILES);
    if (!list) return;
    int count = 0;

    for (int d = 0; d < 2 && count < MEDIA_MAX_FILES; d++) {
        const char *dir = (d == 0) ? dir_path : dir_path2;
        if (!dir) continue;
        DIR *dp = opendir(dir);
        if (!dp) continue;
        struct dirent *entry;
        char full_path[MEDIA_MAX_PATH_LEN];
        while ((entry = readdir(dp)) != NULL && count < MEDIA_MAX_FILES) {
            size_t len = strlen(entry->d_name);
            if (len > 4 && strcmp(entry->d_name + len - 4, ".mp4") == 0) {
                snprintf(full_path, sizeof(full_path), "%s/%s", dir, entry->d_name);
                list[count] = strdup(full_path);
                if (list[count]) count++;
            }
        }
        closedir(dp);
    }

    if (count > 1)
        qsort(list, (size_t)count, sizeof(char *), compare_video_mtime_desc);

    out->u.scan_video.paths = list;
    out->u.scan_video.count = count;
}

static int is_jpg_file(const char *filename)
{
    if (!filename) return 0;
    size_t len = strlen(filename);
    if (len < 4) return 0;
    if (strcasecmp(filename + len - 4, ".jpg") == 0) return 1;
    if (len >= 5 && strcasecmp(filename + len - 5, ".jpeg") == 0) return 1;
    return 0;
}

static int compare_photo_desc(const void *a, const void *b)
{
    const char *sa = *(const char **)a;
    const char *sb = *(const char **)b;
    return strcmp(sb, sa);
}

static int photo_list_capacity = 0;
static int photo_list_count = 0;
static char **photo_list = NULL;

static void photo_list_add(const char *full_path)
{
    if (photo_list_count >= photo_list_capacity) {
        int new_cap = photo_list_capacity ? photo_list_capacity * 2 : 64;
        char **new_list = (char **)realloc(photo_list, new_cap * sizeof(char *));
        if (!new_list) return;
        photo_list = new_list;
        photo_list_capacity = new_cap;
    }
    photo_list[photo_list_count] = strdup(full_path);
    if (photo_list[photo_list_count]) photo_list_count++;
}

static void scan_photo_dir_recursive(const char *dir_path)
{
    DIR *dir = opendir(dir_path);
    if (!dir) return;
    struct dirent *entry;
    struct stat st;
    char full_path[512];
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        if (stat(full_path, &st) != 0) continue;
        if (S_ISDIR(st.st_mode))
            scan_photo_dir_recursive(full_path);
        else if (S_ISREG(st.st_mode) && is_jpg_file(entry->d_name))
            photo_list_add(full_path);
    }
    closedir(dir);
}

static void do_scan_photo(int tab_index, media_result_t *out)
{
    out->type = MEDIA_RESULT_SCAN_PHOTO;
    out->u.scan_photo.tab_index = tab_index;
    out->u.scan_photo.paths = NULL;
    out->u.scan_photo.count = 0;

    const char *dir = (tab_index == MEDIA_PHOTO_TAB_FRONT) ? MEDIA_PHOTO_DIR_FRONT : MEDIA_PHOTO_DIR_REAR;
    photo_list = NULL;
    photo_list_capacity = 0;
    photo_list_count = 0;
    scan_photo_dir_recursive(dir);
    if (photo_list_count > 1)
        qsort(photo_list, (size_t)photo_list_count, sizeof(char *), compare_photo_desc);
    out->u.scan_photo.paths = photo_list;
    out->u.scan_photo.count = photo_list_count;
}

static void result_push(const media_result_t *r)
{
    if (s_result_count >= RESULT_QUEUE_MAX) return;
    result_node_t *node = (result_node_t *)malloc(sizeof(result_node_t));
    if (!node) return;
    memcpy(&node->result, r, sizeof(media_result_t));
    node->next = NULL;
    if (s_result_tail) {
        s_result_tail->next = node;
        s_result_tail = node;
    } else {
        s_result_head = s_result_tail = node;
    }
    s_result_count++;
    if (s_result_notify_fn)
        s_result_notify_fn(s_result_notify_userdata);
}

static int result_pop(media_result_t *out)
{
    if (!s_result_head || !out) return 0;
    result_node_t *node = s_result_head;
    s_result_head = node->next;
    if (!s_result_head) s_result_tail = NULL;
    s_result_count--;
    memcpy(out, &node->result, sizeof(media_result_t));
    free(node);
    return 1;
}

static void *worker_thread(void *arg)
{
    (void)arg;
    while (1) {
        task_item_t task = { TASK_NONE, 0 };
        pthread_mutex_lock(&s_mutex);
        while (!s_quit && s_task_count == 0)
            pthread_cond_wait(&s_cond, &s_mutex);
        if (s_quit) {
            pthread_mutex_unlock(&s_mutex);
            break;
        }
        task = s_tasks[s_task_head];
        s_task_head = (s_task_head + 1) % MAX_TASKS;
        s_task_count--;
        pthread_mutex_unlock(&s_mutex);

        media_result_t res = { MEDIA_RESULT_NONE, { 0 } };

        switch (task.type) {
        case TASK_STOP_PREVIEW:
            if (s_stop_preview_fn)
                s_stop_preview_fn(s_stop_preview_userdata);
            res.type = MEDIA_RESULT_STOP_PREVIEW;
            pthread_mutex_lock(&s_mutex);
            s_stop_preview_done = 1;
            result_push(&res);
            pthread_mutex_unlock(&s_mutex);
            break;

        case TASK_STORAGE_QUERY: {
            long long used_mb = -1, total_mb = -1;
            if (tf_card_get_storage_mb(&used_mb, &total_mb) != 0)
                used_mb = total_mb = -1;
            res.type = MEDIA_RESULT_STORAGE_QUERY;
            res.u.storage.used_mb = used_mb;
            res.u.storage.total_mb = total_mb;
            pthread_mutex_lock(&s_mutex);
            result_push(&res);
            pthread_mutex_unlock(&s_mutex);
            break;
        }

        case TASK_FORMAT_SD: {
            int ok = 0;
            if (tf_card_format(MEDIA_TF_DEVICE, "SD_CARD", 1) == 0)
                ok = 1;
            res.type = MEDIA_RESULT_FORMAT_SD;
            res.u.format.success = ok;
            pthread_mutex_lock(&s_mutex);
            result_push(&res);
            pthread_mutex_unlock(&s_mutex);
            break;
        }

        case TASK_SCAN_VIDEO:
            do_scan_video(task.tab_index, &res);
            pthread_mutex_lock(&s_mutex);
            result_push(&res);
            pthread_mutex_unlock(&s_mutex);
            break;

        case TASK_SCAN_PHOTO:
            do_scan_photo(task.tab_index, &res);
            pthread_mutex_lock(&s_mutex);
            result_push(&res);
            pthread_mutex_unlock(&s_mutex);
            break;

        default:
            break;
        }
    }
    return NULL;
}

static int task_submit(task_type_t type, int tab_index)
{
    pthread_mutex_lock(&s_mutex);
    if (s_task_count >= MAX_TASKS) {
        pthread_mutex_unlock(&s_mutex);
        return -1;
    }
    s_tasks[s_task_tail].type = type;
    s_tasks[s_task_tail].tab_index = tab_index;
    s_task_tail = (s_task_tail + 1) % MAX_TASKS;
    s_task_count++;
    pthread_cond_signal(&s_cond);
    pthread_mutex_unlock(&s_mutex);
    return 0;
}

int media_work_init(void)
{
    if (s_worker != (pthread_t)0)
        return 0;
    s_quit = 0;
    s_task_head = s_task_tail = s_task_count = 0;
    s_result_head = s_result_tail = NULL;
    s_result_count = 0;
    s_stop_preview_done = 0;
    if (pthread_create(&s_worker, NULL, worker_thread, NULL) != 0)
        return -1;
    return 0;
}

void media_work_deinit(void)
{
    pthread_mutex_lock(&s_mutex);
    s_quit = 1;
    s_result_notify_fn = NULL;
    s_result_notify_userdata = NULL;
    pthread_cond_signal(&s_cond);
    pthread_mutex_unlock(&s_mutex);
    if (s_worker != (pthread_t)0) {
        pthread_join(s_worker, NULL);
        s_worker = (pthread_t)0;
    }
    while (s_result_head) {
        result_node_t *n = s_result_head;
        s_result_head = n->next;
        media_work_free_result(&n->result);
        free(n);
    }
    s_result_tail = NULL;
    s_result_count = 0;
}

void media_work_set_stop_preview_callback(media_work_stop_preview_fn fn, void *userdata)
{
    s_stop_preview_fn = fn;
    s_stop_preview_userdata = userdata;
}

void media_work_submit_stop_preview(void)
{
    pthread_mutex_lock(&s_mutex);
    s_stop_preview_done = 0;
    pthread_mutex_unlock(&s_mutex);
    task_submit(TASK_STOP_PREVIEW, 0);
}

int media_work_poll_done(void)
{
    int done = 0;
    pthread_mutex_lock(&s_mutex);
    if (s_stop_preview_done) {
        s_stop_preview_done = 0;
        done = 1;
    }
    pthread_mutex_unlock(&s_mutex);
    return done;
}

void media_work_submit_storage_query(void)
{
    task_submit(TASK_STORAGE_QUERY, 0);
}

void media_work_submit_format_sd(void)
{
    task_submit(TASK_FORMAT_SD, 0);
}

void media_work_submit_scan_video(int tab_index)
{
    task_submit(TASK_SCAN_VIDEO, tab_index);
}

void media_work_submit_scan_photo(int tab_index)
{
    task_submit(TASK_SCAN_PHOTO, tab_index);
}

void media_work_set_result_notify_callback(media_work_result_notify_fn fn, void *userdata)
{
    s_result_notify_fn = fn;
    s_result_notify_userdata = userdata;
}

int media_work_poll_results(media_result_t *out)
{
    if (!out) return 0;
    pthread_mutex_lock(&s_mutex);
    int got = result_pop(out);
    pthread_mutex_unlock(&s_mutex);
    return got;
}

void media_work_free_result(media_result_t *r)
{
    if (!r) return;
    if (r->type == MEDIA_RESULT_SCAN_VIDEO && r->u.scan_video.paths) {
        for (int i = 0; i < r->u.scan_video.count; i++)
            free(r->u.scan_video.paths[i]);
        free(r->u.scan_video.paths);
        r->u.scan_video.paths = NULL;
        r->u.scan_video.count = 0;
    } else if (r->type == MEDIA_RESULT_SCAN_PHOTO && r->u.scan_photo.paths) {
        for (int i = 0; i < r->u.scan_photo.count; i++)
            free(r->u.scan_photo.paths[i]);
        free(r->u.scan_photo.paths);
        r->u.scan_photo.paths = NULL;
        r->u.scan_photo.count = 0;
    }
}
