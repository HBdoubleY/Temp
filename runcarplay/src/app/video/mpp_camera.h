#ifndef __MPP_CAMRA_H__
#define __MPP_CAMRA_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
// #include <cdx_list.h>
#include <pthread.h>
#include "mm_comm_sys.h"
#include "mpi_sys.h"
#include "mm_comm_vi.h"
#include "mpi_vi.h"
#include <mpi_isp.h>
#include "vencoder.h"
#include "mpi_venc.h"
#include "mm_comm_video.h"
#include "mm_comm_mux.h"
#include "mpi_mux.h"
#include "tmessage.h"
#include "tsemaphore.h"
#include <memoryAdapter.h>
#include "sc_interface.h"
#include <confparser.h>
#include <mm_comm_vo.h>
#include <mpi_vo.h>
#include <mpi_clock.h>
#include <plat_type.h>
#include <linux/fb.h>
#include <mpi_mux.h>
#include <mpi_venc.h>
#include <mpi_ai.h>
#include <mpi_aenc.h>
#include "mm_comm_aio.h"
#include "mm_comm_aenc.h"
#include <media_common_aio.h>
#include "queue_mpp.h"
#include <semaphore.h>

#define TP2804_I2C_ADDR        0x44
#define I2C_BUS0                "/dev/i2c-0"
#define I2C_BUS1                "/dev/i2c-1"

#ifndef I2C_SLAVE_FORCE 
#define I2C_SLAVE_FORCE 0x0706
#endif

#define REG_VIDEO_INPUT_STATUS 0x01
#define REG_GENERAL_PAGE_CTRL  0x40

#define BIT_VDLOSS             (1 << 7) // 视频丢失标志
#define BIT_VLOCK              (1 << 6) // 垂直锁相环锁定
#define BIT_HLOCK              (1 << 5) // 水平锁相环锁定
#define BIT_SLOCK              (1 << 4) // 载波锁相环锁定
#define BIT_VDET               (1 << 3) // 视频检测
#define BIT_EQDET              (1 << 2) // 均衡器检测
#define BIT_NINTL              (1 << 1) // 隔行视频
#define BIT_CDET               (1 << 0) // 载波检测


#define FRONT_PIC_PATH "/mnt/extsd/DVRpic/frontPic"
#define REAR_PIC_PATH "/mnt/extsd/DVRpic/rearPic"
#define FRONT_VIDEO_PATH "/mnt/extsd/recorder/frontCamera"
#define REAR_VIDEO_PATH "/mnt/extsd/recorder/rearCamera"
#define FRONT_VIDEO_URGENT_PATH "/mnt/extsd/recorderUrgent/frontCamera"
#define REAR_VIDEO_URGENT_PATH "/mnt/extsd/recorderUrgent/rearCamera"

#define MAX_FILE_PATH_SIZE (256)
#define MAX_FILE_PATH_LEN  (128)



typedef enum H264_PROFILE_E
{
   H264_PROFILE_BASE = 0,
   H264_PROFILE_MAIN,
   H264_PROFILE_HIGH,
}H264_PROFILE_E;

typedef struct
{
    char strFilePath[MAX_FILE_PATH_SIZE];
    struct list_head mList;
}FilePathNode;

typedef struct M_VIPP_VI
{
    VI_DEV mViDev;
    VI_CHN mViChn;
    VI_ATTR_S mViAttr;
    int mWidth;
    int mHeight;
    int mPixFmt;
    int mFrameRate;
    int mViBufferNum;
    int mEnableWDR;
    int mViDropFrameNum;
    enum v4l2_colorspace mColorSpace;
    int HorizontalMirror;
    int FlipMirror;

}m_vipp_vi;

typedef struct M_VIPP_VO
{
    VO_DEV mVoDev;
    VO_CHN mVoChn;
    VO_LAYER mVoLayer;
    VO_LAYER mUILayer;
    int mLayerNum;
    VO_VIDEO_LAYER_ATTR_S mLayerAttr;
    VO_INTF_TYPE_E mDispType;
    VO_INTF_SYNC_E mDispSync;
    int mDisplayX;
    int mDisplayY;   
    int mWidth;
    int mHeight;

    QueueMpp *G2dQueue;
    QueueMpp *mppQueue;
}m_vipp_vo;

