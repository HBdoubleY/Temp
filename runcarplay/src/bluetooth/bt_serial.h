#ifndef BT_SERIAL_H
#define BT_SERIAL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*bt_data_callback)(const char *line, void *user_arg);

int bt_serial_init(const char *dev, int baudrate, bt_data_callback cb, void *user_arg);

int bt_serial_send(const char *cmd);

int bt_serial_send_raw(const char *data, int len);

void bt_serial_cleanup(void);

int get_BT_connect_state(void);

const char *get_BT_connected_name(void);

void on_bt_data(const char *line, void *arg);

#ifdef __cplusplus
}
#endif

#endif // BT_SERIAL_H