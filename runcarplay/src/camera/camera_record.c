#include "camera_record.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

/*
 * camera_record.c
 * ---------------
 * 录像模块核心实现:
 * - 每路录像内置帧队列
 * - producer 线程从 VI 取帧，consumer 线程送入编码并处理分段
 * - 封装 VI->VENC->MUX 生命周期
 */

static uint64_t camera_record_now_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000ULL + (uint64_t)tv.tv_usec / 1000ULL;
}

/* 初始化每路录像的私有帧队列。 */
static void camera_frame_queue_init(camera_frame_queue_t *q)
{
    memset(q, 0, sizeof(*q));
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
}

/* 通知队列停止，唤醒阻塞线程退出。 */
static void camera_frame_queue_stop(camera_frame_queue_t *q)
{
    pthread_mutex_lock(&q->mutex);
    q->stop_flag = 1;
    pthread_cond_broadcast(&q->cond);
    pthread_mutex_unlock(&q->mutex);
}

/* 释放队列内缓存节点。 */
static void camera_frame_queue_deinit(camera_frame_queue_t *q)
{
    camera_frame_node_t *n = NULL;
    camera_frame_node_t *tmp = NULL;
    pthread_mutex_lock(&q->mutex);
    n = q->head;
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    pthread_mutex_unlock(&q->mutex);

    while (n) {
        tmp = n->next;
        free(n);
        n = tmp;
    }
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->cond);
}

static int camera_frame_queue_push(camera_frame_queue_t *q, const VIDEO_FRAME_INFO_S *frame)
{
    camera_frame_node_t *n = (camera_frame_node_t *)malloc(sizeof(*n));
    if (!n) {
        return -1;
    }
    memset(n, 0, sizeof(*n));
    n->frame = *frame;

    pthread_mutex_lock(&q->mutex);
    if (q->stop_flag) {
        pthread_mutex_unlock(&q->mutex);
        free(n);
        return -1;
    }
    if (q->tail) {
        q->tail->next = n;
    } else {
        q->head = n;
    }
    q->tail = n;
    q->size++;
    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->mutex);
    return 0;
}

static int camera_frame_queue_pop(camera_frame_queue_t *q, VIDEO_FRAME_INFO_S *frame)
{
    camera_frame_node_t *n = NULL;
    pthread_mutex_lock(&q->mutex);
    while (!q->stop_flag && q->head == NULL) {
        pthread_cond_wait(&q->cond, &q->mutex);
    }
    if (q->stop_flag && q->head == NULL) {
        pthread_mutex_unlock(&q->mutex);
        return -1;
    }
    n = q->head;
    q->head = n->next;
    if (q->head == NULL) {
        q->tail = NULL;
    }
    q->size--;
    pthread_mutex_unlock(&q->mutex);

    *frame = n->frame;
    free(n);
    return 0;
}

static int camera_record_open_segment_file(camera_record_channel_t *ch, const char *dir)
{
    char path[256];
    if (dir == NULL) {
        return -1;
    }

    snprintf(path, sizeof(path), "%s/%s_%03d.ts",
             dir,
             ch->sensor == CAMERA_SENSOR_FRONT ? "front" : "rear",
             ch->segment_index++);

    ch->mux_fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (ch->mux_fd < 0) {
        printf("camera_record: open segment failed %s errno=%d\n", path, errno);
        return -1;
    }
    printf("camera_record: segment open => %s\n", path);
    return 0;
}

static int camera_record_create_venc(camera_record_channel_t *ch)
{
    VENC_CHN_ATTR_S attr;
    VENC_FRAME_RATE_S fps;
    int c;

    memset(&attr, 0, sizeof(attr));
    attr.VeAttr.Type = ch->cfg.payload_type;
    attr.VeAttr.SrcPicWidth = ch->cfg.width;
    attr.VeAttr.SrcPicHeight = ch->cfg.height;
    attr.VeAttr.Field = VIDEO_FIELD_FRAME;
    attr.VeAttr.PixelFormat = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    attr.VeAttr.AttrH264e.PicWidth = ch->cfg.width;
    attr.VeAttr.AttrH264e.PicHeight = ch->cfg.height;
    attr.VeAttr.AttrH264e.Profile = 2;
    attr.RcAttr.mRcMode = VENC_RC_MODE_H264CBR;
    attr.RcAttr.mAttrH264Cbr.mBitRate = ch->cfg.bit_rate;
    attr.GopAttr.enGopMode = VENC_GOPMODE_NORMALP;

    for (c = 0; c < VENC_MAX_CHN_NUM; ++c) {
        int ret = AW_MPI_VENC_CreateChn(c, &attr);
        if (ret == SUCCESS) {
            ch->venc_chn = c;
            memset(&fps, 0, sizeof(fps));
            fps.SrcFrmRate = ch->cfg.frame_rate;
            fps.DstFrmRate = ch->cfg.frame_rate;
            AW_MPI_VENC_SetFrameRate(ch->venc_chn, &fps);
            return 0;
        }
        if (ret != ERR_VENC_EXIST) {
            printf("camera_record: create venc failed ret=0x%x\n", ret);
            return -1;
        }
    }
    return -1;
}

