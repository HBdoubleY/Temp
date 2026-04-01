/**
 * @file OTA.c
 * @brief OTA升级模块实现
 * 
 * 实现固件版本检查、下载和升级功能
 */

#include "OTA.h"
#include "lvgl/lvgl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <ctype.h>
#include <sys/reboot.h>
#include <sys/stat.h>

/*============================================================================
 * 常量定义
 *============================================================================*/

#define BUFFER_SIZE               8192
#define OTA_SERVER_HOST           "43.132.151.53"
#define OTA_VERSION_PATH          "/V853/Version"
#define OTA_FIRMWARE_PATH         "/V853/runcarplay"
#define LOCAL_VERSION_PATH        "/opt/work/app/Version"
#define OTA_VERSION_TEMP_PATH     "/opt/work/app/Version-ota"
#define OTA_FIRMWARE_TEMP_PATH    "/opt/work/app/carplay/runcarplay-ota"
#define OTA_FIRMWARE_FINAL_PATH   "/opt/work/app/carplay/runcarplay"

/*============================================================================
 * 全局变量定义
 *============================================================================*/

/* OTA UI全局实例 */
ota_interface_t g_ota_ui = {0};

/* OTA 模块语言设置（参考媒体库） */
// language_t ota_language_setting = LANG_CHINESE;
language_t ota_language_setting = LANG_CHINESE;

static lv_color_t ota_check_btn_text_color(void) {
    return (theme_get_type() == THEME_OTA_NIGHT) ? lv_color_white() : TH_TEXT_INVERSE;
}

static inline uint32_t ota_gen(void){
    return g_ota_ui.generation;
}

static inline bool ota_alive(uint32_t gen){
    return (!g_ota_ui.stop_requested) && g_ota_ui.is_initialized && (g_ota_ui.container != NULL) && (gen == g_ota_ui.generation);
}


/*============================================================================
 * 私有函数声明
 *============================================================================*/

static int http_download(const char *url, const char *local_path);
static bool file_download(const char *file, const char *local_file);
static bool chufa_upgrade(void);
static bool network_is_available_quick(void);
static int parse_version_triplet(const char *filepath, int *major, int *minor, int *pathv);
static void read_version_info(char *ver_out, size_t ver_len, char *date_out, size_t date_len);

/* 定时器回调 */
static void overlay_del_cb(lv_timer_t *t);
static void download_progress_cb(lv_timer_t *t);
static void delayed_upgrade_cb(lv_timer_t *t);

/* UI辅助函数 */
static void show_timed_modal(const char *title_text, const char *sub_text, uint32_t border_hex,
                             const lv_font_t *title_font, const lv_font_t *sub_font);
static void show_confirm_modal(const char *title_text, const char *sub_text,
                               const char *yes_text, const char *no_text);

/* 事件回调 */
static void check_btn_event_cb(lv_event_t *e);
static void confirm_yes_cb(lv_event_t *e);
static void confirm_no_cb(lv_event_t *e);

/*============================================================================
 * HTTP下载相关函数
 *============================================================================*/

/**
 * @brief HTTP下载文件
 * @param url 下载URL
 * @param local_path 本地保存路径
 * @return 0成功，-1失败
 */
static int http_download(const char *url, const char *local_path) {
    char host[256] = {0};
    char path[1024] = {0};
    int port = 80;
    
    /* 解析URL */
    if (sscanf(url, "http://%[^/]%s", host, path) != 2) {
        if (sscanf(url, "http://%[^:]:%d%s", host, &port, path) != 3) {
            fprintf(stderr, "[OTA] Invalid URL\n");
            return -1;
        }
    }
    if (strlen(path) == 0) strcpy(path, "/");
    
    printf("[OTA] Downloading: http://%s:%d%s\n", host, port, path);
    
    /* 创建socket */
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("[OTA] Failed to create socket\n");
        return -1;
    }
    
    /* DNS解析 */
    struct hostent *he = gethostbyname(host);
    if (!he) {
        /* 如果是IP地址，直接使用 */
        struct in_addr addr;
        if (inet_aton(host, &addr)) {
            /* 直接使用IP地址连接 */
            struct sockaddr_in server = {0};
            server.sin_family = AF_INET;
            server.sin_port = htons(port);
            server.sin_addr = addr;
            
            if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
                printf("[OTA] Failed to connect to %s\n", host);
                close(sock);
                return -1;
            }
        } else {
            printf("[OTA] DNS resolution failed for %s\n", host);
            close(sock);
            return -1;
        }
    } else {
        /* 使用DNS解析结果连接 */
        struct sockaddr_in server = {0};
        server.sin_family = AF_INET;
        server.sin_port = htons(port);
        memcpy(&server.sin_addr, he->h_addr_list[0], he->h_length);
        
        if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
            printf("[OTA] Failed to connect to %s\n", host);
            close(sock);
            return -1;
        }
    }
    
    /* 发送HTTP请求 */
    char request[2048];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Connection: close\r\n"
             "User-Agent: V853-Downloader\r\n"
             "\r\n", path, host);
    
    write(sock, request, strlen(request));
    
    /* 打开本地文件 */
    FILE *fp = fopen(local_path, "wb");
    if (!fp) {
        printf("[OTA] Failed to create local file: %s\n", local_path);
        close(sock);
        return -1;
    }
    
    /* 接收响应 */
    char buffer[BUFFER_SIZE];
    ssize_t n;
    long total = 0;
    int in_body = 0;
    long content_length = -1;
    
    while ((n = read(sock, buffer, sizeof(buffer))) > 0) {
        if (!in_body) {
            char *header_end = strstr(buffer, "\r\n\r\n");
            if (header_end) {
                int header_len = header_end - buffer + 4;
                
                /* 提取HTTP头部进行解析 */
                if (header_len > 0 && header_len < (int)sizeof(buffer)) {
                    char header_copy[BUFFER_SIZE];
                    memcpy(header_copy, buffer, header_len);
                    header_copy[header_len] = '\0';
                    
                    /* 检查HTTP状态码 */
                    if (strstr(header_copy, "200 OK")) {
                        /* 解析Content-Length */
                        char *cl = strstr(header_copy, "Content-Length:");
                        if (cl) {
                            sscanf(cl, "Content-Length: %ld", &content_length);
                        }
                        
                        /* 写入body部分 */
                        fwrite(buffer + header_len, 1, n - header_len, fp);
                        total = n - header_len;
                        in_body = 1;
                        
                        /* 更新下载进度（仅更新共享变量，不调用LVGL） */
                        g_ota_ui.download_done = total;
                        g_ota_ui.download_total = content_length;
                    } else {
                        printf("[OTA] Server returned error: %.*s\n", (int)header_len, header_copy);
                        fclose(fp);
                        close(sock);
                        remove(local_path);
                        return -1;
                    }
                }
            }
        } else {
            fwrite(buffer, 1, n, fp);
            total += n;
            
            /* 更新下载进度 */
            g_ota_ui.download_done = total;
        }
    }

    fclose(fp);
    close(sock);

    if (total > 0) {
        printf("[OTA] Downloaded: %ld bytes\n", total);
        return 0;
    }
    return -1;
}

