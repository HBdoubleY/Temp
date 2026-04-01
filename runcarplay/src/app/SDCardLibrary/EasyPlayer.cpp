#define LOG_TAG "sample_Player"
#include <utils/plat_log.h>

#include <sys/stat.h>
#include <cstdio>
#include <csignal>
#include <iostream>
#include <cstring>

#include <hwdisplay.h>
#include <confparser.h>
#include <mpi_sys.h>
#include <mpi_vo.h>
#include <mpi_vdec.h>

#include "EasyPlayer.h"

using namespace std;
using namespace EyeseeLinux;

// PlayerListener 实现
MP4Player::PlayerListener::PlayerListener(MP4Player* pOwner)
    : mpOwner(pOwner)
{
}

void MP4Player::PlayerListener::onCompletion(EyeseePlayer* pMp)
{
    alogd("receive onCompletion message!");
    if (mpOwner) {
        // 可以根据需要处理循环播放等逻辑
    }
}

bool MP4Player::PlayerListener::onError(EyeseePlayer* pMp, int what, int extra)
{
    alogd("receive onError message![%d][%d]", what, extra);
    if (mpOwner) {
        // 处理错误
    }
    return true;
}

void MP4Player::PlayerListener::onVideoSizeChanged(EyeseePlayer* pMp, int width, int height)
{
    alogd("receive onVideoSizeChanged message!size[%dx%d]", width, height);
}

bool MP4Player::PlayerListener::onInfo(EyeseePlayer* pMp, int what, int extra)
{
    alogd("receive onInfo message! media_info_type[%d] extra[%d]", what, extra);
    return true;
}

void MP4Player::PlayerListener::onSeekComplete(EyeseePlayer* pMp)
{
    alogd("receive onSeekComplete message!");
}

// MP4Player 实现
MP4Player::MP4Player()
    : mCurrentState(STATE_IDLE)
    , mCurrentRotation(ROTATION_270)
    , mUILayer(HLAY(2, 0))
    , mVoDev(-1)
    , mVoLayer(-1)
    , mpPlayer(NULL)
    , mDisplayX(0)
    , mDisplayY(0)
    , mDisplayWidth(640)
    , mDisplayHeight(360)
    , mPlayerListener(this)
    , mfVps(1.0)
{
    cdx_sem_init(&mSemExit, 0);
    memset(&mSysConf, 0, sizeof(MPP_SYS_CONF_S));
    memset(&mLayerAttr, 0, sizeof(VO_VIDEO_LAYER_ATTR_S));
}

MP4Player::~MP4Player()
{
    StopPlay();
    cdx_sem_deinit(&mSemExit);
}

bool MP4Player::Initialize(int displayWidth, int displayHeight)
{
    if (mCurrentState != STATE_IDLE) {
        aloge("Player already initialized!");
        return false;
    }
    
    mDisplayWidth = displayWidth;
    mDisplayHeight = displayHeight;
    
    // // 初始化MPP系统
    // if (!InitializeMPPSystem()) {
    //     aloge("Initialize MPP system failed!");
    //     return false;
    // }
    
    // 初始化视频输出
    if (!InitializeVideoOutput()) {
        printf("Initialize video output failed!\n");
        CleanupResources();
        return false;
    }
    
    // 创建播放器
    mpPlayer = new EyeseePlayer;
    if (!mpPlayer) {
        printf("Create player failed!\n");
        CleanupResources();
        return false;
    }
    
    // 设置监听器
    mpPlayer->setOnCompletionListener(&mPlayerListener);
    mpPlayer->setOnErrorListener(&mPlayerListener);
    mpPlayer->setOnVideoSizeChangedListener(&mPlayerListener);
    mpPlayer->setOnInfoListener(&mPlayerListener);
    mpPlayer->setOnSeekCompleteListener(&mPlayerListener);
    
    // 强制禁止音频播放（解决声音问题）
    mpPlayer->grantPlayAudio(false);
    
    // 设置显示层
    mpPlayer->setDisplay(mVoLayer);
    
    // 设置输出像素格式为NV21
    mpPlayer->setOutputPixelFormat(MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420);
    
    // 启用缩放模式
    mpPlayer->enableScaleMode(true, 0, 0);
    
    // 设置不循环播放
    mpPlayer->setLooping(false);
    
    // 设置正常播放速度
    mpPlayer->setVps(1.0f);
    
    // 应用初始旋转设置
    if (!ApplyRotation()) {
        printf("Apply initial rotation failed!\n");
        CleanupResources();
        return false;
    }
    
    mCurrentState = STATE_INITIALIZED;
    printf("MP4Player initialized successfully!\n");
    return true;
}

