#include "camera_vi.h"

#include <stdio.h>
#include <string.h>

#include <mpi_videoformat_conversion.h>

/*
 * camera_vi.c
 * -----------
 * 该文件负责:
 * - VI/ISP 创建与销毁
 * - 前后摄启停
 * - 统一取帧与还帧接口
 */

/* 将外部简化配置映射到底层 VI_ATTR_S。 */
static void camera_vi_fill_attr(const camera_vi_attr_t *cfg, VI_ATTR_S *attr)
{
    memset(attr, 0, sizeof(*attr));
    attr->type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    attr->memtype = V4L2_MEMORY_MMAP;
    attr->format.pixelformat = map_PIXEL_FORMAT_E_to_V4L2_PIX_FMT(cfg->pixel_format);
    attr->format.field = V4L2_FIELD_NONE;
    attr->format.colorspace = cfg->color_space;
    attr->format.width = cfg->width;
    attr->format.height = cfg->height;
    attr->nbufs = cfg->vi_buf_num;
    attr->nplanes = 2;
    attr->fps = cfg->frame_rate;
    attr->use_current_win = 0;
    attr->wdr_mode = cfg->enable_wdr;
    attr->capturemode = V4L2_MODE_VIDEO;
    attr->drop_frame_num = cfg->vi_drop_frame_num;
    attr->mbEncppEnable = TRUE;
}

/* 创建一路 VI 节点（VIPP + VirChn）。 */
static int camera_vi_create_one(camera_vi_node_t *node)
{
    ERRORTYPE ret;
    camera_vi_fill_attr(&node->cfg, &node->vi_attr);

    ret = AW_MPI_VI_CreateVipp(node->vi_dev);
    if (ret != SUCCESS) {
        printf("camera_vi: CreateVipp failed dev=%d ret=0x%x\n", node->vi_dev, ret);
        return -1;
    }

    ret = AW_MPI_VI_SetVippAttr(node->vi_dev, &node->vi_attr);
    if (ret != SUCCESS) {
        printf("camera_vi: SetVippAttr failed dev=%d ret=0x%x\n", node->vi_dev, ret);
        AW_MPI_VI_DestoryVipp(node->vi_dev);
        return -1;
    }

    ret = AW_MPI_VI_CreateVirChn(node->vi_dev, node->vi_chn, NULL);
    if (ret != SUCCESS) {
        printf("camera_vi: CreateVirChn failed dev=%d chn=%d ret=0x%x\n",
               node->vi_dev, node->vi_chn, ret);
        AW_MPI_VI_DestoryVipp(node->vi_dev);
        return -1;
    }

    node->created = true;
    return 0;
}

/* 销毁一路 VI 节点。 */
static void camera_vi_destroy_one(camera_vi_node_t *node)
{
    if (!node->created) {
        return;
    }

    AW_MPI_VI_DisableVirChn(node->vi_dev, node->vi_chn);
    AW_MPI_VI_DestoryVirChn(node->vi_dev, node->vi_chn);
    AW_MPI_VI_DisableVipp(node->vi_dev);
    AW_MPI_VI_DestoryVipp(node->vi_dev);
    node->created = false;
    node->started = false;
}

/* 启动一路 VI + ISP。 */
static int camera_vi_start_one(camera_vi_node_t *node)
{
    ERRORTYPE ret;
    if (!node->created) {
        return -1;
    }

    ret = AW_MPI_ISP_Run(node->isp_dev);
    if (ret != SUCCESS) {
        printf("camera_vi: ISP_Run failed isp=%d ret=0x%x\n", node->isp_dev, ret);
    }

    ret = AW_MPI_VI_EnableVipp(node->vi_dev);
    if (ret != SUCCESS) {
        printf("camera_vi: EnableVipp failed dev=%d ret=0x%x\n", node->vi_dev, ret);
        return -1;
    }

    AW_MPI_VI_SetVippMirror(node->vi_dev, node->cfg.mirror);
    AW_MPI_VI_SetVippFlip(node->vi_dev, node->cfg.flip);

    ret = AW_MPI_VI_EnableVirChn(node->vi_dev, node->vi_chn);
    if (ret != SUCCESS) {
        printf("camera_vi: EnableVirChn failed dev=%d chn=%d ret=0x%x\n",
               node->vi_dev, node->vi_chn, ret);
        AW_MPI_VI_DisableVipp(node->vi_dev);
        return -1;
    }

    node->started = true;
    return 0;
}

