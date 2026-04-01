#include <utils/plat_log.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/time.h>

#include "media/mm_comm_vi.h"
#include "media/mpi_vi.h"
#include "media/mpi_isp.h"

#include "gtm_ldci_common.h"

#define GTM_LDCI_COMMON_NUM_MAX       (4)

#define LDCI_VIDEO_WIDTH_DEFAULT      (160)
#define LDCI_VIDEO_HEIGHT_DEFAULT     (90)
#define LDCI_VIDEO_FPS                (20)
#define LDCI_VIDEO_BUF_CNT            (3)

typedef struct gtm_ldci_common_context {
    int mbExitFlag;
    pthread_t mStreamThreadId;
    ISP_DEV mIsp;
    VI_DEV mVipp;
    VI_CHN mViChn;
    VI_ATTR_S mViAttr;
} gtm_ldci_common_context;

static gtm_ldci_common_context gGtmLdciCommonContext[GTM_LDCI_COMMON_NUM_MAX];

static struct gtm_ldci_common_context *gtm_ldci_get_common_context(int isp)
{
    if (isp >= GTM_LDCI_COMMON_NUM_MAX)
    {
        aloge("fatal error! invalid isp id %d >= %d\n", isp, GTM_LDCI_COMMON_NUM_MAX);
        return NULL;
    }
    return &gGtmLdciCommonContext[isp];
}

static void* ldci_get_frame_thread(void *pThreadData)
{
    struct gtm_ldci_common_context *pContext = (struct gtm_ldci_common_context*)pThreadData;
    char strThreadName[64];
    sprintf(strThreadName, "ldci_isp%d_vi%d-%d", pContext->mIsp, pContext->mVipp, pContext->mViChn);
    prctl(PR_SET_NAME, (unsigned long)strThreadName, 0, 0, 0);

    ERRORTYPE ret = SUCCESS;
    int frm_cnt = 0;

    while (!pContext->mbExitFlag)
    {
        VIDEO_FRAME_INFO_S stFrameInfo;
        memset(&stFrameInfo, 0, sizeof(VIDEO_FRAME_INFO_S));
        ret = AW_MPI_VI_GetFrame(pContext->mVipp, pContext->mViChn, &stFrameInfo, 2000);
        if (SUCCESS == ret)
        {
            unsigned int dataWidth  = stFrameInfo.VFrame.mOffsetRight - stFrameInfo.VFrame.mOffsetLeft;
            unsigned int dataHeight = stFrameInfo.VFrame.mOffsetBottom - stFrameInfo.VFrame.mOffsetTop;
            ldci_frame_config_t ldci_frame;
            memset(&ldci_frame, 0, sizeof(ldci_frame_config_t));
            ldci_frame.buf = stFrameInfo.VFrame.mpVirAddr[0];
            ldci_frame.size = dataWidth*dataHeight;
            ldci_frame.width = dataWidth;
            ldci_frame.height = dataHeight;
            AW_MPI_ISP_SetLdciFrame(pContext->mIsp, &ldci_frame);
            /*if (frm_cnt++ % 30 == 0)
            {
                time_t now;
                struct tm *timenow;
                time(&now);
                timenow = localtime(&now);
                alogd("vipp%d ch%d, %dx%d, %d-%d-%d-%d, mpts=%lld; local time is %s, %d", \
                    pContext->mVipp, pContext->mViChn, 
                    stFrameInfo.VFrame.mWidth, stFrameInfo.VFrame.mHeight,
                    stFrameInfo.VFrame.mOffsetLeft, stFrameInfo.VFrame.mOffsetRight,
                    stFrameInfo.VFrame.mOffsetBottom, stFrameInfo.VFrame.mOffsetTop,
                    stFrameInfo.VFrame.mpts, asctime(timenow), dataWidth*dataHeight);
            }*/
            AW_MPI_VI_ReleaseFrame(pContext->mVipp, pContext->mViChn, &stFrameInfo);
        }
        else
        {
            alogw("fatal error! vipp[%d] getFrame failed! check code! ret=0x%x", pContext->mVipp, ret);
            continue;
        }
    }
    alogd("exit");
    return NULL;
}

