#ifndef QUEUE_MPP_H
#define QUEUE_MPP_H

#include <pthread.h>
#include "mm_comm_video.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    VIDEO_FRAME_INFO_S frame;
} QueueMppData;

typedef struct _QueueNode {
    void *data;
    struct _QueueNode *next;
} QueueNode, *PQueueNode;

typedef struct {
    PQueueNode front;
    PQueueNode rear;
    pthread_mutex_t mutex;
    int count;  // 添加计数器，避免每次计算大小
} QueueMpp;

// 创建队列
QueueMpp* queue_mpp_create(void);

// 销毁队列（释放所有节点和数据）
void queue_mpp_destroy(QueueMpp *queue);

// 清空队列（释放所有节点和数据，但不销毁队列结构）
void queue_mpp_clear(QueueMpp *queue);

// 获取队列大小
int queue_mpp_size(QueueMpp *queue);

// 判断队列是否为空
int queue_mpp_empty(QueueMpp *queue);

// 入队
int queue_mpp_push(QueueMpp *queue, void *data);

// 获取队首元素（不弹出）
void* queue_mpp_front(QueueMpp *queue);

// 出队（返回被弹出的数据，需要调用者释放）
void* queue_mpp_pop(QueueMpp *queue);

// 出队（内部释放数据）
int queue_mpp_pop_and_free(QueueMpp *queue);

#ifdef __cplusplus
}
#endif

#endif // QUEUE_MPP_H