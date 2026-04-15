#include "mpp_camera.h"
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "plat_log.h"
#include <mm_common.h>
#include <mpi_videoformat_conversion.h>
#include <mpi_region.h>
#include <mpi_vi_private.h>
#include <sys/mman.h> 
#include <sys/ioctl.h>
#include <sys_linux_ioctl.h>
#include <ClockCompPortIndex.h>
#include <endian.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include "vo/hwdisplay.h"
#include "lvgl_main.h"
#include "g2d_driver.h"
#include <PIXEL_FORMAT_E_g2d_format_convert.h>
#include <VIDEO_FRAME_INFO_S.h>
#include <mpi_videoformat_conversion.h>
#include <SystemBase.h>
#include <aenc_sw_lib.h>
#include "lvgl_system.h"
#include "Rtc2C.h"

// #include "rtsp_server.h"

static void creatVideoPathAndPicPath(void);
static ERRORTYPE InitMppCameraData(mpp_camera_para_conf *pContext);
static ERRORTYPE setDstSizeAndDispPos(mpp_camera_para_conf *pContext, int DstWidth, int DstHeight, int DisplayX, int DisplayY);
static ERRORTYPE setConfigPara(mpp_camera_para_conf *pContext);
static ERRORTYPE createViChn(mpp_camera_para_conf *pContext, VI_DEV videv, VI_CHN vichn);
static ERRORTYPE createVoChn(mpp_camera_para_conf *pContext,VO_DEV vodev,VO_CHN vochn);
static ERRORTYPE prepare(mpp_camera_para_conf *pContext);
static ERRORTYPE createVencChn(mpp_camera_para_conf *pContext);
static ERRORTYPE createMuxChn(mpp_camera_para_conf *pContext);
static ERRORTYPE startVideoRecording(mpp_camera_para_conf *pContext);
static ERRORTYPE stopVideoRecording(mpp_camera_para_conf *pContext);
static void DestroyMsgqQueue(mpp_camera_para_conf *pContext);
static int CreateMsgQueueThread(mpp_camera_para_conf *pContext);
static ERRORTYPE createAIChn(mpp_camera_para_conf *pContext, int AiDev, int AiChn);
static ERRORTYPE createAencChn(mpp_camera_para_conf *pContext);
static ERRORTYPE createVenc_picChn(mpp_camera_para_conf *pContext);
static void destoryVenc_picChn(mpp_camera_para_conf *pContext);
static void savePic(mpp_camera_para_conf *pContext);

#ifndef AWALIGN
#define AWALIGN(x, a)              ((a) * (((x) + (a) - 1) / (a)))
#endif

#define AI2MUX 
#define TACKPIC
#define DEFAULT_SIMPLE_CACHE_SIZE_VFS       (64*1024)
#define ISP_RUN (1)

extern int g_g2dfd;

extern QueueMpp *F_videoFile;
extern QueueMpp *R_videoFile;
extern QueueMpp *F_U_videoFile;
extern QueueMpp *R_U_videoFile;
extern QueueMpp *F_picFile;
extern QueueMpp *R_picFile;

pthread_mutex_t Vi2Vomutex = PTHREAD_MUTEX_INITIALIZER;

static int G2D_Convert_rotate(VIDEO_FRAME_INFO_S *src ,VIDEO_FRAME_INFO_S *dst,int rotate){
    // printf("%s:%d\n",__func__,__LINE__);
    int ret = 0;
    g2d_blt_h blit;
    g2d_fmt_enh eSrcFormat, eDstFormat;
    ret = convert_PIXEL_FORMAT_E_to_g2d_fmt_enh(src->VFrame.mPixelFormat, &eSrcFormat);
    if(ret!=SUCCESS)
    {
        printf("fatal error! src pixel format[0x%x] is invalid!\n", src->VFrame.mPixelFormat);
        return -1;
    }
    ret = convert_PIXEL_FORMAT_E_to_g2d_fmt_enh(dst->VFrame.mPixelFormat, &eDstFormat);
    if(ret!=SUCCESS)
    {
        printf("fatal error! dst pixel format[0x%x] is invalid!\n", dst->VFrame.mPixelFormat);
        return -1;
    }

    //config blit
    memset(&blit, 0, sizeof(g2d_blt_h));
    switch(rotate)
    {
        case 0:
            blit.flag_h = G2D_BLT_NONE_H;   //G2D_ROT_0, G2D_BLT_NONE_H
            break;
        case 90:
            blit.flag_h = G2D_ROT_90;
            break;
        case 180:
            blit.flag_h = G2D_ROT_180;
            break;
        case 270:
            blit.flag_h = G2D_ROT_270;
            break;
        default:
            // printf("fatal error! rotation[%d] is invalid!", pThiz->mConvertParam.mDstRotate);
            blit.flag_h = G2D_BLT_NONE_H;
            break;
    }
    //blit.src_image_h.bbuff = 1;
    //blit.src_image_h.color = 0xff;
    blit.src_image_h.format = eSrcFormat;
    blit.src_image_h.laddr[0] = src->VFrame.mPhyAddr[0];
    blit.src_image_h.laddr[1] = src->VFrame.mPhyAddr[1];
    blit.src_image_h.laddr[2] = src->VFrame.mPhyAddr[2];
    //blit.src_image_h.haddr[] = 
    blit.src_image_h.width = src->VFrame.mWidth;
    blit.src_image_h.height = src->VFrame.mHeight;
    blit.src_image_h.align[0] = 0;
    blit.src_image_h.align[1] = 0;
    blit.src_image_h.align[2] = 0;
    blit.src_image_h.clip_rect.x = 0;
    blit.src_image_h.clip_rect.y = 0;
    blit.src_image_h.clip_rect.w = src->VFrame.mWidth;
    blit.src_image_h.clip_rect.h = src->VFrame.mHeight;
    blit.src_image_h.gamut = G2D_BT601;
    blit.src_image_h.bpremul = 0;
    //blit.src_image_h.alpha = 0xff;
    blit.src_image_h.mode = G2D_GLOBAL_ALPHA;   //G2D_PIXEL_ALPHA, G2D_GLOBAL_ALPHA
    blit.src_image_h.fd = -1;
    blit.src_image_h.use_phy_addr = 1;

    //blit.dst_image_h.bbuff = 1;
    //blit.dst_image_h.color = 0xff;
    blit.dst_image_h.format = eDstFormat;
    blit.dst_image_h.laddr[0] = dst->VFrame.mPhyAddr[0];
    blit.dst_image_h.laddr[1] = dst->VFrame.mPhyAddr[1];
    blit.dst_image_h.laddr[2] = dst->VFrame.mPhyAddr[2];
    //blit.dst_image_h.haddr[] = 
    blit.dst_image_h.width = dst->VFrame.mWidth;
    blit.dst_image_h.height = dst->VFrame.mHeight;
    blit.dst_image_h.align[0] = 0;
    blit.dst_image_h.align[1] = 0;
    blit.dst_image_h.align[2] = 0;
    blit.dst_image_h.clip_rect.x = 0;
    blit.dst_image_h.clip_rect.y = 0;
    blit.dst_image_h.clip_rect.w = dst->VFrame.mWidth;
    blit.dst_image_h.clip_rect.h = dst->VFrame.mHeight;
    blit.dst_image_h.gamut = G2D_BT601;
    blit.dst_image_h.bpremul = 0;
    //blit.dst_image_h.alpha = 0xff;
    blit.dst_image_h.mode = G2D_GLOBAL_ALPHA;   //G2D_PIXEL_ALPHA, G2D_GLOBAL_ALPHA
    blit.dst_image_h.fd = -1;
    blit.dst_image_h.use_phy_addr = 1;
    // printf("src->VFrame.mWidth:%d,src->VFrame.mHeight:%d,dst->VFrame.mWidth:%d,dst->VFrame.mHight:%d, eSrcFormat:%d,eDstFormat:%d,g_g2dfd:%d\n",src->VFrame.mWidth,src->VFrame.mHeight,dst->VFrame.mWidth,dst->VFrame.mHeight,eSrcFormat,eDstFormat,g_g2dfd);
    ret = ioctl(g_g2dfd, G2D_CMD_BITBLT_H, (unsigned long)&blit);
    if(ret < 0)
    {
        printf("fatal error! bit-block(image) transfer failed[%d]\n", ret);
    }

    dst->VFrame.mOffsetTop = 0;
    dst->VFrame.mOffsetBottom = dst->VFrame.mHeight;
    dst->VFrame.mOffsetLeft = 0;
    dst->VFrame.mOffsetRight = dst->VFrame.mWidth;
    return ret;
}

static int G2D_Convert_Scale(VIDEO_FRAME_INFO_S *src ,VIDEO_FRAME_INFO_S *dst){
    // printf("%s:%d\n",__func__,__LINE__);
    int ret = 0;
    g2d_blt_h blit;
    memset(&blit, 0, sizeof(g2d_blt_h));
    g2d_fmt_enh eSrcFormat, eDstFormat;
    ret = convert_PIXEL_FORMAT_E_to_g2d_fmt_enh(src->VFrame.mPixelFormat, &eSrcFormat);
    if(ret!=SUCCESS)
    {
        printf("fatal error! src pixel format[0x%x] is invalid!\n", src->VFrame.mPixelFormat);
        return -1;
    }
    ret = convert_PIXEL_FORMAT_E_to_g2d_fmt_enh(dst->VFrame.mPixelFormat, &eDstFormat);
    if(ret!=SUCCESS)
    {
        printf("fatal error! dst pixel format[0x%x] is invalid!\n", dst->VFrame.mPixelFormat);
        return -1;
    }

    blit.flag_h = G2D_BLT_NONE_H;
    blit.src_image_h.format = eSrcFormat;
    blit.src_image_h.laddr[0] = src->VFrame.mPhyAddr[0];
    blit.src_image_h.laddr[1] = src->VFrame.mPhyAddr[1];
    blit.src_image_h.laddr[2] = src->VFrame.mPhyAddr[2];
    //blit.src_image_h.haddr[] = 
    blit.src_image_h.width = src->VFrame.mWidth;
    blit.src_image_h.height = src->VFrame.mHeight;
    blit.src_image_h.align[0] = 0;
    blit.src_image_h.align[1] = 0;
    blit.src_image_h.align[2] = 0;
    blit.src_image_h.clip_rect.x = 0;
    blit.src_image_h.clip_rect.y = 0;
    blit.src_image_h.clip_rect.w = src->VFrame.mWidth;
    blit.src_image_h.clip_rect.h = src->VFrame.mHeight;
    blit.src_image_h.gamut = G2D_BT601;
    blit.src_image_h.bpremul = 0;
    //blit.src_image_h.alpha = 0xff;
    blit.src_image_h.mode = G2D_PIXEL_ALPHA;   //G2D_PIXEL_ALPHA, G2D_GLOBAL_ALPHA
    blit.src_image_h.fd = -1;
    blit.src_image_h.use_phy_addr = 1;

    //blit.dst_image_h.bbuff = 1;
    //blit.dst_image_h.color = 0xff;
    blit.dst_image_h.format = eDstFormat;
    blit.dst_image_h.laddr[0] = dst->VFrame.mPhyAddr[0];
    blit.dst_image_h.laddr[1] = dst->VFrame.mPhyAddr[1];
    blit.dst_image_h.laddr[2] = dst->VFrame.mPhyAddr[2];
    //blit.dst_image_h.haddr[] = 
    blit.dst_image_h.width = dst->VFrame.mWidth;
    blit.dst_image_h.height = dst->VFrame.mHeight;
    blit.dst_image_h.align[0] = 0;
    blit.dst_image_h.align[1] = 0;
    blit.dst_image_h.align[2] = 0;
    blit.dst_image_h.clip_rect.x = 0;
    blit.dst_image_h.clip_rect.y = 0;
    blit.dst_image_h.clip_rect.w = dst->VFrame.mWidth;
    blit.dst_image_h.clip_rect.h = dst->VFrame.mHeight;

    blit.dst_image_h.gamut = G2D_BT601;
    blit.dst_image_h.bpremul = 0;
    //blit.dst_image_h.alpha = 0xff;
    blit.dst_image_h.mode = G2D_PIXEL_ALPHA;   //G2D_PIXEL_ALPHA, G2D_GLOBAL_ALPHA
    blit.dst_image_h.fd = -1;
    blit.dst_image_h.use_phy_addr = 1;
    // printf("src->VFrame.mWidth:%d,src->VFrame.mHeight:%d,dst->VFrame.mWidth:%d,dst->VFrame.mHight:%d, eSrcFormat:%d,eDstFormat:%d,g_g2dfd:%d\n",src->VFrame.mWidth,src->VFrame.mHeight,dst->VFrame.mWidth,dst->VFrame.mHeight,eSrcFormat,eDstFormat,g_g2dfd);
    ret = ioctl(g_g2dfd, G2D_CMD_BITBLT_H, (unsigned long)&blit);
    if(ret < 0)
    {
        printf("fatal error! bit-block(image) transfer failed[%d]\n", ret);
    }

    dst->VFrame.mOffsetTop = 0;
    dst->VFrame.mOffsetBottom = dst->VFrame.mHeight;
    dst->VFrame.mOffsetLeft = 0;
    dst->VFrame.mOffsetRight = dst->VFrame.mWidth;
    return ret;
}


#if 1
static void *Vi2VencFrameThread(void *pThreadData)
{
    mpp_camera_para_conf *pContext = (mpp_camera_para_conf*)pThreadData;

    int ret = 0;
    
    // 1. 创建双缓冲避免竞争

    
    while (pContext->mExitFlag == 0)
    {

        
        if (pContext->mRecorderFlag) {
            if (queue_mpp_empty(pContext->m_vo.mppQueue)) {
                usleep(1000); // 缩短休眠时间
                continue;
            }
            
            VIDEO_FRAME_INFO_S *FrameInfo = (VIDEO_FRAME_INFO_S *)queue_mpp_pop(pContext->m_vo.mppQueue);
        
#ifdef TACKPIC        
        if(pContext->mTakePicFlag){
            ret = AW_MPI_VENC_SendFrame(pContext->m_venc_pic.mVEncChn, FrameInfo, 0);
            if (ret != SUCCESS)
            {
                printf("fatal error, venc_pic send frame sync failed!\n");
            }
            savePic(pContext);
            pContext->mTakePicFlag = 0;
        }
#endif  

            ret = AW_MPI_VENC_SendFrame(pContext->m_venc.mVEncChn, FrameInfo, 0);
            if (ret < 0)
            {
                printf("fatal error, venc send frame sync failed!\n");
            }
            
            
        } else {
            usleep(1000000); // 1000ms
        }
        
    }
    
  
    return NULL;
}
#else

#endif


// #define MAX_QUEUE_FRAMES 15
static void *FsyncFrameThread(void *pThreadData)
{
    mpp_camera_para_conf *pContext = (mpp_camera_para_conf*)pThreadData;
    while (pContext->mExitFlag == 0)
    {    
        
        if(pContext->mRecorderFlag){
            pthread_mutex_lock(&pContext->m_mux.fsyncMutex);
            int fd = open(pContext->m_mux.dstVideoFile, O_RDONLY);
            if(fd){
                pthread_mutex_unlock(&pContext->m_mux.fsyncMutex);
                fsync(fd);
                close(fd);
            }
            sleep(1);
        }else{
            sleep(1);
        }
    }
    return;
}