int gtm_ldci_common_open(int isp, gtm_ldci_common_config_param *param)
{
    int ret = 0;
    int width = 0;
    int height = 0;
    int fps = 0;
    int vipp_buf_num = 0;

    if (isp >= GTM_LDCI_COMMON_NUM_MAX)
    {
        aloge("fatal error! invalid isp id %d >= %d\n", isp, GTM_LDCI_COMMON_NUM_MAX);
        return -1;
    }

    struct gtm_ldci_common_context *pContext = gtm_ldci_get_common_context(isp);
    memset(pContext, 0, sizeof(gtm_ldci_common_context));

    pContext->mIsp = isp;
    pContext->mVipp = param->ldci_vipp;
    pContext->mViChn = 0;

    ret |= AW_MPI_ISP_SetLdciSource(isp, ISP_LDCI_VIDEO_EXTERNAL);

    if (0 == param->ldci_width)
        width = LDCI_VIDEO_WIDTH_DEFAULT;
    else
        width = param->ldci_width;

    if (0 == param->ldci_height)
        height = LDCI_VIDEO_HEIGHT_DEFAULT;
    else
        height = param->ldci_height;

    if (0 == param->ldci_fps)
        fps = LDCI_VIDEO_FPS;
    else
        fps = param->ldci_fps;

    if (0 == param->vipp_buf_num)
        vipp_buf_num = LDCI_VIDEO_BUF_CNT;
    else
        vipp_buf_num = param->vipp_buf_num;

    pContext->mViAttr.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    pContext->mViAttr.memtype = V4L2_MEMORY_MMAP;
    pContext->mViAttr.format.field = V4L2_FIELD_NONE;
    pContext->mViAttr.nbufs = vipp_buf_num;
    pContext->mViAttr.nplanes = 1;
    pContext->mViAttr.format.width = width;
    pContext->mViAttr.format.height = height;
    pContext->mViAttr.fps = fps;
    pContext->mViAttr.capturemode = V4L2_MODE_VIDEO;
    pContext->mViAttr.use_current_win = 0;
    pContext->mViAttr.wdr_mode = 0;
    pContext->mViAttr.format.pixelformat = V4L2_PIX_FMT_GREY;
    //pContext->mViAttr.format.colorspace = V4L2_COLORSPACE_REC709;
    ret |= AW_MPI_VI_CreateVipp(pContext->mVipp);
    ret |= AW_MPI_VI_SetVippAttr(pContext->mVipp, &pContext->mViAttr);

    alogd("ret=%d", ret);

    return ret;
}

int gtm_ldci_common_start(int isp)
{
    int ret = 0;

    if (isp >= GTM_LDCI_COMMON_NUM_MAX)
    {
        aloge("fatal error! invalid isp id %d >= %d\n", isp, GTM_LDCI_COMMON_NUM_MAX);
        return -1;
    }

    struct gtm_ldci_common_context *pContext = gtm_ldci_get_common_context(isp);

    pContext->mViChn = 0;

    //ret |= AW_MPI_ISP_Run(pContext->mIsp);
    ret |= AW_MPI_VI_EnableVipp(pContext->mVipp);
    ret |= AW_MPI_VI_CreateVirChn(pContext->mVipp, pContext->mViChn, NULL);

    ret |= AW_MPI_VI_EnableVirChn(pContext->mVipp, pContext->mViChn);
    pContext->mbExitFlag = 0;
    int result = pthread_create(&pContext->mStreamThreadId, NULL, ldci_get_frame_thread, (void*)pContext);
    if (result != 0)
    {
        ret |= result;
        aloge("fatal error! pthread create fail[%d]", result);
    }

    alogd("ret=%d", ret);

    return ret;
}

int gtm_ldci_common_stop(int isp)
{
    int ret = 0;

    if (isp >= GTM_LDCI_COMMON_NUM_MAX)
    {
        aloge("fatal error! invalid isp id %d >= %d\n", isp, GTM_LDCI_COMMON_NUM_MAX);
        return -1;
    }

    struct gtm_ldci_common_context *pContext = gtm_ldci_get_common_context(isp);

    pContext->mbExitFlag = 1;
    pthread_join(pContext->mStreamThreadId, NULL);
    ret |= AW_MPI_VI_DisableVirChn(pContext->mVipp, pContext->mViChn);

    alogd("ret=%d", ret);

    return ret;
}

int gtm_ldci_common_close(int isp)
{
    int ret = 0;

    if (isp >= GTM_LDCI_COMMON_NUM_MAX)
    {
        aloge("fatal error! invalid isp id %d >= %d\n", isp, GTM_LDCI_COMMON_NUM_MAX);
        return -1;
    }

    struct gtm_ldci_common_context *pContext = gtm_ldci_get_common_context(isp);

    ret |= AW_MPI_VI_DestroyVirChn(pContext->mVipp, pContext->mViChn);

    ret |= AW_MPI_VI_DisableVipp(pContext->mVipp);
    ret |= AW_MPI_ISP_Stop(pContext->mIsp);
    ret |= AW_MPI_VI_DestroyVipp(pContext->mVipp);

    alogd("ret=%d", ret);

    return ret;
}

