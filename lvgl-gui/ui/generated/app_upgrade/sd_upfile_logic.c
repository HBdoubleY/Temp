#include <stdio.h>
#include <sys/stat.h>
#include <fs.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sysinfo.h>
#include <stdlib.h>
#include <stdbool.h>
#define SD_NODE     "/dev/mmcblk0p1"
#define SD_MOUNT_POINT     "/mnt/extsd"
#define UPGRADE_DIR        "/mnt/extsd/runcarplay"
#define LOCK_FILE          "/var/run/sd_updater.pid"
#define CHECK_INTERVAL     5  // 检查间隔(秒)
typedef unsigned long size_t;
// 检查SD卡是否存在
int is_sd_mounted() {
    struct stat st;
    return (stat(SD_NODE, &st) == 0);
}

// 挂载SD卡
int mount_sd_card() {
    if (is_sd_mounted()) {
        //挂载   
        system("mount /dev/mmcblk0p1 /mnt/extsd/");
    }else {
        // mount("/dev/mmcblk1p1", SD_MOUNT_POINT, "vfat", MS_NOATIME, NULL);
    }  
    return 1;
}

// 通过stat结构体 获得文件大小，单位字节
static size_t getFileSize(const char *fileName) {
	if (fileName == NULL) {
		return 0;
	}
	struct stat statbuf;

	// 提供文件名字符串，获得文件属性结构体
	if( stat(fileName, &statbuf) < 0) {
		return 0;
	}

	// 获取文件大小
	size_t filesize = statbuf.st_size;

	return filesize;
}

int checkSysMem(int limit_MB)
{
	struct sysinfo info;
	int iRetVal = -1;/*用于获取函数的返回值，默认为-1*/
	iRetVal = sysinfo(&info);
	if(iRetVal == 0) {
		long freeram =  info.freeram;
		if(freeram < limit_MB*1024*1024) {
			printf("drop_caches, memory %ld KB\n", freeram>>10);
			system("echo 3 > /proc/sys/vm/drop_caches");
			return 1;
		}  
		return 0;
	}
	return 1;
}

// 检查升级包
int check_upgrade_package() {
    char upgrade_flag[256];
    char version_file[256];
    
    snprintf(upgrade_flag, sizeof(upgrade_flag), "%s", UPGRADE_DIR);
    // snprintf(version_file, sizeof(version_file), "%s/version.info", UPGRADE_DIR);
    
    // 检查升级包是否存在
    if (access(upgrade_flag, F_OK)) {
        return 1;
    }
    
    return 0;
}

