#include <utils/plat_log.h>
#include <utils/plat_math.h>
#include <string.h>
#include <assert.h>
#include <VIDEO_FRAME_INFO_S.h>
#include <mpi_sys.h>
#include <mpi_isp.h>
#include <mpi_videoformat_conversion.h>
#include <mpi_vi.h>
#include "aiservice_mpp_helper.h"
#include "aiservice_detect.h"

ERRORTYPE mpp_helper_init(void)
{
#if 0
    ERRORTYPE ret;
    MPP_SYS_CONF_S stSysConf;

    memset(&stSysConf, 0, sizeof(MPP_SYS_CONF_S));
    stSysConf.nAlignWidth = 32;
    AW_MPI_SYS_SetConf(&stSysConf);
    ret = AW_MPI_SYS_Init();
    return ret;
#endif
    return SUCCESS;
}

ERRORTYPE mpp_helper_uninit(void)
{
#if 0
    ERRORTYPE ret;
    /* exit mpp systerm */
    ret = AW_MPI_SYS_Exit();
    return ret;
#endif
    return SUCCESS;
}

static ERRORTYPE mpp_helper_create_vipp(ISP_DEV isp, MPP_CHN_S viChn, int vipp_buf_num, int width, int height, int fps, PIXEL_FORMAT_E format)
{
    ERRORTYPE ret;
    VI_ATTR_S viAttr;
    MPPCallbackInfo cbinfo;

    memset(&viAttr, 0x00, sizeof(VI_ATTR_S));
    memset(&cbinfo, 0x00, sizeof(MPPCallbackInfo));

    ret = AW_MPI_VI_CreateVipp(viChn.mDevId);
    _CHECK_RET(ret);

    ret = AW_MPI_VI_GetVippAttr(viChn.mDevId, &viAttr);
    _CHECK_RET(ret);

    memset(&viAttr, 0x00, sizeof(VI_ATTR_S));
    // disable online function, mOnlineShareBufNum is invalid if mOnlineEnable is zero.
    viAttr.mOnlineEnable = 0;
    viAttr.mOnlineShareBufNum = 0;
    viAttr.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    viAttr.memtype = V4L2_MEMORY_MMAP;
    viAttr.format.pixelformat = map_PIXEL_FORMAT_E_to_V4L2_PIX_FMT(format);
    viAttr.format.field = V4L2_FIELD_NONE;
    viAttr.format.colorspace = V4L2_COLORSPACE_JPEG;
    viAttr.format.width = width;
    viAttr.format.height = height;
    viAttr.fps = fps;
    viAttr.use_current_win = viChn.mDevId > 0 ? 1 : 0;
    viAttr.nbufs = vipp_buf_num;
    viAttr.nplanes = 2;
    viAttr.drop_frame_num = 0;
    ret = AW_MPI_VI_SetVippAttr(viChn.mDevId, &viAttr);
    _CHECK_RET(ret);

    AW_MPI_ISP_Run(isp);

    ret = AW_MPI_VI_EnableVipp(viChn.mDevId);
    _CHECK_RET(ret);

    // if enable CONFIG_ENABLE_SENSOR_FLIP_OPTION, flip operations must be after stream_on.
//    ret = AW_MPI_VI_SetVippMirror(viChn.mDevId, 0);
//    _CHECK_RET(ret);
//    ret = AW_MPI_VI_SetVippFlip(viChn.mDevId, 0);//0,1
//    _CHECK_RET(ret);

    ret = AW_MPI_VI_CreateVirChn(viChn.mDevId, viChn.mChnId, NULL);
    _CHECK_RET(ret);

    alogd("vipp%d chn%d create success", viChn.mDevId, viChn.mChnId);

    return ret;
}

static ERRORTYPE mpp_helper_destroy_vipp(MPP_CHN_S viChn)
{
    ERRORTYPE ret;
    ret = AW_MPI_VI_DestroyVirChn(viChn.mDevId, viChn.mChnId);
    _CHECK_RET(ret);

    ret = AW_MPI_VI_DisableVipp(viChn.mDevId);
    _CHECK_RET(ret);

    ret = AW_MPI_VI_DestroyVipp(viChn.mDevId);
    _CHECK_RET(ret);
    return ret;
}

static ERRORTYPE mpp_helper_enable_vipp(MPP_CHN_S viChn)
{
    ERRORTYPE ret;
    ret = AW_MPI_VI_EnableVirChn(viChn.mDevId, viChn.mChnId);
    _CHECK_RET(ret);

    return ret;
}

