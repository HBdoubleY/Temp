#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "sys/statvfs.h"
#include <sys/ioctl.h>
#include "sys/mman.h"
#include <dirent.h>
#include <limits.h>
#include <sys/reboot.h>
#include "lvgl.h"
#include "gui_guider.h"

#ifndef AWALIGN
#define AWALIGN(x, a)              ((a) * (((x) + (a) - 1) / (a)))
#endif

static int dispfd = -1;
static pthread_t g_screenshot_thread = 0;
static bool g_thread_should_exit = false;

static pthread_t LightThreadId = -1;
static bool LightThreadFlag = false;
#define GPADC2_PATH "/sys/class/gpadc/data"
int MPI_init(void){
    printf("#######--------------%s---%d---------------------\n",__func__,__LINE__);

    MPP_SYS_CONF sysconf;
    memset(&sysconf, 0 , sizeof(MPP_SYS_CONF));
    sysconf.nAlignWidth = 32;
    AW_MPI_SYS_SetConf(&sysconf);
    int ret = AW_MPI_SYS_Init();
    if(ret < 0){
        printf("sys MPI Init failed!");
        return -1;
    }
    g2d_MemOpen();//ion_mem开启
    return 0;
}

int G2dConvert_scale(G2D_CTX *p_g2d_ctx, int g2dfd)
{
    printf("#######--------------%s---%d---------------------\n",__func__,__LINE__);

    int ret = 0;
    g2d_blt_h blit;
    //config blit
    memset(&blit, 0, sizeof(g2d_blt_h));

    blit.flag_h = G2D_BLT_NONE_H;       // angle rotation used
    blit.src_image_h.format = p_g2d_ctx->src_frm_info.format;
    blit.src_image_h.laddr[0] = (unsigned int)p_g2d_ctx->src_frm_info.p_phy_addr[0];
    blit.src_image_h.laddr[1] = (unsigned int)p_g2d_ctx->src_frm_info.p_phy_addr[1];
    blit.src_image_h.laddr[2] = (unsigned int)p_g2d_ctx->src_frm_info.p_phy_addr[2];
    //blit.src_image_h.haddr[] = 
    blit.src_image_h.width = p_g2d_ctx->src_frm_info.frm_width;
    blit.src_image_h.height = p_g2d_ctx->src_frm_info.frm_height;
    blit.src_image_h.align[0] = 0;
    blit.src_image_h.align[1] = 0;
    blit.src_image_h.align[2] = 0;
    blit.src_image_h.clip_rect.x = p_g2d_ctx->mSrcRect.RectX;
    blit.src_image_h.clip_rect.y = p_g2d_ctx->mSrcRect.RectY;
    blit.src_image_h.clip_rect.w = p_g2d_ctx->mSrcRect.RectW;
    blit.src_image_h.clip_rect.h = p_g2d_ctx->mSrcRect.RectH;
    blit.src_image_h.gamut = G2D_BT709;
    blit.src_image_h.bpremul = 0;
    //blit.src_image_h.alpha = 0xff;
    blit.src_image_h.mode = G2D_PIXEL_ALPHA;   //G2D_PIXEL_ALPHA, G2D_GLOBAL_ALPHA
    blit.src_image_h.fd = -1;
    blit.src_image_h.use_phy_addr = 1;

    //blit.dst_image_h.bbuff = 1;
    //blit.dst_image_h.color = 0xff;
    blit.dst_image_h.format = p_g2d_ctx->dst_frm_info.format;
    blit.dst_image_h.laddr[0] = (unsigned int)p_g2d_ctx->dst_frm_info.p_phy_addr[0];
    blit.dst_image_h.laddr[1] = (unsigned int)p_g2d_ctx->dst_frm_info.p_phy_addr[1];
    blit.dst_image_h.laddr[2] = (unsigned int)p_g2d_ctx->dst_frm_info.p_phy_addr[2];
    //blit.dst_image_h.haddr[] = 
    blit.dst_image_h.width = p_g2d_ctx->dst_frm_info.frm_width;
    blit.dst_image_h.height = p_g2d_ctx->dst_frm_info.frm_height;
    blit.dst_image_h.align[0] = 0;
    blit.dst_image_h.align[1] = 0;
    blit.dst_image_h.align[2] = 0;
    blit.dst_image_h.clip_rect.x = p_g2d_ctx->mDstRect.RectX;
    blit.dst_image_h.clip_rect.y = p_g2d_ctx->mDstRect.RectY;
    blit.dst_image_h.clip_rect.w = p_g2d_ctx->mDstRect.RectW;
    blit.dst_image_h.clip_rect.h = p_g2d_ctx->mDstRect.RectH;
    blit.dst_image_h.gamut = G2D_BT709;
    blit.dst_image_h.bpremul = 0;
    //blit.dst_image_h.alpha = 0xff;
    blit.dst_image_h.mode = G2D_PIXEL_ALPHA;   //G2D_PIXEL_ALPHA, G2D_GLOBAL_ALPHA
    blit.dst_image_h.fd = -1;
    blit.dst_image_h.use_phy_addr = 1;

    ret = ioctl(g2dfd, G2D_CMD_BITBLT_H, (unsigned long)&blit);
    if(ret < 0)
    {
        printf("fatal error! bit-block(image) transfer failed[%d]", ret);
        system("cd /sys/class/sunxi_dump;echo 0x14A8000,0x14A8100 > dump;cat dump");
    }
    return ret;
}