/*============================================================================
 * 下载线程相关
 *============================================================================*/

/**
 * @brief 下载线程参数结构体
 */
struct dl_thread_arg {
    const char *remote_path;
    const char *local_path;
};

/**
 * @brief 文件下载函数
 * @param file 远程文件路径
 * @param local_file 本地保存路径
 * @return 成功返回true
 */
static bool file_download(const char *file, const char *local_file) {
    char url[256];
    snprintf(url, sizeof(url), "http://%s%s", OTA_SERVER_HOST, file);
    
    printf("[OTA] === Attempting download: %s ===\n", url);
    
    if (http_download(url, local_file) == 0) {
        printf("[OTA] Success! File saved to: %s\n", local_file);
        return true;
    }
    
    return false;
}

/**
 * @brief 下载线程函数
 * @param arg 线程参数
 * @return NULL
 */
static void *download_thread_func(void *arg) {
    struct dl_thread_arg a;
    memcpy(&a, arg, sizeof(a));
    free(arg);

    g_ota_ui.download_thread_running = 1;
    
    /* 重置下载状态 */
    g_ota_ui.download_finished = 0;
    g_ota_ui.download_result = 0;
    g_ota_ui.download_total = -1;
    g_ota_ui.download_done = 0;

    if (!g_ota_ui.stop_requested) {
    bool ok = file_download(a.remote_path, a.local_path);
        if (!g_ota_ui.stop_requested) {
    g_ota_ui.download_finished = 1;
    g_ota_ui.download_result = ok ? 1 : 0;
        }
    }

    g_ota_ui.download_thread_running = 0;
    
    return NULL;
}

/**
 * @brief 下载进度更新定时器回调
 * @param t 定时器对象
 */
static void download_progress_cb(lv_timer_t *t) {
    uint32_t gen = (uint32_t)(uintptr_t)t->user_data;
    if(!ota_alive(gen)) return;
    
    if (!g_ota_ui.download_bar && !g_ota_ui.download_label) return;
    
    long done = g_ota_ui.download_done;
    long tot = g_ota_ui.download_total;
    int percent = 0;
    
    if (tot > 0) {
        percent = (int)((done * 100) / tot);
    }
    
    lv_bar_set_value(g_ota_ui.download_bar, percent, LV_ANIM_OFF);
    
    if (g_ota_ui.download_label) {
        char pbuf[64];
        if (tot > 0) {
            snprintf(pbuf, sizeof(pbuf), "%d%%", percent);
        } else {
            snprintf(pbuf, sizeof(pbuf), "%ld bytes", done);
        }
        lv_label_set_text(g_ota_ui.download_label, pbuf);
    }

    /* 检查下载是否完成 */
    if (g_ota_ui.download_finished) {
        /* 停止定时器 */
        if (g_ota_ui.download_timer) {
            lv_timer_del(g_ota_ui.download_timer);
            g_ota_ui.download_timer = NULL;
        }
        
        /* 关闭模态窗口 */
        if (g_ota_ui.modal_overlay) {
            lv_obj_del(g_ota_ui.modal_overlay);
            g_ota_ui.modal_overlay = NULL;
        }
        
        /* 清理UI指针 */
        g_ota_ui.download_bar = NULL;
        g_ota_ui.download_label = NULL;

        /* 显示结果提示 */
        if (g_ota_ui.download_result) {
            show_timed_modal(get_string_for_language(ota_language_setting, "ota_download_completed"),
                            get_string_for_language(ota_language_setting, "ota_system_upgrade"),
                            g_theme->success, &v853Font_OTA_36, &v853Font_OTA_28);
            /* 3 秒后触发升级，避免立刻重启看不到提示 */
            if (g_ota_ui.upgrade_timer) {
                lv_timer_del(g_ota_ui.upgrade_timer);
                g_ota_ui.upgrade_timer = NULL;
            }
            g_ota_ui.upgrade_timer = lv_timer_create(delayed_upgrade_cb, 3000, (void *)(uintptr_t)ota_gen());
            if (!g_ota_ui.upgrade_timer) {
                /* 如果创建定时器失败，则直接触发升级（兜底） */
                chufa_upgrade();
            }
        } else {
            show_timed_modal(get_string_for_language(ota_language_setting, "ota_download_failed"),
                            get_string_for_language(ota_language_setting, "ota_file_download_err"),
                            g_theme->error, &v853Font_OTA_36, &v853Font_OTA_28);
        }
    }
}

/**
 * @brief 升级延时定时器回调（在 LVGL 上下文中安全调用）
 */
