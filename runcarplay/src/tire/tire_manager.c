#include "tire_manager.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

static const char *TIRE_BRAND_HEX = "444A54504D53";
#define TIRE_PAIR_DIR "/opt/private"
#define TIRE_PAIR_FRONT_FILE "/opt/private/tire_pair_front.txt"
#define TIRE_PAIR_REAR_FILE  "/opt/private/tire_pair_rear.txt"

#ifndef TIRE_BLE_TIMEOUT_ENABLE
#define TIRE_BLE_TIMEOUT_ENABLE 0
#endif
#define TIRE_BLE_TIMEOUT_MS 30000u


static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

static int g_pair_front = 0;
static int g_pair_rear = 0;
static char g_front_suffix[7] = {0}; // 6 hex chars + '\0'
static char g_rear_suffix[7] = {0};

static uint64_t g_front_last_rx_ms = 0;
static uint64_t g_rear_last_rx_ms = 0;

static int g_front_pressure_kpa = 0;
static int g_front_temp_c = 0;
static int g_rear_pressure_kpa = 0;
static int g_rear_temp_c = 0;

static uint64_t now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000u + (uint64_t)ts.tv_nsec / 1000000u;
}

static int hex_char_to_val(char c) {
    if (c >= '0' && c <= '9') return (c - '0');
    c = (char)toupper((unsigned char)c);
    if (c >= 'A' && c <= 'F') return (c - 'A' + 10);
    return -1;
}

static int hex_pair_to_byte(char high, char low) {
    int h = hex_char_to_val(high);
    int l = hex_char_to_val(low);
    if (h < 0 || l < 0) return -1;
    return (h << 4) | l;
}

static int normalize_suffix6(char out[7], const char *suffix6_hex) {
    if (!out || !suffix6_hex) return 0;
    if (strlen(suffix6_hex) != 6) return 0;
    for (int i = 0; i < 6; i++) {
        int v = hex_char_to_val(suffix6_hex[i]);
        if (v < 0) return 0;
        out[i] = (char)toupper((unsigned char)suffix6_hex[i]);
    }
    out[6] = '\0';
    return 1;
}

static int read_suffix_from_file(const char *file, char out[7]) {
    if (!file || !out) return 0;
    FILE *fp = fopen(file, "r");
    if (!fp) return 0;

    char buf[64] = {0};
    if (!fgets(buf, sizeof(buf), fp)) {
        fclose(fp);
        return 0;
    }
    fclose(fp);

    size_t len = strlen(buf);
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r' || buf[len - 1] == ' ' || buf[len - 1] == '\t')) {
        buf[len - 1] = '\0';
        len--;
    }

    char tmp[7] = {0};
    if (!normalize_suffix6(tmp, buf)) return 0;
    memcpy(out, tmp, 7);
    return 1;
}

static void write_suffix_to_file(const char *file, const char suffix6[7]) {
    if (!file || !suffix6) return;

    struct stat st;
    if (stat(TIRE_PAIR_DIR, &st) != 0) {
        mkdir(TIRE_PAIR_DIR, 0755);
    }

    FILE *fp = fopen(file, "w");
    if (!fp) return;
    fprintf(fp, "%s\n", suffix6);
    fclose(fp);
}

void tire_manager_init(void) {
    char front_buf[7] = {0};
    char rear_buf[7] = {0};
    int front_ok = read_suffix_from_file(TIRE_PAIR_FRONT_FILE, front_buf);
    int rear_ok = read_suffix_from_file(TIRE_PAIR_REAR_FILE, rear_buf);

    pthread_mutex_lock(&g_lock);

    if (front_ok) {
        memcpy(g_front_suffix, front_buf, 7);
        g_pair_front = 1;
    } else {
        g_front_suffix[0] = '\0';
        g_pair_front = 0;
    }

    if (rear_ok) {
        memcpy(g_rear_suffix, rear_buf, 7);
        g_pair_rear = 1;
    } else {
        g_rear_suffix[0] = '\0';
        g_pair_rear = 0;
    }

    g_front_last_rx_ms = 0;
    g_rear_last_rx_ms = 0;
    g_front_pressure_kpa = 0;
    g_front_temp_c = 0;
    g_rear_pressure_kpa = 0;
    g_rear_temp_c = 0;

    pthread_mutex_unlock(&g_lock);
}