static ERRORTYPE mpp_helper_disable_vipp(MPP_CHN_S viChn)
{
    ERRORTYPE ret;
    ret = AW_MPI_VI_DisableVirChn(viChn.mDevId, viChn.mChnId);
    _CHECK_RET(ret);

    return ret;
}

ERRORTYPE mpp_helper_start_npu(int isp, int vipp, int vi_buf_num, int width, int height, int fps, int format)
{
    ERRORTYPE ret;
    MPP_CHN_S viChn = {MOD_ID_VIU, vipp, 0};

    ret = mpp_helper_create_vipp(isp, viChn, vi_buf_num, width, height, fps, format);
    _CHECK_RET(ret);

    ret = mpp_helper_enable_vipp(viChn);
    _CHECK_RET(ret);

    return ret;
}

ERRORTYPE mpp_helper_stop_npu(int isp, int vipp)
{
    ERRORTYPE ret;
    MPP_CHN_S viChn = {MOD_ID_VIU, vipp, 0};

    ret = mpp_helper_disable_vipp(viChn);
    _CHECK_RET(ret);

    AW_MPI_ISP_Stop(isp);

    ret = mpp_helper_destroy_vipp(viChn);
    _CHECK_RET(ret);

    return ret;
}

void paint_object_detect_region_body(Awnn_Result_t *res, int ch_idx)
{
    int i = 0;
    RGN_ATTR_S rgnAttr;
    RGN_CHN_ATTR_S rgnChnAttr;

    aialgo_context_t *pctx = get_aicontext();
    assert(pctx != NULL);

    MPP_CHN_S ViChn = {MOD_ID_VIU, pctx->attr.ch_info[ch_idx].draw_orl_vipp, 0};

    alogv("ch_idx=%d, region hdl base=%d, vipp=%d, orl num old=%d cur=%d", ch_idx,
        pctx->region_info[ch_idx].region_hdl_base, pctx->attr.ch_info[ch_idx].draw_orl_vipp,
        pctx->region_info[ch_idx].old_num_of_boxes, res->valid_cnt);

    for (i = 0; i < pctx->region_info[ch_idx].old_num_of_boxes; i ++)
    {
        alogv("ch_idx=%d, detach region hdl=%d, vipp=%d", ch_idx, i + pctx->region_info[ch_idx].region_hdl_base, pctx->attr.ch_info[ch_idx].draw_orl_vipp);
        AW_MPI_RGN_DetachFromChn(i + pctx->region_info[ch_idx].region_hdl_base, &ViChn);
        AW_MPI_RGN_Destroy(i + pctx->region_info[ch_idx].region_hdl_base);
    }

    for (i = 0; i < res->valid_cnt; i++)
    {
        memset(&rgnAttr, 0x00, sizeof(RGN_ATTR_S));
        rgnAttr.enType = ORL_RGN;
        AW_MPI_RGN_Create(i + pctx->region_info[ch_idx].region_hdl_base, &rgnAttr);

        int left   = res->boxes[i].xmin * pctx->attr.ch_info[ch_idx].draw_orl_src_width   / pctx->human_src_width;
        int up     = res->boxes[i].ymin * pctx->attr.ch_info[ch_idx].draw_orl_src_height  / pctx->human_src_height;
        int right  = res->boxes[i].xmax * pctx->attr.ch_info[ch_idx].draw_orl_src_width   / pctx->human_src_width;
        int bottom = res->boxes[i].ymax * pctx->attr.ch_info[ch_idx].draw_orl_src_height  / pctx->human_src_height;

        memset(&rgnChnAttr, 0x00, sizeof(RGN_CHN_ATTR_S));
        rgnChnAttr.unChnAttr.stOrlChn.stRect.X        = left;
        rgnChnAttr.unChnAttr.stOrlChn.stRect.Y        = up;
        rgnChnAttr.unChnAttr.stOrlChn.stRect.Width    = right - left;
        rgnChnAttr.unChnAttr.stOrlChn.stRect.Height   = bottom - up;

        if (0 == res->boxes[i].label)
            rgnChnAttr.bShow = TRUE;
        else
            rgnChnAttr.bShow = FALSE;

        rgnChnAttr.enType                             = ORL_RGN;
        rgnChnAttr.unChnAttr.stOrlChn.enAreaType      = AREA_RECT;
        /**
          Recommended colors for drawing rectangular boxes on Vipp:
          Y255 U0   V255 : R255 G208 B27   : 0xffd01b
          Y0   U0   V255 : R178 G0   B0    : 0xb20000
          Y255 U255 V0   : R74  G255 B255  : 0x4affff
          Y0   U255 V0   : R0   G47  B225  : 0x002fe1
          The mutual conversion between RGB and YUV:
          https://www.mikekohn.net/file_formats/yuv_rgb_converter.php
        */
        rgnChnAttr.unChnAttr.stOrlChn.mColor          = 0x002fe1;
        rgnChnAttr.unChnAttr.stOrlChn.mThick          = 1;
        rgnChnAttr.unChnAttr.stOrlChn.mLayer          = i;
        alogv("ch_idx=%d, attach region hdl=%d, vipp=%d", ch_idx, i + pctx->region_info[ch_idx].region_hdl_base, pctx->attr.ch_info[ch_idx].draw_orl_vipp);
        AW_MPI_RGN_AttachToChn(i + pctx->region_info[ch_idx].region_hdl_base, &ViChn, &rgnChnAttr);
    }

    pctx->region_info[ch_idx].old_num_of_boxes = res->valid_cnt;
    return;
}