static void delayed_upgrade_cb(lv_timer_t *t) {
    uint32_t gen = (uint32_t)(uintptr_t)t->user_data;
    if(!ota_alive(gen)) return;

    if (g_ota_ui.upgrade_timer == t) {
        g_ota_ui.upgrade_timer = NULL;
    }
    chufa_upgrade();
}

/*============================================================================
 * 升级相关函数
 *============================================================================*/

/**
 * @brief 触发升级（替换固件文件）
 * @return 成功返回true
 */
static bool chufa_upgrade(void) {
    printf("[OTA] Triggering upgrade with new paths...\n");

    /* 确保临时固件具有可执行权限 */
    if (chmod(OTA_FIRMWARE_TEMP_PATH, 0755) != 0) {
        perror("[OTA] Failed to chmod firmware temp file");
        return false;
    }

    /* 先替换固件文件 */
    if (rename(OTA_FIRMWARE_TEMP_PATH, OTA_FIRMWARE_FINAL_PATH) != 0) {
        perror("[OTA] Failed to replace firmware (rename runcarplay-ota)");
        return false;
    }
    printf("[OTA] Firmware replaced successfully: %s -> %s\n",
           OTA_FIRMWARE_TEMP_PATH, OTA_FIRMWARE_FINAL_PATH);

    /* 再替换版本文件 */
    if (rename(OTA_VERSION_TEMP_PATH, LOCAL_VERSION_PATH) != 0) {
        perror("[OTA] Failed to update Version file (rename Version-ota)");
        /* 固件已更新但版本文件未同步，避免继续自动重启 */
        return false;
    }
    printf("[OTA] Version file updated successfully: %s -> %s\n",
           OTA_VERSION_TEMP_PATH, LOCAL_VERSION_PATH);

    /* 同步磁盘并通过 C 接口重启 */
    sync();
    printf("[OTA] Rebooting system via reboot(RB_AUTOBOOT)...\n");
    reboot(RB_AUTOBOOT);

    return true;
}

/*============================================================================
 * 网络检查函数
 *============================================================================*/

/**
 * @brief 快速检查网络可用性
 * @return 网络可用返回true
 */
static bool network_is_available_quick(void) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return false;

    /* 设置超时 */
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 300000;  /* 300ms超时 */
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    /* 直接连接DNS服务器测试网络 */
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(53);
    inet_pton(AF_INET, "114.114.114.114", &addr.sin_addr);
    
    int ret = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    close(sock);
    
    return (ret == 0 || errno == EINPROGRESS);
}

/*============================================================================
 * 版本信息解析函数
 *============================================================================*/

/**
 * @brief 解析版本文件中的版本号
 * @param filepath 版本文件路径
 * @param major 主版本号输出
 * @param minor 次版本号输出
 * @param pathv 补丁版本号输出
 * @return 成功返回0，失败返回-1
 */
static int parse_version_triplet(const char *filepath, int *major, int *minor, int *pathv) {
    if (!filepath || !major || !minor || !pathv) return -1;
    
    FILE *fp = fopen(filepath, "r");
    if (!fp) return -1;
    
    *major = -1; 
    *minor = -1; 
    *pathv = -1;
    
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        char *s = line;
        while (*s && isspace((unsigned char)*s)) s++;
        
        if (strncmp(s, "major:", 6) == 0) {
            sscanf(s + 6, "%d", major);
        } else if (strncmp(s, "minor:", 6) == 0) {
            sscanf(s + 6, "%d", minor);
        } else if (strncmp(s, "path:", 5) == 0) {
            sscanf(s + 5, "%d", pathv);
        }
    }
    
    fclose(fp);
    
    if (*major < 0 || *minor < 0 || *pathv < 0) return -1;
    return 0;
}

/**
 * @brief 读取本地版本信息
 * @param ver_out 版本号输出缓冲区
 * @param ver_len 版本号缓冲区大小
 * @param date_out 日期输出缓冲区
 * @param date_len 日期缓冲区大小
 */
static void read_version_info(char *ver_out, size_t ver_len, char *date_out, size_t date_len) {
    FILE *fp = fopen(LOCAL_VERSION_PATH, "r");
    int major = -1, minor = -1, p = -1;
    char datebuf[32] = {0};

    if (!fp) {
        snprintf(ver_out, ver_len, "V1.0.0");
        snprintf(date_out, date_len, "Unknown");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        char *s = line;
        while (*s && isspace((unsigned char)*s)) s++;
        
        if (strncmp(s, "major:", 6) == 0) {
            sscanf(s + 6, "%d", &major);
        } else if (strncmp(s, "minor:", 6) == 0) {
            sscanf(s + 6, "%d", &minor);
        } else if (strncmp(s, "path:", 5) == 0) {
            sscanf(s + 5, "%d", &p);
        } else if (strncmp(s, "date:", 5) == 0) {
            char *d = s + 5;
            while (*d && isspace((unsigned char)*d)) d++;
            
            size_t i = 0;
            while (*d && i + 1 < sizeof(datebuf) && (isdigit((unsigned char)*d) || *d == '-')) {
                datebuf[i++] = *d++;
            }
            datebuf[i] = '\0';
        }
    }
    fclose(fp);

    if (major < 0) major = 1;
    if (minor < 0) minor = 0;
    if (p < 0) p = 0;

    snprintf(ver_out, ver_len, "V%d.%d.%d", major, minor, p);

    if (datebuf[0]) {
        /* 格式化日期：20260204 -> 2026-02-04 */
        size_t L = strlen(datebuf);
        if (L == 8 && isdigit((unsigned char)datebuf[0])) {
            char dfmt[16];
            snprintf(dfmt, sizeof(dfmt), "%.4s-%.2s-%.2s", datebuf, datebuf + 4, datebuf + 6);
            snprintf(date_out, date_len, "%s", dfmt);
        } else {
            snprintf(date_out, date_len, "%s", datebuf);
        }
    } else {
        snprintf(date_out, date_len, "Unknown");
    }
}

/*============================================================================
 * 模态窗口UI函数
 *============================================================================*/