void tire_pair_set_front_suffix(const char *suffix6_hex) {
    char tmp[7];
    if (!normalize_suffix6(tmp, suffix6_hex)) return;

    pthread_mutex_lock(&g_lock);
    memcpy(g_front_suffix, tmp, 7);
    g_pair_front = 1;
    g_front_last_rx_ms = 0;
    pthread_mutex_unlock(&g_lock);

    // write_suffix_to_file(TIRE_PAIR_FRONT_FILE, g_front_suffix);
    write_suffix_to_file(TIRE_PAIR_FRONT_FILE, tmp);
}

void tire_pair_set_rear_suffix(const char *suffix6_hex) {
    char tmp[7];
    if (!normalize_suffix6(tmp, suffix6_hex)) return;

    pthread_mutex_lock(&g_lock);
    memcpy(g_rear_suffix, tmp, 7);
    g_pair_rear = 1;
    g_rear_last_rx_ms = 0;
    pthread_mutex_unlock(&g_lock);

    // write_suffix_to_file(TIRE_PAIR_REAR_FILE, g_rear_suffix);
    write_suffix_to_file(TIRE_PAIR_REAR_FILE, tmp);
}

void tire_pair_clear_all(void) {
    pthread_mutex_lock(&g_lock);
    g_pair_front = 0;
    g_pair_rear = 0;
    g_front_suffix[0] = '\0';
    g_rear_suffix[0] = '\0';
    g_front_last_rx_ms = 0;
    g_rear_last_rx_ms = 0;
    g_front_pressure_kpa = 0;
    g_front_temp_c = 0;
    g_rear_pressure_kpa = 0;
    g_rear_temp_c = 0;
    pthread_mutex_unlock(&g_lock);

    unlink(TIRE_PAIR_FRONT_FILE);
    unlink(TIRE_PAIR_REAR_FILE);
}

static void extract_suffix6_from_mac_hex(const char mac_hex[13], char out_suffix6[7]) {
    // mac_hex: 12 hex chars + '\0'
    // suffix6: last 3 bytes => last 6 hex chars
    memcpy(out_suffix6, mac_hex + 6, 6);
    out_suffix6[6] = '\0';
    for (int i = 0; i < 6; i++) {
        out_suffix6[i] = (char)toupper((unsigned char)out_suffix6[i]);
    }
}

static void on_wheel_update_locked(int wheel_is_front, int pressure_kpa, int temp_c, uint64_t ts_ms) {
    if (wheel_is_front) {
        g_front_pressure_kpa = pressure_kpa;
        g_front_temp_c = temp_c;
        g_front_last_rx_ms = ts_ms;
    } else {
        g_rear_pressure_kpa = pressure_kpa;
        g_rear_temp_c = temp_c;
        g_rear_last_rx_ms = ts_ms;
    }
}

