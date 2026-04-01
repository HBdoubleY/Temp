#ifndef _EASY_PLAYER_H_
#define _EASY_PLAYER_H_

#include <string>
#include <mm_comm_sys.h>
#include <mm_comm_video.h>
#include <mm_comm_vo.h>
#include <tsemaphore.h>
#include <EyeseePlayer.h>

class MP4Player
{
public:
    // 播放器状态
    enum PlayerState {
        STATE_IDLE = 0,
        STATE_INITIALIZED,
        STATE_PREPARED,
        STATE_PLAYING,
        STATE_PAUSED,
        STATE_STOPPED,
        STATE_ERROR
    };

    // 旋转角度
    enum Rotation {
        ROTATION_0 = 0,     // 0度
        ROTATION_90 = 1,    // 90度顺时针
        ROTATION_180 = 2,   // 180度
        ROTATION_270 = 3    // 270度顺时针
    };

    MP4Player();
    ~MP4Player();

    bool Initialize(int displayWidth = 640, int displayHeight = 360);
    bool SetDisplayRect(int x, int y, int width, int height);
    bool PlayFile(const std::string& filePath);
    bool StopPlay();
    bool SetVolume(float leftVolume, float rightVolume);
    bool SeekTo(int milliseconds);
    bool SetVideoSpeed(float number);
    bool Pause();
    bool Resume();
    int GetCurrentPosition();
    int GetDuration();
    bool SetRotation(Rotation rotation);
    Rotation GetRotation() const { return mCurrentRotation; }
    PlayerState GetState() const { return mCurrentState; }
    bool IsPlaying() const { return mCurrentState == STATE_PLAYING; }
    bool CreateAndBindVOChannel();

private:
    bool InitializeMPPSystem();
    bool InitializeVideoOutput();
    bool CleanupResources();
    
    bool ApplyRotation();
    
    class PlayerListener : public EyeseeLinux::EyeseePlayer::OnCompletionListener
                        , public EyeseeLinux::EyeseePlayer::OnErrorListener
                        , public EyeseeLinux::EyeseePlayer::OnVideoSizeChangedListener
                        , public EyeseeLinux::EyeseePlayer::OnInfoListener
                        , public EyeseeLinux::EyeseePlayer::OnSeekCompleteListener
    {
    public:
        PlayerListener(MP4Player* pOwner);
        void onCompletion(EyeseeLinux::EyeseePlayer* pMp);
        bool onError(EyeseeLinux::EyeseePlayer* pMp, int what, int extra);
        void onVideoSizeChanged(EyeseeLinux::EyeseePlayer* pMp, int width, int height);
        bool onInfo(EyeseeLinux::EyeseePlayer* pMp, int what, int extra);
        void onSeekComplete(EyeseeLinux::EyeseePlayer* pMp);
        
    private:
        MP4Player* const mpOwner;
    };

private:
    PlayerState mCurrentState;
    Rotation mCurrentRotation;

    MPP_SYS_CONF_S mSysConf;
    int mUILayer;
    
    VO_DEV mVoDev;
    VO_LAYER mVoLayer;
    VO_VIDEO_LAYER_ATTR_S mLayerAttr;
    
    EyeseeLinux::EyeseePlayer* mpPlayer;
    PlayerListener mPlayerListener;
    
    // 显示
    int mDisplayX;
    int mDisplayY;
    int mDisplayWidth;
    int mDisplayHeight;
    
    float mfVps;

    // 同步信号
    cdx_sem_t mSemExit;
    
    MP4Player(const MP4Player&);
    MP4Player& operator=(const MP4Player&);

    int mVoChn;  // 视频输出通道
};

#endif  /* _EASY_PLAYER_H_ */