/**
 * @brief 模态窗口自动关闭定时器回调
 * @param t 定时器对象
 */
static void modal_mask_swallow_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch(code) {
        case LV_EVENT_PRESSED:
        case LV_EVENT_PRESSING:
        case LV_EVENT_RELEASED:
        case LV_EVENT_CLICKED:
        case LV_EVENT_LONG_PRESSED:
        case LV_EVENT_LONG_PRESSED_REPEAT:
        case LV_EVENT_GESTURE:
        case LV_EVENT_SCROLL:
        case LV_EVENT_SCROLL_BEGIN:
        case LV_EVENT_SCROLL_END:
            lv_event_stop_bubbling(e);
            break;
        default:
            break;
    }
}

static void overlay_del_cb(lv_timer_t *t) {
    uint32_t gen = (uint32_t)(uintptr_t)t->user_data;
    if(!ota_alive(gen)) return;
    
    if (g_ota_ui.modal_overlay && lv_obj_is_valid(g_ota_ui.modal_overlay)) {
        lv_obj_del(g_ota_ui.modal_overlay);
        g_ota_ui.modal_overlay = NULL;
    }
    
    if (g_ota_ui.unavailble_net_timer) {
        lv_timer_pause(g_ota_ui.unavailble_net_timer);
    }
}

/**
 * @brief 显示自动关闭的模态提示窗口
 * @param title_text 标题文字
 * @param sub_text 副标题文字
 * @param border_hex 边框颜色（十六进制）
 * @param title_font 标题字体
 * @param sub_font 副标题字体
 */