bool MP4Player::SetDisplayRect(int x, int y, int width, int height)
{
    if (mCurrentState == STATE_IDLE) {
        aloge("Player not initialized!");
        return false;
    }
    
    mDisplayX = x;
    mDisplayY = y;
    mDisplayWidth = width;
    mDisplayHeight = height;
    
    // 更新显示属性
    if (mVoLayer >= 0) {
        AW_MPI_VO_GetVideoLayerAttr(mVoLayer, &mLayerAttr);
        mLayerAttr.stDispRect.X = x;
        mLayerAttr.stDispRect.Y = y;
        mLayerAttr.stDispRect.Width = width;
        mLayerAttr.stDispRect.Height = height;
        
        if (AW_MPI_VO_SetVideoLayerAttr(mVoLayer, &mLayerAttr) != SUCCESS) {
            aloge("Set video layer attribute failed!");
            return false;
        }
    }
    
    alogd("Display rect set to [%d, %d, %d, %d] layer = %d\n", x, y, width, height, mVoLayer);
    return true;
}

bool MP4Player::PlayFile(const std::string& filePath)
{
    if (mCurrentState != STATE_INITIALIZED && mCurrentState != STATE_STOPPED) {
        aloge("Player not in correct state! Current state: %d", mCurrentState);
        return false;
    }
    
    if (filePath.empty()) {
        aloge("File path is empty!");
        return false;
    }
    
    // 检查文件是否存在
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) != 0) {
        aloge("File not exist: %s", filePath.c_str());
        return false;
    }
    
    // 设置数据源
    mpPlayer->setDataSource(filePath);
    
    // 准备播放器
    if (mpPlayer->prepare() != SUCCESS) {
        aloge("Prepare player failed!");
        return false;
    }
    
    mCurrentState = STATE_PREPARED;
    
    // 设置音量为0（确保无声播放）
    mpPlayer->setVolume(0.0f, 0.0f);
    mpPlayer->grantPlayAudio(false);
    
    // 开始播放
    if (mpPlayer->start() != SUCCESS) {
        aloge("Start player failed!");
        return false;
    }
    
    mCurrentState = STATE_PLAYING;
    alogd("Start playing file: %s", filePath.c_str());
    return true;
}

bool MP4Player::StopPlay()
{
    if (mCurrentState == STATE_IDLE) {
        return true;
    }
    
    if (mpPlayer) {
        if (mCurrentState == STATE_PLAYING || mCurrentState == STATE_PAUSED) {
            mpPlayer->stop();
        }
        mpPlayer->reset();
        delete mpPlayer;
        mpPlayer = NULL;
    }
    
    CleanupResources();
    mCurrentState = STATE_STOPPED;
    alogd("Player stopped");
    return true;
}

bool MP4Player::SetVideoSpeed(float number) {
    mfVps = number;
    if (mpPlayer)
    {
        mpPlayer->setVps(mfVps);
        alogd("Video speed set to [%f] successful", mfVps);
        return true;
    }
    aloge("Video speed set fail", mfVps);
    return false;
}

bool MP4Player::SetVolume(float leftVolume, float rightVolume)
{
    if (mCurrentState != STATE_PLAYING && mCurrentState != STATE_PAUSED) {
        aloge("Player not playing!");
        return false;
    }
    
    // 虽然音频有问题，但保留接口
    if (mpPlayer) {
        mpPlayer->setVolume(leftVolume, rightVolume);
        alogd("Volume set to left[%f], right[%f]", leftVolume, rightVolume);
        return true;
    }
    
    return false;
}

bool MP4Player::SeekTo(int milliseconds)
{
    if (mCurrentState != STATE_PLAYING && mCurrentState != STATE_PAUSED) {
        aloge("Player not in playable state!");
        return false;
    }
    
    if (mpPlayer) {
        if (mpPlayer->seekTo(milliseconds) == SUCCESS) {
            alogd("Seek to %d milliseconds", milliseconds);
            return true;
        }
    }
    
    aloge("Seek failed!");
    return false;
}

bool MP4Player::Pause()
{
    if (mCurrentState != STATE_PLAYING) {
        aloge("Player not playing!");
        return false;
    }
    
    if (mpPlayer) {
        mpPlayer->pause();
        mCurrentState = STATE_PAUSED;
        alogd("Player paused");
        return true;
    }
    
    return false;
}

bool MP4Player::Resume()
{
    if (mCurrentState != STATE_PAUSED) {
        aloge("Player not paused!");
        return false;
    }
    
    if (mpPlayer) {
        mpPlayer->start();
        mCurrentState = STATE_PLAYING;
        alogd("Player resumed");
        return true;
    }
    
    return false;
}

int MP4Player::GetCurrentPosition()
{
    if (mpPlayer && (mCurrentState == STATE_PLAYING || mCurrentState == STATE_PAUSED)) {
        return mpPlayer->getCurrentPosition();
    }
    return 0;
}

int MP4Player::GetDuration()
{
    if (mpPlayer && (mCurrentState == STATE_PLAYING || mCurrentState == STATE_PAUSED)) {
        return mpPlayer->getDuration();
    }
    return 0;
}