int G2dConvert_rotate(G2D_CTX *p_g2d_ctx, int g2dfd, g2d_blt_flags_h ops)
{
    printf("#######--------------%s---%d---------------------\n",__func__,__LINE__);

    int ret = 0;
    g2d_blt_h blit;
    memset(&blit, 0, sizeof(g2d_blt_h));

    blit.flag_h = ops;
    blit.src_image_h.format = p_g2d_ctx->src_frm_info.format;
    blit.src_image_h.laddr[0] = (unsigned int)p_g2d_ctx->src_frm_info.p_phy_addr[0];
    blit.src_image_h.laddr[1] = (unsigned int)p_g2d_ctx->src_frm_info.p_phy_addr[1];
    blit.src_image_h.laddr[2] = (unsigned int)p_g2d_ctx->src_frm_info.p_phy_addr[2];
    //blit.src_image_h.haddr[] = 
    blit.src_image_h.width = p_g2d_ctx->src_frm_info.frm_width;
    blit.src_image_h.height = p_g2d_ctx->src_frm_info.frm_height;
    blit.src_image_h.align[0] = 0;
    blit.src_image_h.align[1] = 0;
    blit.src_image_h.align[2] = 0;
    blit.src_image_h.clip_rect.x = p_g2d_ctx->mSrcRect.RectX;
    blit.src_image_h.clip_rect.y = p_g2d_ctx->mSrcRect.RectY;
    blit.src_image_h.clip_rect.w = p_g2d_ctx->mSrcRect.RectW;
    blit.src_image_h.clip_rect.h = p_g2d_ctx->mSrcRect.RectH;
    blit.src_image_h.gamut = G2D_BT709;
    blit.src_image_h.bpremul = 0;
    blit.src_image_h.mode = G2D_PIXEL_ALPHA;   //G2D_PIXEL_ALPHA, G2D_GLOBAL_ALPHA
    blit.src_image_h.fd = -1;
    blit.src_image_h.use_phy_addr = 1;

    blit.dst_image_h.format = p_g2d_ctx->dst_frm_info.format;
    blit.dst_image_h.laddr[0] = (unsigned int)p_g2d_ctx->dst_frm_info.p_phy_addr[0];
    blit.dst_image_h.laddr[1] = (unsigned int)p_g2d_ctx->dst_frm_info.p_phy_addr[1];
    blit.dst_image_h.laddr[2] = (unsigned int)p_g2d_ctx->dst_frm_info.p_phy_addr[2];
    //blit.dst_image_h.haddr[] = 
    blit.dst_image_h.width = p_g2d_ctx->dst_frm_info.frm_width;
    blit.dst_image_h.height = p_g2d_ctx->dst_frm_info.frm_height;
    blit.dst_image_h.align[0] = 0;
    blit.dst_image_h.align[1] = 0;
    blit.dst_image_h.align[2] = 0;
    blit.dst_image_h.clip_rect.x = p_g2d_ctx->mDstRect.RectX;
    blit.dst_image_h.clip_rect.y = p_g2d_ctx->mDstRect.RectY;
    blit.dst_image_h.clip_rect.w = p_g2d_ctx->mDstRect.RectW;
    blit.dst_image_h.clip_rect.h = p_g2d_ctx->mDstRect.RectH;
    blit.dst_image_h.gamut = G2D_BT709;
    blit.dst_image_h.bpremul = 0;
    //blit.dst_image_h.alpha = 0xff;
    blit.dst_image_h.mode = G2D_PIXEL_ALPHA;   //G2D_PIXEL_ALPHA, G2D_GLOBAL_ALPHA
    blit.dst_image_h.fd = -1;
    blit.dst_image_h.use_phy_addr = 1;

    ret = ioctl(g2dfd, G2D_CMD_BITBLT_H, (unsigned long)&blit);
    if(ret < 0)
    {
        printf("fatal error! bit-block(image) transfer failed[%d]", ret);
        system("cd /sys/class/sunxi_dump;echo 0x14A8000,0x14A8100 > dump;cat dump");
    }

    return ret;
} 