static void *GetCSIFrameThread(void *pThreadData)
{
    long lastTime = 0;
    mpp_camera_para_conf *pContext = (mpp_camera_para_conf*)pThreadData;
    int ret = 0;
    VIDEO_FRAME_INFO_S stFrameInfo;
    VENC_JPEG_THUMB_BUFFER_S mJpegThumbBuf;
    pContext->m_vo.mppQueue = queue_mpp_create();
    memset(&stFrameInfo, 0, sizeof(VIDEO_FRAME_INFO_S));
    // printf("theard create videv:%d vichn:%d,create vodev:%d,vochn%d,volayer:%d,mExitFlag:%d\n",pContext->m_vi.mViDev,pContext->m_vi.mViChn,pContext->m_vo.mVoDev,pContext->m_vo.mVoChn,pContext->m_vo.mVoLayer,pContext->mExitFlag);
#if 1 
    VIDEO_FRAME_INFO_S *dstScaBuf = NULL;
        dstScaBuf = (VIDEO_FRAME_INFO_S *)malloc( sizeof(VIDEO_FRAME_INFO_S));
        memset(dstScaBuf, 0, sizeof(VIDEO_FRAME_INFO_S));
        if (!dstScaBuf) {
            printf("Failed to allocate dst dstScaBuf\n");
            goto mem_err;
        }
        
        dstScaBuf->mId = 0;
        // 注意：G2D旋转后宽高对调
        dstScaBuf->VFrame.mWidth = AWALIGN(pContext->m_vi.mWidth / 2, 16);
        dstScaBuf->VFrame.mHeight = AWALIGN(pContext->m_vi.mHeight / 2, 16);
        dstScaBuf->VFrame.mPixelFormat = pContext->m_vi.mPixFmt;

        
        printf("Allocating buffer: %dx%d, fmt: %x\n", 
                dstScaBuf->VFrame.mWidth, 
               dstScaBuf->VFrame.mHeight,
               dstScaBuf->VFrame.mPixelFormat);
        
        // 分配内存
        switch (pContext->m_vi.mPixFmt) {
        case MM_PIXEL_FORMAT_YUV_SEMIPLANAR_420:
        case MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420:
            ret = AW_MPI_SYS_MmzAlloc_Cached(
                &dstScaBuf->VFrame.mPhyAddr[0],
                &dstScaBuf->VFrame.mpVirAddr[0],
                dstScaBuf->VFrame.mWidth * dstScaBuf->VFrame.mHeight);
            if (ret != SUCCESS) {
                printf("Buffer plane0 alloc failed: %d\n", ret);
                goto mem_err;
            }
            
            ret = AW_MPI_SYS_MmzAlloc_Cached(
                &dstScaBuf->VFrame.mPhyAddr[1],
                &dstScaBuf->VFrame.mpVirAddr[1],
                dstScaBuf->VFrame.mWidth * dstScaBuf->VFrame.mHeight / 2);
            if (ret != SUCCESS) {
                printf("Buffer plane1 alloc failed: %d\n", ret);
                // 释放plane0
                if (dstScaBuf->VFrame.mpVirAddr[0]) {
                    AW_MPI_SYS_MmzFree(dstScaBuf->VFrame.mPhyAddr[0], 
                                      dstScaBuf->VFrame.mpVirAddr[0]);
                }
                goto mem_err;
            }
            break;
            
        case MM_PIXEL_FORMAT_YUV_PLANAR_420:
        case MM_PIXEL_FORMAT_YVU_PLANAR_420:
        default:
            printf("Unsupported pixel format: %x\n", pContext->m_vi.mPixFmt);
            free(dstScaBuf);
            dstScaBuf = NULL;
            goto mem_err;
        }
#endif
    VIDEO_FRAME_INFO_S *dstBuffers[2] = {NULL, NULL};
    int currentBuffer = 0;

    for (int i = 0; i < 2; i++) {
        dstBuffers[i] = (VIDEO_FRAME_INFO_S *)malloc( sizeof(VIDEO_FRAME_INFO_S));
        memset(dstBuffers[i], 0, sizeof(VIDEO_FRAME_INFO_S));
        if (!dstBuffers[i]) {
            printf("Failed to allocate dst buffer %d\n", i);
            goto mem_err;
        }
        
        dstBuffers[i]->mId = 0;
        // 注意：G2D旋转后宽高对调
        dstBuffers[i]->VFrame.mWidth = AWALIGN(pContext->m_vi.mHeight / 2, 16);
        dstBuffers[i]->VFrame.mHeight = AWALIGN(pContext->m_vi.mWidth / 2, 16);
        dstBuffers[i]->VFrame.mPixelFormat = pContext->m_vi.mPixFmt;

        
        printf("Allocating buffer %d: %dx%d, fmt: %x\n", 
               i, dstBuffers[i]->VFrame.mWidth, 
               dstBuffers[i]->VFrame.mHeight,
               dstBuffers[i]->VFrame.mPixelFormat);
        
        // 分配内存
        switch (pContext->m_vi.mPixFmt) {
        case MM_PIXEL_FORMAT_YUV_SEMIPLANAR_420:
        case MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420:
            ret = AW_MPI_SYS_MmzAlloc_Cached(
                &dstBuffers[i]->VFrame.mPhyAddr[0],
                &dstBuffers[i]->VFrame.mpVirAddr[0],
                dstBuffers[i]->VFrame.mWidth * dstBuffers[i]->VFrame.mHeight);
            if (ret != SUCCESS) {
                printf("Buffer %d plane0 alloc failed: %d\n", i, ret);
                goto mem_err;
            }
            
            ret = AW_MPI_SYS_MmzAlloc_Cached(
                &dstBuffers[i]->VFrame.mPhyAddr[1],
                &dstBuffers[i]->VFrame.mpVirAddr[1],
                dstBuffers[i]->VFrame.mWidth * dstBuffers[i]->VFrame.mHeight / 2);
            if (ret != SUCCESS) {
                printf("Buffer %d plane1 alloc failed: %d\n", i, ret);
                // 释放plane0
                if (dstBuffers[i]->VFrame.mpVirAddr[0]) {
                    AW_MPI_SYS_MmzFree(dstBuffers[i]->VFrame.mPhyAddr[0], 
                                      dstBuffers[i]->VFrame.mpVirAddr[0]);
                }
                goto mem_err;
            }
            break;
            
        case MM_PIXEL_FORMAT_YUV_PLANAR_420:
        case MM_PIXEL_FORMAT_YVU_PLANAR_420:
        default:
            printf("Unsupported pixel format: %x\n", pContext->m_vi.mPixFmt);
            free(dstBuffers[i]);
            dstBuffers[i] = NULL;
            goto mem_err;
        }
    }
    
    printf("G2D buffers allocation completed\n");
    while (pContext->mExitFlag == 0)
    {      
 
        if ((ret = AW_MPI_VI_GetFrame(pContext->m_vi.mViDev, pContext->m_vi.mViChn, &stFrameInfo, 500)) < 0)
        {
            printf("fatal error, vi get frame failed!\n");
            continue;
        }

        // printf("stFrameInfo.VFrame.mOffsetBottom:%d,stFrameInfo.VFrame.mOffsetTop:%d,stFrameInfo.VFrame.mOffsetLeft:%d,stFrameInfo.VFrame.mOffsetRight:%d",stFrameInfo.VFrame.mOffsetBottom,stFrameInfo.VFrame.mOffsetTop,stFrameInfo.VFrame.mOffsetLeft,stFrameInfo.VFrame.mOffsetRight);
        if(pContext->mVoFlag){          
            // 将缓冲区推入队列
            pthread_mutex_lock(&Vi2Vomutex);
            ret = G2D_Convert_Scale(&stFrameInfo, dstScaBuf);
            if (ret != 0) {
                printf("G2D rotation failed: %d\n", ret);
                continue;
            }           
            ret = G2D_Convert_rotate(dstScaBuf, dstBuffers[currentBuffer], 270);
            if (ret != 0) {
                printf("G2D rotation failed: %d\n", ret);
                continue;
            }
            pthread_mutex_unlock(&Vi2Vomutex);
            ret = AW_MPI_VO_SendFrame(pContext->m_vo.mVoLayer,pContext->m_vo.mVoChn, dstBuffers[currentBuffer], 0);
            if (ret != SUCCESS)
            {
                printf("fatal error, venc_pic send frame sync failed!\n");
            }
            currentBuffer = (currentBuffer + 1) % 2;
        }
    
        if(pContext->mRecorderFlag){
            queue_mpp_push(pContext->m_vo.mppQueue, &stFrameInfo);
        }

  
        ret = AW_MPI_VI_ReleaseFrame(pContext->m_vi.mViDev, pContext->m_vi.mViChn, &stFrameInfo);
        if (ret < 0)
        {
            printf("fatal error, vi release frame failed!\n");
        }
    }
mem_err:
    // 正确释放内存
    for (int i = 0; i < 2; i++) {
        if (dstBuffers[i]) {
            // 根据像素格式正确释放
            switch (dstBuffers[i]->VFrame.mPixelFormat) {
            case MM_PIXEL_FORMAT_YUV_SEMIPLANAR_420:
            case MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420:
                if (dstBuffers[i]->VFrame.mpVirAddr[0]) {
                    AW_MPI_SYS_MmzFree(dstBuffers[i]->VFrame.mPhyAddr[0], 
                                      dstBuffers[i]->VFrame.mpVirAddr[0]);
                }
                if (dstBuffers[i]->VFrame.mpVirAddr[1]) {
                    AW_MPI_SYS_MmzFree(dstBuffers[i]->VFrame.mPhyAddr[1], 
                                      dstBuffers[i]->VFrame.mpVirAddr[1]);
                }
                break;
                
            case MM_PIXEL_FORMAT_YUV_PLANAR_420:
            case MM_PIXEL_FORMAT_YVU_PLANAR_420:
                if (dstBuffers[i]->VFrame.mpVirAddr[0]) {
                    AW_MPI_SYS_MmzFree(dstBuffers[i]->VFrame.mPhyAddr[0], 
                                      dstBuffers[i]->VFrame.mpVirAddr[0]);
                }
                if (dstBuffers[i]->VFrame.mpVirAddr[1]) {
                    AW_MPI_SYS_MmzFree(dstBuffers[i]->VFrame.mPhyAddr[1], 
                                      dstBuffers[i]->VFrame.mpVirAddr[1]);
                }
                if (dstBuffers[i]->VFrame.mpVirAddr[2]) {
                    AW_MPI_SYS_MmzFree(dstBuffers[i]->VFrame.mPhyAddr[2], 
                                      dstBuffers[i]->VFrame.mpVirAddr[2]);
                }
                break;
            }
            
            free(dstBuffers[i]);
        }
    }   
    return;
}

/*
    create file save path
    parameter : void
*/
#if 0
static void creatVideoPathAndPicPath(void){
        if (access("/mnt/extsd/", F_OK) == 0) {
        if (access("/mnt/extsd/recorder", F_OK) != 0){
            system("mkdir /mnt/extsd/recorder"); 
        }
        if (access("/mnt/extsd/recorderUrgent", F_OK) != 0){
            system("mkdir /mnt/extsd/recorderUrgent"); 
        }       
        if (access("/mnt/extsd/recorder/frontCamera", F_OK) != 0){
            system("mkdir /mnt/extsd/recorder/frontCamera"); 
        }
        if (access("/mnt/extsd/recorder/rearCamera", F_OK) != 0){
            system("mkdir /mnt/extsd/recorder/rearCamera"); 
        }
        if (access("/mnt/extsd/recorderUrgent/frontCamera", F_OK) != 0){
            system("mkdir /mnt/extsd/recorderUrgent/frontCamera"); 
        }
        if (access("/mnt/extsd/recorderUrgent/rearCamera", F_OK) != 0){
            system("mkdir /mnt/extsd/recorderUrgent/rearCamera"); 
        }
        if (access("/mnt/extsd/DVRpic", F_OK) != 0){
            system("mkdir /mnt/extsd/DVRpic"); 
        }
        if (access("/mnt/extsd/DVRpic/frontPic", F_OK) != 0){
            system("mkdir /mnt/extsd/DVRpic/frontPic"); 
        }
        if (access("/mnt/extsd/DVRpic/rearPic", F_OK) != 0){
            system("mkdir /mnt/extsd/DVRpic/rearPic"); 
        }  
    }

}
#else 
static void creatVideoPathAndPicPath(void){
        if (access("/mnt/extsd/", F_OK) == 0) {
     
        if (access("/mnt/extsd/recorder/frontCamera", F_OK) != 0){
            system("mkdir -p /mnt/extsd/recorder/frontCamera"); 
        }
        if (access("/mnt/extsd/recorder/rearCamera", F_OK) != 0){
            system("mkdir -p /mnt/extsd/recorder/rearCamera"); 
        }
        if (access("/mnt/extsd/recorderUrgent/frontCamera", F_OK) != 0){
            system("mkdir -p /mnt/extsd/recorderUrgent/frontCamera"); 
        }
        if (access("/mnt/extsd/recorderUrgent/rearCamera", F_OK) != 0){
            system("mkdir -p /mnt/extsd/recorderUrgent/rearCamera"); 
        }

        if (access("/mnt/extsd/DVRpic/frontPic", F_OK) != 0){
            system("mkdir -p /mnt/extsd/DVRpic/frontPic"); 
        }
        if (access("/mnt/extsd/DVRpic/rearPic", F_OK) != 0){
            system("mkdir -p /mnt/extsd/DVRpic/rearPic"); 
        }  
    }

}
#endif
/*
    get file name
    parameter: pContext : camera device
*/
#if 0
static void getFileNameByCurTime(mpp_camera_para_conf *pContext, char *pNameBuf)
{
    printf("%s:%d\n",__func__,__LINE__);
    TTime time;
    RtcGetTime2C(&time);
    if(pContext->m_vi.mViDev == 0){
        if(g_sys_Data.recorderMode == RECORDER_NORMAL){
            sprintf(pNameBuf,"%s/Front%d-%d-%d_%d-%d-%d.%s", FRONT_VIDEO_PATH,time.nYear,time.nMonth,time.nDay,time.nHour,time.nMinute,time.nSecond,(MEDIA_FILE_FORMAT_TS==pContext->m_mux.eFileFormat)?"ts":"mp4");            
        }else{
            sprintf(pNameBuf,"%s/Front%d-%d-%d_%d-%d-%d.%s", FRONT_VIDEO_URGENT_PATH,time.nYear,time.nMonth,time.nDay,time.nHour,time.nMinute,time.nSecond,(MEDIA_FILE_FORMAT_TS==pContext->m_mux.eFileFormat)?"ts":"mp4");
        }
    }else{
        if(g_sys_Data.recorderMode == RECORDER_NORMAL){
            sprintf(pNameBuf,"%s/Rear%d-%d-%d_%d-%d-%d.%s", REAR_VIDEO_PATH,time.nYear,time.nMonth,time.nDay,time.nHour,time.nMinute,time.nSecond,(MEDIA_FILE_FORMAT_TS==pContext->m_mux.eFileFormat)?"ts":"mp4");
        }else{
            sprintf(pNameBuf,"%s/Rear%d-%d-%d_%d-%d-%d.%s", REAR_VIDEO_URGENT_PATH,time.nYear,time.nMonth,time.nDay,time.nHour,time.nMinute,time.nSecond,(MEDIA_FILE_FORMAT_TS==pContext->m_mux.eFileFormat)?"ts":"mp4");
        }
    }
    return;
}
#else
static void getFileNameByCurTime(mpp_camera_para_conf *pContext, char *pNameBuf)
{
    printf("%s:%d\n",__func__,__LINE__);
    TTime time;
    RtcGetTime2C(&time);
    if(pContext->m_vi.mViDev == 0){
        if(g_sys_Data.recorderMode == RECORDER_NORMAL){
            sprintf(pNameBuf,"%s/Front%d-%d-%d_%d-%d-%d.%s", FRONT_VIDEO_PATH,time.nYear,time.nMonth,time.nDay,time.nHour,time.nMinute,time.nSecond,"mp4");            
        }else{
            sprintf(pNameBuf,"%s/Front%d-%d-%d_%d-%d-%d.%s", FRONT_VIDEO_URGENT_PATH,time.nYear,time.nMonth,time.nDay,time.nHour,time.nMinute,time.nSecond,"mp4");
        }
    }else{
        if(g_sys_Data.recorderMode == RECORDER_NORMAL){
            sprintf(pNameBuf,"%s/Rear%d-%d-%d_%d-%d-%d.%s", REAR_VIDEO_PATH,time.nYear,time.nMonth,time.nDay,time.nHour,time.nMinute,time.nSecond,"mp4");
        }else{
            sprintf(pNameBuf,"%s/Rear%d-%d-%d_%d-%d-%d.%s", REAR_VIDEO_URGENT_PATH,time.nYear,time.nMonth,time.nDay,time.nHour,time.nMinute,time.nSecond,"mp4");
        }
    }
    return;
}
#endif