static int camera_record_create_mux(camera_record_channel_t *ch, const char *dir)
{
    MUX_CHN_ATTR_S attr;
    int i;

    memset(&attr, 0, sizeof(attr));
    attr.mVideoAttrValidNum = 1;
    attr.mVideoAttr[0].mVideoEncodeType = ch->cfg.payload_type;
    attr.mVideoAttr[0].mWidth = ch->cfg.width;
    attr.mVideoAttr[0].mHeight = ch->cfg.height;
    attr.mVideoAttr[0].mVideoFrmRate = ch->cfg.frame_rate * 1000;
    attr.mVideoAttr[0].mVeChn = ch->venc_chn;
    attr.mMediaFileFormat = MEDIA_FILE_FORMAT_TS;
    attr.mMaxFileDuration = ch->cfg.segment_duration_s * 1000;
    attr.mFsWriteMode = FSWRITEMODE_DIRECT;
    attr.mSimpleCacheSize = 64 * 1024;

    if (camera_record_open_segment_file(ch, dir) != 0) {
        return -1;
    }

    for (i = 0; i < MUX_MAX_CHN_NUM; ++i) {
        int ret = AW_MPI_MUX_CreateChn(i, &attr, ch->mux_fd, 0);
        if (ret == SUCCESS) {
            ch->mux_chn = i;
            return 0;
        }
        if (ret != ERR_MUX_EXIST) {
            printf("camera_record: create mux failed ret=0x%x\n", ret);
            close(ch->mux_fd);
            ch->mux_fd = -1;
            return -1;
        }
    }

    close(ch->mux_fd);
    ch->mux_fd = -1;
    return -1;
}

/* producer: 持续取帧并入队，避免编码阻塞反压到采集侧。 */
static void *camera_record_producer(void *arg)
{
    camera_record_channel_t *ch = (camera_record_channel_t *)arg;
    VIDEO_FRAME_INFO_S frame;
    ch->producer_running = 1;
    while (ch->frame_queue.stop_flag == 0) {
        memset(&frame, 0, sizeof(frame));
        if (camera_vi_get_frame(ch->vi_ctx, ch->sensor, &frame, 1000) == SUCCESS) {
            if (camera_frame_queue_push(&ch->frame_queue, &frame) != 0) {
                camera_vi_release_frame(ch->vi_ctx, ch->sensor, &frame);
            }
        }
    }
    ch->producer_running = 0;
    return NULL;
}

/* consumer: 出队后送编码，并按时长进行分段切换。 */
static void *camera_record_consumer(void *arg)
{
    camera_record_channel_t *ch = (camera_record_channel_t *)arg;
    VIDEO_FRAME_INFO_S frame;
    int64_t pts = 0;
    ch->consumer_running = 1;
    while (ch->frame_queue.stop_flag == 0) {
        if (camera_frame_queue_pop(&ch->frame_queue, &frame) != 0) {
            break;
        }

        if (AW_MPI_VENC_SendFrame(ch->venc_chn, &frame, 1000) != SUCCESS) {
            printf("camera_record: send frame failed venc=%d\n", ch->venc_chn);
        }
        camera_vi_release_frame(ch->vi_ctx, ch->sensor, &frame);

        pts += 1000 / (ch->cfg.frame_rate > 0 ? ch->cfg.frame_rate : 25);
        if (ch->cfg.segment_duration_s > 0 &&
            (int)(camera_record_now_ms() - ch->segment_start_ms) >= ch->cfg.segment_duration_s * 1000) {
            int new_fd;
            char path[256];
            snprintf(path, sizeof(path), "%s/%s_%03d.ts",
                     ch->output_dir ? ch->output_dir : ".",
                     ch->sensor == CAMERA_SENSOR_FRONT ? "front" : "rear",
                     ch->segment_index++);
            new_fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0666);
            if (new_fd >= 0) {
                AW_MPI_MUX_SwitchFd(ch->mux_chn, new_fd, pts);
                close(ch->mux_fd);
                ch->mux_fd = new_fd;
                ch->segment_start_ms = camera_record_now_ms();
            }
        }
    }
    ch->consumer_running = 0;
    return NULL;
}