int G2dConvert_formatconversion(G2D_CTX *p_g2d_ctx, int g2dfd)
{
    printf("#######--------------%s---%d---------------------\n",__func__,__LINE__);

    int ret = 0;
    g2d_blt_h blit;

    memset(&blit, 0, sizeof(g2d_blt_h));

    blit.flag_h = G2D_BLT_NONE_H;       // angle rotation used
    blit.src_image_h.format = p_g2d_ctx->src_frm_info.format;
    blit.src_image_h.laddr[0] = (unsigned int)p_g2d_ctx->src_frm_info.p_phy_addr[0];
    blit.src_image_h.laddr[1] = (unsigned int)p_g2d_ctx->src_frm_info.p_phy_addr[1];
    blit.src_image_h.laddr[2] = (unsigned int)p_g2d_ctx->src_frm_info.p_phy_addr[2];
    //blit.src_image_h.haddr[] =
    blit.src_image_h.width = p_g2d_ctx->src_frm_info.frm_width;
    blit.src_image_h.height = p_g2d_ctx->src_frm_info.frm_height;
    blit.src_image_h.align[0] = 0;
    blit.src_image_h.align[1] = 0;
    blit.src_image_h.align[2] = 0;
    blit.src_image_h.clip_rect.x = p_g2d_ctx->mSrcRect.RectX;
    blit.src_image_h.clip_rect.y = p_g2d_ctx->mSrcRect.RectY;
    blit.src_image_h.clip_rect.w = p_g2d_ctx->mSrcRect.RectW;
    blit.src_image_h.clip_rect.h = p_g2d_ctx->mSrcRect.RectH;
    blit.src_image_h.gamut = G2D_BT709;
    blit.src_image_h.bpremul = 0;
    //blit.src_image_h.alpha = 0xff;
    blit.src_image_h.mode = G2D_PIXEL_ALPHA;   //G2D_PIXEL_ALPHA, G2D_GLOBAL_ALPHA
    blit.src_image_h.fd = -1;
    blit.src_image_h.use_phy_addr = 1;

    blit.dst_image_h.format = p_g2d_ctx->dst_frm_info.format;
    blit.dst_image_h.laddr[0] = (unsigned int)p_g2d_ctx->dst_frm_info.p_phy_addr[0];
    blit.dst_image_h.laddr[1] = (unsigned int)p_g2d_ctx->dst_frm_info.p_phy_addr[1];
    blit.dst_image_h.laddr[2] = (unsigned int)p_g2d_ctx->dst_frm_info.p_phy_addr[2];
    //blit.dst_image_h.haddr[] =
    blit.dst_image_h.width = p_g2d_ctx->dst_frm_info.frm_width;
    blit.dst_image_h.height = p_g2d_ctx->dst_frm_info.frm_height;
    blit.dst_image_h.align[0] = 0;
    blit.dst_image_h.align[1] = 0;
    blit.dst_image_h.align[2] = 0;
    blit.dst_image_h.clip_rect.x = p_g2d_ctx->mDstRect.RectX;
    blit.dst_image_h.clip_rect.y = p_g2d_ctx->mDstRect.RectY;
    blit.dst_image_h.clip_rect.w = p_g2d_ctx->mDstRect.RectW;
    blit.dst_image_h.clip_rect.h = p_g2d_ctx->mDstRect.RectH;
    blit.dst_image_h.gamut = G2D_BT709;
    blit.dst_image_h.bpremul = 0;
    //blit.dst_image_h.alpha = 0xff;
    blit.dst_image_h.mode = G2D_PIXEL_ALPHA;   //G2D_PIXEL_ALPHA, G2D_GLOBAL_ALPHA
    blit.dst_image_h.fd = -1;
    blit.dst_image_h.use_phy_addr = 1;

    ret = ioctl(g2dfd, G2D_CMD_BITBLT_H, (unsigned long)&blit);
    if(ret < 0)
    {
        printf("fatal error! bit-block(image) transfer failed[%d]", ret);
        system("cd /sys/class/sunxi_dump;echo 0x14A8000,0x14A8100 > dump;cat dump");
    }

    return ret;
}