static int setNextFileToMuxer(mpp_camera_para_conf *pRecorder, char* path, int64_t fallocateLength, int muxChn)
{
    printf("%s:%d\n",__func__,__LINE__);
    int result = 0;
    ERRORTYPE ret;
    if(path != NULL)
    {
        int fd = open(path, O_RDWR | O_CREAT, 0666);
        if (fd < 0)
        {
            printf("fatal error! fail to open %s\n", path);
            return -1;
        }

        if (pRecorder->m_mux.mMuxChn == muxChn)
        {
            ret = AW_MPI_MUX_SwitchFd(pRecorder->m_mux.mMuxChn, fd, (int)fallocateLength);
            if(ret != SUCCESS)
            {
                printf("fatal error! muxChn[%d] switch fd[%d] fail[0x%x]!\n", pRecorder->m_mux.mMuxChn, fd, ret);
                result = -1;
            }
        }
        else
        {
            printf("fatal error! muxChn is not match:[0x%x!=0x%x]\n", pRecorder->m_mux.mMuxChn, muxChn);
            result = -1;
        }

        close(fd);

        return result;
    }
    else
    {
        return -1;
    }
}

static void DestroyMsgqQueue(mpp_camera_para_conf *pContext){
    message_destroy(&pContext->mMsgQueue);
}

static void *MsgQueueThread(void *pThreadData)
{
    mpp_camera_para_conf *pContext = (mpp_camera_para_conf*)pThreadData;
    message_t stCmdMsg;
    Vi2Venc2MuxerMsgType cmd;
    int nCmdPara;
    char fileName[MAX_FILE_PATH_SIZE] = {0};
    printf("message queue thread start.\n");
    while (1)
    {
        if (0 == get_message(&pContext->mMsgQueue, &stCmdMsg))
        {
            cmd = stCmdMsg.command;
            nCmdPara = stCmdMsg.para0;

            switch (cmd)
            {
                case Rec_NeedSetNextFd:
                {
                    mpp_camera_para_conf_data *pMsgData = stCmdMsg.mpData;
                    int muxChn = nCmdPara;
                    
                    
                    getFileNameByCurTime(pContext, fileName);
                    printf("muxChn[%d] set next fd, filepath=%s\n", muxChn, fileName);
                    setNextFileToMuxer(pMsgData->MppCameraParaConfData, fileName, 0, pMsgData->MppCameraParaConfData->m_mux.mMuxChn);

                    //free message mpdata
                    free(stCmdMsg.mpData);
                    stCmdMsg.mpData = NULL;
                    break;
                }
                case Rec_FileDone:
                {      

                    pthread_mutex_lock(&pContext->m_mux.fsyncMutex);
                    strcpy(pContext->m_mux.dstVideoFile, fileName);
                    pthread_mutex_unlock(&pContext->m_mux.fsyncMutex);
                        mpp_camera_para_conf_data *pMsgData = stCmdMsg.mpData;
                        MUX_CHN_ATTR_S muxChnAttr;
                        memset(&muxChnAttr,0,sizeof(MUX_CHN_ATTR_S));
                        AW_MPI_MUX_GetChnAttr(pMsgData->MppCameraParaConfData->m_mux.mMuxChn,&muxChnAttr);
                        recorder_time recorderTime;
                        if(g_sys_Data.recorderMode == RECORDER_NORMAL){
                            recorderTime = g_sys_Data.recorderTime;
                            deletFileInRecorderPath(0);
                            if(pContext->m_vi.mViDev == 0)
                                queue_mpp_push(F_videoFile, pContext->m_mux.dstVideoFile);
                            else
                                queue_mpp_push(R_videoFile, pContext->m_mux.dstVideoFile);
                        }else if(g_sys_Data.recorderMode == RECORDER_URGENT){
                            recorderTime = g_sys_Data.recorderUrgentTime;
                            deletFileInRecorderPath(1);
                            if(pContext->m_vi.mViDev == 0)
                                queue_mpp_push(F_U_videoFile, pContext->m_mux.dstVideoFile);
                            else
                                queue_mpp_push(R_U_videoFile, pContext->m_mux.dstVideoFile);
                        }
                        switch (recorderTime)
                        {
                        case RECORDER_1:
                            /* code */
                            muxChnAttr.mMaxFileDuration = 60 *1000;
                            break;
                        case RECORDER_3:
                            /* code */
                            muxChnAttr.mMaxFileDuration = 180 *1000;
                            break;
                        case RECORDER_5:
                            /* code */
                            muxChnAttr.mMaxFileDuration = 300 *1000;
                            break;        
                        default:
                            muxChnAttr.mMaxFileDuration = 60 *1000;
                            break;
                        }
                        if(AW_MPI_MUX_SetChnAttr(pMsgData->MppCameraParaConfData->m_mux.mMuxChn,&muxChnAttr)){
                            printf("fatal error! set mux chnal attr fail!\n");
                        }
                    printf("recorder_mode : %d, recorderTime : %d \n", g_sys_Data.recorderMode , recorderTime);
                    printf("Rec_FileDone\n");
                    break;
                }
                case MsgQueue_Stop:
                {
                    goto _Exit;
                    break;
                }
                default :
                {
                    break;
                }
            }
        }
        else
        {
            TMessage_WaitQueueNotEmpty(&pContext->mMsgQueue, 0);
        }
    }

_Exit:
    printf("message queue thread stop.\n");
    return NULL;
}


static int CreateMsgQueueThread(mpp_camera_para_conf *pContext){
    int result = 0;
    if(pContext->m_vi.mViDev < 0){
        printf("invalid camera dev!!!\n");
        return -1;
    }
    result = pthread_create(&pContext->mMsgQueueThreadId, NULL, MsgQueueThread, pContext);
    if (result != 0)
    {
        printf("fatal error! create message queue thread fail[%d]!\n", result);
        return -1;
    }
    else
    {
        printf("create message queue success threadId[0x%x].\n", &pContext->mMsgQueueThreadId);
    }
    return result;
}

static ERRORTYPE InitMppCameraData(mpp_camera_para_conf *pContext){
    printf("%s:%d\n",__func__,__LINE__);
    if (pContext == NULL)
    {
        printf("malloc struct fail\n");
        return FAILURE;
    }
    pContext->m_vi.mViChn = MM_INVALID_CHN;
    pContext->m_vi.mViDev = MM_INVALID_DEV;
    pContext->m_vo.mVoDev = MM_INVALID_DEV;
    pContext->m_vo.mVoChn = MM_INVALID_CHN;
    pContext->m_venc.mVEncChn = MM_INVALID_CHN;
    pContext->m_venc_pic.mVEncChn = MM_INVALID_CHN;
    pContext->m_mux.mMuxChn = MM_INVALID_CHN;
    pContext->m_ai.mAiChn.mChnId = MM_INVALID_CHN;
    pContext->m_aenc.mAEncChn.mChnId = MM_INVALID_CHN;

    if (message_create(&pContext->mMsgQueue) < 0)
    {
        printf("message create fail!\n");
        return FAILURE;
    }

    return SUCCESS;
}

static ERRORTYPE setConfigPara(mpp_camera_para_conf *pContext){
    pContext->m_vi.mWidth = 1920;
    pContext->m_vi.mHeight = 1080;
    pContext->m_vi.mFrameRate = 25;
    pContext->m_vi.mPixFmt = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    pContext->m_vi.mColorSpace = V4L2_COLORSPACE_JPEG;
    pContext->m_vi.mViBufferNum = 5;
    pContext->m_vi.mViDropFrameNum = 0;
    pContext->m_vi.mEnableWDR = 0;

    pContext->m_vo.mDispType = VO_INTF_LCD;
    pContext->m_vo.mDispSync = VO_OUTPUT_NTSC;

    pContext->m_venc.mWidth = 1920;
    pContext->m_venc.mHeight = 1080;
    pContext->m_venc.mFrameRate = 25;
    pContext->m_venc.mBitRate = 1048576 * 8;
    pContext->m_venc.mRcMode = 0;
    pContext->m_venc.mEncoderFmt = PT_H264;

    pContext->m_ai.mBitRate = 32000;
    pContext->m_ai.mBitWidth = 16;
    pContext->m_ai.mSampleRate = 16000;
    pContext->m_ai.mAIChnCnt = 1;
    pContext->m_aenc.mCodecType = PT_AAC;

    return 0;
}


/*
    Set display position
    parameter: pContext : camera device
                DstWidth : display width
                DstHeight : display height
                DisplayX : display position X
                DisplayY : display position Y
*/
static ERRORTYPE setDstSizeAndDispPos(mpp_camera_para_conf *pContext, int Width, int Height, int DisplayX, int DisplayY){
    pContext->m_vo.mWidth  = Width;
    pContext->m_vo.mHeight = Height;   
    pContext->m_vo.mDisplayX   = DisplayX;
    pContext->m_vo.mDisplayY   = DisplayY; 
}


static ERRORTYPE MPPCallbackWrapper(void *cookie, MPP_CHN_S *pChn, MPP_EVENT_TYPE event, void *pEventData)
{
    // printf("%s:%d\n",__func__,__LINE__);
    mpp_camera_para_conf *pContext = (mpp_camera_para_conf *)cookie;
    ERRORTYPE ret = 0;

    if (MOD_ID_VIU == pChn->mModId)
    {
        switch(event)
        {
            case MPP_EVENT_VI_TIMEOUT:
            {
                printf("receive vi timeout. vipp:%d, chn:%d\n", pChn->mDevId, pChn->mChnId);
                break;
            }
	    default:
		printf("fatal error! unknow event type[0x%x]\n", event);
		break;
        }
    }

    else if(MOD_ID_VOU == pChn->mModId){
        // printf("VO callback: VO Layer[%d] chn[%d] event:%d\n", pChn->mDevId, pChn->mChnId, event);
        switch(event)
        {
            case MPP_EVENT_RELEASE_VIDEO_BUFFER:
            {
                // VIDEO_FRAME_INFO_S *pFrameInfo = (VIDEO_FRAME_INFO_S*)pEventData;
                // printf("vo layer[%d] release frame id[0x%x]!\n", pChn->mDevId, pFrameInfo->mId);
                break;
            }
            case MPP_EVENT_SET_VIDEO_SIZE:
            {
                SIZE_S *pDisplaySize = (SIZE_S*)pEventData;
                printf("vo layer[%d] report video display size[%dx%d]\n", pChn->mDevId, pDisplaySize->Width, pDisplaySize->Height);
                break;
            }
            case MPP_EVENT_RENDERING_START:
            {
                printf("vo layer[%d] report rendering start\n", pChn->mDevId);
                break;
            }
            default:
            {
                printf("fatal error! unknown event[0x%x] from channel[0x%x][0x%x][0x%x]!\n", event, pChn->mModId, pChn->mDevId, pChn->mChnId);
                ret = ERR_VO_ILLEGAL_PARAM;
                break;
            }
        }
    }
    else if (MOD_ID_VENC == pChn->mModId)
    {
        VENC_CHN mVEncChn = pChn->mChnId;
        switch(event)
        {
 
            case MPP_EVENT_VENC_BUFFER_FULL:
            {
                printf("vencChn[%d] vbv buffer full\n", pChn->mChnId);
                break;
            }
            case MPP_EVENT_VENC_TIMEOUT:
            {
                uint64_t framePts = *(uint64_t*)pEventData;
                printf("Be careful! detect encode timeout, pts[%lld]us", framePts);
                break;
            }
            case MPP_EVENT_LINKAGE_ISP2VE_PARAM_EXTRA:
            {
                VENC_Isp2VeExtraParam *pExtraParam = (VENC_Isp2VeExtraParam *)pEventData;
                pExtraParam->eEnCameraMove = CAMERA_ADAPTIVE_STATIC;
                break;
            }
            default:
            {
                break;
            }
        }
    }
    else if(MOD_ID_MUX == pChn->mModId)
    {
        switch(event)
        {
            case MPP_EVENT_RECORD_DONE:
            {
                message_t stCmdMsg;
                InitMessage(&stCmdMsg);
                mpp_camera_para_conf_data stMsgData;

                printf("MuxChn[%d] record file done.", *(int*)pEventData);
                stMsgData.MppCameraParaConfData = (mpp_camera_para_conf*)cookie;
                stCmdMsg.command = Rec_FileDone;
                stCmdMsg.para0 = *(int*)pEventData;
                stCmdMsg.mDataSize = sizeof(mpp_camera_para_conf_data);
                stCmdMsg.mpData = &stMsgData;

                putMessageWithData(&pContext->mMsgQueue, &stCmdMsg);
                break;
            }
            case MPP_EVENT_NEED_NEXT_FD:
            {
                message_t stMsgCmd;
                InitMessage(&stMsgCmd);
                mpp_camera_para_conf_data stMessageData;

                printf("MuxChn[%d] need next fd.\n", *(int*)pEventData);
                stMessageData.MppCameraParaConfData = (mpp_camera_para_conf*)cookie;
                stMsgCmd.command = Rec_NeedSetNextFd;
                stMsgCmd.para0 = *(int *)pEventData;
                stMsgCmd.mDataSize = sizeof(mpp_camera_para_conf_data);
                stMsgCmd.mpData = &stMessageData;

                putMessageWithData(&pContext->mMsgQueue, &stMsgCmd);
                break;
            }
            case MPP_EVENT_BSFRAME_AVAILABLE:
            {
                printf("mux bs frame available\n");
                break;
            }
            default:
            {
                printf("fatal error! unknown event[0x%x]\n", event);
                break;
            }
        }
    }
    return SUCCESS;
}

static ERRORTYPE MPPCallbackWrapper1(void *cookie, MPP_CHN_S *pChn, MPP_EVENT_TYPE event, void *pEventData)
{
    printf("%s:%d\n",__func__,__LINE__);
    ERRORTYPE ret = SUCCESS;
    mpp_camera_para_conf *pContext = (mpp_camera_para_conf *)cookie;
    if(MOD_ID_VENC == pChn->mModId)
    {
        VENC_CHN mVEncChn = pChn->mChnId;
        switch(event)
        {
            case MPP_EVENT_RELEASE_VIDEO_BUFFER:
            {
                printf("jpeg encoder MPP_EVENT_RELEASE_VIDEO_BUFFER\n");
            }
            case MPP_EVENT_VENC_BUFFER_FULL:
            {

                printf("jpeg encoder chn[%d] vbvBuffer full\n", pChn->mChnId);
                break;
            }
            default:
            {
                printf("fatal error! unknown event[0x%x] from channel[0x%x][0x%x][0x%x]!\n", event, pChn->mModId, pChn->mDevId, pChn->mChnId);
                ret = ERR_VENC_ILLEGAL_PARAM;
                break;
            }
        }
    }
    else
    {
        printf("fatal error! why modId[0x%x]?\n", pChn->mModId);
        ret = FAILURE;
    }
    return ret;
}

static void configViAttr(mpp_camera_para_conf *pContext, VI_ATTR_S *pViAttr)
{
    pViAttr->type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    pViAttr->memtype = V4L2_MEMORY_MMAP;
    pViAttr->format.pixelformat = map_PIXEL_FORMAT_E_to_V4L2_PIX_FMT(pContext->m_vi.mPixFmt);
    pViAttr->format.field = V4L2_FIELD_NONE;
    pViAttr->format.colorspace = pContext->m_vi.mColorSpace;
    pViAttr->format.width = pContext->m_vi.mWidth;
    pViAttr->format.height = pContext->m_vi.mHeight;
    pViAttr->nbufs = pContext->m_vi.mViBufferNum;
    pViAttr->nplanes = 2;
    pViAttr->fps = pContext->m_vi.mFrameRate;
    pViAttr->use_current_win = 0;
    pViAttr->wdr_mode = pContext->m_vi.mEnableWDR;
    pViAttr->capturemode = V4L2_MODE_VIDEO;
    pViAttr->drop_frame_num = pContext->m_vi.mViDropFrameNum;
    pViAttr->mbEncppEnable = TRUE;
}

