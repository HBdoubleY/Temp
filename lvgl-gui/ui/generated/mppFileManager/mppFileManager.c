#include <stdio.h>
#include "mppFileManager.h"
#include <dirent.h>
#include <limits.h>
#include "common.h"
 
QueueMpp *F_videoFile = NULL;
QueueMpp *R_videoFile = NULL;
QueueMpp *F_U_videoFile = NULL;
QueueMpp *R_U_videoFile = NULL;
QueueMpp *F_picFile = NULL;
QueueMpp *R_picFile = NULL;

static int compare_mtime_asc(const void* a, const void* b) {
    FileEntry* fa = (FileEntry*)a;
    FileEntry* fb = (FileEntry*)b;
    
    if (fa->mtime < fb->mtime) return -1;
    if (fa->mtime > fb->mtime) return 1;
    return 0;
}

int scan_directory(const char* dir_path, QueueMpp *fileQueue) {
    // printf("%s:%d\n",__func__,__LINE__);
    if (!fileQueue) {
        printf("错误: fileQueue 为 NULL\n");
        return -1;
    }
    DIR* dir = opendir(dir_path);
    if (!dir) {
        printf("无法打开目录: %s\n", dir_path);
        return -1;
    }

    FileEntry *file_list = (FileEntry*)malloc(MAX_FILES * sizeof(FileEntry));
    if (!file_list) {
        printf("内存分配失败\n");
        closedir(dir);
        return -1;
    }
    
    // 初始化
    memset(file_list, 0, MAX_FILES * sizeof(FileEntry));
    struct dirent* entry;
    int file_count = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        // 跳过 . 和 ..
        if (strcmp(entry->d_name, ".") == 0 || 
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        // 构建完整路径
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        // 检查是否为普通文件
        struct stat st;
        if (stat(full_path, &st) != 0) {
            continue;  // 跳过无法访问的文件
        }
        
        if (!S_ISREG(st.st_mode)) {
            continue;  // 跳过目录和特殊文件
        }
        // 添加到数组
        if (file_count < MAX_FILES) {
            strncpy(file_list[file_count].name, entry->d_name, MAX_NAME_LEN - 1);
            file_list[file_count].mtime = st.st_mtime;
            file_count++;
        } else {
            printf("达到最大文件数限制(%d)\n", MAX_FILES);
            break;
        }
    }
    closedir(dir);

    qsort(file_list, file_count, sizeof(FileEntry), compare_mtime_asc);
    for(int i = 0; i < file_count; i++){
        char *name_copy = strdup(file_list[i].name);
        if (name_copy) {
            queue_mpp_push(fileQueue, name_copy);
        } else {
            printf("内存分配失败，跳过文件: %s\n", file_list[i].name);
    }
    }
    free(file_list);
    return 0;
}

static int deleteEarliestFile(char *filename) {
    printf("%s\n", __func__);
    if (strlen(filename) > 0) {
        printf("准备删除最早文件: %s\n", filename);
        
        if (remove(filename) == 0) {
            printf("成功删除: %s\n", filename);
            return 0;  // 成功删除
        } else {
            perror("删除失败");
            return -1;
        }
    }
    
    printf("没有找到符合条件的文件\n");
    return -1;  // 没有找到文件
}

// 保留8%的磁盘空间，至少1GB
#define DISK_RESERVE_PERCENT 8
#define MIN_RESERVE_SIZE (1 * 1024 * 1024 * 1024LL)  // 1GB

static pthread_t delThreadId;
static int g_isDeleting = 0;  // 防止重复启动删除线程