/* 停止一路 VI + ISP。 */
static void camera_vi_stop_one(camera_vi_node_t *node)
{
    if (!node->created || !node->started) {
        return;
    }
    AW_MPI_VI_DisableVirChn(node->vi_dev, node->vi_chn);
    AW_MPI_VI_DisableVipp(node->vi_dev);
    AW_MPI_ISP_Stop(node->isp_dev);
    node->started = false;
}

camera_vi_node_t *camera_vi_get_node(camera_vi_ctx_t *ctx, camera_sensor_id_e sensor)
{
    if (ctx == NULL) {
        return NULL;
    }
    if (sensor == CAMERA_SENSOR_FRONT) {
        return &ctx->front;
    }
#if CAMERA_ENABLE_SECOND_SENSOR
    if (sensor == CAMERA_SENSOR_REAR) {
        return &ctx->rear;
    }
#endif
    return NULL;
}

int camera_vi_init(camera_vi_ctx_t *ctx, const camera_vi_attr_t *front_cfg
#if CAMERA_ENABLE_SECOND_SENSOR
    , const camera_vi_attr_t *rear_cfg
#endif
)
{
    if (ctx == NULL || front_cfg == NULL) {
        return -1;
    }

    memset(ctx, 0, sizeof(*ctx));

    ctx->front.vi_dev = 0;
    ctx->front.vi_chn = 0;
    ctx->front.isp_dev = 0;
    ctx->front.cfg = *front_cfg;
    if (camera_vi_create_one(&ctx->front) != 0) {
        return -1;
    }

#if CAMERA_ENABLE_SECOND_SENSOR
    if (rear_cfg != NULL) {
        ctx->rear.vi_dev = 8;
        ctx->rear.vi_chn = 0;
        ctx->rear.isp_dev = 4;
        ctx->rear.cfg = *rear_cfg;
        if (camera_vi_create_one(&ctx->rear) != 0) {
            camera_vi_destroy_one(&ctx->front);
            return -1;
        }
    }
#endif

    ctx->inited = true;
    return 0;
}

int camera_vi_start(camera_vi_ctx_t *ctx)
{
    if (ctx == NULL || !ctx->inited) {
        return -1;
    }

    if (camera_vi_start_one(&ctx->front) != 0) {
        return -1;
    }

#if CAMERA_ENABLE_SECOND_SENSOR
    if (ctx->rear.created && camera_vi_start_one(&ctx->rear) != 0) {
        camera_vi_stop_one(&ctx->front);
        return -1;
    }
#endif
    return 0;
}

int camera_vi_stop(camera_vi_ctx_t *ctx)
{
    if (ctx == NULL || !ctx->inited) {
        return -1;
    }
#if CAMERA_ENABLE_SECOND_SENSOR
    camera_vi_stop_one(&ctx->rear);
#endif
    camera_vi_stop_one(&ctx->front);
    return 0;
}

int camera_vi_deinit(camera_vi_ctx_t *ctx)
{
    if (ctx == NULL || !ctx->inited) {
        return -1;
    }

    camera_vi_stop(ctx);
#if CAMERA_ENABLE_SECOND_SENSOR
    camera_vi_destroy_one(&ctx->rear);
#endif
    camera_vi_destroy_one(&ctx->front);
    ctx->inited = false;
    return 0;
}

int camera_vi_get_frame(camera_vi_ctx_t *ctx, camera_sensor_id_e sensor, VIDEO_FRAME_INFO_S *frame, int timeout_ms)
{
    camera_vi_node_t *node = camera_vi_get_node(ctx, sensor);
    if (node == NULL || frame == NULL || !node->started) {
        return -1;
    }
    return AW_MPI_VI_GetFrame(node->vi_dev, node->vi_chn, frame, timeout_ms);
}

int camera_vi_release_frame(camera_vi_ctx_t *ctx, camera_sensor_id_e sensor, VIDEO_FRAME_INFO_S *frame)
{
    camera_vi_node_t *node = camera_vi_get_node(ctx, sensor);
    if (node == NULL || frame == NULL || !node->started) {
        return -1;
    }
    return AW_MPI_VI_ReleaseFrame(node->vi_dev, node->vi_chn, frame);
}
