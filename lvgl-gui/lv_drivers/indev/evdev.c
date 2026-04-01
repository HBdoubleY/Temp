/**
 * @file evdev.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "evdev.h"
#if USE_EVDEV != 0 || USE_BSD_EVDEV

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>  /* 添加errno头文件 */
#if USE_BSD_EVDEV
#include <dev/evdev/input.h>
#else
#include <linux/input.h>
#endif

#if USE_XKB
#include "xkb.h"
#endif /* USE_XKB */

/*********************
 *      DEFINES
 *********************/
#define EVDEV_NOISE_FILTER_TIME_MS 100  /* 噪声过滤时间，50ms内无事件则认为释放 */

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
int map(int x, int in_min, int in_max, int out_min, int out_max);
static long long get_time_ms(void);

/**********************
 *  STATIC VARIABLES
 **********************/
int evdev_fd = -1;
int evdev_root_x;
int evdev_root_y;
int evdev_button;

int evdev_key_val;

static long long last_event_time = 0;  /* 最后一次事件的时间戳 */
static int last_valid_x = 0;           /* 最后一次有效的X坐标 */
static int last_valid_y = 0;           /* 最后一次有效的Y坐标 */

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the evdev interface
 */
void evdev_init(void)
{
    if (!evdev_set_file(EVDEV_NAME)) {
        return;
    }

#if USE_XKB
    xkb_init();
#endif
}

/**
 * reconfigure the device file for evdev
 * @param dev_name set the evdev device filename
 * @return true: the device file set complete
 *         false: the device file doesn't exist current system
 */
bool evdev_set_file(char* dev_name)
{ 
    if(evdev_fd != -1) {
        close(evdev_fd);
        evdev_fd = -1;
    }
    
#if USE_BSD_EVDEV
    evdev_fd = open(dev_name, O_RDWR | O_NOCTTY);
#else
    /* 先尝试触摸屏专用节点，失败则使用默认节点 */
    if (access("/dev/input/touchscreen", F_OK) == 0) {
        evdev_fd = open("/dev/input/touchscreen", O_RDWR | O_NOCTTY | O_NDELAY);
    } else {
        evdev_fd = open(dev_name, O_RDWR | O_NOCTTY | O_NDELAY);
    }
#endif

    if(evdev_fd == -1) {
        perror("unable to open evdev interface:");
        return false;
    }

#if USE_BSD_EVDEV
    fcntl(evdev_fd, F_SETFL, O_NONBLOCK);
#else
    /* 设置为非阻塞模式 */
    int flags = fcntl(evdev_fd, F_GETFL, 0);
    fcntl(evdev_fd, F_SETFL, flags | O_NONBLOCK);
#endif

    /* 初始化全局变量 */
    evdev_root_x = 0;
    evdev_root_y = 0;
    evdev_key_val = 0;
    evdev_button = LV_INDEV_STATE_REL;
    last_event_time = get_time_ms();
    last_valid_x = 0;
    last_valid_y = 0;

    return true;
}

/**
 * Get the current position and state of the evdev
 * @param data store the evdev data here
 */
