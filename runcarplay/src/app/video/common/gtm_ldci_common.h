#ifndef __GTM_LDCI_COMMON_H__
#define __GTM_LDCI_COMMON_H__

#ifdef __cplusplus
       extern "C" {
#endif

typedef struct gtm_ldci_common_config_param {
    int ldci_vipp;
    int ldci_width;
    int ldci_height;
    int ldci_fps;
    int vipp_buf_num;
} gtm_ldci_common_config_param;

int gtm_ldci_common_open(int isp, gtm_ldci_common_config_param *param);
int gtm_ldci_common_start(int isp);
int gtm_ldci_common_stop(int isp);
int gtm_ldci_common_close(int isp);

#ifdef __cplusplus
       }
#endif

#endif