void paint_object_detect_region_face(Awnn_Result_t *res, int ch_idx)
{
    int i = 0;
    RGN_ATTR_S rgnAttr;
    RGN_CHN_ATTR_S rgnChnAttr;
    
    aialgo_context_t *pctx = get_aicontext();
    assert(pctx != NULL);

    MPP_CHN_S ViChn = {MOD_ID_VIU, pctx->attr.ch_info[ch_idx].draw_orl_vipp, 0};

    alogv("ch_idx=%d, region hdl base=%d, vipp=%d, orl num old=%d cur=%d", ch_idx,
        pctx->region_info[ch_idx].region_hdl_base, pctx->attr.ch_info[ch_idx].draw_orl_vipp,
        pctx->region_info[ch_idx].old_num_of_boxes, res->valid_cnt);

    for (i = 0; i < pctx->region_info[ch_idx].old_num_of_boxes; i ++)
    {
        alogv("ch_idx=%d, detach region hdl=%d, vipp=%d", ch_idx, i + pctx->region_info[ch_idx].region_hdl_base, pctx->attr.ch_info[ch_idx].draw_orl_vipp);
        AW_MPI_RGN_DetachFromChn(i + pctx->region_info[ch_idx].region_hdl_base, &ViChn);
        AW_MPI_RGN_Destroy(i + pctx->region_info[ch_idx].region_hdl_base);
    }

    for (i = 0; i < res->valid_cnt; i++)
    {
        memset(&rgnAttr, 0x00, sizeof(RGN_ATTR_S));
        rgnAttr.enType = ORL_RGN;
        AW_MPI_RGN_Create(i + pctx->region_info[ch_idx].region_hdl_base, &rgnAttr);

        int left   = res->boxes[i].xmin * pctx->attr.ch_info[ch_idx].draw_orl_src_width   / pctx->face_src_width;
        int up     = res->boxes[i].ymin * pctx->attr.ch_info[ch_idx].draw_orl_src_height  / pctx->face_src_height;
        int right  = res->boxes[i].xmax * pctx->attr.ch_info[ch_idx].draw_orl_src_width   / pctx->face_src_width;
        int bottom = res->boxes[i].ymax * pctx->attr.ch_info[ch_idx].draw_orl_src_height  / pctx->face_src_height;

        memset(&rgnChnAttr, 0x00, sizeof(RGN_CHN_ATTR_S));
        rgnChnAttr.unChnAttr.stOrlChn.stRect.X        = left;
        rgnChnAttr.unChnAttr.stOrlChn.stRect.Y        = up;
        rgnChnAttr.unChnAttr.stOrlChn.stRect.Width    = right - left;
        rgnChnAttr.unChnAttr.stOrlChn.stRect.Height   = bottom - up;

        if (0 == res->boxes[i].label)
            rgnChnAttr.bShow = TRUE;
        else
            rgnChnAttr.bShow = FALSE;

        rgnChnAttr.enType                             = ORL_RGN;
        rgnChnAttr.unChnAttr.stOrlChn.enAreaType      = AREA_RECT;
        rgnChnAttr.unChnAttr.stOrlChn.mColor          = 0xFF0000;
        rgnChnAttr.unChnAttr.stOrlChn.mThick          = 1;
        rgnChnAttr.unChnAttr.stOrlChn.mLayer          = i;
        alogv("ch_idx=%d, attach region hdl=%d, vipp=%d", ch_idx, i + pctx->region_info[ch_idx].region_hdl_base, pctx->attr.ch_info[ch_idx].draw_orl_vipp);
        AW_MPI_RGN_AttachToChn(i + pctx->region_info[ch_idx].region_hdl_base, &ViChn, &rgnChnAttr);
    }

    pctx->region_info[ch_idx].old_num_of_boxes = res->valid_cnt;
    return;
}