float PressureUnitConversion(float pressureData){
    printf("#######--------------%s---%d---------------------\n",__func__,__LINE__);

    float result = 0;
    if(!g_sys_Data.pressureUnit){//bar
        result = pressureData/14.5;       
    }else{
        result = pressureData*14.5;
    }
    return result;
}

float TempUnitConversion(int tempData){
    printf("#######--------------%s---%d---------------------\n",__func__,__LINE__);

    float result = 0;
    if(!g_sys_Data.tempUnit){//°C
        result = (tempData * 9/5) + 32;
    }else{
        result = (tempData - 32) * 5/9;
    }
    return result;
}

void SetBackLight(int brightness){
    printf("#######--------------%s---%d---------------------\n",__func__,__LINE__);
    char cmd[128];
    int value = 255 - (brightness * 255 + 50) / 100;
    if(dispfd < 0){
        dispfd = open(DISP_DEVICE, O_RDWR);
        if (dispfd < 0) {
            printf("open display dev fail!\n");
            return -1;
        }
    }
    unsigned long para[3] = {};
    para[0] = 0;
    para[1] = value;
    ioctl(dispfd, DISP_LCD_SET_BRIGHTNESS, (void *)para);
}

static void *GpadcVoltageValueThread(void *arg){
    printf("%s:%d\n",__func__,__LINE__);
    system("echo gpadc2,1 > /sys/class/gpadc/status");
    system("echo 2 > /sys/class/gpadc/data");
    system("sync");
    int gpadc_fd = open(GPADC2_PATH, O_RDONLY | O_NONBLOCK);
    if(gpadc_fd <= 0){
        printf("open dev err:%s!!!\n",GPADC2_PATH);
        return;
    }
    while (LightThreadFlag)
    {
        char data[24]= {};
        int vol = 0;

            lseek(gpadc_fd, 0, SEEK_SET);
            int ret = read(gpadc_fd,&data,sizeof(data));
            if(ret >= 0){
                data[ret] = '\0';
                g_sys_Data.gpadcVol = atoi(data);
                // printf("adc data,vol:%dmv!!!\n",g_sys_Data.gpadcVol);
        }
        sleep(1);
    }
    
    close(gpadc_fd);
    return;
}

int createLightPerceptionThread(void){
    printf("%s:%d\n",__func__,__LINE__);
    if(LightThreadFlag) return;
    LightThreadFlag = true;
    int ret = pthread_create(&LightThreadId, NULL, GpadcVoltageValueThread, NULL);
    if(ret < 0){
        printf("creat GpadcVoltageValueThread failed\n");
        return -1;
    }else{
        pthread_detach(LightThreadId);
        printf("creat GpadcVoltageValueThread success\n");
        return 0;
    }
}

