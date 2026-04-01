#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "queue_mpp.h"

// 创建队列
QueueMpp* queue_mpp_create(void)
{
    printf("%s:%d\n",__func__,__LINE__);
    QueueMpp *queue = (QueueMpp *)malloc(sizeof(QueueMpp));
    if (queue == NULL) {
        return NULL;
    }
    
    queue->front = NULL;
    queue->rear = NULL;
    queue->count = 0;
    
    if (pthread_mutex_init(&queue->mutex, NULL) != 0) {
        free(queue);
        return NULL;
    }
    
    return queue;
}

// 销毁队列
void queue_mpp_destroy(QueueMpp *queue)
{
    if (queue == NULL) {
        return;
    }
    
    // 先清空队列中的所有节点
    queue_mpp_clear(queue);
    
    // 销毁互斥锁
    pthread_mutex_destroy(&queue->mutex);
    
    // 释放队列结构
    free(queue);
}

// 清空队列
void queue_mpp_clear(QueueMpp *queue)
{
    if (queue == NULL) {
        return;
    }
    
    pthread_mutex_lock(&queue->mutex);
    
    PQueueNode current = queue->front;
    while (current != NULL) {
        PQueueNode next = current->next;
        
        // 释放节点数据
        if (current->data != NULL) {
            free(current->data);
        }
        
        // 释放节点
        free(current);
        current = next;
    }
    
    queue->front = NULL;
    queue->rear = NULL;
    queue->count = 0;
    
    pthread_mutex_unlock(&queue->mutex);
}

// 获取队列大小
int queue_mpp_size(QueueMpp *queue)
{
    if (queue == NULL) {
        return -1;
    }
    
    pthread_mutex_lock(&queue->mutex);
    int size = queue->count;
    pthread_mutex_unlock(&queue->mutex);
    
    return size;
}

// 判断队列是否为空
int queue_mpp_empty(QueueMpp *queue)
{
    if (queue == NULL) {
        return 1;
    }
    
    pthread_mutex_lock(&queue->mutex);
    int empty = (queue->front == NULL) ? 1 : 0;
    pthread_mutex_unlock(&queue->mutex);
    
    return empty;
}

// 入队
int queue_mpp_push(QueueMpp *queue, void *data)
{
    // printf("%s:%d\n",__func__,__LINE__);
    if (queue == NULL || data == NULL) {
        return -1;
    }
    
    // 创建新节点
    PQueueNode new_node = (PQueueNode)malloc(sizeof(QueueNode));
    if (new_node == NULL) {
        return -1;
    }
    
    new_node->data = data;
    new_node->next = NULL;
    
    pthread_mutex_lock(&queue->mutex);
    
    if (queue->rear == NULL) {
        // 队列为空
        queue->front = new_node;
        queue->rear = new_node;
    } else {
        // 队列不为空，添加到尾部
        queue->rear->next = new_node;
        queue->rear = new_node;
    }
    
    queue->count++;
    
    pthread_mutex_unlock(&queue->mutex);
    
    return 0;
}

// 获取队首元素
void* queue_mpp_front(QueueMpp *queue)
{
    if (queue == NULL) {
        return NULL;
    }
    
    pthread_mutex_lock(&queue->mutex);
    
    void *data = NULL;
    if (queue->front != NULL) {
        data = queue->front->data;
    }
    
    pthread_mutex_unlock(&queue->mutex);
    
    return data;
}

// 出队（返回数据，需要调用者释放）
void* queue_mpp_pop(QueueMpp *queue)
{
    // printf("%s:%d\n",__func__,__LINE__);
    if (queue == NULL) {
        return NULL;
    }
    
    pthread_mutex_lock(&queue->mutex);
    
    if (queue->front == NULL) {
        pthread_mutex_unlock(&queue->mutex);
        return NULL;
    }
    
    PQueueNode node = queue->front;
    void *data = node->data;
    
    // 更新队列头部
    queue->front = node->next;
    
    // 如果队列变空，更新尾部
    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    
    // 释放节点
    free(node);
    queue->count--;
    
    pthread_mutex_unlock(&queue->mutex);
    
    return data;
}

// 出队并释放数据
int queue_mpp_pop_and_free(QueueMpp *queue)
{
    if (queue == NULL) {
        return -1;
    }
    
    pthread_mutex_lock(&queue->mutex);
    
    if (queue->front == NULL) {
        pthread_mutex_unlock(&queue->mutex);
        return -1;
    }
    
    PQueueNode node = queue->front;
    
    // 更新队列头部
    queue->front = node->next;
    
    // 如果队列变空，更新尾部
    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    
    // 释放数据和节点
    if (node->data != NULL) {
        free(node->data);
    }
    free(node);
    queue->count--;
    
    pthread_mutex_unlock(&queue->mutex);
    
    return 0;
}