typedef struct M_VIPP_VENC
{
    VENC_CHN mVEncChn;
    VENC_CHN_ATTR_S mVEncAttr;
    VENC_RC_PARAM_S mVencRcParam;
    int mWidth;
    int mHeight;
    int mEncoderFmt;
    int mFrameRate;
    int mBitRate;
    int mProductMode;
    int mRcMode;
    int mVencDropFrameNum; // for online
    int mEncodeRotate;  //clockwise.

    VENC_PARAM_JPEG_S mJpegParam;
    VENC_EXIFINFO_S mExifInfo;
    /* crop params */

    int mOverlayHandle;
    int overlay_x;
    int overlay_y;
    bool upTimeInfo;
    pthread_t upTimeInfoId; 
}m_vipp_venc;



typedef struct M_VIPP_MUX
{
    MUX_CHN mMuxChn;
    MUX_CHN_ATTR_S mMuxChnAttr;
    // pthread_mutex_t mMuxChnListLock;
    // struct list_head mMuxChnList;   //MUX_CHN_INFO_S
    // struct list_head mMuxerFileListArray;    //FilePathNode
    MEDIA_FILE_FORMAT_E eFileFormat;
    char dstVideoFile[MAX_FILE_PATH_LEN];
    int mMaxFileDuration;
    pthread_mutex_t fsyncMutex;
    int fsyncFd;
}m_vipp_mux;

typedef struct M_VIPP_AI{
    AUDIO_DEV mAIDevId;
    MPP_CHN_S mAiChn;
    AIO_ATTR_S mAioAttr;
    AI_CHN mAIChnId;
    int mBitWidth;
    int mBitRate;
    int mSampleRate;
    int mAIChnCnt;
}m_vipp_ai;

typedef struct M_VIPP_AENC{
    AENC_CHN mAEncChnId;
    MPP_CHN_S mAEncChn;
    AENC_CHN_ATTR_S mAEncAttr;
    PAYLOAD_TYPE_E mCodecType;
}m_vipp_aenc;

typedef struct M_PARA_CONF{

    ISP_CFG_MODE mIspWdrSetting;



    // BOOL mHorizonFlipFlag; // mirror


    int mEnableFastEnc;
    int mQuality;
    int mRecFileCnt;
}m_para_conf;

typedef enum RecordState
{
    REC_NOT_PREPARED = 0,
    REC_PREPARED,
    REC_RECORDING,
    REC_STOP,
    REC_ERROR,
}RECSTATE_E;

typedef enum Vi2Venc2MuxerMsgType
{
    Rec_NeedSetNextFd = 0,
    Rec_FileDone,
    Vi_Timeout,
    MsgQueue_Stop,
}Vi2Venc2MuxerMsgType;



typedef struct MPP_CAMERA_PARA_CONF{
    m_vipp_vi m_vi;
    m_vipp_vo m_vo;
    m_vipp_venc m_venc;
    m_vipp_venc m_venc_pic;
    m_vipp_mux m_mux;
    m_vipp_ai m_ai;
    m_vipp_aenc m_aenc;
    ISP_DEV mIspDev;
    m_para_conf mConfigPara;
    pthread_t mCSIFrameThreadId;
    pthread_t mVi2VencThreadId;
    // pthread_t mVi2G2dThreadId;
    pthread_t mfsyncThreadId;
    int mExitFlag;
    int mVoFlag;
    int mRecorderFlag;
    int mTakePicFlag;

    pthread_t mMsgQueueThreadId;
    message_queue_t mMsgQueue;

    // RECSTATE_E mCurrentState;
}mpp_camera_para_conf;

typedef struct MPP_CAMERA_PARA_CONF_DATA{
    mpp_camera_para_conf *MppCameraParaConfData;
}mpp_camera_para_conf_data;



ERRORTYPE initVi(mpp_camera_para_conf *pContext, VI_DEV videv, VI_CHN vichn);
ERRORTYPE deinitVi(mpp_camera_para_conf *pContext);
ERRORTYPE recording(mpp_camera_para_conf *pContext);
ERRORTYPE stopRecording(mpp_camera_para_conf *pContext);
ERRORTYPE changeVideoRecordingMode(mpp_camera_para_conf *pContext);
ERRORTYPE startPreview(mpp_camera_para_conf *pContext, int DstWidth, int DstHeight, int DisplayX, int DisplayY);
ERRORTYPE stopPreview(mpp_camera_para_conf *pContext);
int mpp_camera_take_storage_fault(void);
void dashTimeMark(mpp_camera_para_conf *pContext, bool flag);
void SoundRecording(mpp_camera_para_conf *pContext, bool flag);
void startTakePic(mpp_camera_para_conf *pContext);

int tp2804_i2c_init(int* i2c_fd, const char *i2c_bus, uint8_t dev_addr);
void tp2804_i2c_deinit(int* i2c_fd) ;
bool tp2804_check_camera_connected(int i2c_fd);
#endif