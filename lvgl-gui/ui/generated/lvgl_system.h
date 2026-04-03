#ifndef SYSTEM_H
#define SYSTEM_H
#ifdef __cplusplus
extern "C" {
#endif  

#include "i18n.h"
#include "linksupport.h"
#include "mpp_camera.h"
#include "ComStruct.h"

#define DEVICE_MODEL    "L310"
#define DEVICE_SERIAL_NUMBER    "V853-20260331_DVR"  
#define APP_VERSION "V853DVR-20260402-V1"
#define HARDWARE_VERSION    "MOTO_V853_V0.2"
#define DVR_SET_SOFTWARE_VERSION    "v1.0.0"

extern const char* app_language[];

typedef enum THEME_MODE{
    THEME_DAY = 0x0,
    THEME_DARK,
    THEME_AUTO,
    THEME_NONO,
}THEME_MODE;

typedef enum SPLITSCREEN_DISP{
    DISPLAY_NORMAL,
    DISPLAY_FRONT,
    DISPLAY_REAR,
    DISPLAY_FRONT_REAR,
}SPLITSCREEN_DISP;

typedef enum PREVIEW_MODE{
    PREVIEW_NONE = 0x0,
    PREVIEW_FRONT,
    PREVIEW_REAR,
    PREVIEW_FRONT_REAR,
}preview_mode;

typedef enum RECORDER_TIME{
    RECORDER_1 = 0x1,
    RECORDER_3,
    RECORDER_5,
}recorder_time;//recording time


//不支持  缩放 和 编码前图像处理   同时启用
typedef enum RECORDER_RESOLUTION{
    RECORDER_480P = 0x0,
    RECORDER_720P,
    RECORDER_1080P,
}recorder_resolution;//recorder resolution

typedef enum RECORDER_MODE{
    RECORDER_NONE = 0x0,
    RECORDER_NORMAL,
    RECORDER_URGENT,
}recorder_mode;//recorder mode

typedef enum SCREENSAVETIME{
    T_10S = 10,
    T_30S = 30,
    T_1MIN = 60,
    T_5MIN = 5*60,
    T_10MIN = 10*60,
    T_30MIN = 30*60,
}SCREENSAVETIME;

typedef struct SET_AGINGMODE{
    bool memInfoSW;
    bool netBandWidth;
    bool touchPointSW;
    bool screenSaveSw;
    SCREENSAVETIME screenSaveTime;
    bool scrennShotSW;
}SET_AGINGMODE;

typedef struct{
    LinkType linktype;  //car link type
    int eventtype;//
    bool TFmounted;
    int gpadcVol; //unit:mv
    bool carTripSwitch; //false: close; true:open;
    bool pressureUnit; //false:bar ture:psi
    bool tempUnit;  //false:°C ture：°F
    int tempMax;
    int fTemp;
    int bTemp;
    float pressureMax;
    float pressureMin;
    float fPressure;
    float bPressure;
    int backlight;
    THEME_MODE themeMode;
    language_t current_language;
    bool resetFactory; 
    bool lockScreenFlag;
    bool frontCamera;
    bool rearCamera;

    int carSpeed;
    bool ScreenPlayBackOut;
    mpp_camera_para_conf vipp0_config;
    mpp_camera_para_conf vipp8_config;
    bool powerOnRecorder;
    preview_mode previewMode;
    recorder_time recorderTime;
    recorder_time recorderUrgentTime;
    // recorder_resolution recorderResolution;
    recorder_mode recorderMode;
    bool SoundRecorder;
    bool rearCameraImage;
    bool SpeedMark;
    bool TimeMark;
    bool SpeedUnit;
    SPLITSCREEN_DISP splitScreenDisp;

    SET_AGINGMODE agingMode;
}sys_data;

sys_data g_sys_Data;


#ifdef __cplusplus
}
#endif
#endif
