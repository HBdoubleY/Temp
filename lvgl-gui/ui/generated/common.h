#ifndef COMMON_H
#define COMMON_H
// #include "bluetoothadapter.h"

#if defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif
#include "lvgl_system.h"
#include "math.h"
#include "lv_drivers/display/g2d_driver.h"
#include "lv_drivers/display/sample_g2d_mem.h"
#include "hwdisplay.h"
#include "lv_timer.h"

#define DISP_DEVICE "/dev/disp"
#define DISP_FB "/dev/fb0"
#define DISP_CAPTUER_PATH "/mnt/extsd/screenShot"

#define KB 1024.0       // 2^10
#define MB 1048576.0    // 2^20 
#define GB 1073741824.0 // 2^30
#define TF_MOUNT_PIONT "/mnt/extsd" 

#define VIDEO_FRONT_FILE_PATH "/mnt/extsd/recorder/frontCamera" 
#define VIDEO_REAR_FILE_PATH "/mnt/extsd/recorder/rearCamera"
#define URGENT_VIDEO_FRONT_FILE_PATH "/mnt/extsd/recorderUrgent/frontCamera" 
#define URGENT_VIDEO_REAR_FILE_PATH "/mnt/extsd/recorderUrgent/rearCamera"
#define FRONT_PIC_PATH "/mnt/extsd/DVRpic/frontPic"
#define REAR_PIC_PATH "/mnt/extsd/DVRpic/rearPic"



typedef struct {
    unsigned long total;      // 总内存，单位kB
    unsigned long free;       // 空闲内存，单位kB
    unsigned long available;  // 可用内存，单位kB
    unsigned long buffers;    // 缓冲内存，单位kB
    unsigned long cached;     // 缓存内存，单位kB
} MemoryInfo;

//帧信息
typedef struct
{
    g2d_fmt_enh format;
    unsigned int frm_width;
    unsigned int frm_height;
    void *p_vir_addr[3];
    void *p_phy_addr[3];
}FRM_INFO;

typedef struct MPP_SYS_CONF
{
    unsigned int nAlignWidth;
    char mkfcTmpDir[256];
} MPP_SYS_CONF;

typedef struct{
    int RectX;
    int RectY;
    int RectW;
    int RectH;
}G2D_Rect;

typedef struct
{ 
    FRM_INFO src_frm_info;
    FRM_INFO dst_frm_info;
    G2D_Rect mSrcRect;
    G2D_Rect mDstRect;
    FILE *fd_in;
    FILE *fd_out; 
}G2D_CTX;


// BMP文件头结构
#pragma pack(push, 1)
typedef struct {
    unsigned short type;              // 文件类型，必须为"BM"
    unsigned int size;                // 文件大小，字节为单位
    unsigned short reserved1;         // 保留，必须为0
    unsigned short reserved2;         // 保留，必须为0
    unsigned int offset;              // 位图数据偏移量
} BITMAPFILEHEADER;

typedef struct {
    unsigned int size;                // 本结构体大小
    int width;                        // 位图宽度，像素为单位
    int height;                       // 位图高度，像素为单位
    unsigned short planes;            // 位图平面数，必须为1
    unsigned short bit_count;         // 每像素位数
    unsigned int compression;         // 压缩类型
    unsigned int size_image;          // 位图数据大小，字节为单位
    int x_pels_per_meter;             // 水平分辨率
    int y_pels_per_meter;             // 垂直分辨率
    unsigned int clr_used;            // 使用的颜色索引数
    unsigned int clr_important;       // 重要颜色索引数
} BITMAPINFOHEADER;

int MPI_init(void);
int G2dConvert_scale(G2D_CTX *p_g2d_ctx, int g2dfd);
int G2dConvert_rotate(G2D_CTX *p_g2d_ctx, int g2dfd, g2d_blt_flags_h ops);
int G2dConvert_formatconversion(G2D_CTX *p_g2d_ctx, int g2dfd);

//压力单位转换 
float PressureUnitConversion(float pressureData);
//温度单位转换
float TempUnitConversion(int tempData);

void systemMeminfo(MemoryInfo* mem_info);
void SetBackLight(int brightness);
int createLightPerceptionThread(void);
void destoryLightPerceptionThread();
bool checkTFCardMountProc(void);
int day_of_week_kim_larson(int year, int month, int day);

void RestorefactorySettings(void);

void GetDiskSpace(uint64_t *totalsize, uint64_t *freeDisk);

bool screenshot_thread_start(void);
bool screenshot_thread_stop(void);

#if defined(__cplusplus)||defined(c_plusplus)
}
#endif
#endif
