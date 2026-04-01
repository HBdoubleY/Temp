#ifndef TIRE_MANAGER_H
#define TIRE_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void tire_manager_init(void);

void tire_on_ble_line(const char *line);

bool tire_front_get_kpa_temp(int *pressure_kpa_out, int *temp_c_out);
bool tire_rear_get_kpa_temp(int *pressure_kpa_out, int *temp_c_out);

bool tire_front_is_paired(void);
bool tire_rear_is_paired(void);

bool tire_rear_get_suffix6(char out_suffix6[7]);

bool tire_front_get_suffix6(char out_suffix6[7]);

void tire_pair_set_front_suffix(const char *suffix6_hex);
void tire_pair_set_rear_suffix(const char *suffix6_hex);

void tire_pair_clear_all(void);

#ifdef __cplusplus
}
#endif

#endif /* TIRE_MANAGER_H */

