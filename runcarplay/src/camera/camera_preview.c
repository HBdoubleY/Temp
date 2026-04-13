#include "camera_preview.h"

#include <stdio.h>
#include <string.h>

/*
 * camera_preview.c
 * ----------------
 * 该文件实现 VO 层与 VI 输入通道绑定逻辑。
 * 通过 camera_preview_show() 完成 front/rear/dual 三种模式切换。
 */

/* 申请一个可用 VO layer。 */
static int camera_preview_open_layer(VO_DEV vo_dev, VO_LAYER *layer)
{
    int i = 0;
    for (i = 0; i < VO_MAX_LAYER_NUM; ++i) {
        if (AW_MPI_VO_EnableVideoLayer(i) == SUCCESS) {
            *layer = i;
            return 0;
        }
    }
    printf("camera_preview: no free vo layer\n");
    return -1;
}

/* 在指定 layer 下创建可用 chn。 */
static int camera_preview_open_chn(VO_LAYER layer, VO_CHN *chn)
{
    int c = 0;
    for (c = 0; c < VO_MAX_CHN_NUM; ++c) {
        int ret = AW_MPI_VO_CreateChn(layer, c);
        if (ret == SUCCESS) {
            *chn = c;
            return 0;
        }
        if (ret != ERR_VO_CHN_NOT_DISABLE) {
            printf("camera_preview: create chn failed layer=%d chn=%d ret=0x%x\n", layer, c, ret);
            return -1;
        }
    }
    return -1;
}

/* 设置 layer 的显示区域与层级。 */
static int camera_preview_apply_rect(VO_LAYER layer, const camera_preview_rect_t *rect)
{
    VO_VIDEO_LAYER_ATTR_S attr;
    memset(&attr, 0, sizeof(attr));
    if (AW_MPI_VO_GetVideoLayerAttr(layer, &attr) != SUCCESS) {
        return -1;
    }
    attr.stDispRect.X = rect->x;
    attr.stDispRect.Y = rect->y;
    attr.stDispRect.Width = rect->width;
    attr.stDispRect.Height = rect->height;
    attr.mDispFrmRt = 25;
    attr.bDoubleFrame = true;
    attr.bClusterMode = false;
    if (AW_MPI_VO_SetVideoLayerAttr(layer, &attr) != SUCCESS) {
        return -1;
    }
    AW_MPI_VO_SetVideoLayerPriority(layer, rect->zorder);
    return 0;
}

static int camera_preview_apply_rect_by_sensor(camera_preview_ctx_t *ctx, camera_sensor_id_e sensor)
{
    if (sensor == CAMERA_SENSOR_FRONT) {
        return camera_preview_apply_rect(ctx->front_layer, &ctx->cfg.front_rect);
    }
#if CAMERA_ENABLE_SECOND_SENSOR
    if (sensor == CAMERA_SENSOR_REAR) {
        return camera_preview_apply_rect(ctx->rear_layer, &ctx->cfg.rear_rect);
    }
#endif
    return -1;
}

/* 停止显示通道，不销毁资源。 */
static void camera_preview_hide(camera_preview_ctx_t *ctx, int front, int rear)
{
    if (front) {
        AW_MPI_VO_StopChn(ctx->front_layer, ctx->front_chn);
    }
    if (rear) {
        AW_MPI_VO_StopChn(ctx->rear_layer, ctx->rear_chn);
    }
}

/*
 * 核心切换函数:
 * 1) 先停掉当前显示
 * 2) 按目标模式执行 VI->VO bind/unbind
 * 3) 启动对应 VO 通道
 */
static int camera_preview_show(camera_preview_ctx_t *ctx, camera_preview_mode_e mode)
{
    camera_vi_node_t *front = camera_vi_get_node(ctx->vi_ctx, CAMERA_SENSOR_FRONT);
    camera_vi_node_t *rear = camera_vi_get_node(ctx->vi_ctx, CAMERA_SENSOR_REAR);

    camera_preview_hide(ctx, 1, 1);

    if (mode == CAMERA_PREVIEW_FRONT || mode == CAMERA_PREVIEW_DUAL) {
        AW_MPI_SYS_Bind(&(MPP_CHN_S){MOD_ID_VIU, front->vi_dev, front->vi_chn},
                        &(MPP_CHN_S){MOD_ID_VOU, ctx->front_layer, ctx->front_chn});
        AW_MPI_VO_StartChn(ctx->front_layer, ctx->front_chn);
    } else {
        AW_MPI_SYS_UnBind(&(MPP_CHN_S){MOD_ID_VIU, front->vi_dev, front->vi_chn},
                          &(MPP_CHN_S){MOD_ID_VOU, ctx->front_layer, ctx->front_chn});
    }

#if CAMERA_ENABLE_SECOND_SENSOR
    if (rear && rear->created) {
        if (mode == CAMERA_PREVIEW_REAR || mode == CAMERA_PREVIEW_DUAL) {
            AW_MPI_SYS_Bind(&(MPP_CHN_S){MOD_ID_VIU, rear->vi_dev, rear->vi_chn},
                            &(MPP_CHN_S){MOD_ID_VOU, ctx->rear_layer, ctx->rear_chn});
            AW_MPI_VO_StartChn(ctx->rear_layer, ctx->rear_chn);
        } else {
            AW_MPI_SYS_UnBind(&(MPP_CHN_S){MOD_ID_VIU, rear->vi_dev, rear->vi_chn},
                              &(MPP_CHN_S){MOD_ID_VOU, ctx->rear_layer, ctx->rear_chn});
        }
    }
#else
    (void)rear;
    if (mode != CAMERA_PREVIEW_FRONT) {
        printf("camera_preview: rear camera disabled by CAMERA_ENABLE_SECOND_SENSOR\n");
        return -1;
    }
#endif

    ctx->mode = mode;
    return 0;
}