void tire_on_ble_line(const char *line) {
    if (!line) return;
    if (strncmp(line, "LP", 2) != 0) return;

    if (strlen(line) < 2 + 12) return;
    char mac_hex[13] = {0};
    memcpy(mac_hex, line + 2, 12);
    mac_hex[12] = '\0';

    char suffix6[7] = {0};
    extract_suffix6_from_mac_hex(mac_hex, suffix6);

    const char *brand_pos = strstr(line, TIRE_BRAND_HEX);
    if (!brand_pos) return;

    const char *p = brand_pos + strlen(TIRE_BRAND_HEX);
    if (strlen(p) < 4 + 16) return;
    p += 4;

    char data_hex[17] = {0};
    memcpy(data_hex, p, 16);
    data_hex[16] = '\0';

    int b[8];
    for (int i = 0; i < 8; i++) {
        int v = hex_pair_to_byte(data_hex[i * 2], data_hex[i * 2 + 1]);
        if (v < 0) return;
        b[i] = v;
    }

    printf("recive tire = %s\n", line);

    int temp_c = (int)(int8_t)b[3];
    int pressure_kpa = ((b[4] & 0xFF) << 8) | (b[5] & 0xFF);

    uint64_t ts = now_ms();

    pthread_mutex_lock(&g_lock);
    int is_front = (g_pair_front && strncmp(suffix6, g_front_suffix, 6) == 0);
    int is_rear = (g_pair_rear && strncmp(suffix6, g_rear_suffix, 6) == 0);

    if (is_front) {
        on_wheel_update_locked(1, pressure_kpa, temp_c, ts);
    }
    if (is_rear) {
        on_wheel_update_locked(0, pressure_kpa, temp_c, ts);
    }
    pthread_mutex_unlock(&g_lock);
}

static int fresh_ok(uint64_t last_rx_ms, uint64_t ts_ms) {
    if (last_rx_ms == 0) return 0;
    (void)ts_ms;
#if TIRE_BLE_TIMEOUT_ENABLE
    return (now_ms() - last_rx_ms) <= (uint64_t)TIRE_BLE_TIMEOUT_MS;
#else
    return 1;
#endif
}

bool tire_front_get_kpa_temp(int *pressure_kpa_out, int *temp_c_out) {
    if (!pressure_kpa_out || !temp_c_out) return false;

    uint64_t ts = now_ms();
    pthread_mutex_lock(&g_lock);
    int paired = g_pair_front;
    int fresh = paired && fresh_ok(g_front_last_rx_ms, ts);
    int p = g_front_pressure_kpa;
    int t = g_front_temp_c;
    pthread_mutex_unlock(&g_lock);

    if (!fresh) return false;
    *pressure_kpa_out = p;
    *temp_c_out = t;
    return true;
}

bool tire_rear_get_kpa_temp(int *pressure_kpa_out, int *temp_c_out) {
    if (!pressure_kpa_out || !temp_c_out) return false;

    uint64_t ts = now_ms();
    pthread_mutex_lock(&g_lock);
    int paired = g_pair_rear;
    int fresh = paired && fresh_ok(g_rear_last_rx_ms, ts);
    int p = g_rear_pressure_kpa;
    int t = g_rear_temp_c;
    pthread_mutex_unlock(&g_lock);

    if (!fresh) return false;
    *pressure_kpa_out = p;
    *temp_c_out = t;
    return true;
}

bool tire_front_is_paired(void) {
    pthread_mutex_lock(&g_lock);
    int paired = g_pair_front;
    pthread_mutex_unlock(&g_lock);
    return paired != 0;
}

bool tire_rear_is_paired(void) {
    pthread_mutex_lock(&g_lock);
    int paired = g_pair_rear;
    pthread_mutex_unlock(&g_lock);
    return paired != 0;
}

bool tire_rear_get_suffix6(char out_suffix6[7]) {
    if (!out_suffix6) return false;
    pthread_mutex_lock(&g_lock);
    if (g_pair_rear == 0) {
        pthread_mutex_unlock(&g_lock);
        return false;
    }
    memcpy(out_suffix6, g_rear_suffix, 7);
    pthread_mutex_unlock(&g_lock);
    return true;
}

bool tire_front_get_suffix6(char out_suffix6[7]) {
    if (!out_suffix6) return false;
    pthread_mutex_lock(&g_lock);
    if (g_pair_front == 0) {
        pthread_mutex_unlock(&g_lock);
        return false;
    }
    memcpy(out_suffix6, g_front_suffix, 7);
    pthread_mutex_unlock(&g_lock);
    return true;
}

