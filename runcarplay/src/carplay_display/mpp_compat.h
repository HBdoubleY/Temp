#ifndef CARPLAY_DISPLAY_MPP_COMPAT_H
#define CARPLAY_DISPLAY_MPP_COMPAT_H

#include <mm_common.h>
#include <mm_comm_sys.h>
#include <mpi_sys.h>
#include <mm_comm_vdec.h>
#include <mpi_vdec.h>
#include <mm_comm_vo.h>
#include <mpi_vo.h>
#include <mm_comm_clock.h>
#include <mpi_clock.h>
#include <mm_comm_video.h>
#include <ClockCompPortIndex.h>

/* Alias for carplay_display.c (SDK uses VDEC_CHN_ATTR_S) */
typedef VDEC_CHN_ATTR_S VDEC_ATTR_S;

#endif /* CARPLAY_DISPLAY_MPP_COMPAT_H */
