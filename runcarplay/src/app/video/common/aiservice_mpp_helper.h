#ifndef __AISERVICE_MPP_HELPER_H__
#define __AISERVICE_MPP_HELPER_H__

#include <stdio.h>
#include <plat_type.h>
#include <media/mm_comm_region.h>
#include <media/mpi_region.h>
#include "awnn_det.h"

#define _CHECK_RET( ret )  do { \
        if( SUCCESS != ret ) { \
            aloge("Error no %d.\n", ret); \
            return ret; \
        } \
    } while(0)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

ERRORTYPE mpp_helper_init(void);
ERRORTYPE mpp_helper_uninit(void);
ERRORTYPE mpp_helper_start_npu(int isp, int vipp, int vipp_buf_num, int width, int height, int fps, int format);
ERRORTYPE mpp_helper_stop_npu(int isp, int vipp);
void paint_object_detect_region_body(Awnn_Result_t *res, int ch_idx);
void paint_object_detect_region_face(Awnn_Result_t *res, int ch_idx);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __AISERVICE_MPP_HELPER_H__ */