void evdev_read(lv_indev_drv_t * drv, lv_indev_data_t * data)
{
    struct input_event in;
    int read_result;
    long long now = get_time_ms();
    
    /* ★★★ 关键修改1：默认设置为释放状态，避免状态卡死 ★★★ */
    lv_indev_state_t current_state = LV_INDEV_STATE_REL;
    int current_x = evdev_root_x;
    int current_y = evdev_root_y;
    
    /* 读取所有待处理的事件（非阻塞模式） */
    while((read_result = read(evdev_fd, &in, sizeof(struct input_event))) > 0) {
        /* 更新最后事件时间戳 */
        last_event_time = now;
        
        if(in.type == EV_REL) {
            /* 相对坐标处理（鼠标、触摸板） */
            if(in.code == REL_X) {
                #if EVDEV_SWAP_AXES
                    evdev_root_y += in.value;
                #else
                    evdev_root_x += in.value;
                #endif
            } else if(in.code == REL_Y) {
                #if EVDEV_SWAP_AXES
                    evdev_root_x += in.value;
                #else
                    evdev_root_y += in.value;
                #endif
            }
            /* 只要有相对移动事件，认为是在按压状态 */
            if(in.value != 0) {
                evdev_button = LV_INDEV_STATE_PR;
            }
        } else if(in.type == EV_ABS) {
            /* 绝对坐标处理（触摸屏） */
            if(in.code == ABS_X) {
                #if EVDEV_SWAP_AXES
                    evdev_root_y = in.value;
                #else
                    evdev_root_x = in.value;
                #endif
            } else if(in.code == ABS_Y) {
                #if EVDEV_SWAP_AXES
                    evdev_root_x = in.value;
                #else
                    evdev_root_y = in.value;
                #endif
            } else if(in.code == ABS_MT_POSITION_X) {
                #if EVDEV_SWAP_AXES
                    evdev_root_y = in.value;
                #else
                    evdev_root_x = in.value;
                #endif
            } else if(in.code == ABS_MT_POSITION_Y) {
                #if EVDEV_SWAP_AXES
                    evdev_root_x = in.value;
                #else
                    evdev_root_y = in.value;
                #endif
            } else if(in.code == ABS_MT_TRACKING_ID) {
                /* ★★★ 关键修改2：MT协议跟踪ID处理 ★★★ */
                if(in.value == -1) {
                    evdev_button = LV_INDEV_STATE_REL;
                } else if(in.value >= 0) {
                    evdev_button = LV_INDEV_STATE_PR;
                }
            } else if(in.code == ABS_MT_TOUCH_MAJOR) {
                /* 触摸面积，大于0表示有触摸 */
                if(in.value == 0) {
                    evdev_button = LV_INDEV_STATE_REL;
                } else {
                    evdev_button = LV_INDEV_STATE_PR;
                }
            } else if(in.code == ABS_PRESSURE) {
                /* 压力值处理 */
                if(in.value == 0) {
                    evdev_button = LV_INDEV_STATE_REL;
                } else if(in.value > 0) {
                    evdev_button = LV_INDEV_STATE_PR;
                }
            }
        } else if(in.type == EV_KEY) {
            /* 按键事件处理 */
            if(in.code == BTN_MOUSE || in.code == BTN_TOUCH || in.code == BTN_TOOL_FINGER) {
                if(in.value == 0) {
                    evdev_button = LV_INDEV_STATE_REL;
                } else if(in.value == 1) {
                    evdev_button = LV_INDEV_STATE_PR;
                }
            } else if(drv->type == LV_INDEV_TYPE_KEYPAD) {
                /* 键盘处理部分保持不变 */
#if USE_XKB
                data->key = xkb_process_key(in.code, in.value != 0);
#else
                switch(in.code) {
                    case KEY_BACKSPACE:
                        data->key = LV_KEY_BACKSPACE;
                        break;
                    case KEY_ENTER:
                        data->key = LV_KEY_ENTER;
                        break;
                    case KEY_PREVIOUS:
                        data->key = LV_KEY_PREV;
                        break;
                    case KEY_NEXT:
                        data->key = LV_KEY_NEXT;
                        break;
                    case KEY_UP:
                        data->key = LV_KEY_UP;
                        break;
                    case KEY_LEFT:
                        data->key = LV_KEY_LEFT;
                        break;
                    case KEY_RIGHT:
                        data->key = LV_KEY_RIGHT;
                        break;
                    case KEY_DOWN:
                        data->key = LV_KEY_DOWN;
                        break;
                    case KEY_TAB:
                        data->key = LV_KEY_NEXT;
                        break;
                    default:
                        data->key = 0;
                        break;
                }
#endif /* USE_XKB */
                if (data->key != 0) {
                    data->state = (in.value) ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
                }
                evdev_key_val = data->key;
                evdev_button = data->state;
                return;
            }
        } else if(in.type == EV_SYN) {
            /* ★★★ 关键修改3：同步事件处理 ★★★ */
            /* 同步事件表示一组触摸数据已经完整发送 */
            /* 这里不需要特殊处理，但可以用于调试 */
        }
        
        /* 更新当前状态为最新的按钮状态 */
        if(evdev_button == LV_INDEV_STATE_PR || evdev_button == LV_INDEV_STATE_REL) {
            current_state = evdev_button;
            if(current_state == LV_INDEV_STATE_PR) {
                /* 只有在按压状态时才更新有效坐标 */
                current_x = evdev_root_x;
                current_y = evdev_root_y;
                last_valid_x = evdev_root_x;
                last_valid_y = evdev_root_y;
            }
        }
    }
    
    /* 检查读取错误 */
    if(read_result < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        /* 真正的读取错误，但不要频繁打印 */
        static int error_count = 0;
        if(error_count++ % 100 == 0) {
            perror("evdev read error");
        }
    }
    
    /* ★★★ 关键修改4：处理键盘类型设备 ★★★ */
    if(drv->type == LV_INDEV_TYPE_KEYPAD) {
        data->key = evdev_key_val;
        data->state = evdev_button;
        return;
    }
    
    /* 非指针设备直接返回 */
    if(drv->type != LV_INDEV_TYPE_POINTER) {
        return;
    }
    
    /* ★★★ 关键修改5：噪声过滤和超时处理 ★★★ */
    /* 如果最近没有新事件，且有足够时间没有事件，强制释放 */
    if (evdev_button == LV_INDEV_STATE_PR && (now - last_event_time) > EVDEV_NOISE_FILTER_TIME_MS) {
        current_state = LV_INDEV_STATE_REL;
        evdev_button = LV_INDEV_STATE_REL;  /* 同步全局状态 */
    }
    
    /* ★★★ 关键修改6：始终使用最新状态，确保释放状态被正确传递 ★★★ */
    data->state = current_state;
    
    /* 坐标处理：按压时使用当前坐标，释放时使用最后一次有效坐标（或0） */
    if(current_state == LV_INDEV_STATE_PR) {
        data->point.x = current_x;
        data->point.y = current_y;
    } else {
        /* 释放状态时，可以使用最后一次有效坐标，或者设为0 */
        /* 这里使用最后一次有效坐标，LVGL内部会处理 */
        data->point.x = last_valid_x;
        data->point.y = last_valid_y;
    }
    
    /* 坐标校准（如果需要） */
#if EVDEV_CALIBRATE
    data->point.x = map(data->point.x, EVDEV_HOR_MIN, EVDEV_HOR_MAX, 0, drv->disp->driver->hor_res);
    data->point.y = map(data->point.y, EVDEV_VER_MIN, EVDEV_VER_MAX, 0, drv->disp->driver->ver_res);
#endif

    /* 坐标边界检查 */
    if(data->point.x < 0) data->point.x = 0;
    if(data->point.y < 0) data->point.y = 0;
    if(data->point.x >= drv->disp->driver->hor_res)
        data->point.x = drv->disp->driver->hor_res - 1;
    if(data->point.y >= drv->disp->driver->ver_res)
        data->point.y = drv->disp->driver->ver_res - 1;

    return;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * 数值映射函数
 */
int map(int x, int in_min, int in_max, int out_min, int out_max)
{
    if(in_max == in_min) return out_min;
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/**
 * 获取当前系统时间（毫秒）
 */
static long long get_time_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

#endif