static ERRORTYPE createViChn(mpp_camera_para_conf *pContext, VI_DEV videv, VI_CHN vichn){
    printf("%s:%d\n",__func__,__LINE__);
    ERRORTYPE ret;
    pContext->m_vi.mViDev = videv;
    if(videv == 0){
        pContext->mIspDev = 0;
    }else{
        pContext->mIspDev = 4;
    }
    pContext->m_vi.mViChn = vichn;

    ret = AW_MPI_VI_CreateVipp(pContext->m_vi.mViDev);
    if (ret != SUCCESS)
    {
        printf("fatal error! AW_MPI_VI CreateVipp failed\n");
    }

    ret = AW_MPI_VI_GetVippAttr(pContext->m_vi.mViDev, &pContext->m_vi.mViAttr);
    if (ret != SUCCESS)
    {
        printf("fatal error! AW_MPI_VI GetVippAttr failed\n");
    }

    memset(&pContext->m_vi.mViAttr, 0, sizeof(VI_ATTR_S));  
    configViAttr(pContext, &pContext->m_vi.mViAttr);

    ret = AW_MPI_VI_SetVippAttr(pContext->m_vi.mViDev, &pContext->m_vi.mViAttr);
    if (ret != SUCCESS)
    {
        printf("fatal error! AW_MPI_VI SetVippAttr failed\n");
    }

    MPPCallbackInfo cbInfo;
    cbInfo.cookie = (void*)pContext;
    cbInfo.callback = (MPPCallbackFuncType)&MPPCallbackWrapper;
    AW_MPI_VI_RegisterCallback(pContext->m_vi.mViDev, &cbInfo);

 #if ISP_RUN
    if (pContext->mIspDev >= 0)
    {
        ret = AW_MPI_ISP_Run(pContext->mIspDev);
        if (SUCCESS != ret)
        {
            if(pContext->mIspDev != 4){
                printf("fatal error! ISP[%d] init fail!", pContext->mIspDev);
                return -1;
            }else{
                 printf("fatal error! ISP[%d] init fail!, don't return", pContext->mIspDev);   
            }
        }
    }
#endif  

    ret = AW_MPI_VI_CreateVirChn(pContext->m_vi.mViDev, pContext->m_vi.mViChn, NULL);
    if (ret != SUCCESS)
    {
        printf("fatal error! createVirChn[%d] fail!\n", pContext->m_vi.mViChn);
    }    

    ret = AW_MPI_VI_EnableVipp(pContext->m_vi.mViDev);
    if (ret != SUCCESS)
    {
        printf("fatal error! enableVipp fail!\n");
    } 
#if ISP_RUN
    if(pContext->m_vi.mViDev == 8){
        AW_MPI_VI_SetVippMirror(pContext->m_vi.mViDev, (int)g_sys_Data.rearCameraImage);//水平镜像：0：不镜像 1：镜像 
    }  
    // AW_MPI_VI_SetVippFlip(pContext->m_vi.mViDev, pContext->m_vi.FlipMirror);//垂直镜像：0,1
#endif
    
#ifndef AUTO_GETDATA
    pContext->mExitFlag = 0;
    ret = pthread_create(&pContext->mCSIFrameThreadId, NULL, GetCSIFrameThread, pContext);
    if (ret != 0)
    {
        printf("fatal error! create GetCSIFrame Thread fail[%d]\n", ret);
        return ret;
    }
    else
    {
        printf("create GetCSIFrame Thread success! threadId[0x%x]\n", &pContext->mCSIFrameThreadId);
    }
#endif

    ret = AW_MPI_VI_EnableVirChn(pContext->m_vi.mViDev, pContext->m_vi.mViChn);
    if (ret != SUCCESS)
    {
        printf("Vir enable error!\n");
        return ret;
    }
    return ret;
}

static ERRORTYPE createVoChn(mpp_camera_para_conf *pContext,VO_DEV vodev,VO_CHN vochn){
    printf("%s:%d\n",__func__,__LINE__);
    if(pContext->m_vi.mViDev < 0){
        printf("invalid camera dev!!!\n");
        return -1;
    }    
    int hlay0 = 0;
    int result = SUCCESS;    
    pContext->m_vo.mVoDev = vodev;
    pContext->m_vo.mVoChn = vochn;
    AW_MPI_VO_Enable(pContext->m_vo.mVoDev);
    pContext->m_vo.mUILayer = HLAY(2,0);
    AW_MPI_VO_AddOutsideVideoLayer(pContext->m_vo.mUILayer);
    AW_MPI_VO_CloseVideoLayer(pContext->m_vo.mUILayer); /* close ui layer. */
    VO_PUB_ATTR_S spPubAttr;
    AW_MPI_VO_GetPubAttr(pContext->m_vo.mVoDev, &spPubAttr);
    spPubAttr.enIntfType = pContext->m_vo.mDispType;
    spPubAttr.enIntfSync = pContext->m_vo.mDispSync;

    AW_MPI_VO_SetPubAttr(pContext->m_vo.mVoDev, &spPubAttr);
    hlay0 = 0;
    while(hlay0 < VO_MAX_LAYER_NUM)
    {
        if(SUCCESS == AW_MPI_VO_EnableVideoLayer(hlay0))
        {
            break;
        }
        hlay0++;
    }
    if(hlay0 >= VO_MAX_LAYER_NUM)
    {
        printf("fatal error! enable video layer fail!\n");
    }

    pContext->m_vo.mVoLayer = hlay0;
    AW_MPI_VO_GetVideoLayerAttr(pContext->m_vo.mVoLayer, &pContext->m_vo.mLayerAttr);
    pContext->m_vo.mLayerAttr.stDispRect.X = pContext->m_vo.mDisplayX;
    pContext->m_vo.mLayerAttr.stDispRect.Y = pContext->m_vo.mDisplayY;
    pContext->m_vo.mLayerAttr.stDispRect.Width = pContext->m_vo.mWidth;
    pContext->m_vo.mLayerAttr.stDispRect.Height = pContext->m_vo.mHeight;
    pContext->m_vo.mLayerAttr.mDispFrmRt = 25;
    pContext->m_vo.mLayerAttr.bDoubleFrame = true;
    pContext->m_vo.mLayerAttr.bClusterMode = false;
    AW_MPI_VO_SetVideoLayerAttr(pContext->m_vo.mVoLayer, &pContext->m_vo.mLayerAttr);
    BOOL bSuccessFlag = FALSE;
    while(pContext->m_vo.mVoChn < VO_MAX_CHN_NUM)
    {
        int eRet = AW_MPI_VO_CreateChn(pContext->m_vo.mVoLayer, pContext->m_vo.mVoChn);
        if(SUCCESS == eRet)
        {
            bSuccessFlag = TRUE;
            printf("create vo channel[%d] success!\n", pContext->m_vo.mVoChn);
            break;
        }
        else if(ERR_VO_CHN_NOT_DISABLE == eRet)
        {
            printf("vo channel[%d] is exist, find next!\n", pContext->m_vo.mVoChn);
            pContext->m_vo.mVoChn++;
        }
        else
        {
            printf("fatal error! create vo channel[%d] ret[0x%x]!\n", pContext->m_vo.mVoChn, eRet);
            break;
        }
    }
    if(FALSE == bSuccessFlag)
    {
        pContext->m_vo.mVoChn = MM_INVALID_CHN;
        printf("fatal error! create vo channel fail!\n");
        result = FAILURE;
    }
    MPPCallbackInfo cbInfo;
    cbInfo.cookie = (void*)pContext;
    cbInfo.callback = (MPPCallbackFuncType)&MPPCallbackWrapper;
    AW_MPI_VO_RegisterCallback(pContext->m_vo.mVoLayer, pContext->m_vo.mVoChn, &cbInfo);
    AW_MPI_VO_SetChnDispBufNum(pContext->m_vo.mVoLayer, pContext->m_vo.mVoChn, 2);

    return result;
}

