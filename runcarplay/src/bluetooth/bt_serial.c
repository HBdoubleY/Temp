#include "bt_serial.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>
#include <errno.h>
#include "../tire/tire_manager.h"

static int g_fd = -1;                     
static pthread_t g_thread;                 
static volatile int g_running = 0;         
static bt_data_callback g_callback = NULL; 
static void *g_user_arg = NULL;           
static pthread_mutex_t g_write_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_state_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int speed;
    speed_t code;
} baudrate_map_t;

static const baudrate_map_t baudrate_table[] = {
    {9600,   B9600},
    {19200,  B19200},
    {38400,  B38400},
    {57600,  B57600},
    {115200, B115200},
    {230400, B230400},
    {460800, B460800},
    {500000, B500000},
    {576000, B576000},
    {921600, B921600},
    {1000000,B1000000},
    {0,      0}
};

static speed_t int_to_baudrate(int baudrate) {
    for (const baudrate_map_t *p = baudrate_table; p->speed != 0; p++) {
        if (p->speed == baudrate)
            return p->code;
    }
    return B0;
}

static int configure_serial(int fd, int baudrate) {
    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        return -1;
    }

    speed_t speed = int_to_baudrate(baudrate);
    if (speed == B0) {
        fprintf(stderr, "Unsupported baudrate: %d\n", baudrate);
        return -1;
    }

    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);

    tty.c_cflag &= ~PARENB;    
    tty.c_cflag &= ~CSTOPB;    
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;       
    tty.c_cflag &= ~CRTSCTS;      
    tty.c_cflag |= CREAD | CLOCAL;

    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);        
    tty.c_iflag &= ~(INLCR | ICRNL | IGNCR);       
    tty.c_oflag &= ~OPOST;                         

    tty.c_cc[VTIME] = 0;
    tty.c_cc[VMIN] = 1;

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        return -1;
    }
    return 0;
}

static int set_serial_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        perror("fcntl F_GETFL");
        return -1;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) != 0) {
        perror("fcntl F_SETFL O_NONBLOCK");
        return -1;
    }
    return 0;
}

static ssize_t serial_write_retry(int fd, const char *data, size_t len) {
    size_t off = 0;
    int stall = 0;
    const int max_stall = 200;

    while (off < len && stall < max_stall) {
        ssize_t w = write(fd, data + off, len - off);
        if (w < 0) {
            if (errno == EINTR)
                continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                stall++;
                usleep(500);
                continue;
            }
            return -1;
        }
        if (w == 0) {
            stall++;
            usleep(500);
            continue;
        }
        off += (size_t)w;
        stall = 0;
    }
    return (off == len) ? (ssize_t)len : -1;
}

static void *reader_thread(void *arg) {
    char buf[256];
    char line[512];   
    int pos = 0;

    (void)arg;
    while (1) {
        int running = 0;
        int fd = -1;
        bt_data_callback cb = NULL;
        void *user_arg = NULL;

        pthread_mutex_lock(&g_state_mutex);
        running = g_running;
        fd = g_fd;
        cb = g_callback;
        user_arg = g_user_arg;
        pthread_mutex_unlock(&g_state_mutex);

        if (!running || fd < 0) {
            break;
        }

        ssize_t n = read(fd, buf, sizeof(buf));
        if (n < 0) {
            if (errno == EINTR)
                continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(5000);
                continue;
            }
            perror("read");
            continue;
        }
        if (n == 0)
            continue;

        for (ssize_t i = 0; i < n; i++) {
            char ch = buf[i];
            if (ch == '\n') {

                if (pos > 0) {
                    line[pos] = '\0';
                    if (pos > 0 && line[pos-1] == '\r')
                        line[pos-1] = '\0';
                    if (cb) {
                        cb(line, user_arg);
                    }
                }
                pos = 0;
            } else {
                if (pos < (int)sizeof(line) - 1) {
                    line[pos++] = ch;
                } else {
                    pos = 0;
                }
            }
        }
    }
    return NULL;
}