static void* threadFun_deletFile(void* arg) {
    SCAN_FILE_PATH paths = (SCAN_FILE_PATH)(uintptr_t)arg;
    uint64_t totalsize = 0;
    uint64_t freeDisk = 0;
    uint64_t reserveSize = 0;
    // 计算保留空间大小
    GetDiskSpace(&totalsize, &freeDisk);
    reserveSize = (totalsize * DISK_RESERVE_PERCENT) / 100;
    if (reserveSize < MIN_RESERVE_SIZE) {
        reserveSize = MIN_RESERVE_SIZE;
    }
    
    printf("磁盘信息: 总空间=%lluGB, 当前空闲=%lluGB, 保留空间=%lluGB\n", 
           totalsize/GB, freeDisk/GB, reserveSize/GB);
    
    // 循环删除直到满足保留空间要求
    int deleteCount = 0;
    int maxDeleteCount = 100;  // 安全限制: 最多删除100个文件
    int noFileCount = 0;       // 连续找不到文件计数
    int deleteResult1 = -1;
    int deleteResult2 = -1;
    while (freeDisk <= reserveSize && deleteCount < maxDeleteCount) {
        char str[256] = {};
        char *str1 = NULL;
        switch (paths)
        {
        case REC_PATH:
            str1 = (char *)queue_mpp_pop(F_videoFile);
            
            if(str1){
                sprintf(str, "%s/%s", VIDEO_FRONT_FILE_PATH, str1);
                deleteResult1 = deleteEarliestFile(str);
                free(str1);
                str1 = NULL;
            }
            str1 = (char *)queue_mpp_pop(R_videoFile);
            if(str1){
                sprintf(str, "%s/%s", VIDEO_REAR_FILE_PATH, str1);
                deleteResult2 = deleteEarliestFile(str);
                free(str1);
                str1 = NULL;
            }
            break;
        case U_REC_PATH:
            str1 = (char *)queue_mpp_pop(F_U_videoFile);
            if(str1){
                sprintf(str, "%s/%s", URGENT_VIDEO_FRONT_FILE_PATH, str1);
                deleteResult1 = deleteEarliestFile(str);
                free(str1);
                str1 = NULL;
            }
            str1 = (char *)queue_mpp_pop(R_U_videoFile);
            if(str1){
                sprintf(str, "%s/%s", URGENT_VIDEO_REAR_FILE_PATH, str1);
                deleteResult2 = deleteEarliestFile(str);
                free(str1);
                str1 = NULL;
            }
            break;
        case PIC_PATH:
            str1 = (char *)queue_mpp_pop(F_picFile);
            if(str1){
                sprintf(str, "%s/%s", FRONT_PIC_PATH, str1);
                deleteResult1 = deleteEarliestFile(str);
                free(str1);
                str1 = NULL;
            }
            str1 = (char *)queue_mpp_pop(R_picFile);
            if(str1){
                sprintf(str, "%s/%s", REAR_PIC_PATH, str1);
                deleteResult2 = deleteEarliestFile(str);
                free(str1);
                str1 = NULL;
            }
            break;    
        default:
            break;
        }        
        deleteCount++;
        
        // 检查是否两个目录都没有文件了
        if (deleteResult1 == -1 && deleteResult2 == -1) {
            noFileCount++;
            if (noFileCount >= 2) {  // 连续两次都找不到文件
                printf("两个目录都没有符合条件的文件可删除\n");
                break;
            }
        } else {
            noFileCount = 0;  // 重置计数
        }
        
        // 每删除2个文件检查一次磁盘空间
        if (deleteCount % 2 == 0) {
            GetDiskSpace(&totalsize, &freeDisk);
            printf("已删除%d个文件, 当前空闲空间: %lluGB\n", 
                   deleteCount, freeDisk/GB);
        }

        if (!checkTFCardMountProc()) {
            printf("sd卡拔出, 清理线程退出\n");
            break;
        }
        
        usleep(1000000);  // 每次删除后稍微延迟, 避免过度消耗CPU
    }
    
    GetDiskSpace(&totalsize, &freeDisk);
    printf("磁盘清理完成: 共删除%d个文件, 最终空闲空间=%lluGB\n", 
           deleteCount, freeDisk/GB);
    
    // 清理资源
    g_isDeleting = 0;
    
    return NULL;
}

bool TFFreeMemDetection(void){
    uint64_t totalsize = 0;
    uint64_t freeDisk = 0;
    // 防止重复启动清理线程
    if (g_isDeleting) {
        printf("磁盘清理线程已在运行, 跳过\n");
        return;
    }
    
    GetDiskSpace(&totalsize, &freeDisk);
    
    // 计算保留空间
    uint64_t reserveSize = (totalsize * DISK_RESERVE_PERCENT) / 100;
    if (reserveSize < MIN_RESERVE_SIZE) {
        reserveSize = MIN_RESERVE_SIZE;
    }
    
    printf("磁盘检查: 总空间=%lluGB, 当前空闲=%lluGB, 保留空间=%lluGB\n", 
           totalsize/GB, freeDisk/GB, reserveSize/GB);
    
    // 检查是否满足保留空间要求
    if (freeDisk > reserveSize) {
        printf("磁盘空间充足(%.2f%%, 要求%d%%), 无需删除文件\n", 
               (freeDisk * 100.0) / totalsize, DISK_RESERVE_PERCENT);
        return true;
    }
    
    printf("磁盘空间不足, 启动清理...\n");
    return false;
}

void deletFileInRecorderPath(SCAN_FILE_PATH path) {

    if(TFFreeMemDetection()) return;
    g_isDeleting = 1;
    int ret = pthread_create(&delThreadId, NULL, threadFun_deletFile, (void*)(uintptr_t)path);
    if (ret != 0) {
        printf("创建清理线程失败: %d\n", ret);
        g_isDeleting = 0;
    } else {
        pthread_detach(delThreadId);  // 分离线程, 自动回收资源
    }
}

static void* QueueGetMppFile(void *arg){
    F_videoFile = queue_mpp_create();
    R_videoFile = queue_mpp_create();
    F_U_videoFile = queue_mpp_create();
    R_U_videoFile = queue_mpp_create();
    F_picFile = queue_mpp_create();
    R_picFile = queue_mpp_create();
    scan_directory(VIDEO_FRONT_FILE_PATH, F_videoFile);
    scan_directory(VIDEO_REAR_FILE_PATH, R_videoFile);
    scan_directory(URGENT_VIDEO_FRONT_FILE_PATH, F_U_videoFile);
    scan_directory(URGENT_VIDEO_REAR_FILE_PATH, R_U_videoFile);
    scan_directory(FRONT_PIC_PATH, F_picFile);
    scan_directory(REAR_PIC_PATH, R_picFile);
}

void createQueueGetMppFileThread(){
    int ret = pthread_create(&threadID, NULL, QueueGetMppFile, NULL);
    if(ret != 0){
        printf("create Queue Get Mpp File Thread fail:%d\n", ret);
    }else{
        printf("create Queue Get Mpp File Thread success, threadId:%d", threadID);
    }
    pthread_detach(threadID);
}