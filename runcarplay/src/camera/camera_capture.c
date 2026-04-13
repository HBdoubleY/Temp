#include "camera_capture.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/*
 * camera_capture.c
 * ----------------
 * 拍照模块实现:
 * - 创建 JPEG 编码通道
 * - 与 VI 绑定
 * - 取一帧编码结果并按指定文件名落盘
 */

/* 创建 JPEG 编码通道。 */
static int camera_capture_create_venc(camera_capture_ctx_t *ctx)
{
    VENC_CHN_ATTR_S attr;
    VENC_FRAME_RATE_S fps;
    int chn = 0;

    memset(&attr, 0, sizeof(attr));
    attr.VeAttr.Type = PT_MJPEG;
    attr.VeAttr.SrcPicWidth = ctx->cfg.width;
    attr.VeAttr.SrcPicHeight = ctx->cfg.height;
    attr.VeAttr.Field = VIDEO_FIELD_FRAME;
    attr.VeAttr.PixelFormat = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    attr.VeAttr.MaxKeyInterval = 1;
    attr.VeAttr.mColorSpace = V4L2_COLORSPACE_JPEG;
    attr.RcAttr.mRcMode = VENC_RC_MODE_MJPEGCBR;
    attr.RcAttr.mAttrMjpegeCbr.mBitRate = 8 * 1024 * 1024;

    for (chn = 0; chn < VENC_MAX_CHN_NUM; ++chn) {
        int ret = AW_MPI_VENC_CreateChn(chn, &attr);
        if (ret == SUCCESS) {
            ctx->venc_chn = chn;
            fps.SrcFrmRate = ctx->cfg.frame_rate;
            fps.DstFrmRate = ctx->cfg.frame_rate;
            AW_MPI_VENC_SetFrameRate(ctx->venc_chn, &fps);
            AW_MPI_VENC_SetJpegParam(ctx->venc_chn, &(VENC_PARAM_JPEG_S){ .Qfactor = ctx->cfg.jpeg_qfactor });
            return 0;
        }
        if (ret != ERR_VENC_EXIST) {
            printf("camera_capture: create venc fail ret=0x%x\n", ret);
            return -1;
        }
    }
    return -1;
}

/* 绑定 VI -> VENC。 */
static int camera_capture_bind(camera_capture_ctx_t *ctx)
{
    camera_vi_node_t *node = camera_vi_get_node(ctx->vi_ctx, ctx->sensor);
    MPP_CHN_S src;
    MPP_CHN_S dst;
    if (node == NULL) {
        return -1;
    }
    src.mModId = MOD_ID_VIU;
    src.mDevId = node->vi_dev;
    src.mChnId = node->vi_chn;
    dst.mModId = MOD_ID_VENC;
    dst.mDevId = 0;
    dst.mChnId = ctx->venc_chn;
    return AW_MPI_SYS_Bind(&src, &dst);
}

/* 解绑定 VI -> VENC。 */
static int camera_capture_unbind(camera_capture_ctx_t *ctx)
{
    camera_vi_node_t *node = camera_vi_get_node(ctx->vi_ctx, ctx->sensor);
    MPP_CHN_S src;
    MPP_CHN_S dst;
    if (node == NULL) {
        return -1;
    }
    src.mModId = MOD_ID_VIU;
    src.mDevId = node->vi_dev;
    src.mChnId = node->vi_chn;
    dst.mModId = MOD_ID_VENC;
    dst.mDevId = 0;
    dst.mChnId = ctx->venc_chn;
    return AW_MPI_SYS_UnBind(&src, &dst);
}

int camera_capture_init(camera_capture_ctx_t *ctx, camera_vi_ctx_t *vi_ctx, camera_sensor_id_e sensor, const camera_capture_cfg_t *cfg)
{
    if (ctx == NULL || vi_ctx == NULL || cfg == NULL) {
        return -1;
    }
    memset(ctx, 0, sizeof(*ctx));
    ctx->vi_ctx = vi_ctx;
    ctx->sensor = sensor;
    ctx->cfg = *cfg;
    ctx->venc_chn = MM_INVALID_CHN;

    if (camera_capture_create_venc(ctx) != 0) {
        return -1;
    }

    if (camera_capture_bind(ctx) != 0) {
        AW_MPI_VENC_DestroyChn(ctx->venc_chn);
        ctx->venc_chn = MM_INVALID_CHN;
        return -1;
    }

    ctx->inited = 1;
    return 0;
}

int camera_capture_start(camera_capture_ctx_t *ctx)
{
    VENC_RECV_PIC_PARAM_S recv_param;
    if (ctx == NULL || !ctx->inited) {
        return -1;
    }
    memset(&recv_param, 0, sizeof(recv_param));
    recv_param.mRecvPicNum = -1;
    if (AW_MPI_VENC_StartRecvPic(ctx->venc_chn, &recv_param) != SUCCESS) {
        return -1;
    }
    ctx->started = 1;
    return 0;
}

int camera_capture_take_with_filename(camera_capture_ctx_t *ctx, const char *file_name)
{
    VENC_STREAM_S stream;
    VENC_PACK_S pack;
    int fd;
    if (ctx == NULL || !ctx->started || file_name == NULL || file_name[0] == '\0') {
        return -1;
    }

    memset(&stream, 0, sizeof(stream));
    memset(&pack, 0, sizeof(pack));
    stream.mpPack = &pack;
    stream.mPackCount = 1;

    if (AW_MPI_VENC_GetStream(ctx->venc_chn, &stream, 3000) != SUCCESS) {
        printf("camera_capture: GetStream timeout or failed\n");
        return -1;
    }

    fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
        printf("camera_capture: open %s failed errno=%d\n", file_name, errno);
        AW_MPI_VENC_ReleaseStream(ctx->venc_chn, &stream);
        return -1;
    }

    if (stream.mpPack->mpAddr0 && stream.mpPack->mLen0 > 0) {
        write(fd, stream.mpPack->mpAddr0, stream.mpPack->mLen0);
    }
    if (stream.mpPack->mpAddr1 && stream.mpPack->mLen1 > 0) {
        write(fd, stream.mpPack->mpAddr1, stream.mpPack->mLen1);
    }

    close(fd);
    AW_MPI_VENC_ReleaseStream(ctx->venc_chn, &stream);
    printf("camera_capture: saved jpeg => %s\n", file_name);
    return 0;
}

int camera_capture_stop(camera_capture_ctx_t *ctx)
{
    if (ctx == NULL || !ctx->inited) {
        return -1;
    }
    if (ctx->started) {
        AW_MPI_VENC_StopRecvPic(ctx->venc_chn);
    }
    ctx->started = 0;
    return 0;
}

int camera_capture_deinit(camera_capture_ctx_t *ctx)
{
    if (ctx == NULL || !ctx->inited) {
        return -1;
    }
    camera_capture_stop(ctx);
    camera_capture_unbind(ctx);
    AW_MPI_VENC_ResetChn(ctx->venc_chn);
    AW_MPI_VENC_DestroyChn(ctx->venc_chn);
    ctx->venc_chn = MM_INVALID_CHN;
    ctx->inited = 0;
    return 0;
}
