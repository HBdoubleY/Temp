#ifndef __AWAIISP_COMMON_H__
#define __AWAIISP_COMMON_H__

#include <awaiisp.h>

#ifdef __cplusplus
       extern "C" {
#endif

typedef struct awaiisp_common_config_param {
    awaiisp_config_param config;
    char *isp_cfg_bin_path;
} awaiisp_common_config_param;

typedef struct awaiisp_common_switch_param {
    awaiisp_switch_param config;
    char *isp_cfg_bin_path;
} awaiisp_common_switch_param;

int awaiisp_common_enable(int isp, awaiisp_common_config_param *param);
int awaiisp_common_disable(int isp);
int awaiisp_common_switch_mode(int isp, awaiisp_common_switch_param *param);

#ifdef __cplusplus
       }
#endif

#endif