void destoryLightPerceptionThread(){
    printf("%s:%d\n",__func__,__LINE__);
    LightThreadFlag = false;
}

void GetDiskSpace(uint64_t *totalsize, uint64_t *freeDisk){
    printf("%s:%d\n",__func__,__LINE__);

    struct statvfs vfs;
    if (statvfs(TF_MOUNT_PIONT, &vfs) != 0) {
        perror("获取文件系统信息失败");
        printf("请确保 %s 已正确挂载\n", TF_MOUNT_PIONT);
        return;
    }
    
    // 计算总容量和空闲容量（以字节为单位）
    *totalsize = (unsigned long long)vfs.f_blocks * vfs.f_frsize;
    *freeDisk = (unsigned long long)vfs.f_bfree * vfs.f_frsize;
    uint64_t availableDisk = (unsigned long long)vfs.f_bavail * vfs.f_frsize;
    uint64_t usedDisk = totalsize - freeDisk;
    // printf("totalsize:%lld, %fMB, %fGB;\n freeDisk:%lld, %fMB, %fGB;\n availableDisk:%lld, %fMB, %fGB;\n", totalsize, *totalsize/MB, *totalsize/GB, freeDisk, *freeDisk/MB,*freeDisk/GB, availableDisk,availableDisk/MB,availableDisk/GB);
}

bool checkTFCardMountProc(void){
//    printf("%s:%d\n",__func__,__LINE__);
    if(access("/dev/mmcblk0", F_OK) == 0){
        return true;
    }

    return false;    
}

int day_of_week_kim_larson(int year, int month, int day) {
    // 注意：1月和2月要当作上一年的13月和14月
    if (month < 3) {
        month += 12;
        year -= 1;
    }
    
    // 基姆拉尔森公式
    int weekday = (day + 2*month + 3*(month+1)/5 + year + year/4 - year/100 + year/400) % 7;
    
    // 返回0-6，0表示星期一，1表示星期二，...，6表示星期日
    return weekday;
}

void RestorefactorySettings(void){
    printf("%s:%d\n",__func__,__LINE__);
    if (access("/opt/work/storage_data.txt", F_OK) == 0) {
        system("rm /opt/work/storage_data.txt"); 
    }
    sync(); // 同步磁盘数据
    reboot(RB_AUTOBOOT); 
}

void systemMeminfo(MemoryInfo* mem_info){
    FILE* fp = fopen("/proc/meminfo", "r");
    if (fp == NULL) {
        printf("无法打开/proc/meminfo\n");
        return -1;
    }
    char line[256];
    memset(mem_info, 0, sizeof(MemoryInfo));
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "MemTotal: %lu kB", &mem_info->total) == 1) {
            continue;
        } else if (sscanf(line, "MemFree: %lu kB", &mem_info->free) == 1) {
            continue;
        } else if (sscanf(line, "MemAvailable: %lu kB", &mem_info->available) == 1) {
            continue;
        } else if (sscanf(line, "Buffers: %lu kB", &mem_info->buffers) == 1) {
            continue;
        } else if (sscanf(line, "Cached: %lu kB", &mem_info->cached) == 1) {
            continue;
        }
    }
    fclose(fp);
    return 0;  
}

