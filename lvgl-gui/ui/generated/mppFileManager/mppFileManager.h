#ifndef MPPFILEMANAGER_H_
#define MPPFILEMANAGER_H
#include "queue_mpp.h"
#include "stdbool.h"

#define MAX_FILES 1024         // 最大文件数
#define MAX_NAME_LEN 256       // 文件名最大长度

typedef enum scan_file_path{
    REC_PATH = 0X0,
    U_REC_PATH,
    PIC_PATH,
}SCAN_FILE_PATH;


typedef struct
{
    char fileName[MAX_NAME_LEN];
} QueueMppFile;

typedef struct {
    char name[MAX_NAME_LEN];   // 文件名
    time_t mtime;              // 最后修改时间
} FileEntry;

extern QueueMpp *F_videoFile;
extern QueueMpp *R_videoFile;
extern QueueMpp *F_U_videoFile;
extern QueueMpp *R_U_videoFile;
extern QueueMpp *F_picFile;
extern QueueMpp *R_picFile;


static pthread_t threadID;
bool TFFreeMemDetection(void);
void deletFileInRecorderPath(SCAN_FILE_PATH path);
void createQueueGetMppFileThread();
#endif