static void show_timed_modal(const char *title_text, const char *sub_text, uint32_t border_hex,
                             const lv_font_t *title_font, const lv_font_t *sub_font) {
    uint32_t gen = ota_gen();
    if(!ota_alive(gen)) return;
    /* 如果已有模态窗口，先删除 */
    if (g_ota_ui.modal_overlay) {
        lv_obj_del(g_ota_ui.modal_overlay);
        g_ota_ui.modal_overlay = NULL;
    }
    
    /*
     * 遮罩层必须覆盖整个 OTA 屏幕（包含左侧栏），否则左侧栏仍可点击。
     * 这里不要挂在 g_ota_ui.container（它是右侧内容区的容器），而是挂在它的父对象
     * （即 tab_contents[1]，覆盖整个内容页）。
     */
    lv_obj_t *parent = g_ota_ui.container;
    if(parent == NULL) return;

    /*
     * 为保证遮罩能覆盖整个 OTA 界面（包括左侧菜单栏），遮罩应当挂在 active screen 的 Top Layer。
     * 这样可以避免仅覆盖右侧内容容器、或被其他容器层级挡住。
     */
    lv_obj_t *overlay_parent = lv_layer_top();
    if(overlay_parent == NULL) overlay_parent = lv_scr_act();

    /* 创建遮罩层（严格Modal：覆盖全屏并吞掉所有输入事件） */
    lv_obj_t *overlay = lv_obj_create(overlay_parent);
    lv_obj_set_size(overlay, LV_PCT(100), LV_PCT(100));
    lv_obj_set_pos(overlay, 0, 0);
    lv_obj_set_style_radius(overlay, 0, 0);
    lv_obj_set_style_pad_all(overlay, 0, 0);
    lv_obj_set_style_border_width(overlay, 0, 0);
    lv_obj_set_style_bg_color(overlay, TH_BG_OVERLAY, 0);
    lv_obj_set_style_bg_opa(overlay, LV_OPA_30, 0);
    lv_obj_clear_flag(overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(overlay, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(overlay, modal_mask_swallow_cb, LV_EVENT_ALL, NULL);
    lv_obj_move_foreground(overlay);

    /* 创建卡片 */
    lv_obj_t *card = lv_obj_create(overlay);
    lv_obj_set_size(card, MODAL_WIDTH_LG, MODAL_HEIGHT_SM);
    lv_obj_set_style_radius(card, RADIUS_LG, 0);
    lv_obj_set_style_bg_color(card, TH_BG_MODAL, 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(card, lv_color_hex(border_hex), 0);
    lv_obj_set_style_border_width(card, BORDER_WIDTH_BOLD, 0);
    lv_obj_set_style_shadow_color(card, lv_color_hex(border_hex), 0);
    lv_obj_set_style_shadow_width(card, SHADOW_WIDTH_MD, 0);
    lv_obj_align(card, LV_ALIGN_CENTER, 60, -20);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    /* 标题 */
    lv_obj_t *title_lbl = lv_label_create(card);
    lv_label_set_text(title_lbl, title_text);
    if (title_font) lv_obj_set_style_text_font(title_lbl, title_font, 0);
    lv_obj_set_style_text_color(title_lbl, theme_color(g_theme->warning), 0);
    lv_obj_align(title_lbl, LV_ALIGN_TOP_MID, 0, 18);

    /* 副标题 */
    lv_obj_t *sub = lv_label_create(card);
    lv_label_set_text(sub, sub_text);
    if (sub_font) lv_obj_set_style_text_font(sub, sub_font, 0);
    lv_obj_set_style_text_color(sub, TH_TEXT_SECONDARY, 0);
    lv_label_set_long_mode(sub, LV_LABEL_LONG_WRAP);
    lv_obj_align(sub, LV_ALIGN_CENTER, 0, 34);

    g_ota_ui.modal_overlay = overlay;
    
    /* 设置自动关闭定时器 */
    if (g_ota_ui.unavailble_net_timer) {
        lv_timer_reset(g_ota_ui.unavailble_net_timer);
        lv_timer_resume(g_ota_ui.unavailble_net_timer);
    } else {
        g_ota_ui.unavailble_net_timer = lv_timer_create(overlay_del_cb, 3000, (void *)(uintptr_t)ota_gen());
    }
}

/*============================================================================
 * 确认对话框回调
 *============================================================================*/

/**
 * @brief 取消按钮回调
 */
static void confirm_no_cb(lv_event_t *e) {
    (void)e;
    
    if (g_ota_ui.modal_overlay) {
        lv_obj_del(g_ota_ui.modal_overlay);
        g_ota_ui.modal_overlay = NULL;
    }
    g_ota_ui.confirm_yes_btn = NULL;
}

/**
 * @brief 确认按钮回调 - 开始下载升级
 */
static void confirm_yes_cb(lv_event_t *e) {
    uint32_t gen = ota_gen();
    if(!ota_alive(gen)) return;

    if (g_ota_ui.download_thread_running) {
        return;
    }

    lv_obj_t *btn = lv_event_get_target(e);
    if (btn && lv_obj_is_valid(btn)) {
        lv_obj_add_state(btn, LV_STATE_DISABLED);
        lv_obj_clear_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    }
    
    /* 先关闭确认对话框 */
    if (g_ota_ui.modal_overlay && lv_obj_is_valid(g_ota_ui.modal_overlay)) {
        lv_obj_del(g_ota_ui.modal_overlay);
        g_ota_ui.modal_overlay = NULL;
    }
    g_ota_ui.confirm_yes_btn = NULL;

    lv_obj_t *parent = g_ota_ui.container;
    if(parent == NULL) return;

    lv_obj_t *overlay_parent = lv_layer_top();
    if(overlay_parent == NULL) overlay_parent = lv_scr_act();

    /* 创建下载进度模态窗口（严格Modal） */
    lv_obj_t *overlay = lv_obj_create(overlay_parent);
    lv_obj_set_size(overlay, LV_PCT(100), LV_PCT(100));
    lv_obj_set_pos(overlay, 0, 0);
    lv_obj_set_style_radius(overlay, 0, 0);
    lv_obj_set_style_pad_all(overlay, 0, 0);
    lv_obj_set_style_border_width(overlay, 0, 0);
    lv_obj_set_style_bg_color(overlay, TH_BG_OVERLAY, 0);
    lv_obj_set_style_bg_opa(overlay, LV_OPA_30, 0);
    lv_obj_clear_flag(overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(overlay, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(overlay, modal_mask_swallow_cb, LV_EVENT_ALL, NULL);
    lv_obj_move_foreground(overlay);

    lv_obj_t *card = lv_obj_create(overlay);
    lv_obj_set_size(card, MODAL_WIDTH_LG, 220);
    lv_obj_set_style_radius(card, RADIUS_LG, 0);
    lv_obj_set_style_bg_color(card, TH_BG_MODAL, 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(card, theme_color(g_theme->warning), 0);
    lv_obj_set_style_border_width(card, BORDER_WIDTH_BOLD, 0);
    lv_obj_set_style_shadow_color(card, theme_color(g_theme->warning), 0);
    lv_obj_set_style_shadow_width(card, SHADOW_WIDTH_MD, 0);
    lv_obj_align(card, LV_ALIGN_CENTER, 60, -20);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    /* 标题 */
    lv_obj_t *title_lbl = lv_label_create(card);
    lv_label_set_text(title_lbl, get_string_for_language(ota_language_setting, "ota_downloading"));
    lv_obj_set_style_text_font(title_lbl, &v853Font_OTA_36, 0);
    lv_obj_set_style_text_color(title_lbl, theme_color(g_theme->warning), 0);
    lv_obj_align(title_lbl, LV_ALIGN_TOP_MID, 0, 18);

    /* 进度条 */
    lv_obj_t *bar = lv_bar_create(card);
    lv_bar_set_range(bar, 0, 100);
    lv_obj_set_size(bar, 860, 28);
    lv_obj_align(bar, LV_ALIGN_CENTER, 0, 6);
    lv_obj_set_style_bg_color(bar, TH_PROGRESS_BG, LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, TH_PROGRESS_FILL, LV_PART_INDICATOR);

    /* 进度标签 */
    lv_obj_t *plab = lv_label_create(card);
    lv_label_set_text(plab, "0%");
    lv_obj_set_style_text_font(plab, &v853Font_OTA_28, 0);
    lv_obj_set_style_text_color(plab, TH_TEXT_SECONDARY, 0);
    lv_obj_align(plab, LV_ALIGN_BOTTOM_MID, 0, -18);

    /* 保存到全局变量 */
    g_ota_ui.modal_overlay = overlay;
    g_ota_ui.download_bar = bar;
    g_ota_ui.download_label = plab;
    g_ota_ui.download_total = -1;
    g_ota_ui.download_done = 0;
    g_ota_ui.download_finished = 0;
    g_ota_ui.download_result = 0;
    
    /* 创建进度更新定时器 */
    if (g_ota_ui.download_timer) {
        lv_timer_del(g_ota_ui.download_timer);
    }
    g_ota_ui.download_timer = lv_timer_create(download_progress_cb, 200, (void *)(uintptr_t)ota_gen());

    /* 启动下载线程 */
    pthread_t th;
    struct dl_thread_arg *arg = malloc(sizeof(*arg));
    if (!arg) {
        if (g_ota_ui.download_timer) { 
            lv_timer_del(g_ota_ui.download_timer); 
            g_ota_ui.download_timer = NULL; 
        }
        if (g_ota_ui.modal_overlay) { 
            lv_obj_del(g_ota_ui.modal_overlay); 
            g_ota_ui.modal_overlay = NULL; 
        }
        g_ota_ui.download_bar = NULL; 
        g_ota_ui.download_label = NULL;
        show_timed_modal(get_string_for_language(ota_language_setting, "ota_download_failed"),
                        get_string_for_language(ota_language_setting, "ota_file_download_err"),
                        g_theme->error, &v853Font_OTA_36, &v853Font_OTA_28);
    } else {
        arg->remote_path = OTA_FIRMWARE_PATH;
        arg->local_path  = OTA_FIRMWARE_TEMP_PATH;

        if (pthread_create(&th, NULL, download_thread_func, arg) == 0) {
            pthread_detach(th);
        } else {
            free(arg);
            if (g_ota_ui.download_timer) { 
                lv_timer_del(g_ota_ui.download_timer); 
                g_ota_ui.download_timer = NULL; 
            }
            if (g_ota_ui.modal_overlay) { 
                lv_obj_del(g_ota_ui.modal_overlay); 
                g_ota_ui.modal_overlay = NULL; 
            }
            g_ota_ui.download_bar = NULL; 
            g_ota_ui.download_label = NULL;
            show_timed_modal(get_string_for_language(ota_language_setting, "ota_download_failed"),
                            get_string_for_language(ota_language_setting, "ota_file_download_err"),
                            g_theme->error, &v853Font_OTA_36, &v853Font_OTA_28);
        }
    }
}

/**
 * @brief 显示确认对话框
 */
static void show_confirm_modal(const char *title_text, const char *sub_text,
                               const char *yes_text, const char *no_text) {
    uint32_t gen = ota_gen();
    if(!ota_alive(gen)) return;

    /* 先关闭确认对话框 */
    if (g_ota_ui.modal_overlay && lv_obj_is_valid(g_ota_ui.modal_overlay)) {
        lv_obj_del(g_ota_ui.modal_overlay);
        g_ota_ui.modal_overlay = NULL;
    }

    lv_obj_t *parent = g_ota_ui.container;
    if(parent == NULL) return;

    lv_obj_t *overlay_parent = lv_layer_top();
    if(overlay_parent == NULL) overlay_parent = lv_scr_act();
    
    /* 创建遮罩层（严格Modal） */
    lv_obj_t *overlay = lv_obj_create(overlay_parent);
    lv_obj_set_size(overlay, LV_PCT(100), LV_PCT(100));
    lv_obj_set_pos(overlay, 0, 0);
    lv_obj_set_style_radius(overlay, 0, 0);
    lv_obj_set_style_pad_all(overlay, 0, 0);
    lv_obj_set_style_border_width(overlay, 0, 0);
    lv_obj_set_style_bg_color(overlay, TH_BG_OVERLAY, 0);
    lv_obj_set_style_bg_opa(overlay, LV_OPA_30, 0);
    lv_obj_clear_flag(overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(overlay, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(overlay, modal_mask_swallow_cb, LV_EVENT_ALL, NULL);
    lv_obj_move_foreground(overlay);

    /* 创建卡片 */
    lv_obj_t *card = lv_obj_create(overlay);
    lv_obj_set_size(card, MODAL_WIDTH_LG, 260);
    lv_obj_set_style_radius(card, RADIUS_LG, 0);
    lv_obj_set_style_bg_color(card, TH_BG_MODAL, 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(card, theme_color(g_theme->warning), 0);
    lv_obj_set_style_border_width(card, BORDER_WIDTH_BOLD, 0);
    lv_obj_set_style_shadow_color(card, theme_color(g_theme->warning), 0);
    lv_obj_set_style_shadow_width(card, SHADOW_WIDTH_MD, 0);
    lv_obj_align(card, LV_ALIGN_CENTER, 60, -20);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    /* 标题 */
    lv_obj_t *title_lbl = lv_label_create(card);
    lv_label_set_text(title_lbl, title_text);
    lv_obj_set_style_text_font(title_lbl, &v853Font_OTA_42, 0);
    lv_obj_set_style_text_color(title_lbl, theme_color(g_theme->warning), 0);
    lv_obj_align(title_lbl, LV_ALIGN_TOP_MID, 0, 5);

    /* 副标题 */
    lv_obj_t *sub = lv_label_create(card);
    lv_label_set_text(sub, sub_text);
    lv_obj_set_style_text_font(sub, &v853Font_OTA_32, 0);
    lv_obj_set_style_text_color(sub, TH_TEXT_SECONDARY, 0);
    lv_label_set_long_mode(sub, LV_LABEL_LONG_WRAP);
    lv_obj_align(sub, LV_ALIGN_CENTER, 0, 5);

    /* 确认按钮 */
    lv_obj_t *yes = lv_btn_create(card);
    lv_obj_set_size(yes, BTN_WIDTH_LG, BTN_HEIGHT_LG);
    lv_obj_set_style_bg_color(yes, TH_SUCCESS, 0);
    lv_obj_set_style_radius(yes, RADIUS_MD, 0);
    lv_obj_align(yes, LV_ALIGN_BOTTOM_LEFT, 2, -2);
    lv_obj_add_event_cb(yes, confirm_yes_cb, LV_EVENT_CLICKED, NULL);
    g_ota_ui.confirm_yes_btn = yes;

    lv_obj_t *ylabel = lv_label_create(yes);
    lv_label_set_text(ylabel, yes_text);
    lv_obj_set_style_text_font(ylabel, &v853Font_OTA_26, 0);
    lv_obj_set_style_text_color(ylabel, TH_TEXT_INVERSE, 0);
    lv_obj_center(ylabel);

    /* 取消按钮 */
    lv_obj_t *no = lv_btn_create(card);
    lv_obj_set_size(no, BTN_WIDTH_LG, BTN_HEIGHT_LG);
    lv_obj_set_style_bg_color(no, TH_ERROR, 0);
    lv_obj_set_style_radius(no, RADIUS_MD, 0);
    lv_obj_align(no, LV_ALIGN_BOTTOM_RIGHT, -2, -2);
    lv_obj_add_event_cb(no, confirm_no_cb, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *nlabel = lv_label_create(no);
    lv_label_set_text(nlabel, no_text);
    lv_obj_set_style_text_font(nlabel, &v853Font_OTA_26, 0);
    lv_obj_set_style_text_color(nlabel, TH_TEXT_INVERSE, 0);
    lv_obj_center(nlabel);

    g_ota_ui.modal_overlay = overlay;
}

/*============================================================================
 * 检查更新按钮回调
 *============================================================================*/

/**
 * @brief 检查更新按钮回调
 */
static void check_btn_event_cb(lv_event_t *e) {
    (void)e;
    
    /* 检查网络连接 */
    if (network_is_available_quick()) {
        printf("[OTA] Network available\n");
    } else {
        printf("[OTA] Network unavailable\n");
        show_timed_modal(get_string_for_language(ota_language_setting, "ota_network_unavailable"),
                        get_string_for_language(ota_language_setting, "ota_check_network"),
                        g_theme->warning, &v853Font_OTA_42, &v853Font_OTA_36);
        return;
    }

    /* 下载远程版本文件 */
    char url[256];
    snprintf(url, sizeof(url), "http://%s%s", OTA_SERVER_HOST, OTA_VERSION_PATH);

    /* 进入版本比较前清理旧的临时文件 */
    remove(OTA_VERSION_TEMP_PATH);
    remove(OTA_FIRMWARE_TEMP_PATH);

    if (http_download(url, OTA_VERSION_TEMP_PATH) != 0) {
        show_timed_modal(get_string_for_language(ota_language_setting, "ota_download_failed"),
                        get_string_for_language(ota_language_setting, "ota_download_version_file_failed"),
                        g_theme->error, &v853Font_OTA_36, &v853Font_OTA_28);
        return;
    }

    /* 解析远程版本号 */
    int rmaj, rmin, rp;
    if (parse_version_triplet(OTA_VERSION_TEMP_PATH, &rmaj, &rmin, &rp) != 0) {
        show_timed_modal(get_string_for_language(ota_language_setting, "ota_download_failed"),
                        get_string_for_language(ota_language_setting, "ota_invalid_version"),
                        g_theme->error, &v853Font_OTA_36, &v853Font_OTA_28);
        return;
    }

    /* 解析本地版本号 */
    int lmaj, lmin, lpv;
    if (parse_version_triplet(LOCAL_VERSION_PATH, &lmaj, &lmin, &lpv) != 0) {
        lmaj = 1; lmin = 0; lpv = 0;
    }

    /* 比较版本号 */
    bool remote_newer = false;
    if (rmaj > lmaj) {
        remote_newer = true;
    } else if (rmaj == lmaj && rmin > lmin) {
        remote_newer = true;
    } else if (rmaj == lmaj && rmin == lmin && rp > lpv) {
        remote_newer = true;
    }

    if (!remote_newer) {
        show_timed_modal(get_string_for_language(ota_language_setting, "ota_no_need_upgrade"),
                        get_string_for_language(ota_language_setting, "ota_up_to_date"),
                        g_theme->success, &v853Font_OTA_36, &v853Font_OTA_28);
        /* 可选：保持环境整洁，删除未使用的 Version-ota */
        remove(OTA_VERSION_TEMP_PATH);
        return;
    }

    /* 显示确认升级对话框 */
    char subtxt[128];
    // snprintf(subtxt, sizeof(subtxt), "New version: V%d.%d.%d\nDo you want to upgrade?", rmaj, rmin, rp);
    // snprintf(subtxt, sizeof(subtxt), "A new version has been detected.\nDo you want to upgrade?");
    show_confirm_modal(get_string_for_language(ota_language_setting, "ota_update_available"),
                       get_string_for_language(ota_language_setting, "ota_confirm_upgrade"),
                       get_string_for_language(ota_language_setting, "ota_yes"),
                       get_string_for_language(ota_language_setting, "ota_no"));
}

/*============================================================================
 * 公共函数实现
 *============================================================================*/

/**
 * @brief 初始化OTA升级页面
 * @param parent 父对象
 */
void OTA_init(lv_obj_t *parent) {
    printf("[OTA] Initializing OTA page...\n");

    g_ota_ui.generation++;
    g_ota_ui.stop_requested = 0;

    
    /* 如果已经初始化过，先销毁旧的资源 */
    if (g_ota_ui.is_initialized) {
        printf("[OTA] Already initialized, cleaning up first...\n");
        OTA_deinit();
    }
    
    /* 读取版本信息 */
    char version_text[64] = {0};
    char date_text[64] = {0};
    read_version_info(version_text, sizeof(version_text), date_text, sizeof(date_text));

    /* 创建主容器 */
    g_ota_ui.container = lv_obj_create(parent);
    lv_obj_set_size(g_ota_ui.container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(g_ota_ui.container, PADDING_XL, 0);
    lv_obj_set_style_bg_color(g_ota_ui.container, TH_BG_MAIN, 0);
    lv_obj_set_style_bg_opa(g_ota_ui.container, LV_OPA_COVER, 0);
    lv_obj_clear_flag(g_ota_ui.container, LV_OBJ_FLAG_SCROLLABLE);

    /* 标题 */
    lv_obj_t *title = lv_label_create(g_ota_ui.container);
    lv_label_set_text(title, get_string_for_language(ota_language_setting, "ota_title"));
    lv_obj_set_style_text_color(title, TH_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(title, &v853Font_OTA_42, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    /* 当前版本 */
    lv_obj_t *ver = lv_label_create(g_ota_ui.container);
    char ver_full[128];
    snprintf(ver_full, sizeof(ver_full), get_string_for_language(ota_language_setting, "ota_version_info"),
             version_text[0] ? version_text : "V1.0.0");
    lv_label_set_text(ver, ver_full);
    lv_obj_set_style_text_color(ver, TH_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(ver, &v853Font_OTA_28, 0);
    lv_obj_align(ver, LV_ALIGN_TOP_LEFT, 50, 120);

    /* 注意事项 */
    lv_obj_t *note = lv_label_create(g_ota_ui.container);
    lv_label_set_long_mode(note, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(note, 1100);
    lv_label_set_text(note, get_string_for_language(ota_language_setting, "ota_note"));
    lv_obj_set_style_text_color(note, TH_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(note, &v853Font_OTA_36, 0);  // 改到30
    lv_obj_align(note, LV_ALIGN_TOP_MID, 50, 200);

    /* 检查更新按钮 */
    lv_obj_t *check_btn = lv_btn_create(g_ota_ui.container);
    lv_obj_set_size(check_btn, BTN_WIDTH_XL, BTN_HEIGHT_XL);
    lv_obj_set_style_bg_color(check_btn, TH_PRIMARY_DARK, 0);
    lv_obj_set_style_radius(check_btn, RADIUS_LG, 0);
    lv_obj_align(check_btn, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_add_event_cb(check_btn, check_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *check_label = lv_label_create(check_btn);
    g_ota_ui.check_label = check_label;
    char check_btn_txt[128];
    snprintf(check_btn_txt, sizeof(check_btn_txt), LV_SYMBOL_REFRESH "  %s",
             get_string_for_language(ota_language_setting, "ota_check_updates"));
    lv_label_set_text(check_label, check_btn_txt);
    lv_obj_set_style_text_font(check_label, &v853Font_OTA_32, 0);
    lv_obj_set_style_text_color(check_label, ota_check_btn_text_color(), 0);
    lv_obj_center(check_label);

    /* 上次检查时间 */
    lv_obj_t *time = lv_label_create(g_ota_ui.container);
    char time_txt[128];
    snprintf(time_txt, sizeof(time_txt), get_string_for_language(ota_language_setting, "ota_current_version"),
             date_text[0] ? date_text : "Unknown");
    lv_label_set_text(time, time_txt);
    lv_obj_set_style_text_font(time, &v853Font_OTA_20, 0);
    lv_obj_set_style_text_color(time, TH_TEXT_HINT, 0);
    lv_obj_align(time, LV_ALIGN_BOTTOM_MID, 0, -140);
    
    /* 初始化状态变量 */
    g_ota_ui.modal_overlay = NULL;
    g_ota_ui.confirm_yes_btn = NULL;
    g_ota_ui.download_bar = NULL;
    g_ota_ui.download_label = NULL;
    g_ota_ui.download_timer = NULL;
    g_ota_ui.upgrade_timer = NULL;
    g_ota_ui.download_total = -1;
    g_ota_ui.download_done = 0;
    g_ota_ui.download_finished = 0;
    g_ota_ui.download_result = 0;
    
    /* 创建网络提示定时器（初始暂停） */
    g_ota_ui.unavailble_net_timer = lv_timer_create(overlay_del_cb, 3000, (void *)(uintptr_t)ota_gen());
    if (g_ota_ui.unavailble_net_timer) {
        lv_timer_pause(g_ota_ui.unavailble_net_timer);
    }

    g_ota_ui.is_initialized = true;
    printf("[OTA] OTA page initialized\n");
}

/**
 * @brief 销毁OTA升级页面
 */
void OTA_deinit(void) {
    printf("[OTA] Deinitializing OTA page...\n");
    
    if (!g_ota_ui.is_initialized) {
        printf("[OTA] Not initialized, skip deinit\n");
        return;
    }

    /*
     * 退出清理必须优先阻止所有异步回调（timer/thread）继续触碰 UI。
     * - stop_requested: 让 ota_alive() 立刻失败
     * - generation++: 让所有持有旧 gen 的回调立刻失效（双保险）
     */
    g_ota_ui.stop_requested = 1;
    g_ota_ui.generation++;

    /* 清理定时器 */
    if (g_ota_ui.download_timer) {
        lv_timer_del(g_ota_ui.download_timer);
        g_ota_ui.download_timer = NULL;
    }

    if (g_ota_ui.upgrade_timer) {
        lv_timer_del(g_ota_ui.upgrade_timer);
        g_ota_ui.upgrade_timer = NULL;
    }
    
    if (g_ota_ui.unavailble_net_timer) {
        lv_timer_del(g_ota_ui.unavailble_net_timer);
        g_ota_ui.unavailble_net_timer = NULL;
    }

    /* 等待下载线程退出（短等待，避免卡死） */
    for (int i = 0; i < 50; i++) {
        if (!g_ota_ui.download_thread_running) break;
        usleep(10000);
    }
    
    /*
     * 清理模态窗口（注意：modal_overlay 现在可能挂在 lv_layer_top() 上，
     * 不会随 OTA container/screen 递归删除，因此这里必须兜底删除）
     */
    if (g_ota_ui.modal_overlay && lv_obj_is_valid(g_ota_ui.modal_overlay)) {
        lv_obj_del(g_ota_ui.modal_overlay);
    }
    g_ota_ui.modal_overlay = NULL;
    g_ota_ui.confirm_yes_btn = NULL;
    
    /* 清理主容器 */
    if (g_ota_ui.container) {
        lv_obj_del(g_ota_ui.container);
        g_ota_ui.container = NULL;
    }
    
    /* 重置其他指针 */
    g_ota_ui.check_label = NULL;
    g_ota_ui.download_bar = NULL;
    g_ota_ui.download_label = NULL;
    g_ota_ui.download_total = -1;
    g_ota_ui.download_done = 0;
    g_ota_ui.download_finished = 0;
    g_ota_ui.download_result = 0;
    g_ota_ui.is_initialized = false;

    /* 允许下次进入重新启动 */
    g_ota_ui.stop_requested = 0;
    g_ota_ui.download_thread_running = 0;
    
    printf("[OTA] OTA page deinitialized\n");
}

/**
 * @brief 刷新OTA页面主题
 */
void OTA_refresh_theme(void) {
    if (!g_ota_ui.is_initialized) return;
    
    printf("[OTA] Refreshing theme...\n");
    
    /* 刷新主容器 */
    if (g_ota_ui.container) {
        lv_obj_set_style_bg_color(g_ota_ui.container, TH_BG_MAIN, 0);
    }

    if (g_ota_ui.check_label && lv_obj_is_valid(g_ota_ui.check_label)) {
        lv_obj_set_style_text_color(g_ota_ui.check_label, ota_check_btn_text_color(), 0);
    }
    
    printf("[OTA] Theme refresh completed\n");
}