static int camera_record_start_channel(camera_record_channel_t *ch, const char *out_dir)
{
    MPP_CHN_S vi_chn;
    MPP_CHN_S venc_chn;
    MPP_CHN_S mux_chn;
    VENC_RECV_PIC_PARAM_S recv_param;
    camera_vi_node_t *vi_node = camera_vi_get_node(ch->vi_ctx, ch->sensor);

    if (vi_node == NULL) {
        return -1;
    }

    ch->venc_chn = MM_INVALID_CHN;
    ch->mux_chn = MM_INVALID_CHN;
    ch->mux_fd = -1;
    ch->segment_index = 0;
    ch->segment_start_ms = camera_record_now_ms();

    camera_frame_queue_init(&ch->frame_queue);

    if (camera_record_create_venc(ch) != 0) {
        return -1;
    }
    if (camera_record_create_mux(ch, out_dir) != 0) {
        AW_MPI_VENC_DestroyChn(ch->venc_chn);
        return -1;
    }

    vi_chn.mModId = MOD_ID_VIU;
    vi_chn.mDevId = vi_node->vi_dev;
    vi_chn.mChnId = vi_node->vi_chn;
    venc_chn.mModId = MOD_ID_VENC;
    venc_chn.mDevId = 0;
    venc_chn.mChnId = ch->venc_chn;
    mux_chn.mModId = MOD_ID_MUX;
    mux_chn.mDevId = 0;
    mux_chn.mChnId = ch->mux_chn;

    AW_MPI_SYS_Bind(&vi_chn, &venc_chn);
    AW_MPI_SYS_Bind(&venc_chn, &mux_chn);

    memset(&recv_param, 0, sizeof(recv_param));
    recv_param.mRecvPicNum = -1;
    AW_MPI_VENC_StartRecvPicEx(ch->venc_chn, &recv_param);
    AW_MPI_MUX_StartGrp(ch->mux_chn);

    pthread_create(&ch->producer_tid, NULL, camera_record_producer, ch);
    pthread_create(&ch->consumer_tid, NULL, camera_record_consumer, ch);
    return 0;
}

static void camera_record_stop_channel(camera_record_channel_t *ch)
{
    MPP_CHN_S vi_chn;
    MPP_CHN_S venc_chn;
    MPP_CHN_S mux_chn;
    camera_vi_node_t *vi_node = camera_vi_get_node(ch->vi_ctx, ch->sensor);

    if (vi_node == NULL || ch->venc_chn == MM_INVALID_CHN) {
        return;
    }

    camera_frame_queue_stop(&ch->frame_queue);
    if (ch->producer_running) {
        pthread_join(ch->producer_tid, NULL);
    }
    if (ch->consumer_running) {
        pthread_join(ch->consumer_tid, NULL);
    }

    AW_MPI_MUX_StopGrp(ch->mux_chn);
    AW_MPI_VENC_StopRecvPic(ch->venc_chn);

    vi_chn.mModId = MOD_ID_VIU;
    vi_chn.mDevId = vi_node->vi_dev;
    vi_chn.mChnId = vi_node->vi_chn;
    venc_chn.mModId = MOD_ID_VENC;
    venc_chn.mDevId = 0;
    venc_chn.mChnId = ch->venc_chn;
    mux_chn.mModId = MOD_ID_MUX;
    mux_chn.mDevId = 0;
    mux_chn.mChnId = ch->mux_chn;

    AW_MPI_SYS_UnBind(&vi_chn, &venc_chn);
    AW_MPI_SYS_UnBind(&venc_chn, &mux_chn);

    AW_MPI_MUX_DestroyChn(ch->mux_chn);
    AW_MPI_VENC_ResetChn(ch->venc_chn);
    AW_MPI_VENC_DestroyChn(ch->venc_chn);
    if (ch->mux_fd >= 0) {
        close(ch->mux_fd);
        ch->mux_fd = -1;
    }
    camera_frame_queue_deinit(&ch->frame_queue);
    ch->venc_chn = MM_INVALID_CHN;
    ch->mux_chn = MM_INVALID_CHN;
}