// 16x32 ASCII 字体（包含数字和基本符号）
static unsigned char font_16x32[][64] = {
    // 0 - 16x32
    {0x00, 0x00, 0x00, 0x00, 0x07, 0xE0, 0x1F, 0xF8, 0x3F, 0xFC, 0x7C, 0x3E, 0x78, 0x1E, 0xF0, 0x0F,
     0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F,
     0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0x78, 0x1E, 0x7C, 0x3E,
     0x3F, 0xFC, 0x1F, 0xF8, 0x07, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // 1 - 16x32
    {0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x07, 0x00, 0x0F, 0x00, 0x1F, 0x00, 0x3F, 0x00, 0x7F, 0x00,
     0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00,
     0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00,
     0x03, 0x00, 0x03, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // 2 - 16x32
    {0x00, 0x00, 0x00, 0x00, 0x0F, 0xE0, 0x3F, 0xF8, 0x7F, 0xFC, 0xFC, 0x3E, 0xF8, 0x1E, 0x00, 0x0F,
     0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0E, 0x00, 0x1E, 0x00, 0x3C, 0x00, 0x78, 0x00, 0xF0, 0x01, 0xE0,
     0x03, 0xC0, 0x07, 0x80, 0x0F, 0x00, 0x1E, 0x00, 0x3C, 0x00, 0x78, 0x00, 0xF0, 0x00, 0xF0, 0x00,
     0xF0, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // 3 - 16x32
    {0x00, 0x00, 0x00, 0x00, 0x0F, 0xE0, 0x3F, 0xF8, 0x7F, 0xFC, 0xFC, 0x3E, 0xF8, 0x1E, 0x00, 0x0F,
     0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0E, 0x00, 0x1C, 0x07, 0xF8, 0x07, 0xF8, 0x00, 0x1C, 0x00, 0x0E,
     0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0xF8, 0x1E, 0xFC, 0x3E,
     0x7F, 0xFC, 0x3F, 0xF8, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // 4 - 16x32
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0xF8, 0x01, 0xF8, 0x03, 0xF8, 0x07, 0xF8, 0x0F, 0x78,
     0x1E, 0x78, 0x3C, 0x78, 0x78, 0x78, 0xF0, 0x78, 0xE0, 0x78, 0xC0, 0x78, 0xC0, 0x78, 0xC0, 0x78,
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x78, 0x00, 0x78, 0x00, 0x78, 0x00, 0x78, 0x00, 0x78,
     0x00, 0x78, 0x00, 0x78, 0x00, 0x78, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // 5 - 16x32
    {0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x00, 0xF0, 0x00, 0xF0, 0x00,
     0xF0, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0xFF, 0xE0, 0xFF, 0xF8, 0xFF, 0xFC, 0x00, 0x3E, 0x00, 0x1E,
     0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x1E, 0xF8, 0x3E,
     0xFF, 0xFC, 0x7F, 0xF8, 0x1F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // 6 - 16x32
    {0x00, 0x00, 0x00, 0x00, 0x07, 0xE0, 0x1F, 0xF8, 0x3F, 0xFC, 0x7C, 0x3E, 0x78, 0x1E, 0xF0, 0x0F,
     0xF0, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0xFF, 0xE0, 0xFF, 0xF8, 0xFF, 0xFC, 0xF0, 0x3E, 0xF0, 0x1E,
     0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0x78, 0x1E, 0x7C, 0x3E,
     0x3F, 0xFC, 0x1F, 0xF8, 0x07, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // 7 - 16x32
    {0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x0F, 0x00, 0x1E, 0x00, 0x3C,
     0x00, 0x78, 0x00, 0xF0, 0x01, 0xE0, 0x01, 0xC0, 0x03, 0xC0, 0x07, 0x80, 0x07, 0x00, 0x0F, 0x00,
     0x0F, 0x00, 0x0F, 0x00, 0x1E, 0x00, 0x1E, 0x00, 0x1E, 0x00, 0x1E, 0x00, 0x3C, 0x00, 0x3C, 0x00,
     0x3C, 0x00, 0x3C, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // 8 - 16x32
    {0x00, 0x00, 0x00, 0x00, 0x07, 0xE0, 0x1F, 0xF8, 0x3F, 0xFC, 0x7C, 0x3E, 0x78, 0x1E, 0xF0, 0x0F,
     0xF0, 0x0F, 0xF0, 0x0F, 0x78, 0x1E, 0x7C, 0x3E, 0x3F, 0xFC, 0x1F, 0xF8, 0x7C, 0x3E, 0x78, 0x1E,
     0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0x78, 0x1E, 0x7C, 0x3E,
     0x3F, 0xFC, 0x1F, 0xF8, 0x07, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // 9 - 16x32
    {0x00, 0x00, 0x00, 0x00, 0x07, 0xE0, 0x1F, 0xF8, 0x3F, 0xFC, 0x7C, 0x3E, 0x78, 0x1E, 0xF0, 0x0F,
     0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x1E, 0x78, 0x3E, 0x7F, 0xFE, 0x3F, 0xFC,
     0x0F, 0xFC, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x78, 0x1E, 0x7C, 0x3E,
     0x3F, 0xFC, 0x1F, 0xF8, 0x07, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    
    // 横线 (-) - 索引10
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // 视觉上：在第16-19行中间显示一条横线
    
    // 冒号 (:) - 索引11
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x07, 0x80, 0x07, 0x80, 0x07, 0x80, 0x07, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x07, 0x80, 0x07, 0x80, 0x07, 0x80, 0x07, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // 视觉上：在第12-13行和第20-21行各显示一个点
    
    // 空格 ( ) - 索引12
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
     
    // 点 (.) - 索引13
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x07, 0x80, 0x07, 0x80, 0x07, 0x80, 0x07, 0x80, 0x00, 0x00, 0x00, 0x00},
     
    // 斜杠 (/) - 索引14
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x1E, 0x00, 0x3C, 0x00, 0x78,
     0x00, 0xF0, 0x01, 0xE0, 0x03, 0xC0, 0x07, 0x80, 0x0F, 0x00, 0x1E, 0x00, 0x3C, 0x00, 0x78, 0x00,
     0xF0, 0x00, 0xE0, 0x01, 0xC0, 0x03, 0x80, 0x07, 0x00, 0x0F, 0x00, 0x1E, 0x00, 0x3C, 0x00, 0x78,
     0x00, 0xF0, 0x00, 0xE0, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
};

// 获取字符索引
static int get_char_index(char c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    switch (c) {
        case ':': return 11;
        case ' ': return 12;
        case '-': return 10;
        case '.': return 13;
        case '/': return 14;
        default: return 12; 
    }
}

static void draw_char_to_bitmap(unsigned char *bitmap, int x, int y, int bmp_width, int bmp_height, char c) {
    int char_idx = get_char_index(c);
    unsigned char *font_data = font_16x32[char_idx];
    
    for (int row = 0; row < 32; row++) {
        // 每行2个字节，共16位
        unsigned char byte1 = font_data[row * 2];
        unsigned char byte2 = font_data[row * 2 + 1];
        
        // 绘制第一个字节的8位
        for (int col = 0; col < 8; col++) {
            int pixel_x = x + col;
            int pixel_y = y + row;
            
            if (pixel_x >= bmp_width || pixel_y >= bmp_height)
                continue;
                
            int pixel_index = (pixel_y * bmp_width + pixel_x) * 4; // ARGB8888
            
            if (byte1 & (0x80 >> col)) {
                bitmap[pixel_index + 0] = 0xFF;   // B
                bitmap[pixel_index + 1] = 0xFF;   // G
                bitmap[pixel_index + 2] = 0xFF;   // R
                bitmap[pixel_index + 3] = 0xFF;   // A (不透明)
            } else {
                bitmap[pixel_index + 0] = 0x00;   // B
                bitmap[pixel_index + 1] = 0x00;   // G
                bitmap[pixel_index + 2] = 0x00;   // R
                bitmap[pixel_index + 3] = 0x00;   // A (透明)
            }
        }
        
        // 绘制第二个字节的8位
        for (int col = 8; col < 16; col++) {
            int pixel_x = x + col;
            int pixel_y = y + row;
            
            if (pixel_x >= bmp_width || pixel_y >= bmp_height)
                continue;
                
            int pixel_index = (pixel_y * bmp_width + pixel_x) * 4; // ARGB8888
            
            if (byte2 & (0x80 >> (col - 8))) {
                bitmap[pixel_index + 0] = 0xFF;   // B
                bitmap[pixel_index + 1] = 0xFF;   // G
                bitmap[pixel_index + 2] = 0xFF;   // R
                bitmap[pixel_index + 3] = 0xFF;   // A (不透明)
            } else {
                bitmap[pixel_index + 0] = 0x00;   // B
                bitmap[pixel_index + 1] = 0x00;   // G
                bitmap[pixel_index + 2] = 0x00;   // R
                bitmap[pixel_index + 3] = 0x00;   // A (透明)
            }
        }
    }
}

static void draw_time_string_to_bitmap(unsigned char *bitmap, int bmp_width, int bmp_height) {
    char time_str[32];
    TTime timeinfo;
    RtcGetTime2C(&timeinfo);
    snprintf(time_str, sizeof(time_str), "%04d-%02d-%02d %02d:%02d:%02d",
             timeinfo.nYear,
             timeinfo.nMonth,
             timeinfo.nDay,
             timeinfo.nHour,
             timeinfo.nMinute,
             timeinfo.nSecond);
    
    // 计算字符串总宽度（16x32字体，每个字符16像素宽）
    int str_len = strlen(time_str);
    int str_pixel_width = str_len * 16;
    
    // 计算居中位置
    int start_x = (bmp_width - str_pixel_width) / 2;
    int start_y = (bmp_height - 32) / 2;
    
    if (start_x < 0) start_x = 0;
    if (start_y < 0) start_y = 0;
    
    // 绘制字符串
    for (int i = 0; time_str[i] != '\0'; i++) {
        draw_char_to_bitmap(bitmap, start_x + i * 16, start_y, 
                           bmp_width, bmp_height, time_str[i]);
    }
}

static void GenerateTimestampARGB8888(unsigned char *bitmap, int size, int width, int height) {
    if (!bitmap) return;
    
    // 清除位图（设置为全透明）
    for (int i = 0; i < size; i += 4) {
        bitmap[i + 0] = 0x00;   // B
        bitmap[i + 1] = 0x00;   // G
        bitmap[i + 2] = 0x00;   // R
        bitmap[i + 3] = 0x00;   // A (透明)
    }
    
    draw_time_string_to_bitmap(bitmap, width, height);
}

static void *update_timestamp_thread(void *arg) {
    printf("Timestamp update thread started\n");
    mpp_camera_para_conf *pContext = (mpp_camera_para_conf *)arg;
    while (pContext->m_venc.upTimeInfo) {        
        int bmp_size = pContext->m_venc.overlay_x * pContext->m_venc.overlay_y * 4;
        unsigned char *bitmap = malloc(bmp_size);
        
        if (bitmap) {
            GenerateTimestampARGB8888(bitmap, bmp_size, pContext->m_venc.overlay_x, pContext->m_venc.overlay_y);
            
            BITMAP_S bmp;
            memset(&bmp, 0, sizeof(BITMAP_S));
            bmp.mPixelFormat = MM_PIXEL_FORMAT_RGB_8888;
            bmp.mWidth = pContext->m_venc.overlay_x;
            bmp.mHeight = pContext->m_venc.overlay_y;                
            bmp.mpData = bitmap;

            s32 ret = AW_MPI_RGN_SetBitMap(pContext->m_venc.mOverlayHandle + 1, &bmp);
            if (ret != SUCCESS) {
                printf("Failed to update timestamp bitmap: 0x%x\n", ret);
            }
            
            free(bitmap);
        }                
        sleep(1);
    }
    
    printf("Timestamp update thread stopped\n");
    return NULL;
}

// 清理函数
static ERRORTYPE destroyTimestampOverlay(mpp_camera_para_conf *pContext) {
    if (!pContext) {
        return FAILURE;
    }
      
    if (pContext->m_venc.upTimeInfo) {
        pContext->m_venc.upTimeInfo = 0;
        if (pContext->m_venc.upTimeInfoId) {
            pthread_join(pContext->m_venc.upTimeInfoId, NULL);
            pContext->m_venc.upTimeInfoId = 0;
        }
    }
    
   
    // if (pContext->m_venc.mOverlayHandle != 0) {

        MPP_CHN_S VeChn = {MOD_ID_VENC, 0, pContext->m_venc.mVEncChn};
        AW_MPI_RGN_DetachFromChn(pContext->m_venc.mOverlayHandle + 1, &VeChn);
        AW_MPI_RGN_Destroy(pContext->m_venc.mOverlayHandle + 1);
        
        pContext->m_venc.mOverlayHandle = 0;
        
        printf("Timestamp overlay destroyed\n");
    // }
    
    return SUCCESS;
}

static ERRORTYPE setOverlayToVencChn(mpp_camera_para_conf *pContext){
    printf("%s:%d\n",__func__,__LINE__);
    pContext->m_venc.overlay_x = 368;
    pContext->m_venc.overlay_y = 80;
    //add to venchn 
    RGN_ATTR_S stRegion;
    BITMAP_S stBmp;
    int nSize = 0;
    RGN_CHN_ATTR_S stRgnChnAttr ;
    memset(&stRegion, 0, sizeof(RGN_ATTR_S));
    stRegion.enType = OVERLAY_RGN;
    stRegion.unAttr.stOverlay.mPixelFmt = MM_PIXEL_FORMAT_RGB_8888;   //MM_PIXEL_FORMAT_RGB_8888;
    stRegion.unAttr.stOverlay.mSize.Width = pContext->m_venc.overlay_x;
    stRegion.unAttr.stOverlay.mSize.Height = pContext->m_venc.overlay_y;
    pContext->m_venc.mOverlayHandle = 0;
    AW_MPI_RGN_Create(pContext->m_venc.mOverlayHandle + 1, &stRegion);
    
    memset(&stBmp, 0, sizeof(BITMAP_S));
    stBmp.mPixelFormat = stRegion.unAttr.stOverlay.mPixelFmt;
    stBmp.mWidth = stRegion.unAttr.stOverlay.mSize.Width;
    stBmp.mHeight = stRegion.unAttr.stOverlay.mSize.Height;
    nSize = BITMAP_S_GetdataSize(&stBmp);
    stBmp.mpData = malloc(nSize);
    if(NULL == stBmp.mpData)        
    {            
        printf("fatal error! malloc fail!\n");        
    }

    GenerateTimestampARGB8888(stBmp.mpData, nSize,pContext->m_venc.overlay_x,pContext->m_venc.overlay_y);
    AW_MPI_RGN_SetBitMap(pContext->m_venc.mOverlayHandle + 1, &stBmp);
    free(stBmp.mpData);

    stRgnChnAttr.bShow = false;
    stRgnChnAttr.enType = stRegion.enType;
    stRgnChnAttr.unChnAttr.stOverlayChn.stPoint.X = 16;
    stRgnChnAttr.unChnAttr.stOverlayChn.stPoint.Y = AWALIGN((1080-pContext->m_venc.overlay_y),16);
    stRgnChnAttr.unChnAttr.stOverlayChn.mLayer = 0;
    stRgnChnAttr.unChnAttr.stOverlayChn.mFgAlpha = 0x40; // global alpha mode value for ARGB1555
    stRgnChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.Width = 16;
    stRgnChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.Height = 16;
    stRgnChnAttr.unChnAttr.stOverlayChn.stInvertColor.mLumThresh = 60;
    stRgnChnAttr.unChnAttr.stOverlayChn.stInvertColor.enChgMod = LESSTHAN_UNIT_LUMDIFF_THRESH;
    stRgnChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = false;
    printf("overlay attach to ve\n");
    MPP_CHN_S VeChn = {MOD_ID_VENC, 0, pContext->m_venc.mVEncChn};
    AW_MPI_RGN_AttachToChn(pContext->m_venc.mOverlayHandle + 1,  &VeChn, &stRgnChnAttr);
    printf("overlay attach to ve done\n");

    int thread_ret = pthread_create(&pContext->m_venc.upTimeInfoId, NULL, update_timestamp_thread, (void *)pContext);
    pContext->m_venc.upTimeInfo = 1;
    if(thread_ret != 0){
        printf("fail to create update_timestamp_thread!!!\n");
    }
    return SUCCESS; 
}

static ERRORTYPE configVencChnAttr(mpp_camera_para_conf *pContext, VENC_CHN_ATTR_S *pVencChnAttr, VENC_RC_PARAM_S *pVencRcParam)
{
    printf("%s:%d\n",__func__,__LINE__);
    pVencChnAttr->VeAttr.Type = pContext->m_venc.mEncoderFmt;

    switch(pVencChnAttr->VeAttr.Type)
    {
        case PT_H264:
        {
            pVencChnAttr->VeAttr.AttrH264e.mThreshSize = AWALIGN((pContext->m_venc.mWidth*pContext->m_venc.mHeight*3/2)/3, 1024);
            pVencChnAttr->VeAttr.AttrH264e.BufSize = AWALIGN(pContext->m_venc.mBitRate*4/8 + pVencChnAttr->VeAttr.AttrH264e.mThreshSize, 1024);
            pVencChnAttr->VeAttr.AttrH264e.Profile = 2;//0:base 1:main 2:high
            // pVencChnAttr->VeAttr.AttrH264e.Profile = 0;//0:base 1:main 2:high
            pVencChnAttr->VeAttr.AttrH264e.bByFrame = TRUE;
            pVencChnAttr->VeAttr.AttrH264e.PicWidth  = pContext->m_venc.mWidth;
            pVencChnAttr->VeAttr.AttrH264e.PicHeight = pContext->m_venc.mHeight;
            pVencChnAttr->VeAttr.AttrH264e.mLevel = H264_LEVEL_51;
            pVencChnAttr->VeAttr.AttrH264e.FastEncFlag = FALSE;
            // pVencChnAttr->VeAttr.AttrH264e.FastEncFlag = TRUE;
            pVencChnAttr->VeAttr.AttrH264e.IQpOffset = 0;
            pVencChnAttr->VeAttr.AttrH264e.mbPIntraEnable = TRUE;
            break;
        }
        case PT_H265:
        {
            pVencChnAttr->VeAttr.AttrH265e.mThreshSize = AWALIGN((pContext->m_venc.mWidth*pContext->m_venc.mHeight*3/2)/3, 1024);
            pVencChnAttr->VeAttr.AttrH265e.mBufSize = AWALIGN(pContext->m_venc.mBitRate*4/8 + pVencChnAttr->VeAttr.AttrH264e.mThreshSize, 1024);
            pVencChnAttr->VeAttr.AttrH265e.mProfile = 0; //0:main 1:main10 2:sti11
            pVencChnAttr->VeAttr.AttrH265e.mbByFrame = TRUE;
            pVencChnAttr->VeAttr.AttrH265e.mPicWidth = pContext->m_venc.mWidth;
            pVencChnAttr->VeAttr.AttrH265e.mPicHeight = pContext->m_venc.mHeight;
            pVencChnAttr->VeAttr.AttrH265e.mLevel = H265_LEVEL_62;
            pVencChnAttr->VeAttr.AttrH265e.mFastEncFlag = FALSE;
            pVencChnAttr->VeAttr.AttrH265e.IQpOffset = 0;
            pVencChnAttr->VeAttr.AttrH265e.mbPIntraEnable = TRUE;
            break;
        }
        case PT_MJPEG:
        {
            pVencChnAttr->VeAttr.AttrMjpeg.mbByFrame = TRUE;
            pVencChnAttr->VeAttr.AttrMjpeg.mPicWidth = pContext->m_venc.mWidth;
            pVencChnAttr->VeAttr.AttrMjpeg.mPicHeight = pContext->m_venc.mHeight;
            break;
        }
        default:
        {
            printf("fatal error! not support encode type[%d], check code!\n", pVencChnAttr->VeAttr.Type);
            break;
        }
    }
    pVencChnAttr->VeAttr.SrcPicWidth = pContext->m_vi.mWidth;
    pVencChnAttr->VeAttr.SrcPicHeight = pContext->m_vi.mHeight;
    pVencChnAttr->VeAttr.Field = VIDEO_FIELD_FRAME;
    pVencChnAttr->VeAttr.PixelFormat = pContext->m_vi.mPixFmt;
    pVencChnAttr->VeAttr.mColorSpace = V4L2_COLORSPACE_JPEG;
    pVencChnAttr->VeAttr.Rotate = ROTATE_NONE;
    pVencChnAttr->VeAttr.MaxKeyInterval = 25;
    pVencChnAttr->EncppAttr.eEncppSharpSetting = VencEncppSharp_FollowISPConfig;

    switch(pVencChnAttr->VeAttr.Type)
    {
        case PT_H264:
        {
            switch (pContext->m_venc.mRcMode)
            {
                case 1:
                {
                    pVencChnAttr->RcAttr.mRcMode = VENC_RC_MODE_H264VBR;
                    pVencChnAttr->RcAttr.mAttrH264Vbr.mMaxBitRate = pContext->m_venc.mBitRate;
                    pVencRcParam->ParamH264Vbr.mMaxQp = 51;
                    pVencRcParam->ParamH264Vbr.mMinQp = 10;
                    pVencRcParam->ParamH264Vbr.mMaxPqp = 50;
                    pVencRcParam->ParamH264Vbr.mMinPqp = 10;
                    pVencRcParam->ParamH264Vbr.mQpInit = 38;
                    pVencRcParam->ParamH264Vbr.mMovingTh = 20;
                    pVencRcParam->ParamH264Vbr.mQuality = 10;
                    break;
                }
                case 2:
                {
                    pVencChnAttr->RcAttr.mRcMode = VENC_RC_MODE_H264FIXQP;
                    pVencChnAttr->RcAttr.mAttrH264FixQp.mIQp = 28;
                    pVencChnAttr->RcAttr.mAttrH264FixQp.mPQp = 28;
                    break;
                }
                case 3:
                {
                    pVencChnAttr->RcAttr.mRcMode = VENC_RC_MODE_H264ABR;
                    pVencChnAttr->RcAttr.mAttrH264Abr.mMaxBitRate = pContext->m_venc.mBitRate;
                    pVencChnAttr->RcAttr.mAttrH264Abr.mRatioChangeQp = 85;
                    pVencChnAttr->RcAttr.mAttrH264Abr.mQuality = 8;
                    pVencChnAttr->RcAttr.mAttrH264Abr.mMinIQp = 20;
                    pVencChnAttr->RcAttr.mAttrH264Abr.mMaxQp = 51;
                    pVencChnAttr->RcAttr.mAttrH264Abr.mMinQp = 10;
                    break;
                }
                case 0:
                default:
                {
                    pVencChnAttr->RcAttr.mRcMode = VENC_RC_MODE_H264CBR;
                    pVencChnAttr->RcAttr.mAttrH264Cbr.mBitRate = pContext->m_venc.mBitRate;
                    pVencRcParam->ParamH264Cbr.mMaxQp = 35;
                    pVencRcParam->ParamH264Cbr.mMinQp = 10;
                    pVencRcParam->ParamH264Cbr.mMaxPqp = 45;
                    pVencRcParam->ParamH264Cbr.mMinPqp = 10;
                    pVencRcParam->ParamH264Cbr.mQpInit = 45;
                    break;
                }
            }
            break;
        }
        case PT_H265:
        {
            switch (pContext->m_venc.mRcMode)
            {
                case 1:
                {
                    pVencChnAttr->RcAttr.mRcMode = VENC_RC_MODE_H265VBR;
                    pVencChnAttr->RcAttr.mAttrH265Vbr.mMaxBitRate = pContext->m_venc.mBitRate;
                    pVencRcParam->ParamH265Vbr.mMaxQp = 51;
                    pVencRcParam->ParamH265Vbr.mMinQp = 10;
                    pVencRcParam->ParamH265Vbr.mMaxPqp = 50;
                    pVencRcParam->ParamH265Vbr.mMinPqp = 10;
                    pVencRcParam->ParamH265Vbr.mQpInit = 38;
                    pVencRcParam->ParamH265Vbr.mMovingTh = 20;
                    pVencRcParam->ParamH265Vbr.mQuality = 10;
                    break;
                }
                case 2:
                {
                    pVencChnAttr->RcAttr.mRcMode = VENC_RC_MODE_H265FIXQP;
                    pVencChnAttr->RcAttr.mAttrH265FixQp.mIQp = 28;
                    pVencChnAttr->RcAttr.mAttrH265FixQp.mPQp = 28;
                    break;
                }
                case 3:
                {
                    pVencChnAttr->RcAttr.mRcMode = VENC_RC_MODE_H265ABR;
                    pVencChnAttr->RcAttr.mAttrH265Abr.mMaxBitRate = pContext->m_venc.mBitRate;
                    pVencChnAttr->RcAttr.mAttrH265Abr.mRatioChangeQp = 85;
                    pVencChnAttr->RcAttr.mAttrH265Abr.mQuality = 8;
                    pVencChnAttr->RcAttr.mAttrH265Abr.mMinIQp = 20;
                    pVencChnAttr->RcAttr.mAttrH265Abr.mMaxQp = 51;
                    pVencChnAttr->RcAttr.mAttrH265Abr.mMinQp = 10;
                    break;
                }
                case 0:
                default:
                {
                    pVencChnAttr->RcAttr.mRcMode = VENC_RC_MODE_H265CBR;
                    pVencChnAttr->RcAttr.mAttrH265Cbr.mBitRate = pContext->m_venc.mBitRate;
                    pVencRcParam->ParamH265Cbr.mMaxQp = 51;
                    pVencRcParam->ParamH265Cbr.mMinQp = 1;
                    pVencRcParam->ParamH265Cbr.mMaxPqp = 50;
                    pVencRcParam->ParamH265Cbr.mMinPqp = 10;
                    pVencRcParam->ParamH265Cbr.mQpInit = 38;
                    break;
                }
            }
            break;
        }
        case PT_MJPEG:
        {
            if(pContext->m_venc.mRcMode != 0)
            {
                printf("fatal error! mjpeg don't support rcMode[%d]!\n", pContext->m_venc.mBitRate);
            }
            pVencChnAttr->RcAttr.mRcMode = VENC_RC_MODE_MJPEGCBR;
            pVencChnAttr->RcAttr.mAttrMjpegeCbr.mBitRate = pContext->m_venc.mBitRate;
            break;
        }
        default:
        {
            printf("fatal error! not support encode type[%d], check code!\n", pVencChnAttr->VeAttr.Type);
            break;
        }
    }

    pVencChnAttr->GopAttr.enGopMode = VENC_GOPMODE_NORMALP;
    printf("venc ste Rcmode=%d\n", pVencChnAttr->RcAttr.mRcMode);
    return SUCCESS;
}

static ERRORTYPE createVencChn(mpp_camera_para_conf *pContext)
{
    printf("%s:%d\n",__func__,__LINE__);
    if(pContext->m_vi.mViDev < 0){
        printf("invalid camera dev!!!\n");
        return -1;
    }
    ERRORTYPE ret;
    BOOL nSuccessFlag = FALSE;

    memset(&pContext->m_venc.mVEncAttr, 0, sizeof(VENC_CHN_ATTR_S));
    memset(&pContext->m_venc.mVencRcParam, 0, sizeof(VENC_RC_PARAM_S));

    configVencChnAttr(pContext, &pContext->m_venc.mVEncAttr, &pContext->m_venc.mVencRcParam);
    pContext->m_venc.mVEncChn = 0;

    while (pContext->m_venc.mVEncChn < VENC_MAX_CHN_NUM)
    {
        ret = AW_MPI_VENC_CreateChn(pContext->m_venc.mVEncChn, &pContext->m_venc.mVEncAttr);
        if (SUCCESS == ret)
        {
            nSuccessFlag = TRUE;
            printf("create venc channel[%d] success!\n", pContext->m_venc.mVEncChn);
            break;
        }
        else if (ERR_VENC_EXIST == ret)
        {
            printf("venc channel[%d] is exist, find next!\n", pContext->m_venc.mVEncChn);
            pContext->m_venc.mVEncChn++;
        }
        else
        {
            printf("create venc channel[%d] ret[0x%x], find next!\n", pContext->m_venc.mVEncChn, ret);
            pContext->m_venc.mVEncChn++;
        }
    }

    if (nSuccessFlag == FALSE)
    {
        pContext->m_venc.mVEncChn = MM_INVALID_CHN;
        printf("fatal error! create venc channel fail!\n");
        return FAILURE;
    }
    else
    {
        AW_MPI_VENC_SetRcParam(pContext->m_venc.mVEncChn, &pContext->m_venc.mVencRcParam);

        VENC_FRAME_RATE_S stFrameRate;
        stFrameRate.SrcFrmRate = pContext->m_vi.mFrameRate;
        stFrameRate.DstFrmRate = pContext->m_venc.mFrameRate;
        printf("set srcFrameRate:%d, venc framerate:%d\n", stFrameRate.SrcFrmRate, stFrameRate.DstFrmRate);
        ret = AW_MPI_VENC_SetFrameRate(pContext->m_venc.mVEncChn, &stFrameRate);
        if(ret != SUCCESS)
        {
            printf("fatal error! venc set framerate fail[0x%x]!\n", ret);
        }

        MPPCallbackInfo cbInfo;
        cbInfo.cookie = (void*)pContext;
        cbInfo.callback = (MPPCallbackFuncType)&MPPCallbackWrapper;
        AW_MPI_VENC_RegisterCallback(pContext->m_venc.mVEncChn, &cbInfo);

        printf("create venc chn[%d] success!\n", pContext->m_venc.mVEncChn);

        return SUCCESS;
    }
}

static ERRORTYPE configMuxChnAttr(mpp_camera_para_conf *pContext, MUX_CHN_ATTR_S *pMuxChnAttr)
{
    printf("%s:%d\n",__func__,__LINE__);
    memset(&pContext->m_mux.mMuxChnAttr, 0, sizeof(MUX_CHN_ATTR_S));

    pContext->m_mux.mMuxChnAttr.mVideoAttrValidNum = 1;
    pContext->m_mux.mMuxChnAttr.mVideoAttr[0].mVideoEncodeType = pContext->m_venc.mEncoderFmt;
    pContext->m_mux.mMuxChnAttr.mVideoAttr[0].mWidth = pContext->m_venc.mWidth;
    pContext->m_mux.mMuxChnAttr.mVideoAttr[0].mHeight = pContext->m_venc.mHeight;
    // pContext->m_mux.mMuxChnAttr.mVideoAttr[0].mWidth = 3840;
    // pContext->m_mux.mMuxChnAttr.mVideoAttr[0].mHeight = 2160;
    pContext->m_mux.mMuxChnAttr.mVideoAttr[0].mVideoFrmRate = pContext->m_venc.mFrameRate*1000;
    pContext->m_mux.mMuxChnAttr.mVideoAttr[0].mVeChn = pContext->m_venc.mVEncChn;
    
    pContext->m_mux.mMuxChnAttr.mTextEncodeType = PT_MAX;

    pContext->m_mux.mMuxChnAttr.mChannels = pContext->m_ai.mAIChnCnt;
    pContext->m_mux.mMuxChnAttr.mBitsPerSample = pContext->m_ai.mBitWidth;
    pContext->m_mux.mMuxChnAttr.mSamplesPerFrame = MAXDECODESAMPLE;
    pContext->m_mux.mMuxChnAttr.mSampleRate = pContext->m_ai.mSampleRate;
    pContext->m_mux.mMuxChnAttr.mAudioEncodeType = pContext->m_aenc.mCodecType;  

    pContext->m_mux.mMuxChnAttr.mMediaFileFormat = MEDIA_FILE_FORMAT_TS;
    recorder_time recorderTime;
    if(g_sys_Data.recorderMode == RECORDER_NORMAL){
        recorderTime = g_sys_Data.recorderTime;
    }else if(g_sys_Data.recorderMode == RECORDER_URGENT){
        recorderTime = g_sys_Data.recorderUrgentTime;
    }
    switch (recorderTime)
    {
    case RECORDER_1:
        /* code */
        pContext->m_mux.mMuxChnAttr.mMaxFileDuration = 60 *1000;
        break;
    case RECORDER_3:
        /* code */
        pContext->m_mux.mMuxChnAttr.mMaxFileDuration = 180 *1000;
        break;
    case RECORDER_5:
        /* code */
        pContext->m_mux.mMuxChnAttr.mMaxFileDuration = 300 *1000;
        break;        
    default:
        pContext->m_mux.mMuxChnAttr.mMaxFileDuration = 60 *1000;
        break;
    }
    pContext->m_mux.mMuxChnAttr.mCallbackOutFlag = false;
    pContext->m_mux.mMuxChnAttr.mFsWriteMode = FSWRITEMODE_DIRECT;
    pContext->m_mux.mMuxChnAttr.mSimpleCacheSize = DEFAULT_SIMPLE_CACHE_SIZE_VFS;


    return SUCCESS;
}

static ERRORTYPE createMuxChn(mpp_camera_para_conf *pContext)
{
    printf("%s:%d\n",__func__,__LINE__);
    ERRORTYPE ret;
    BOOL nSuccessFlag = FALSE;
    if(pContext->m_vi.mViDev < 0){
        printf("invalid camera dev!!!\n");
        return -1;
    }
    pthread_mutex_init(&pContext->m_mux.fsyncMutex, NULL);
    
    memset(&pContext->m_mux.mMuxChnAttr, 0, sizeof(MUX_CHN_ATTR_S));
    configMuxChnAttr(pContext, &pContext->m_mux.mMuxChnAttr);
    creatVideoPathAndPicPath();
    pContext->m_mux.eFileFormat = pContext->m_mux.mMuxChnAttr.mMediaFileFormat;
    pthread_mutex_lock(&pContext->m_mux.fsyncMutex);
    getFileNameByCurTime(pContext, pContext->m_mux.dstVideoFile);
    int nFd = open(pContext->m_mux.dstVideoFile, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (nFd < 0)
    {
        printf("fatal error! Failed to open %s\n", pContext->m_mux.dstVideoFile);
        // goto err_openfile;
    }  
    pthread_mutex_unlock(&pContext->m_mux.fsyncMutex);
    pContext->m_mux.mMuxChn = 0;
    while (pContext->m_mux.mMuxChn < MUX_MAX_CHN_NUM)
    {
        ret = AW_MPI_MUX_CreateChn(pContext->m_mux.mMuxChn, &pContext->m_mux.mMuxChnAttr, nFd, 0);
        if (SUCCESS == ret)
        {
            nSuccessFlag = TRUE;
            printf("create muxChn[%d] success!\n", pContext->m_mux.mMuxChn);
            break;
        }
        else if (ERR_MUX_EXIST == ret)
        {
            printf("muxChn[%d] is exist, find next!\n", pContext->m_mux.mMuxChn);
            pContext->m_mux.mMuxChn++;
        }
        else
        {
            printf("create muxChn[%d] ret[0x%x], find next!\n", pContext->m_mux.mMuxChn, ret);
            pContext->m_mux.mMuxChn++;
        }
    }

    if (FALSE == nSuccessFlag)
    {
        pContext->m_mux.mMuxChn = MM_INVALID_CHN;
        printf("fatal error! create mux channel fail!\n");
        return FAILURE;
    }
    else
    {
        MPPCallbackInfo cbInfo;
        cbInfo.cookie = (void*)pContext;
        cbInfo.callback = (MPPCallbackFuncType)&MPPCallbackWrapper;
        AW_MPI_MUX_RegisterCallback(pContext->m_mux.mMuxChn, &cbInfo);
        return SUCCESS;
    }
    if(nFd >= 0){
        close(nFd);
        nFd = -1;
    }
    return ret;
// err_openfile:

    return -1;
}


#ifdef TACKPIC
static ERRORTYPE createVenc_picChn(mpp_camera_para_conf *pContext){
    printf("%s:%d\n",__func__,__LINE__);
    int ret = 0;
    unsigned int nVbvBufSize = 0;
    unsigned int vbvThreshSize = 0;
    int mPictureNum = 1;
    pContext->m_venc_pic.mWidth = 1920;
    pContext->m_venc_pic.mHeight = 1080;

    unsigned int minVbvBufSize = pContext->m_venc_pic.mWidth * pContext->m_venc_pic.mHeight * 3/2;
    vbvThreshSize = pContext->m_venc_pic.mWidth * pContext->m_venc_pic.mHeight ;
    nVbvBufSize = (pContext->m_venc_pic.mWidth * pContext->m_venc_pic.mHeight * 3/2/10 * 1) + vbvThreshSize;
    if(nVbvBufSize < minVbvBufSize)
    {
        nVbvBufSize = minVbvBufSize;
    }
    if(nVbvBufSize > 16*1024*1024)
    {
        printf("Be careful! vbvSize[%d]MB is too large, decrease to threshSize[%d]MB + 1MB\n", nVbvBufSize/(1024*1024), vbvThreshSize/(1024*1024));
        nVbvBufSize = vbvThreshSize + 1*1024*1024;
    }
    nVbvBufSize = AWALIGN(nVbvBufSize, 1024);

    memset(&pContext->m_venc_pic.mVEncAttr, 0, sizeof(VENC_CHN_ATTR_S));
    pContext->m_venc_pic.mVEncAttr.VeAttr.Type = PT_JPEG;
    pContext->m_venc_pic.mVEncAttr.VeAttr.AttrJpeg.MaxPicWidth = 0;
    pContext->m_venc_pic.mVEncAttr.VeAttr.AttrJpeg.MaxPicHeight = 0;
    pContext->m_venc_pic.mVEncAttr.VeAttr.AttrJpeg.BufSize = nVbvBufSize;
    pContext->m_venc_pic.mVEncAttr.VeAttr.AttrJpeg.mThreshSize = vbvThreshSize;
    pContext->m_venc_pic.mVEncAttr.VeAttr.AttrJpeg.bByFrame = TRUE;
    pContext->m_venc_pic.mVEncAttr.VeAttr.AttrJpeg.PicWidth = pContext->m_venc_pic.mWidth;
    pContext->m_venc_pic.mVEncAttr.VeAttr.AttrJpeg.PicHeight = pContext->m_venc_pic.mHeight;
    pContext->m_venc_pic.mVEncAttr.VeAttr.AttrJpeg.bSupportDCF = FALSE;
    pContext->m_venc_pic.mVEncAttr.VeAttr.MaxKeyInterval = 1;
    pContext->m_venc_pic.mVEncAttr.VeAttr.SrcPicWidth = pContext->m_vi.mWidth;
    pContext->m_venc_pic.mVEncAttr.VeAttr.SrcPicHeight = pContext->m_vi.mHeight;
    pContext->m_venc_pic.mVEncAttr.VeAttr.Field = VIDEO_FIELD_FRAME;
    pContext->m_venc_pic.mVEncAttr.VeAttr.PixelFormat = pContext->m_vi.mPixFmt;
    pContext->m_venc_pic.mVEncAttr.VeAttr.OutputFormat = VENC_OUTPUT_SAME_AS_INPUT;
    pContext->m_venc_pic.mVEncAttr.VeAttr.mColorSpace = pContext->m_vi.mColorSpace;
    printf("pixfmt:0x%x, output_pixfmt:0x%x, colorSpace:0x%x\n", pContext->m_venc_pic.mVEncAttr.VeAttr.PixelFormat, pContext->m_venc_pic.mVEncAttr.VeAttr.OutputFormat, pContext->m_venc_pic.mVEncAttr.VeAttr.mColorSpace);

    switch(0)
    {
        case 90:
            pContext->m_venc_pic.mVEncAttr.VeAttr.Rotate = ROTATE_90;
            break;
        case 180:
            pContext->m_venc_pic.mVEncAttr.VeAttr.Rotate = ROTATE_180;
            break;
        case 270:
            pContext->m_venc_pic.mVEncAttr.VeAttr.Rotate = ROTATE_270;
            break;
        default:
            pContext->m_venc_pic.mVEncAttr.VeAttr.Rotate = ROTATE_NONE;
            break;
    }
    printf("rotate:%d\n", pContext->m_venc_pic.mVEncAttr.VeAttr.Rotate);
    bool bSuccessFlag = false;
    pContext->m_venc_pic.mVEncChn = 0;
    while(pContext->m_venc_pic.mVEncChn < VENC_MAX_CHN_NUM)
    {
        ret = AW_MPI_VENC_CreateChn(pContext->m_venc_pic.mVEncChn, &pContext->m_venc_pic.mVEncAttr);
        if(SUCCESS == ret)
        {
            bSuccessFlag = true;
            printf("create venc channel[%d] success!\n", pContext->m_venc_pic.mVEncChn);
            break;
        }
        else if(ERR_VENC_EXIST == ret)
        {
            printf("venc channel[%d] is exist, find next!", pContext->m_venc_pic.mVEncChn);
            pContext->m_venc_pic.mVEncChn++;
        }
        else
        {
            printf("fatal error! create venc channel[%d] ret[0x%x], find next!\n", pContext->m_venc_pic.mVEncChn, ret);
            pContext->m_venc_pic.mVEncChn++;
        }
    }
    if(false == bSuccessFlag)
    {
        pContext->m_venc_pic.mVEncChn = MM_INVALID_CHN;
        printf("fatal error! create venc channel fail!\n");
        return ret;
    }

    MPPCallbackInfo cbInfo;
    cbInfo.cookie = (void*)pContext;
    cbInfo.callback = (MPPCallbackFuncType)&MPPCallbackWrapper1;
    AW_MPI_VENC_RegisterCallback(pContext->m_venc_pic.mVEncChn, &cbInfo);

    memset(&pContext->m_venc_pic.mJpegParam, 0, sizeof(VENC_PARAM_JPEG_S));
    pContext->m_venc_pic.mJpegParam.Qfactor = 90;
    AW_MPI_VENC_SetJpegParam(pContext->m_venc_pic.mVEncChn, &pContext->m_venc_pic.mJpegParam);
    AW_MPI_VENC_ForbidDiscardingFrame(pContext->m_venc_pic.mVEncChn, TRUE);
    return SUCCESS;
}

static void destoryVenc_picChn(mpp_camera_para_conf *pContext){
    printf("%s:%d\n",__func__,__LINE__);
    AW_MPI_VENC_StopRecvPic(pContext->m_venc_pic.mVEncChn);
    //can't reset channel now, because in non-tunnel mode, mpi_venc will force release out frames, but we don't want
    //those out frames to be released before we return them.
    //AW_MPI_VENC_ResetChn(mChn); 
    AW_MPI_VENC_DestroyChn(pContext->m_venc_pic.mVEncChn);
    pContext->m_venc_pic.mVEncChn = MM_INVALID_CHN;
}


static void savePic(mpp_camera_para_conf *pContext){
    int ret;
    char strFilePath[MAX_FILE_PATH_SIZE];
    VENC_STREAM_S mOutStream;
    memset(&mOutStream, 0, sizeof(VENC_STREAM_S));
    VENC_PACK_S pEncPack;
    memset(&pEncPack, 0, sizeof(VENC_PACK_S));
    mOutStream.mpPack = &pEncPack;
    mOutStream.mPackCount = 1;
    ret = AW_MPI_VENC_GetStream(pContext->m_venc_pic.mVEncChn, &mOutStream, 1000); 
    if(ret != SUCCESS)
    {
        printf("fatal error! why get stream fail?\n");
    }else{
        // AW_MPI_VENC_GetJpegThumbBuffer(pContext->m_venc_pic.mVEncChn, &mJpegThumbBuf);
        printf("start open storage path!\n");
        TTime time;
        memset(&time, 0, sizeof(TTime));
        RtcGetTime2C(&time);
        if(pContext->m_vi.mViDev == 0){
            sprintf(strFilePath,  "%s/Front_%d-%d-%d_%d-%d-%d.jpg", FRONT_PIC_PATH,time.nYear,time.nMonth,time.nDay,time.nHour,time.nMinute, time.nSecond);
            queue_mpp_push(F_picFile, strFilePath);
        }else{
            sprintf(strFilePath,  "%s/Rear_%d-%d-%d_%d-%d-%d.jpg", REAR_PIC_PATH,time.nYear,time.nMonth,time.nDay,time.nHour,time.nMinute, time.nSecond);
            queue_mpp_push(R_picFile, strFilePath);
        }
        FILE *fpPic = NULL;
        fpPic = fopen(strFilePath, "wb");
        if(fpPic != NULL)
        {
            printf("start save pic data!\n");
            if(mOutStream.mpPack[0].mpAddr0 != NULL && mOutStream.mpPack[0].mLen0 > 0)
            {
                fwrite(mOutStream.mpPack[0].mpAddr0, 1, mOutStream.mpPack[0].mLen0, fpPic);
                printf("virAddr0=[%p], length=[%d]\n", mOutStream.mpPack[0].mpAddr0, mOutStream.mpPack[0].mLen0);
            }
            if(mOutStream.mpPack[0].mpAddr1 != NULL && mOutStream.mpPack[0].mLen1 > 0)
            {
                fwrite(mOutStream.mpPack[0].mpAddr1, 1, mOutStream.mpPack[0].mLen1, fpPic);
                printf("virAddr1=[%p], length=[%d]\n", mOutStream.mpPack[0].mpAddr1, mOutStream.mpPack[0].mLen1);
            }
            if(mOutStream.mpPack[0].mpAddr2 != NULL && mOutStream.mpPack[0].mLen2 > 0)
            {
                fwrite(mOutStream.mpPack[0].mpAddr2, 1, mOutStream.mpPack[0].mLen2, fpPic);
                printf("virAddr2=[%p], length=[%d]\n", mOutStream.mpPack[0].mpAddr2, mOutStream.mpPack[0].mLen2);
            }
            fsync(fpPic);
            fclose(fpPic);

        }
        else
        {
            printf("fatal error! open file[%s] fail! errno(%d)\n", strFilePath, errno);
        }               
    }
    ret = AW_MPI_VENC_ReleaseStream(pContext->m_venc_pic.mVEncChn,  &mOutStream);
    if (ret < 0)
    {
        printf("fatal error, venc_pic release frame failed!\n");
    }
    printf("store jpeg in file[%s]\n", strFilePath);
    destoryVenc_picChn(pContext);   
}
#endif

#ifdef AI2MUX
static void configAiChnAttr(mpp_camera_para_conf *pContext){
    printf("%s:%d\n",__func__,__LINE__);
    pContext->m_ai.mAioAttr.mChnCnt = pContext->m_ai.mAIChnCnt;
    pContext->m_ai.mAioAttr.enBitwidth = map_BitWidth_to_AUDIO_BIT_WIDTH_E(pContext->m_ai.mBitWidth);
    pContext->m_ai.mAioAttr.enSamplerate = map_SampleRate_to_AUDIO_SAMPLE_RATE_E(pContext->m_ai.mSampleRate);
    pContext->m_ai.mAioAttr.enSoundmode = 0;
}

static ERRORTYPE createAIChn(mpp_camera_para_conf *pContext, int AiDev, int AiChn)
{
    printf("%s:%d\n",__func__,__LINE__);
    pContext->m_ai.mAIDevId = AiDev;
    pContext->m_ai.mAIChnId = AiChn;
    configAiChnAttr(pContext);
    AW_MPI_AI_SetPubAttr(pContext->m_ai.mAIDevId, &pContext->m_ai.mAioAttr);
    AW_MPI_AI_Enable(pContext->m_ai.mAIDevId);

    BOOL nSuccessFlag = FALSE;
    ERRORTYPE ret = 0;
    while (pContext->m_ai.mAIChnId < AIO_MAX_CHN_NUM)
    {
        ret = AW_MPI_AI_CreateChn(pContext->m_ai.mAIDevId, pContext->m_ai.mAIChnId, NULL);
        if (SUCCESS == ret)
        {
            nSuccessFlag = TRUE;
            printf("create ai channel[%d] success!\n", pContext->m_ai.mAIChnId);
            break;
        }
        else if (ERR_AI_EXIST == ret)
        {
            printf("ai channel[%d] exist, find next!\n", pContext->m_ai.mAIChnId);
            pContext->m_ai.mAIChnId++;
        }
        else if (ERR_AI_NOT_ENABLED == ret)
        {
            printf("audio_hw_ai not started!\n");
            break;
        }
        else
        {
            printf("create ai channel[%d] fail! ret[0x%x]!\n", pContext->m_ai.mAIChnId, ret);
            break;
        }
    }
    if(FALSE == nSuccessFlag)
    {
        pContext->m_ai.mAIChnId = MM_INVALID_CHN;
        printf("fatal error! create ai channel fail!\n");
        ret = -1;
    }
    else
    {
        pContext->m_ai.mAiChn.mModId = MOD_ID_AI;
        pContext->m_ai.mAiChn.mDevId = pContext->m_ai.mAIDevId;
        pContext->m_ai.mAiChn.mChnId = pContext->m_ai.mAIChnId;
    }
    return ret;
}

static void configAencChnAttr(mpp_camera_para_conf *pContext){
    printf("%s:%d\n",__func__,__LINE__);
    pContext->m_aenc.mAEncAttr.AeAttr.Type = pContext->m_aenc.mCodecType;
    pContext->m_aenc.mAEncAttr.AeAttr.channels = pContext->m_ai.mAIChnCnt;
    pContext->m_aenc.mAEncAttr.AeAttr.bitsPerSample = pContext->m_ai.mBitWidth;
    pContext->m_aenc.mAEncAttr.AeAttr.sampleRate = pContext->m_ai.mSampleRate;
    pContext->m_aenc.mAEncAttr.AeAttr.bitRate = pContext->m_ai.mBitRate;
    pContext->m_aenc.mAEncAttr.AeAttr.attachAACHeader = 0;
    pContext->m_aenc.mAEncAttr.AeAttr.mInBufSize = 0;
    pContext->m_aenc.mAEncAttr.AeAttr.mOutBufCnt = 0;
}

static ERRORTYPE createAencChn(mpp_camera_para_conf *pContext){
    printf("%s:%d\n",__func__,__LINE__);
    ERRORTYPE ret = SUCCESS;
    BOOL nSuccessFlag = false;
    configAencChnAttr(pContext);
    pContext->m_aenc.mAEncChnId = 0;
    while(pContext->m_aenc.mAEncChnId < AENC_MAX_CHN_NUM){
        ret = AW_MPI_AENC_CreateChn(pContext->m_aenc.mAEncChnId, &pContext->m_aenc.mAEncAttr);
        if(ret == SUCCESS){
            nSuccessFlag = true;
            printf("create aenc[%d] success!!!\n", pContext->m_aenc.mAEncChnId);
            break;
        }else if(ret == ERR_AENC_EXIST){
            printf("aenc chnnel[%d] not exit,find next!!!\n",pContext->m_aenc.mAEncChnId);
            pContext->m_aenc.mAEncChnId++;
        }else{
            printf("creat aenc chnnel[%d] ret[%d],find next!!!\n",pContext->m_aenc.mAEncChnId, ret);
            pContext->m_aenc.mAEncChnId++;
        }
    }
    if(nSuccessFlag == false){
        pContext->m_aenc.mAEncChnId = MM_INVALID_CHN;
        printf("fatal error! create aenc chnnel fail!!!\n");
        ret = -1;
    }else{
        pContext->m_aenc.mAEncChn.mChnId = pContext->m_aenc.mAEncChnId;
        pContext->m_aenc.mAEncChn.mDevId = 0;
        pContext->m_aenc.mAEncChn.mModId = MOD_ID_AENC;
    }
    return ret;
}
#endif
// ERRORTYPE 

static ERRORTYPE prepare(mpp_camera_para_conf *pContext){
    printf("%s:%d\n",__func__,__LINE__);
    ERRORTYPE result = FAILURE;
    if(pContext->m_vi.mViDev < 0){
        printf("invalid camera dev!!!\n");
        return -1;
    }
    if(pContext->m_mux.mMuxChn >= 0 && pContext->m_venc.mVEncChn >= 0){
        //set spspps
        if (pContext->m_venc.mEncoderFmt == PT_H264)
        {
            VencHeaderData H264SpsPpsInfo;
            memset(&H264SpsPpsInfo, 0, sizeof(VencHeaderData));
            result = AW_MPI_VENC_GetH264SpsPpsInfo(pContext->m_venc.mVEncChn, &H264SpsPpsInfo);
            if (SUCCESS != result)
            {
                aloge("fatal error, venc GetH264SpsPpsInfo failed! ret=%d", result);
                return result;
            }
            AW_MPI_MUX_SetH264SpsPpsInfo(pContext->m_mux.mMuxChn, pContext->m_venc.mVEncChn, &H264SpsPpsInfo);
        }
        else if(pContext->m_venc.mEncoderFmt == PT_H265)
        {
            VencHeaderData H265SpsPpsInfo;
            memset(&H265SpsPpsInfo, 0, sizeof(VencHeaderData));
            result = AW_MPI_VENC_GetH265SpsPpsInfo(pContext->m_venc.mVEncChn, &H265SpsPpsInfo);
            if (SUCCESS != result)
            {
                aloge("fatal error, venc GetH265SpsPpsInfo failed! result=%d", result);
                return result;
            }
            AW_MPI_MUX_SetH265SpsPpsInfo(pContext->m_mux.mMuxChn, pContext->m_venc.mVEncChn, &H265SpsPpsInfo);
        }
    }

    if (pContext->m_venc.mVEncChn >= 0 && pContext->m_mux.mMuxChn >= 0)
    {
        MPP_CHN_S MuxChn = {MOD_ID_MUX, 0, pContext->m_mux.mMuxChn};
        MPP_CHN_S VeChn = {MOD_ID_VENC, 0, pContext->m_venc.mVEncChn};

        AW_MPI_SYS_Bind(&VeChn, &MuxChn);
        // pContext->mCurrentState = REC_PREPARED;
    }
//------------------------------------------------------------------------------------
    setOverlayToVencChn(pContext);

    if(pContext->m_ai.mAiChn.mChnId >= 0 && pContext->m_aenc.mAEncChn.mChnId >= 0 && pContext->m_mux.mMuxChn >= 0){
        AW_MPI_SYS_Bind(&pContext->m_ai.mAiChn, &pContext->m_aenc.mAEncChn);
        MPP_CHN_S MuxChn = {MOD_ID_MUX, 0, pContext->m_mux.mMuxChn};
        AW_MPI_SYS_Bind(&pContext->m_aenc.mAEncChn, &MuxChn);
    }

    result = AW_MPI_AI_EnableChn(pContext->m_ai.mAIDevId, pContext->m_ai.mAIChnId);
    if (result != SUCCESS)
    {
        printf("AI enable error!\n");
        return result;
    }
    return SUCCESS;
}



static ERRORTYPE startVideoRecording(mpp_camera_para_conf *pContext){
    printf("%s:%d\n",__func__,__LINE__);
    if (pContext->m_mux.mMuxChn >= 0)
    {
        pContext->mRecorderFlag = 1;
        AW_MPI_VENC_StartRecvPic(pContext->m_venc.mVEncChn);
        AW_MPI_AENC_StartRecvPcm(pContext->m_aenc.mAEncChnId); 
        AW_MPI_MUX_StartChn(pContext->m_mux.mMuxChn);

    }
    return SUCCESS;
}


static ERRORTYPE stopVideoRecording(mpp_camera_para_conf *pContext){
    printf("%s:%d\n",__func__,__LINE__);
    pContext->mRecorderFlag = 0;
    pthread_mutex_destroy(&pContext->m_mux.fsyncMutex);
    destroyTimestampOverlay(pContext);
    AW_MPI_MUX_StopChn(pContext->m_mux.mMuxChn, FALSE);
    AW_MPI_VENC_StopRecvPic(pContext->m_venc.mVEncChn);
    AW_MPI_AENC_StopRecvPcm(pContext->m_aenc.mAEncChnId);
    if (pContext->m_venc.mVEncChn >= 0 && pContext->m_mux.mMuxChn >= 0)
    {
        MPP_CHN_S MuxChn = {MOD_ID_MUX, 0, pContext->m_mux.mMuxChn};
        MPP_CHN_S VeChn = {MOD_ID_VENC, 0, pContext->m_venc.mVEncChn};

        AW_MPI_SYS_UnBind(&VeChn, &MuxChn);
    }
    if(pContext->m_ai.mAiChn.mChnId >= 0 && pContext->m_aenc.mAEncChn.mChnId >= 0 && pContext->m_mux.mMuxChn >= 0){  
        MPP_CHN_S MuxChn = {MOD_ID_MUX, 0, pContext->m_mux.mMuxChn};
        AW_MPI_SYS_UnBind(&pContext->m_aenc.mAEncChn, &MuxChn);
        AW_MPI_AI_DisableChn(pContext->m_ai.mAIDevId, pContext->m_ai.mAIChnId);
        AW_MPI_SYS_UnBind(&pContext->m_ai.mAiChn, &pContext->m_aenc.mAEncChn);
    }

    AW_MPI_MUX_DestroyChn(pContext->m_mux.mMuxChn);
    AW_MPI_VENC_DestroyChn(pContext->m_venc.mVEncChn);
    AW_MPI_AENC_DestroyChn(pContext->m_aenc.mAEncChnId); 
    AW_MPI_AI_DestroyChn(pContext->m_ai.mAIDevId, pContext->m_ai.mAIChnId);

   
    return SUCCESS;
}



ERRORTYPE initVi(mpp_camera_para_conf *pContext, VI_DEV videv, VI_CHN vichn){
    printf("%s:%d\n",__func__,__LINE__);
    InitMppCameraData(pContext);
    setConfigPara(pContext);
    createViChn(pContext, videv, vichn);
    CreateMsgQueueThread(pContext);	
#if 1
    int ret = pthread_create(&pContext->mVi2VencThreadId, NULL, Vi2VencFrameThread, pContext);
    if (ret != 0)
    {
        printf("fatal error! create Vi2VencFrameThread Thread fail[%d]\n", ret);
        return ret;
    }
    else
    {
        printf("create Vi2VencFrameThread Thread success! threadId[0x%x]\n", &pContext->mVi2VencThreadId);
    }
#endif

    ret = pthread_create(&pContext->mfsyncThreadId, NULL, FsyncFrameThread, pContext);
    if (ret != 0)
    {
        printf("fatal error! create FsyncFrameThread Thread fail[%d]\n", ret);
        return ret;
    }
    else
    {
        printf("create FsyncFrameThread Thread success! threadId[0x%x]\n", &pContext->mfsyncThreadId);
    }
    return 0;    
}

ERRORTYPE deinitVi(mpp_camera_para_conf *pContext){
    message_t stMsgCmd;
    pContext->mExitFlag = 1;
    pthread_join(pContext->mCSIFrameThreadId, NULL);
    pthread_join(pContext->mVi2VencThreadId, NULL);
    pthread_join(pContext->mfsyncThreadId, NULL);
    stMsgCmd.command = MsgQueue_Stop;
    put_message(&pContext->mMsgQueue, &stMsgCmd);
    pthread_join(pContext->mMsgQueueThreadId, NULL);
    DestroyMsgqQueue(pContext);
    AW_MPI_VI_DisableVirChn(pContext->m_vi.mViDev, pContext->m_vi.mViChn);
    AW_MPI_VI_DestroyVirChn(pContext->m_vi.mViDev, pContext->m_vi.mViChn);
    AW_MPI_VI_DisableVipp(pContext->m_vi.mViDev);
    AW_MPI_VI_DestroyVipp(pContext->m_vi.mViDev);
    AW_MPI_ISP_Stop(pContext->mIspDev);

}

ERRORTYPE recording(mpp_camera_para_conf *pContext){
    printf("%s:%d\n",__func__,__LINE__);
    ERRORTYPE ret = SUCCESS; 
    createAIChn(pContext, 0, 0);
    createAencChn(pContext);		
    createVencChn(pContext);
    createMuxChn(pContext);
    prepare(pContext);
    startVideoRecording(pContext);
    return ret; 
}

ERRORTYPE stopRecording(mpp_camera_para_conf *pContext){
    printf("%s:%d\n",__func__,__LINE__);
    ERRORTYPE ret = SUCCESS; 
    stopVideoRecording(pContext);
    return ret; 
}

ERRORTYPE changeVideoRecordingMode(mpp_camera_para_conf *pContext){
    printf("%s:%d\n",__func__,__LINE__);
    pthread_mutex_lock(&pContext->m_mux.fsyncMutex);
    getFileNameByCurTime(pContext, pContext->m_mux.dstVideoFile);
    int fd = open(pContext->m_mux.dstVideoFile, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if(fd < 0){
        printf("open pContext->m_mux.dstVideoFile fail!!\n");
    }else{
        pthread_mutex_unlock(&pContext->m_mux.fsyncMutex);
        switch (g_sys_Data.recorderMode)
        {
        case RECORDER_NONE:
            /* code */
            // AW_MPI_MUX_SwitchFileNormal(pContext->m_mux.mMuxChn,fd,0);
            break;
        case RECORDER_NORMAL:
            /* code */
            AW_MPI_MUX_SwitchFileNormal(pContext->m_mux.mMuxChn,fd,0);
            break;
        case RECORDER_URGENT:
            AW_MPI_MUX_SwitchFileNormal(pContext->m_mux.mMuxChn,fd,0);
            /* code */
            break;    
        default:
            break;
        }
    }
}

ERRORTYPE startPreview(mpp_camera_para_conf *pContext, int DstWidth, int DstHeight, int DisplayX, int DisplayY){
    printf("%s:%d\n",__func__,__LINE__);
    if(pContext->mVoFlag == 1) return;
    ERRORTYPE ret = SUCCESS;

    setDstSizeAndDispPos(pContext,DstWidth,DstHeight,DisplayX,DisplayY);
    createVoChn(pContext, 0, 0);

    
    pContext->mVoFlag = 1;
    ret = AW_MPI_VO_StartChn(pContext->m_vo.mVoLayer, pContext->m_vo.mVoChn);
    return ret;
}

ERRORTYPE stopPreview(mpp_camera_para_conf *pContext){
    printf("%s:%d\n",__func__,__LINE__);
    if(pContext->mVoFlag == 0) return;
    pContext->mVoFlag = 0;   
    AW_MPI_VO_StopChn(pContext->m_vo.mVoLayer, pContext->m_vo.mVoChn);
    AW_MPI_VO_DestroyChn(pContext->m_vo.mVoLayer, pContext->m_vo.mVoChn);
    AW_MPI_VO_DisableVideoLayer(pContext->m_vo.mVoLayer);
    AW_MPI_VO_CloseVideoLayer(pContext->m_vo.mUILayer);
    AW_MPI_VO_RemoveOutsideVideoLayer(pContext->m_vo.mUILayer);
    AW_MPI_VO_Disable(pContext->m_vo.mVoDev);
    return SUCCESS;
}

void startTakePic(mpp_camera_para_conf *pContext){
    printf("%s:%d\n",__func__,__LINE__);
    if(pContext->m_vi.mViDev < 0){
        printf("invalid camera dev!!!\n");
        return -1;
    }
    if(pContext->mTakePicFlag){
        printf("last pic is taking! return.\n");
        return;
    }
    createVenc_picChn(pContext);
    int ret = AW_MPI_VENC_StartRecvPic(pContext->m_venc_pic.mVEncChn);
    if(SUCCESS != ret)
    {
        printf("fatal error:%x jpegEnc AW_MPI_VENC_StartRecvPic\n",ret);
        return ret;
    }
    memset(&pContext->m_venc_pic.mExifInfo, 0, sizeof(VENC_EXIFINFO_S));
    TTime time;
    memset(&time, 0, sizeof(time));
    RtcGetTime2C(&time);
    snprintf((char*)pContext->m_venc_pic.mExifInfo.DateTime, MM_DATA_TIME_LENGTH, "%4d:%02d:%02d %02d:%02d:%02d",
        time.nYear,time.nMonth,time.nDay, time.nHour, time.nMinute, time.nSecond);
    pContext->m_venc_pic.mExifInfo.ThumbWidth = 320;
    pContext->m_venc_pic.mExifInfo.ThumbHeight = 240;
    pContext->m_venc_pic.mExifInfo.thumb_quality = 60;
    pContext->m_venc_pic.mExifInfo.Orientation = 0;
    //pCap->mExifInfo.fr32FNumber = FRACTION32(10, 26);
    pContext->m_venc_pic.mExifInfo.FNumber.num = 26;
    pContext->m_venc_pic.mExifInfo.FNumber.den = 10;
    pContext->m_venc_pic.mExifInfo.MeteringMode = METERING_MODE_AVERAGE;
    //pCap->mExifInfo.fr32FocalLength = FRACTION32(100, 228);
    pContext->m_venc_pic.mExifInfo.FocalLength.num = 228;
    pContext->m_venc_pic.mExifInfo.FocalLength.den = 100;
    pContext->m_venc_pic.mExifInfo.WhiteBalance = 0;
    pContext->m_venc_pic.mExifInfo.FocalLengthIn35mmFilm = 18;
    strcpy((char*)pContext->m_venc_pic.mExifInfo.ImageName, "aw-photo");
    AW_MPI_VENC_SetJpegExifInfo(pContext->m_venc_pic.mVEncChn, &pContext->m_venc_pic.mExifInfo);    
    pContext->mTakePicFlag = 1;
    printf("start...\n");
}

void dashTimeMark(mpp_camera_para_conf *pContext, bool flag){
    RGN_CHN_ATTR_S stRgnChnAttr;
    MPP_CHN_S VeChn = {MOD_ID_VENC, 0, pContext->m_venc.mVEncChn};
    AW_MPI_RGN_GetDisplayAttr(pContext->m_venc.mOverlayHandle + 1,  &VeChn, &stRgnChnAttr);
    stRgnChnAttr.bShow = flag;
    AW_MPI_RGN_SetDisplayAttr(pContext->m_venc.mOverlayHandle + 1,  &VeChn, &stRgnChnAttr);
}

void SoundRecording(mpp_camera_para_conf *pContext, bool flag){
    printf("%s:%d\n",__func__,__LINE__);
    AW_MPI_AI_SetChnMute(pContext->m_ai.mAIDevId, pContext->m_ai.mAIChnId, flag);

}

#if 1
int tp2804_i2c_init(int* i2c_fd, const char *i2c_bus, uint8_t dev_addr) {
    if(i2c_fd == NULL || i2c_bus == NULL){
        printf("invaild parameter!!!\n");
        return -1;
    }
    *i2c_fd = open(i2c_bus, O_RDWR);
    if (*i2c_fd < 0) {
        perror("open i2c bus fail!!!");
        return -1;
    }

    if (ioctl(*i2c_fd, I2C_SLAVE_FORCE, dev_addr) < 0) {
        perror("can't set i2c dev addr!!!");
        close(*i2c_fd);
        *i2c_fd = -1;
        return -1;
    }
    printf("open i2c fd:%d success!!!\n", *i2c_fd);
    return 0;
}

void tp2804_i2c_deinit(int* i2c_fd) {
    if (*i2c_fd >= 0) {
        close(*i2c_fd);
        *i2c_fd = -1;
    }
}


static int tp2804_write_reg(int i2c_fd, uint8_t reg, uint8_t value) {
    if (i2c_fd < 0) {
        fprintf(stderr, "err: i2c non't init\n");
        return -1;
    }

    uint8_t buf[2] = {reg, value};
    if (write(i2c_fd, buf, 2) != 2) {
        perror("write to i2c reg fail");
        return -1;
    }
    return 0;
}


static int tp2804_read_reg(int i2c_fd, uint8_t reg, uint8_t *value) {
    if (i2c_fd < 0) {
        fprintf(stderr, "err: i2c non't init\n");
        return -1;
    }

    
    if (write(i2c_fd, &reg, 1) != 1) {
        perror("write to i2c reg fail");
        return -1;
    }

    if (read(i2c_fd, value, 1) != 1) {
        perror("read to i2c reg fail");
        return -1;
    }
    return 0;
}

bool tp2804_check_camera_connected(int i2c_fd) {
    uint8_t status_reg = 0;
    
    if (tp2804_read_reg(i2c_fd, REG_VIDEO_INPUT_STATUS, &status_reg) != 0) {
        fprintf(stderr, "read to status reg fail\n");
        return 0;
    }

    // printf("Video Input Status Register = 0x%02X\n", status_reg);
    // printf("  VDLOSS (Video Loss)    : %s\n", (status_reg & BIT_VDLOSS) ? "1 (No Video)" : "0 (Video Present)");
    // printf("  VLOCK (V PLL Lock)     : %s\n", (status_reg & BIT_VLOCK) ? "1 (Locked)" : "0 (Not Locked)");
    // printf("  HLOCK (H PLL Lock)     : %s\n", (status_reg & BIT_HLOCK) ? "1 (Locked)" : "0 (Not Locked)");
    // printf("  SLOCK (Carrier Lock)   : %s\n", (status_reg & BIT_SLOCK) ? "1 (Locked)" : "0 (Not Locked)");
    // printf("  VDET  (Video Detected) : %s\n", (status_reg & BIT_VDET) ? "1 (Detected)" : "0 (No Video)");
    // printf("  EQDET (EQ Detected)    : %s\n", (status_reg & BIT_EQDET) ? "1 (Detected)" : "0 (Not Detected)");
    // printf("  NINTL (Interlaced)     : %s\n", (status_reg & BIT_NINTL) ? "1 (Progressive)" : "0 (Interlaced)");
    // printf("  CDET  (Carrier Detect) : %s\n", (status_reg & BIT_CDET) ? "1 (No Carrier)" : "0 (Carrier Detected)");

    
    if (status_reg & BIT_VDLOSS) {
        // printf("Not connected camera dev (VDLOSS=1)\n");
        return false; 
    } else {
        // printf("Connected camera dev (VDLOSS=0)\n");
        return true;
    }
}
#endif

#if 0
void mppFunTheard(void *pThreadData){
    mpp_camera_para_conf *pContext = (mpp_camera_para_conf*)pThreadData;
    message_t stCmdMsg;
    mppFunCmd cmd;
    while (1)
    {
        if (0 == get_message(&pContext->mMppMsgQueue, &stCmdMsg))
        {
            cmd = stCmdMsg.command;
            switch (cmd)
            {
            case M_PREVIEW:

                break;
            case M_STOP_PREVIEW:

                break;
            case M_RECORDER:
                CreateMsgQueueThread(pContext);
                createAIChn(pContext, 0, 0);
                createAencChn(pContext);	
                createVencChn(pContext);
                createMuxChn(pContext);
                prepare(pContext);
                startVideoRecording(pContext);
                break;
            case M_STOP_RECORDER:
                stopVideoRecording(pContext);
                break;
            case M_REC_SOUND:
                SoundRecording(pContext, true);
                break;
            case M_MUTE_SOUND:
                SoundRecording(pContext, true);
                break;
            case M_TACKPIC:
                startTakePic(pContext);
                break;
            case M_TIMEMARK:
                dashSpeedMark(pContext, true);
                break;
            case M_NO_TIMEMARK:
                dashSpeedMark(pContext, false);
                break;
            // case :
            //     break;
            // case :
            //     break;            
            default:
                break;
            }
        }
        else
        {
            TMessage_WaitQueueNotEmpty(&pContext->mMppMsgQueue, 0);
        }
    }
    

}

int CreatemppFunTheard(mpp_camera_para_conf *pContext){
    int result = 0;
    if(pContext->m_vi.mViDev < 0){
        printf("invalid camera dev!!!\n");
        return -1;
    }
    result = pthread_create(&pContext->mMppFunThreadId, NULL, mppFunTheard, pContext);
    if (result != 0)
    {
        printf("fatal error! CreatemppFunTheard fail[%d]!\n", result);
        return -1;
    }
    else
    {
        printf("create CreatemppFunTheard success threadId[0x%x].\n", &pContext->mMppFunThreadId);
    }
    return result;
}

void DestroyMppMsgQueue(mpp_camera_para_conf *pContext){
    message_destroy(&pContext->mMppMsgQueue);
}
#endif