// 保存为BMP文件
int save_as_bmp(const char *filename, unsigned char *data, unsigned int width, unsigned int height, unsigned int format) {
    FILE *fp = NULL;
    BITMAPFILEHEADER bmfh;
    BITMAPINFOHEADER bmih;
    unsigned char *bgr_buffer = NULL;
    unsigned int row, col;
    unsigned int bytes_per_pixel = 0;
    unsigned int bytes_per_line = 0;
    
    switch (format) {
        case DISP_FORMAT_ARGB_8888:
            bytes_per_pixel = 4;
            break;
        case DISP_FORMAT_RGB_888:
        case DISP_FORMAT_BGR_888:
            bytes_per_pixel = 3;
            break;
        case DISP_FORMAT_RGB_565:
        case DISP_FORMAT_BGR_565:
            bytes_per_pixel = 2;
            break;
        default:
            fprintf(stderr, "不支持的像素格式: 0x%x\n", format);
            return -1;
    }
    

    bytes_per_line = width * 3;  
    if (bytes_per_line % 4 != 0) {
        bytes_per_line = (bytes_per_line + 3) & ~3;  
    }
    
    // 分配BGR缓冲区
    bgr_buffer = (unsigned char *)malloc(height * bytes_per_line);
    if (bgr_buffer == NULL) {
        fprintf(stderr, "错误：无法分配BGR缓冲区\n");
        return -1;
    }
    
    // 转换颜色格式
    if (format == DISP_FORMAT_ARGB_8888) {

        for (row = 0; row < height; row++) {
            for (col = 0; col < width; col++) {
                unsigned int src_idx = (row * width + col) * 4;
                unsigned int dst_idx = (height - 1 - row) * bytes_per_line + col * 3;
                
               
                bgr_buffer[dst_idx + 0] = data[src_idx + 0];  // B
                bgr_buffer[dst_idx + 1] = data[src_idx + 1];  // G
                bgr_buffer[dst_idx + 2] = data[src_idx + 2];  // R
            }
        }
    } else if (format == DISP_FORMAT_RGB_888) {

        for (row = 0; row < height; row++) {
            for (col = 0; col < width; col++) {
                unsigned int src_idx = (row * width + col) * 3;
                unsigned int dst_idx = (height - 1 - row) * bytes_per_line + col * 3;
                
                bgr_buffer[dst_idx + 0] = data[src_idx + 0];  // B
                bgr_buffer[dst_idx + 1] = data[src_idx + 1];  // G
                bgr_buffer[dst_idx + 2] = data[src_idx + 2];  // R
            }
        }
    } else {
        fprintf(stderr, "警告：格式转换未完全实现，直接复制数据\n");
        memcpy(bgr_buffer, data, width * height * bytes_per_pixel);
    }
 
    memset(&bmfh, 0, sizeof(bmfh));
    bmfh.type = 0x4D42;  // "BM"
    bmfh.size = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 
                bytes_per_line * height;
    bmfh.offset = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    

    memset(&bmih, 0, sizeof(bmih));
    bmih.size = sizeof(BITMAPINFOHEADER);
    bmih.width = width;
    bmih.height = height;
    bmih.planes = 1;
    bmih.bit_count = 24;  
    bmih.size_image = bytes_per_line * height;
    bmih.x_pels_per_meter = 3780;  
    bmih.y_pels_per_meter = 3780;  

    fp = fopen(filename, "wb");
    if (fp == NULL) {
        perror("错误：无法创建BMP文件");
        free(bgr_buffer);
        return -1;
    }
    
    fwrite(&bmfh, sizeof(bmfh), 1, fp);
    fwrite(&bmih, sizeof(bmih), 1, fp);
    fwrite(bgr_buffer, 1, bytes_per_line * height, fp);
    fsync(fp);
    fclose(fp);
    free(bgr_buffer);
    printf("save file : %s\n", filename);
    return 0;
}