int bt_serial_init(const char *dev, int baudrate, bt_data_callback cb, void *user_arg) {
    if (!dev || !cb) return -1;

    pthread_mutex_lock(&g_state_mutex);
    if (g_running || g_fd >= 0) {
        pthread_mutex_unlock(&g_state_mutex);
        fprintf(stderr, "bt_serial already initialized\n");
        return -1;
    }
    pthread_mutex_unlock(&g_state_mutex);

    int fd = open(dev, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    if (configure_serial(fd, baudrate) < 0) {
        close(fd);
        return -1;
    }
    if (set_serial_nonblocking(fd) < 0) {
        close(fd);
        return -1;
    }

    pthread_mutex_lock(&g_state_mutex);
    g_fd = fd;
    g_callback = cb;
    g_user_arg = user_arg;
    g_running = 1;
    pthread_mutex_unlock(&g_state_mutex);

    if (pthread_create(&g_thread, NULL, reader_thread, NULL) != 0) {
        perror("pthread_create");
        close(fd);
        pthread_mutex_lock(&g_state_mutex);
        g_fd = -1;
        g_running = 0;
        g_callback = NULL;
        g_user_arg = NULL;
        pthread_mutex_unlock(&g_state_mutex);
        return -1;
    }

    bt_serial_send("LS1");
    printf("ble start\n\n\n");

    return 0;
}

int bt_serial_send(const char *cmd) {
    int fd = -1;
    if (!cmd) return -1;

    pthread_mutex_lock(&g_state_mutex);
    fd = g_fd;
    pthread_mutex_unlock(&g_state_mutex);
    if (fd < 0) return -1;

    char buf[512];
    int len = snprintf(buf, sizeof(buf), "AT#%s\r\n", cmd);
    if (len >= (int)sizeof(buf)) {
        fprintf(stderr, "Command too long\n");
        return -1;
    }

    pthread_mutex_lock(&g_write_mutex);
    ssize_t ret = serial_write_retry(fd, buf, (size_t)len);
    pthread_mutex_unlock(&g_write_mutex);
    return (int)ret;
}

int bt_serial_send_raw(const char *data, int len) {
    int fd = -1;
    if (!data || len <= 0) return -1;
    pthread_mutex_lock(&g_state_mutex);
    fd = g_fd;
    pthread_mutex_unlock(&g_state_mutex);
    if (fd < 0) return -1;

    pthread_mutex_lock(&g_write_mutex);
    ssize_t ret = serial_write_retry(fd, data, (size_t)len);
    pthread_mutex_unlock(&g_write_mutex);
    return (int)ret;
}

void bt_serial_cleanup(void) {
    int was_running = 0;
    int fd = -1;
    pthread_t thread = 0;

    pthread_mutex_lock(&g_state_mutex);
    was_running = g_running;
    fd = g_fd;
    thread = g_thread;
    g_running = 0;
    g_fd = -1;
    g_callback = NULL;
    g_user_arg = NULL;
    pthread_mutex_unlock(&g_state_mutex);

    if (fd >= 0) {
        close(fd);
    }

    if (was_running) {
        pthread_join(thread, NULL);
    }
}

static int hfp_connected = 0;
static char connected_addr[64] = {0};
static char connected_device_name[64] = {0};

void on_bt_data(const char *line, void *arg) {
    if (strncmp(line, "IB", 2) == 0) {

        printf("\n\n\n\n\n\n\nHFP connected = 1\n\n\n\n\n\n\n");
        // strncpy(connected_addr, line+2, 12);
        // connected_addr[12] = '\0';
        pthread_mutex_lock(&g_state_mutex);
        hfp_connected = 1;
        pthread_mutex_unlock(&g_state_mutex);
        
        // printf("HFP connected to %s\n", connected_addr);
    }
    else if (strncmp(line, "IA", 2) == 0) {

        pthread_mutex_lock(&g_state_mutex);
        hfp_connected = 0;
        connected_device_name[0] = '\0';
        pthread_mutex_unlock(&g_state_mutex);
        // connected_addr[0] = '\0';
        printf("HFP disconnected\n");
    }
    // else if (strncmp(line, "JH", 2) == 0 || strncmp(line, "AD", 2) == 0) {
    //     // 当前连接设备地址（部分版本用JH，部分用AD）
    //     strncpy(connected_addr, line+2, 12);
    //     connected_addr[12] = '\0';
    //     printf("Current connected device: %s\n", connected_addr);
    // }
    else if (strncmp(line, "SA", 2) == 0 ) {

        size_t max_len = sizeof(connected_device_name) - 1;
        size_t src_len = strlen(line + 2);
        size_t copy_len = (src_len < max_len) ? src_len : max_len;

        pthread_mutex_lock(&g_state_mutex);
        memset(connected_device_name, 0, sizeof(connected_device_name));
        memcpy(connected_device_name, line + 2, copy_len);
        connected_device_name[copy_len] = '\0';
        pthread_mutex_unlock(&g_state_mutex);
        printf("\n\n\nCurrent connected device: %s\n\n\n\n", connected_device_name);
    }
    // else if (strncmp(line, "LP", 2) == 0 && strncmp(line, "00EEBC", 6) == 0) {
    else if (strncmp(line, "LP", 2) == 0) {

        tire_on_ble_line(line);
    }

}

int get_BT_connect_state(void) {
    int connected = 0;
    pthread_mutex_lock(&g_state_mutex);
    connected = hfp_connected;
    pthread_mutex_unlock(&g_state_mutex);
    return connected;
}

const char *get_BT_connected_name(void) {
    static __thread char name_copy[64];
    pthread_mutex_lock(&g_state_mutex);
    memcpy(name_copy, connected_device_name, sizeof(name_copy));
    name_copy[sizeof(name_copy) - 1] = '\0';
    pthread_mutex_unlock(&g_state_mutex);
    return name_copy;
}