bool MP4Player::SetRotation(Rotation rotation)
{
    if (mCurrentState == STATE_IDLE) {
        aloge("Player not initialized!");
        return false;
    }
    
    // 检查旋转角度是否有效
    if (rotation < ROTATION_0 || rotation > ROTATION_270) {
        aloge("Invalid rotation value: %d", rotation);
        return false;
    }
    
    mCurrentRotation = rotation;
    
    // 应用旋转设置
    if (!ApplyRotation()) {
        aloge("Apply rotation failed!");
        return false;
    }
    
    alogd("Rotation set to %d degrees", rotation * 90);
    return true;
}

bool MP4Player::ApplyRotation()
{
    if (!mpPlayer) {
        return false;
    }
    
    // 将我们的Rotation枚举转换为EyeseePlayer的ROTATE_E
    ROTATE_E eyeseeRotation;
    switch (mCurrentRotation) {
        case ROTATION_0:
            eyeseeRotation = ROTATE_NONE;
            break;
        case ROTATION_90:
            eyeseeRotation = ROTATE_90;
            break;
        case ROTATION_180:
            eyeseeRotation = ROTATE_180;
            break;
        case ROTATION_270:
            eyeseeRotation = ROTATE_270;
            break;
        default:
            eyeseeRotation = ROTATE_NONE;
            break;
    }
    
    if (mpPlayer->setRotation(eyeseeRotation) != SUCCESS) {
        aloge("Set rotation failed!");
        return false;
    }
    
    return true;
}

bool MP4Player::InitializeMPPSystem()
{
    // 退出MPP系统
    AW_MPI_SYS_Exit();

    // 配置MPP系统
    mSysConf.nAlignWidth = 32;
    AW_MPI_SYS_SetConf(&mSysConf);
    
    if (AW_MPI_SYS_Init() != SUCCESS) {
        aloge("MPI system init failed!");
        return false;
    }
    
    // 设置VDEC频率（使用默认值）
    AW_MPI_VDEC_SetVEFreq(MM_INVALID_CHN, 0);
    
    return true;
}

bool MP4Player::InitializeVideoOutput()
{
    // // 禁用VO设备
    // if (mVoDev >= 0) {
    //     AW_MPI_VO_Disable(mVoDev);
    //     mVoDev = -1;
    // }
    // 启用VO设备
    mVoDev = 0;
    if (AW_MPI_VO_Enable(mVoDev) != SUCCESS) {
        printf("Enable VO device failed!\n");
        return false;
    }
    
    // 设置公共属性
    VO_PUB_ATTR_S stPubAttr;
    AW_MPI_VO_GetPubAttr(mVoDev, &stPubAttr);
    stPubAttr.enIntfType = VO_INTF_LCD;        // 使用LCD接口
    stPubAttr.enIntfSync = VO_OUTPUT_720P60;  // 1080P60输出
    if (AW_MPI_VO_SetPubAttr(mVoDev, &stPubAttr) != SUCCESS) {
        printf("Set VO public attribute failed!\n");
        return false;
    }

    // 添加外部视频层
    AW_MPI_VO_AddOutsideVideoLayer(mUILayer);
    AW_MPI_VO_CloseVideoLayer(mUILayer);  // 关闭UI层

    // 启用视频层
    mVoLayer = 0;
    while (mVoLayer < VO_MAX_LAYER_NUM) {
        if (AW_MPI_VO_EnableVideoLayer(mVoLayer) == SUCCESS) {
            break;
        }
        mVoLayer++;
    }
    if (mVoLayer >= VO_MAX_LAYER_NUM) {
        printf("Enable video layer failed!\n");
        return false;
    }

    // 设置视频层属性 - 严格按照sample代码的方式
    AW_MPI_VO_GetVideoLayerAttr(mVoLayer, &mLayerAttr);

    mLayerAttr.stDispRect.X = mDisplayX;
    mLayerAttr.stDispRect.Y = mDisplayY;
    mLayerAttr.stDispRect.Width = mDisplayWidth;
    mLayerAttr.stDispRect.Height = mDisplayHeight;

    if (AW_MPI_VO_SetVideoLayerAttr(mVoLayer, &mLayerAttr) != SUCCESS) {
        printf("Set video layer attribute failed!\n");
        return false;
    }

    return true;
}

bool MP4Player::CleanupResources()
{
    // 关闭视频层
    if (mVoLayer >= 0) {
        AW_MPI_VO_DisableVideoLayer(mVoLayer);
        mVoLayer = -1;
    }
    
    // 移除外部视频层
    AW_MPI_VO_RemoveOutsideVideoLayer(mUILayer);
    
    // 禁用VO设备
    if (mVoDev >= 0) {
        AW_MPI_VO_Disable(mVoDev);
        mVoDev = -1;
    }
    
    // // 退出MPP系统
    // AW_MPI_SYS_Exit();
    
    return true;
}