static int screenShotFun(char *output_filename){
    printf("%s:%d\n",__func__,__LINE__);

    if(!checkTFCardMountProc()) return -1;

    int fbfd = -1;
    void *fb_address = NULL;
    struct fb_fix_screeninfo finfo;
    struct fb_var_screeninfo vinfo;
    long int screen_size = 0; 
    if(dispfd < 0){
        dispfd = open(DISP_DEVICE, O_RDWR);
        if (dispfd < 0) {
            printf("open display dev fail!\n");
            return -1;
        }
    }
    
    if(access(DISP_CAPTUER_PATH, F_OK) != 0){
        system("mkdir /mnt/extsd/screenShot"); 
    }
    fbfd = open(DISP_FB, O_RDWR);
    if (fbfd < 0) {
        printf("open display fb fail!\n");
        close(dispfd);
        return -1;
    }
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        printf("Error reading fixed screen info\n");
        close(fbfd);
        return NULL;
    }
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        printf("Error reading variable screen info\n");
        close(fbfd);
        return NULL;
    }
    screen_size = vinfo.yres_virtual * finfo.line_length;
    fb_address = (char *)mmap(0, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if (fb_address == (char *)-1) {
        printf("Error mapping framebuffer device to memory\n");
        close(fb_address);
        return NULL;
    }
    unsigned long arg[3];
    disp_capture_info info;
    memset(&info, 0, sizeof(disp_capture_info));
    arg[0] = 0;
    if(ioctl(dispfd, DISP_CAPTURE_START, (void*)arg) != 0){
        printf("disp capture fail1\n");
        return -1;
    }

    int screen_width = ioctl(dispfd, DISP_GET_SCN_WIDTH, (void*)arg);
    int screen_height = ioctl(dispfd, DISP_GET_SCN_HEIGHT, (void*)arg);
    info.window.x = 0;
    info.window.y = 0;
    info.window.width = screen_width;
    info.window.height = screen_height;
    info.out_frame.format = DISP_FORMAT_ARGB_8888;
    info.out_frame.size[0].width = screen_width;
    info.out_frame.size[0].height = screen_height;
    info.out_frame.crop.x = 0;
    info.out_frame.crop.y = 0;
    info.out_frame.crop.width = screen_width;
    info.out_frame.crop.height = screen_height;
    info.out_frame.addr[0] = fb_address; //buffer address
    arg[0] = 0;//显示通道0
    arg[1] = (unsigned long)&info;
    if(ioctl(dispfd, DISP_CAPTURE_COMMIT, (void*)arg) != 0){
        printf("start disp capture fail1\n");
        return -1;
    }

    if(ioctl(dispfd, DISP_CAPTURE_STOP, (void*)arg) != 0){
        printf("stop disp capture fail1\n");
        return -1;
    }
    char str[50] = {};
    sprintf(str, "%s/%s", DISP_CAPTUER_PATH, output_filename);
    if (output_filename) {
        save_as_bmp(str, fb_address, screen_width, screen_height, DISP_FORMAT_ARGB_8888);
    }  
    if (fb_address) {
        munmap(fb_address, finfo.smem_len);
    }
    if (fbfd >= 0) {
        close(fbfd);
    }
    // if (dispfd >= 0) {
    //     close(dispfd);
    // } 
    sync();
    return 0;
}

static void* screenshot_thread_func(void* arg) {
    printf("Screenshot thread started\n");
    int g_pic_num = 0;
    while (1) {
        g_thread_should_exit = !checkTFCardMountProc();
        if (g_thread_should_exit) {
            printf("Screenshot thread exiting by request\n");
            break;
        }
        

        char filename[32];
        snprintf(filename, sizeof(filename), "pic%d.bmp", g_pic_num);
        g_pic_num++;
        screenShotFun(filename);
        for (int i = 0; i < 20; i++) {
            if (g_thread_should_exit) {
                printf("Screenshot thread exiting during sleep\n");
                goto thread_exit;
            }
            usleep(100000);  // 100ms
        }
    }
    
thread_exit:
    printf("Screenshot thread stopped. Total pictures: %d\n", g_pic_num);
    return NULL;
}

// 启动截图线程
bool screenshot_thread_start(void) {
    g_thread_should_exit = false;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    
    int err = pthread_create(&g_screenshot_thread, &attr, screenshot_thread_func, NULL);
    pthread_attr_destroy(&attr);
    if (err != 0) {
        printf("Failed to create screenshot thread: %s\n", strerror(err));
        return false;
    }
    
    printf("Screenshot thread created successfully (ID: %lu)\n",  (unsigned long)g_screenshot_thread);

    return true;
}

// 停止截图线程
bool screenshot_thread_stop(void) {
    g_thread_should_exit = true;  
    return true;
}