int camera_record_init(camera_record_ctx_t *ctx, camera_vi_ctx_t *vi_ctx, const camera_record_cfg_t *cfg)
{
    if (ctx == NULL || vi_ctx == NULL || cfg == NULL) {
        return -1;
    }
    memset(ctx, 0, sizeof(*ctx));
    ctx->cfg = *cfg;
    ctx->state = CAMERA_RECORD_INITED;
    ctx->inited = 1;

    ctx->front.vi_ctx = vi_ctx;
    ctx->front.sensor = CAMERA_SENSOR_FRONT;
    ctx->front.output_dir = cfg->output_dir_front;
    ctx->front.cfg = cfg->front;

#if CAMERA_ENABLE_SECOND_SENSOR
    ctx->rear.vi_ctx = vi_ctx;
    ctx->rear.sensor = CAMERA_SENSOR_REAR;
    ctx->rear.output_dir = cfg->output_dir_rear;
    ctx->rear.cfg = cfg->rear;
#endif
    if (!ctx->cfg.enable_segment) {
        ctx->front.cfg.segment_duration_s = 0;
#if CAMERA_ENABLE_SECOND_SENSOR
        ctx->rear.cfg.segment_duration_s = 0;
#endif
    }
    return 0;
}

int camera_record_set_segment(camera_record_ctx_t *ctx, int segment_duration_s)
{
    if (ctx == NULL || !ctx->inited || segment_duration_s <= 0) {
        return -1;
    }
    ctx->front.cfg.segment_duration_s = segment_duration_s;
#if CAMERA_ENABLE_SECOND_SENSOR
    ctx->rear.cfg.segment_duration_s = segment_duration_s;
#endif
    return 0;
}

int camera_record_set_segment_params(camera_record_ctx_t *ctx, int enable_segment, int front_segment_seconds, int rear_segment_seconds)
{
    if (ctx == NULL || !ctx->inited) {
        return -1;
    }
    ctx->cfg.enable_segment = enable_segment ? 1 : 0;

    if (front_segment_seconds > 0) {
        ctx->front.cfg.segment_duration_s = front_segment_seconds;
    }
#if CAMERA_ENABLE_SECOND_SENSOR
    if (rear_segment_seconds > 0) {
        ctx->rear.cfg.segment_duration_s = rear_segment_seconds;
    }
#else
    (void)rear_segment_seconds;
#endif

    if (!ctx->cfg.enable_segment) {
        ctx->front.cfg.segment_duration_s = 0;
#if CAMERA_ENABLE_SECOND_SENSOR
        ctx->rear.cfg.segment_duration_s = 0;
#endif
    }
    return 0;
}

int camera_record_start(camera_record_ctx_t *ctx)
{
    if (ctx == NULL || !ctx->inited || ctx->state == CAMERA_RECORD_RUNNING) {
        return -1;
    }

    if (camera_record_start_channel(&ctx->front, ctx->cfg.output_dir_front) != 0) {
        return -1;
    }

#if CAMERA_ENABLE_SECOND_SENSOR
    if (ctx->cfg.enable_dual) {
        if (camera_record_start_channel(&ctx->rear, ctx->cfg.output_dir_rear) != 0) {
            camera_record_stop_channel(&ctx->front);
            return -1;
        }
    }
#endif
    ctx->state = CAMERA_RECORD_RUNNING;
    return 0;
}

int camera_record_stop(camera_record_ctx_t *ctx)
{
    if (ctx == NULL || !ctx->inited || ctx->state != CAMERA_RECORD_RUNNING) {
        return -1;
    }
    ctx->state = CAMERA_RECORD_STOPPING;
#if CAMERA_ENABLE_SECOND_SENSOR
    if (ctx->cfg.enable_dual) {
        camera_record_stop_channel(&ctx->rear);
    }
#endif
    camera_record_stop_channel(&ctx->front);
    ctx->state = CAMERA_RECORD_INITED;
    return 0;
}

int camera_record_deinit(camera_record_ctx_t *ctx)
{
    if (ctx == NULL || !ctx->inited) {
        return -1;
    }
    if (ctx->state == CAMERA_RECORD_RUNNING) {
        camera_record_stop(ctx);
    }
    ctx->inited = 0;
    ctx->state = CAMERA_RECORD_IDLE;
    return 0;
}
