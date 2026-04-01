#ifndef _EASY_PLAYER_CAPI_H_
#define _EASY_PLAYER_CAPI_H_

#ifdef __cplusplus
extern "C" {
#endif

// C语言句柄
typedef void* EasyPlayerHandle;

// 播放器状态
typedef enum {
    EASY_PLAYER_STATE_IDLE = 0,
    EASY_PLAYER_STATE_INITIALIZED,
    EASY_PLAYER_STATE_PREPARED,
    EASY_PLAYER_STATE_PLAYING,
    EASY_PLAYER_STATE_PAUSED,
    EASY_PLAYER_STATE_STOPPED,
    EASY_PLAYER_STATE_ERROR
} EasyPlayerState;

// 旋转角度
typedef enum {
    EASY_PLAYER_ROTATION_0 = 0,     // 0度
    EASY_PLAYER_ROTATION_90 = 1,    // 90度顺时针
    EASY_PLAYER_ROTATION_180 = 2,   // 180度
    EASY_PLAYER_ROTATION_270 = 3    // 270度顺时针
} EasyPlayerRotation;

typedef enum {
    EASY_PLAYER_SUCCESS = 0,
    EASY_PLAYER_ERROR_INVALID_HANDLE = -1,
    EASY_PLAYER_ERROR_INIT_FAILED = -2,
    EASY_PLAYER_ERROR_FILE_NOT_EXIST = -3,
    EASY_PLAYER_ERROR_PLAY_FAILED = -4,
    EASY_PLAYER_ERROR_INVALID_PARAM = -5,
    EASY_PLAYER_ERROR_WRONG_STATE = -6,
    EASY_PLAYER_ERROR_UNKNOWN = -99
} EasyPlayerResult;

/**
 * @brief 创建播放器实例
 * @return 成功返回播放器句柄，失败返回NULL
 */
EasyPlayerHandle easy_player_create(void);

/**
 * @brief 销毁播放器实例并释放资源
 * @param handle 播放器句柄
 * @return 成功返回EASY_PLAYER_SUCCESS，失败返回错误码
 */
EasyPlayerResult easy_player_destroy(EasyPlayerHandle handle);

/**
 * @brief 初始化播放器
 * @param handle 播放器句柄
 * @param displayWidth 显示宽度
 * @param displayHeight 显示高度
 * @return 成功返回EASY_PLAYER_SUCCESS，失败返回错误码
 */
EasyPlayerResult easy_player_initialize(EasyPlayerHandle handle, int displayWidth, int displayHeight);

/**
 * @brief 设置显示区域
 * @param handle 播放器句柄
 * @param x 显示区域X坐标
 * @param y 显示区域Y坐标
 * @param width 显示宽度
 * @param height 显示高度
 * @return 成功返回EASY_PLAYER_SUCCESS，失败返回错误码
 */
EasyPlayerResult easy_player_set_display_rect(EasyPlayerHandle handle, int x, int y, int width, int height);

/**
 * @brief 播放指定文件
 * @param handle 播放器句柄
 * @param filePath 文件路径
 * @return 成功返回EASY_PLAYER_SUCCESS，失败返回错误码
 */
EasyPlayerResult easy_player_play_file(EasyPlayerHandle handle, const char* filePath);

/**
 * @brief 停止播放
 * @param handle 播放器句柄
 * @return 成功返回EASY_PLAYER_SUCCESS，失败返回错误码
 */
EasyPlayerResult easy_player_stop(EasyPlayerHandle handle);

/**
 * @brief 暂停播放
 * @param handle 播放器句柄
 * @return 成功返回EASY_PLAYER_SUCCESS，失败返回错误码
 */
EasyPlayerResult easy_player_pause(EasyPlayerHandle handle);

/**
 * @brief 恢复播放
 * @param handle 播放器句柄
 * @return 成功返回EASY_PLAYER_SUCCESS，失败返回错误码
 */
EasyPlayerResult easy_player_resume(EasyPlayerHandle handle);

/**
 * @brief 跳转到指定位置
 * @param handle 播放器句柄
 * @param milliseconds 目标位置（毫秒）
 * @return 成功返回EASY_PLAYER_SUCCESS，失败返回错误码
 */
EasyPlayerResult easy_player_seek_to(EasyPlayerHandle handle, int milliseconds);

/**
 * @brief 设置音量
 * @param handle 播放器句柄
 * @param leftVolume 左声道音量（0.0-1.0）
 * @param rightVolume 右声道音量（0.0-1.0）
 * @return 成功返回EASY_PLAYER_SUCCESS，失败返回错误码
 */
EasyPlayerResult easy_player_set_volume(EasyPlayerHandle handle, float leftVolume, float rightVolume);

/**
 * @brief 设置旋转角度
 * @param handle 播放器句柄
 * @param rotation 旋转角度
 * @return 成功返回EASY_PLAYER_SUCCESS，失败返回错误码
 */
EasyPlayerResult easy_player_set_rotation(EasyPlayerHandle handle, EasyPlayerRotation rotation);

/**
 * @brief 获取当前播放位置
 * @param handle 播放器句柄
 * @param position 输出参数：当前播放位置（毫秒）
 * @return 成功返回EASY_PLAYER_SUCCESS，失败返回错误码
 */
EasyPlayerResult easy_player_get_current_position(EasyPlayerHandle handle, int* position);

/**
 * @brief 获取视频总时长
 * @param handle 播放器句柄
 * @param duration 输出参数：视频总时长（毫秒）
 * @return 成功返回EASY_PLAYER_SUCCESS，失败返回错误码
 */
EasyPlayerResult easy_player_get_duration(EasyPlayerHandle handle, int* duration);

/**
 * @brief 获取播放器状态
 * @param handle 播放器句柄
 * @param state 输出参数：播放器状态
 * @return 成功返回EASY_PLAYER_SUCCESS，失败返回错误码
 */
EasyPlayerResult easy_player_get_state(EasyPlayerHandle handle, EasyPlayerState* state);

/**
 * @brief 获取当前旋转角度
 * @param handle 播放器句柄
 * @param rotation 输出参数：当前旋转角度
 * @return 成功返回EASY_PLAYER_SUCCESS，失败返回错误码
 */
EasyPlayerResult easy_player_get_rotation(EasyPlayerHandle handle, EasyPlayerRotation* rotation);

/**
 * @brief 检查是否正在播放
 * @param handle 播放器句柄
 * @return 正在播放返回1，否则返回0，错误返回-1
 */
int easy_player_is_playing(EasyPlayerHandle handle);

/**
 * @brief 获取错误码描述信息
 * @param result 错误码
 * @return 错误描述字符串
 */
const char* easy_player_get_error_string(EasyPlayerResult result);

/**
 * @brief set video speed
 * @param handle 播放器句柄
 * @return 成功返回EASY_PLAYER_SUCCESS，失败返回错误码
 */
EasyPlayerResult easy_player_set_video_speed(EasyPlayerHandle handle, float num);

#ifdef __cplusplus
}
#endif

#endif /* _EASY_PLAYER_CAPI_H_ */