int camera_preview_init(camera_preview_ctx_t *ctx, camera_vi_ctx_t *vi_ctx, const camera_preview_cfg_t *cfg)
{
    VO_PUB_ATTR_S pub_attr;
    if (ctx == NULL || vi_ctx == NULL || cfg == NULL) {
        return -1;
    }
    memset(ctx, 0, sizeof(*ctx));
    ctx->vi_ctx = vi_ctx;
    ctx->cfg = *cfg;

    AW_MPI_VO_Enable(ctx->cfg.vo_dev);
    AW_MPI_VO_GetPubAttr(ctx->cfg.vo_dev, &pub_attr);
    pub_attr.enIntfType = ctx->cfg.disp_type;
    pub_attr.enIntfSync = ctx->cfg.disp_sync;
    AW_MPI_VO_SetPubAttr(ctx->cfg.vo_dev, &pub_attr);

    if (camera_preview_open_layer(ctx->cfg.vo_dev, &ctx->front_layer) != 0) {
        return -1;
    }
    if (camera_preview_open_chn(ctx->front_layer, &ctx->front_chn) != 0) {
        AW_MPI_VO_DisableVideoLayer(ctx->front_layer);
        return -1;
    }
    if (camera_preview_apply_rect(ctx->front_layer, &ctx->cfg.front_rect) != 0) {
        AW_MPI_VO_DestroyChn(ctx->front_layer, ctx->front_chn);
        AW_MPI_VO_DisableVideoLayer(ctx->front_layer);
        return -1;
    }

#if CAMERA_ENABLE_SECOND_SENSOR
    if (camera_preview_open_layer(ctx->cfg.vo_dev, &ctx->rear_layer) != 0) {
        AW_MPI_VO_DestroyChn(ctx->front_layer, ctx->front_chn);
        AW_MPI_VO_DisableVideoLayer(ctx->front_layer);
        return -1;
    }
    if (camera_preview_open_chn(ctx->rear_layer, &ctx->rear_chn) != 0) {
        AW_MPI_VO_DisableVideoLayer(ctx->rear_layer);
        AW_MPI_VO_DestroyChn(ctx->front_layer, ctx->front_chn);
        AW_MPI_VO_DisableVideoLayer(ctx->front_layer);
        return -1;
    }
    if (camera_preview_apply_rect(ctx->rear_layer, &ctx->cfg.rear_rect) != 0) {
        AW_MPI_VO_DestroyChn(ctx->rear_layer, ctx->rear_chn);
        AW_MPI_VO_DisableVideoLayer(ctx->rear_layer);
        AW_MPI_VO_DestroyChn(ctx->front_layer, ctx->front_chn);
        AW_MPI_VO_DisableVideoLayer(ctx->front_layer);
        return -1;
    }
#endif

    ctx->inited = 1;
    return 0;
}

int camera_preview_start(camera_preview_ctx_t *ctx, camera_preview_mode_e mode)
{
    if (ctx == NULL || !ctx->inited) {
        return -1;
    }
    if (camera_preview_show(ctx, mode) != 0) {
        return -1;
    }
    ctx->started = 1;
    return 0;
}

int camera_preview_switch(camera_preview_ctx_t *ctx, camera_preview_mode_e mode)
{
    if (ctx == NULL || !ctx->started) {
        return -1;
    }
    return camera_preview_show(ctx, mode);
}

int camera_preview_set_display_rect(camera_preview_ctx_t *ctx, camera_sensor_id_e sensor, const camera_preview_rect_t *rect, int apply_now)
{
    if (ctx == NULL || rect == NULL || !ctx->inited) {
        return -1;
    }
    if (sensor == CAMERA_SENSOR_FRONT) {
        ctx->cfg.front_rect = *rect;
    }
#if CAMERA_ENABLE_SECOND_SENSOR
    else if (sensor == CAMERA_SENSOR_REAR) {
        ctx->cfg.rear_rect = *rect;
    }
#endif
    else {
        return -1;
    }

    if (apply_now) {
        return camera_preview_apply_rect_by_sensor(ctx, sensor);
    }
    return 0;
}

int camera_preview_stop(camera_preview_ctx_t *ctx)
{
    if (ctx == NULL || !ctx->inited) {
        return -1;
    }
    camera_preview_hide(ctx, 1, 1);
    ctx->started = 0;
    return 0;
}

int camera_preview_deinit(camera_preview_ctx_t *ctx)
{
    if (ctx == NULL || !ctx->inited) {
        return -1;
    }

    camera_preview_stop(ctx);

#if CAMERA_ENABLE_SECOND_SENSOR
    AW_MPI_VO_DestroyChn(ctx->rear_layer, ctx->rear_chn);
    AW_MPI_VO_DisableVideoLayer(ctx->rear_layer);
#endif
    AW_MPI_VO_DestroyChn(ctx->front_layer, ctx->front_chn);
    AW_MPI_VO_DisableVideoLayer(ctx->front_layer);
    AW_MPI_VO_Disable(ctx->cfg.vo_dev);
    ctx->inited = 0;
    return 0;
}
