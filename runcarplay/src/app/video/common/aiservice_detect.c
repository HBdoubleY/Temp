#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <utils/plat_log.h>
#include <utils/plat_math.h>
#include <sys/prctl.h>
#include "aiservice.h"
#include "aiservice_detect.h"
#include "aiservice_mpp_helper.h"
#include "aiservice_hw_scale.h"
#include <VIDEO_FRAME_INFO_S.h>

//#define AI_SERVICE_DEBUG_NPU_DET_DISABLE

#define AI_SERVICE_DEBUG_FPS_ENV_NAME      "AI_SERVICE_DEBUG_FPS"
#define AI_SERVICE_DEBUG_RESULT_ENV_NAME   "AI_SERVICE_DEBUG_RESULT"
#define AI_SERVICE_DEBUG_RUNTIME_ENV_NAME  "AI_SERVICE_DEBUG_RUNTIME"

#define MILLION        1000000L

static aialgo_context_t *gp_ai_service_context = NULL;

static long get_time_in_us(void)
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return (time.tv_usec + time.tv_sec * MILLION);
}

aialgo_context_t *get_aicontext(void)
{
    if (gp_ai_service_context)
    {
        return gp_ai_service_context;
    }
    else
    {
        aloge("fatal error! please execute aw_service_init first?");
        return NULL;
    }
}

int aw_service_init(void)
{
    aialgo_context_t *pctx = (aialgo_context_t *)malloc(sizeof(aialgo_context_t));
    if (NULL == pctx)
    {
        aloge("sys Init failed!");
        return -1;
    }
    gp_ai_service_context = pctx;
    memset(pctx, 0x00, sizeof(aialgo_context_t));

    if (mpp_helper_init())
    {
        aloge("sys Init failed!");
        return -1;
    }

    return 0;
}

int aw_service_uninit(void)
{
    /* exit mpp systerm */
    if (mpp_helper_uninit())
    {
        aloge("sys exit failed!");
    }

    aialgo_context_t *pctx = get_aicontext();
    if (pctx)
    {
        free(pctx);
        pctx = NULL;
    }
    gp_ai_service_context = NULL;

    return 0;
}

static int npu_detect_callback_body(unsigned char *pBuffer, int size, int ch_idx, Awnn_Context_t *context)
{
    unsigned char *body_input_buf[2] = { NULL, NULL };
    aialgo_context_t *pctx = get_aicontext();
    int ret = 0;

    body_input_buf[0] = pBuffer;
    body_input_buf[1] = pBuffer + size * 2 / 3;

    awnn_set_input_buffers(context, body_input_buf);
    long start_time = get_time_in_us();
    awnn_run(context);
    if (1 == pctx->ai_det_runtime_print || 3 == pctx->ai_det_runtime_print)
    {
        alogd("body awnn_run cost %dms", (get_time_in_us() - start_time)/1000);
    }

    Awnn_Post_t post;
    post.type = (AWNN_DET_POST_TYPE)pctx->attr.ch_info[ch_idx].nbg_type;
    post.width = pctx->attr.ch_info[ch_idx].src_width;
    post.height = pctx->attr.ch_info[ch_idx].src_height;
    post.thresh = pctx->attr.ch_info[ch_idx].thresh;
    Awnn_Result_t result;
    awnn_det_post(context, &post, &result);

    if (result.valid_cnt > 0)
    {
        if (pctx->attr.ch_info[ch_idx].draw_orl_enable)
        {
            paint_object_detect_region_body(&result, ch_idx);
        }
        for (int j = 0; j < result.valid_cnt; j ++)
        {
            if (1 == pctx->ai_det_result_print || 5 == pctx->ai_det_result_print)
            {
                alogd("ch_idx=%d, [%d/%d] cls %d, prob %f, rect[%d,%d,%d,%d]", ch_idx, j+1, result.valid_cnt,
                    result.boxes[j].label, result.boxes[j].score,
                    result.boxes[j].xmin, result.boxes[j].ymin, result.boxes[j].xmax, result.boxes[j].ymax);
            }
            else if (2 == pctx->ai_det_result_print || 6 == pctx->ai_det_result_print)
            {
                if (0 == result.boxes[j].label)
                {
                    alogd("ch_idx=%d, [%d/%d] cls %d, prob %f, rect[%d,%d,%d,%d]", ch_idx, j+1, result.valid_cnt,
                        result.boxes[j].label, result.boxes[j].score,
                        result.boxes[j].xmin, result.boxes[j].ymin, result.boxes[j].xmax, result.boxes[j].ymax);
                }
            }
        }
        ret = result.valid_cnt;
    }

    return ret;
}

static void *npu_worker_thread_body(void *para)
{
    int ret = 0;
    aialgo_context_t *pctx = get_aicontext();
    ai_service_info_t *pchinfo = (ai_service_info_t *)para;
    if (!pctx || !pchinfo)
    {
        aloge("fatal error! invalid input params! %p,%p", pctx, pchinfo);
        return NULL;
    }
    prctl(PR_SET_NAME, "npu_body_worker_thread", 0, 0, 0);

    double start_time = get_time_in_us();
    double before     = get_time_in_us();
    int frame_counter = 0;
    statistics_t         statis;
    memset(&statis, 0, sizeof(statistics_t));

#ifndef AI_SERVICE_DEBUG_NPU_DET_DISABLE
    Awnn_Context_t *context = awnn_create(pchinfo->model_file);
    if (NULL == context)
    {
        aloge("fatal error, awnn_create failed.");
        return NULL;
    }
#endif

    // human model requires 32 alignment, 320x180 -> 320x192
    int npu_det_pic_width = AWALIGN(pctx->human_src_width, 32);
    int npu_det_pic_height = AWALIGN(pctx->human_src_height, 32);
    alogd("human_src %dx%d, npu_det_pic %dx%d", pctx->human_src_width, pctx->human_src_height,
        npu_det_pic_width, npu_det_pic_height);

    int yuv_buffer_size = npu_det_pic_width * npu_det_pic_height * 3 / 2;
    unsigned char *yuv_buffer_body = malloc(yuv_buffer_size);
    if(yuv_buffer_body == NULL)
    {
        aloge("fatal error, malloc yuv body buffer failure.");
        return NULL;
    }
    memset(yuv_buffer_body, 0x00, yuv_buffer_size);
    alogd("yuv_buffer_body=%p, yuv_buffer_size=%d", yuv_buffer_body, yuv_buffer_size);

    VIDEO_FRAME_INFO_S stFrameInfo;
    VideoFrameBufferSizeInfo vfbsInfo;

    MPP_CHN_S viChn = {MOD_ID_VIU, pctx->attr.ch_info[pchinfo->ch_idx].vipp, 0};

    while (!pctx->aiservice_exit)
    {
        memset(&stFrameInfo, 0, sizeof(VIDEO_FRAME_INFO_S));
        memset(&vfbsInfo, 0, sizeof(VideoFrameBufferSizeInfo));

        ret = AW_MPI_VI_GetFrame(viChn.mDevId, viChn.mChnId, &stFrameInfo, 1000);
        if (SUCCESS == ret)
        {
            if (SUCCESS == getVideoFrameBufferSizeInfo(&stFrameInfo, &vfbsInfo))
            {
                if (stFrameInfo.VFrame.mpVirAddr[0])
                {
                    memcpy(yuv_buffer_body, stFrameInfo.VFrame.mpVirAddr[0], vfbsInfo.mYSize);
                }
                if (stFrameInfo.VFrame.mpVirAddr[1])
                {
                    memcpy(yuv_buffer_body + yuv_buffer_size*2/3, stFrameInfo.VFrame.mpVirAddr[1], vfbsInfo.mUSize);
                }
                if (stFrameInfo.VFrame.mpVirAddr[2])
                {
                    aloge("fatal error, the V component should be null.");
                }
            }

            AW_MPI_VI_ReleaseFrame(viChn.mDevId, viChn.mChnId, &stFrameInfo);

#ifndef AI_SERVICE_DEBUG_NPU_DET_DISABLE
            npu_detect_callback_body(yuv_buffer_body, yuv_buffer_size, pchinfo->ch_idx, context);
#endif
        }

        if (1 == pctx->ai_det_fps_print || 3 == pctx->ai_det_fps_print)
        {
            double after = get_time_in_us();
            float curr = MILLION / (after - before);
            before = after;

            statis.fps_body = statis.fps_body * 0.9 + curr * 0.1;
            frame_counter ++;
            float spent_time = (get_time_in_us() - start_time) / MILLION;

            //statistic one time each 3 seconds.
            if (spent_time >= 3.0f)
            {
                statis.avg_fps_body = frame_counter / spent_time;
                frame_counter = 0;
                start_time = get_time_in_us();
                alogd("ch_idx=%d, avg body fps %f, body fps %f.", pchinfo->ch_idx, statis.avg_fps_body, statis.fps_body);
            }
        }
    }

#ifndef AI_SERVICE_DEBUG_NPU_DET_DISABLE
    awnn_destroy(context);
#endif

    if (yuv_buffer_body)
    {
        free(yuv_buffer_body);
        yuv_buffer_body = NULL;
    }

    return NULL;
}

static int npu_detect_callback_face(unsigned char *pBuffer, int size, int ch_idx, Awnn_Context_t *context)
{
    unsigned char *face_input_buf[2] = { NULL, NULL };
    aialgo_context_t *pctx = get_aicontext();

    if (pctx->use_g2d_flag)
    {
        unsigned int vi_npu_pic_width = pctx->human_src_width;
        unsigned int vi_npu_pic_height = pctx->human_src_height;
        int scale_target_width = pctx->face_src_width;
        int scale_target_height = pctx->face_src_height;

        if(pctx->pout_yuv == NULL)
        {
            int size = scale_target_width * scale_target_height * 3 / 2;
            pctx->pout_yuv = malloc(size);
            if (!pctx->pout_yuv)
            {
                aloge("malloc poutyuv is null.");
                return 0;
            }
            memset(pctx->pout_yuv, 0x00, size);
        }

        int face_yuv_len = AWALIGN(pctx->face_src_width, 32) * AWALIGN(pctx->face_src_height, 32) * 3 / 2;
        if(pctx->face_yuv == NULL)
        {
            pctx->face_yuv = malloc(face_yuv_len);
            if (!pctx->face_yuv)
            {
                aloge("malloc poutyuv is null.");
                return 0;
            }
            memset(pctx->face_yuv, 0x00, face_yuv_len);
        }

        if(pctx->pin_yuv == NULL)
        {
            int size = vi_npu_pic_width * vi_npu_pic_height * 3 / 2;
            pctx->pin_yuv = malloc(size);
            if (!pctx->pin_yuv)
            {
                aloge("malloc poutyuv is null.");
                return 0;
            }
            memset(pctx->pin_yuv, 0x00, size);
        }

        memcpy(pctx->pin_yuv, pBuffer, vi_npu_pic_width * vi_npu_pic_height);
        memcpy(pctx->pin_yuv + vi_npu_pic_width * vi_npu_pic_height, pBuffer + size * 2 / 3, vi_npu_pic_width * vi_npu_pic_height * 1 / 2);

        //g2d hardware scale.
        scale_picture_nv12_to_nv12_byg2d(pctx->pin_yuv, vi_npu_pic_width, vi_npu_pic_height, pctx->pout_yuv, scale_target_width, scale_target_height, pctx->g2d_fd);

        memcpy(pctx->face_yuv, pctx->pout_yuv, scale_target_width * scale_target_height);
        memcpy(pctx->face_yuv + face_yuv_len * 2 / 3, pctx->pout_yuv + scale_target_width * scale_target_height, scale_target_width * scale_target_height * 1 / 2);

        face_input_buf[0] = pctx->face_yuv;
        face_input_buf[1] = pctx->face_yuv + face_yuv_len * 2 / 3;
    }
    else
    {
        face_input_buf[0] = pBuffer;
        face_input_buf[1] = pBuffer + size * 2 / 3;
    }

    awnn_set_input_buffers(context, face_input_buf);
    long start_time = get_time_in_us();
    awnn_run(context);
    if (2 == pctx->ai_det_runtime_print || 3 == pctx->ai_det_runtime_print)
    {
        alogd("face awnn_run cost %dms", (get_time_in_us() - start_time)/1000);
    }

    Awnn_Post_t post;
    post.type = (AWNN_DET_POST_TYPE)pctx->attr.ch_info[ch_idx].nbg_type;
    post.width = pctx->attr.ch_info[ch_idx].src_width;
    post.height = pctx->attr.ch_info[ch_idx].src_height;
    post.thresh = pctx->attr.ch_info[ch_idx].thresh;
    Awnn_Result_t result;
    awnn_det_post(context, &post, &result);

    if (result.valid_cnt > 0)
    {
        if (pctx->attr.ch_info[ch_idx].draw_orl_enable)
        {
            paint_object_detect_region_face(&result, ch_idx);
        }
        for (int j = 0; j < result.valid_cnt; j++)
        {
            if (3 == pctx->ai_det_result_print || 5 == pctx->ai_det_result_print)
            {
                alogd("ch_idx=%d, [%d/%d] cls %d, prob %f, landmark[%d,%d][%d,%d][%d,%d][%d,%d][%d,%d]", ch_idx, j+1, result.valid_cnt,
                    result.boxes[j].label, result.boxes[j].score,
                    result.boxes[j].landmark_x[0],result.boxes[j].landmark_x[0],
                    result.boxes[j].landmark_x[1],result.boxes[j].landmark_x[1], result.boxes[j].landmark_x[2],result.boxes[j].landmark_x[2],
                    result.boxes[j].landmark_x[3],result.boxes[j].landmark_x[3], result.boxes[j].landmark_x[4],result.boxes[j].landmark_x[4]);
            }
            else if (4 == pctx->ai_det_result_print || 6 == pctx->ai_det_result_print)
            {
                if (0 == result.boxes[j].label)
                {
                    alogd("ch_idx=%d, [%d/%d] cls %d, prob %f, landmark[%d,%d][%d,%d][%d,%d][%d,%d][%d,%d]", ch_idx, j+1, result.valid_cnt,
                        result.boxes[j].label, result.boxes[j].score,
                        result.boxes[j].landmark_x[0],result.boxes[j].landmark_x[0],
                        result.boxes[j].landmark_x[1],result.boxes[j].landmark_x[1], result.boxes[j].landmark_x[2],result.boxes[j].landmark_x[2],
                        result.boxes[j].landmark_x[3],result.boxes[j].landmark_x[3], result.boxes[j].landmark_x[4],result.boxes[j].landmark_x[4]);
                }
            }
        }
    }

    return 0;
}

static void *npu_worker_thread_face(void *para)
{
    int ret = 0;
    aialgo_context_t *pctx = get_aicontext();
    ai_service_info_t *pchinfo = (ai_service_info_t *)para;
    if (!pctx || !pchinfo)
    {
        aloge("fatal error! invalid input params! %p,%p", pctx, pchinfo);
        return NULL;
    }
    prctl(PR_SET_NAME, "npu_face_worker_thread", 0, 0, 0);

    double start_time = get_time_in_us();
    double before     = get_time_in_us();
    int frame_counter = 0;
    statistics_t         statis;
    memset(&statis, 0, sizeof(statistics_t));

#ifndef AI_SERVICE_DEBUG_NPU_DET_DISABLE
    Awnn_Context_t *context = awnn_create(pchinfo->model_file);
    if (NULL == context)
    {
        aloge("fatal error, awnn_create failed.");
        return NULL;
    }
#endif

    if (pctx->use_g2d_flag)
    {
        pctx->g2d_fd = open("/dev/g2d", O_RDWR, 0);
        if(pctx->g2d_fd < 0)
        {
            aloge("open g2d device failed!");
            return NULL;
        }

        if(g2d_mem_open() != 0)
        {
            aloge("init ion device failed!");
            close(pctx->g2d_fd);
            pctx->g2d_fd = -1;

            return NULL;
        }

        scale_create_buffer();
    }

    int yuv_buffer_size = 0;
    if (pctx->use_g2d_flag)
    {
        int npu_det_pic_width = AWALIGN(pctx->human_src_width, 32);
        int npu_det_pic_height = AWALIGN(pctx->human_src_height, 32);
        yuv_buffer_size = npu_det_pic_width * npu_det_pic_height * 3 / 2;
    }
    else
    {
        // face model requires 32 alignment, 480x270 -> 480x288
        yuv_buffer_size = AWALIGN(pctx->face_src_width, 32) * AWALIGN(pctx->face_src_height, 32) * 3 / 2;
    }
    unsigned char *yuv_buffer_face = malloc(yuv_buffer_size);
    if(yuv_buffer_face == NULL)
    {
        aloge("fatal error, malloc yuv face buffer failure.");
        return NULL;
    }
    memset(yuv_buffer_face, 0x00, yuv_buffer_size);
    alogd("yuv_buffer_face=%p, yuv_buffer_size=%d", yuv_buffer_face, yuv_buffer_size);

    VIDEO_FRAME_INFO_S stFrameInfo;
    VideoFrameBufferSizeInfo vfbsInfo;

    MPP_CHN_S viChn = {MOD_ID_VIU, pctx->attr.ch_info[pchinfo->ch_idx].vipp, 0};

    while (!pctx->aiservice_exit)
    {
        memset(&stFrameInfo, 0, sizeof(VIDEO_FRAME_INFO_S));
        memset(&vfbsInfo, 0, sizeof(VideoFrameBufferSizeInfo));

        ret = AW_MPI_VI_GetFrame(viChn.mDevId, viChn.mChnId, &stFrameInfo, 1000);
        if (SUCCESS == ret)
        {
            if (SUCCESS == getVideoFrameBufferSizeInfo(&stFrameInfo, &vfbsInfo))
            {
                if (stFrameInfo.VFrame.mpVirAddr[0])
                {
                    memcpy(yuv_buffer_face, stFrameInfo.VFrame.mpVirAddr[0], vfbsInfo.mYSize);
                }
                if (stFrameInfo.VFrame.mpVirAddr[1])
                {
                    memcpy(yuv_buffer_face + yuv_buffer_size*2/3, stFrameInfo.VFrame.mpVirAddr[1], vfbsInfo.mUSize);
                }
                if (stFrameInfo.VFrame.mpVirAddr[2])
                {
                    aloge("fatal error, the V component should be null.");
                }
            }

            AW_MPI_VI_ReleaseFrame(viChn.mDevId, viChn.mChnId, &stFrameInfo);

#ifndef AI_SERVICE_DEBUG_NPU_DET_DISABLE
            npu_detect_callback_face(yuv_buffer_face, yuv_buffer_size, pchinfo->ch_idx, context);
#endif
        }

        if (2 == pctx->ai_det_fps_print || 3 == pctx->ai_det_fps_print)
        {
            double after = get_time_in_us();
            float curr = MILLION / (after - before);
            before = after;

            statis.fps_face = statis.fps_face * 0.9 + curr * 0.1;
            frame_counter ++;
            float spent_time = (get_time_in_us() - start_time) / MILLION;

            //statistic one time each 3 seconds.
            if (spent_time >= 3.0f)
            {
                statis.avg_fps_face = frame_counter / spent_time;
                frame_counter = 0;
                start_time = get_time_in_us();
                alogd("ch_idx=%d, avg face fps %f, face fps %f.", pchinfo->ch_idx, statis.avg_fps_face, statis.fps_face);
            }
        }
    }

#ifndef AI_SERVICE_DEBUG_NPU_DET_DISABLE
    awnn_destroy(context);
#endif

    if(yuv_buffer_face)
    {
        free(yuv_buffer_face);
        yuv_buffer_face = NULL;
    }

    if (pctx->use_g2d_flag)
    {
        if (pctx->pout_yuv)
        {
            free(pctx->pout_yuv);
            pctx->pout_yuv = NULL;
        }
        if (pctx->pin_yuv)
        {
            free(pctx->pin_yuv);
            pctx->pin_yuv = NULL;
        }
        if (pctx->face_yuv)
        {
            free(pctx->face_yuv);
            pctx->face_yuv = NULL;
        }

        scale_destory_buffer();

        close(pctx->g2d_fd);
        pctx->g2d_fd = -1;

        g2d_mem_close();
    }

    return NULL;
}

static void debug_usage(void)
{
    alogd("*******************************************************");
    alogd("aw service usage:");
    alogd("please set env before testing.");

    alogd("[1] export %s=0", AI_SERVICE_DEBUG_FPS_ENV_NAME);
    alogd("    0: disable print run fps for body and face");
    alogd("    1: enable print run fps for body");
    alogd("    2: enable print run fps for face");
    alogd("    3: enable print run fps for body and face");

    alogd("[2] export %s=0", AI_SERVICE_DEBUG_RESULT_ENV_NAME);
    alogd("    0: disable print det result for body and face");
    alogd("    1: enable print all det result for body");
    alogd("    2: enable print det result(only for label=0) for body");
    alogd("    3: enable print all det result for face");
    alogd("    4: enable print det result(only for label=0) for face");
    alogd("    5: enable print all det result for body and face");
    alogd("    6: enable print det result(only for label=0) for body and face");

    alogd("[3] export %s=0", AI_SERVICE_DEBUG_RUNTIME_ENV_NAME);
    alogd("    0: disable print det run cost time for body and face");
    alogd("    1: enable print det run cost time for body");
    alogd("    2: enable print det run cost time for face");
    alogd("    3: enable print det run cost time for body and face");
    alogd("*******************************************************");
}

static void get_debug_env(aialgo_context_t *pctx)
{
    // 检测环境变量，开启/关闭 人形/人脸检测FPS信息打印
    char *fps_env = getenv(AI_SERVICE_DEBUG_FPS_ENV_NAME);
    alogv("%s=%s", AI_SERVICE_DEBUG_FPS_ENV_NAME, fps_env);
    if (fps_env && strlen(fps_env))
    {
        pctx->ai_det_fps_print = (int)(atoi(fps_env));
    }
    else
    {
        pctx->ai_det_fps_print = 0;
    }
    alogd("ai_det_fps_print:%d", pctx->ai_det_fps_print);

    // 检测环境变量，开启/关闭 人形/人脸检测结果框坐标打印
    char *result_env = getenv(AI_SERVICE_DEBUG_RESULT_ENV_NAME);
    alogv("%s=%s", AI_SERVICE_DEBUG_RESULT_ENV_NAME, result_env);
    if (result_env && strlen(result_env))
    {
        pctx->ai_det_result_print = (int)(atoi(result_env));
    }
    else
    {
        pctx->ai_det_result_print = 0;
    }
    alogd("ai_det_result_print:%d", pctx->ai_det_result_print);

    // 检测环境变量，开启/关闭 人形/人脸检测RUN耗时打印
    char *runtime_env = getenv(AI_SERVICE_DEBUG_RUNTIME_ENV_NAME);
    alogv("%s=%s", AI_SERVICE_DEBUG_RUNTIME_ENV_NAME, runtime_env);
    if (runtime_env && strlen(runtime_env))
    {
        pctx->ai_det_runtime_print = (int)(atoi(runtime_env));
    }
    else
    {
        pctx->ai_det_runtime_print = 0;
    }
    alogd("ai_det_runtime_print:%d", pctx->ai_det_runtime_print);
}

int aw_service_start(ai_service_attr_t *pattr)
{
    int err = 0;
    char *human_nb = NULL;
    char *face_nb = NULL;

    aialgo_context_t *pctx = get_aicontext();
    if (NULL == pctx)
    {
        aloge("fatal error! pctx is NULL");
        return -1;
    }

    memcpy(&pctx->attr, pattr, sizeof(ai_service_attr_t));

    pctx->aiservice_exit = 0;

    unsigned int mem_size = 0;

    for (int i = 0; i < NN_CHN_NUM_MAX; i++)
    {
        if (AWNN_DET_POST_HUMANOID_1 == pctx->attr.ch_info[i].nbg_type ||
            AWNN_DET_POST_HUMANOID_2 == pctx->attr.ch_info[i].nbg_type ||
            AWNN_DET_POST_HUMANOID_3 == pctx->attr.ch_info[i].nbg_type)
        {
            human_nb = pctx->attr.ch_info[i].model_file;
            pctx->human_src_width = pctx->attr.ch_info[i].src_width;
            pctx->human_src_height = pctx->attr.ch_info[i].src_height;
#ifndef AI_SERVICE_DEBUG_NPU_DET_DISABLE
            awnn_info_t *nbinfo = awnn_get_info(human_nb);
            alogd("human nbinfo %s %s %u %u %u %f\n", nbinfo->name, nbinfo->md5,
                nbinfo->width, nbinfo->height, nbinfo->mem_size, nbinfo->thresh);
            mem_size += nbinfo->mem_size;
#endif
        }
        else if (AWNN_DET_POST_FACE_1 == pctx->attr.ch_info[i].nbg_type)
        {
            face_nb = pctx->attr.ch_info[i].model_file;
            pctx->face_src_width = pctx->attr.ch_info[i].src_width;
            pctx->face_src_height = pctx->attr.ch_info[i].src_height;
#ifndef AI_SERVICE_DEBUG_NPU_DET_DISABLE
            awnn_info_t *nbinfo = awnn_get_info(face_nb);
            alogd("face nbinfo %s %s %u %u %u %f\n", nbinfo->name, nbinfo->md5,
                nbinfo->width, nbinfo->height, nbinfo->mem_size, nbinfo->thresh);
            mem_size += nbinfo->mem_size;
#endif
        }
    }

    alogd("human src_width:%d, src_height:%d, face src_width:%d, src_height:%d",
        pctx->human_src_width, pctx->human_src_height, pctx->face_src_width, pctx->face_src_height);

    if (NULL == human_nb && NULL == face_nb)
    {
        alogw("both human and face are disabled!");
        return 0;
    }

    if (human_nb)
    {
        if (access(human_nb, F_OK) != 0)
        {
            aloge("fatal error! human_nb %s is not exist.", human_nb);
            return -1;
        }
    }

    if (face_nb)
    {
        if (access(face_nb, F_OK) != 0)
        {
            aloge("fatal error! face_nb %s is not exist.", face_nb);
            return -1;
        }
    }

    //debug_usage();

    get_debug_env(pctx);

    // 人形和人脸的数据来自同一个vipp，需要使用G2D转换
    if (human_nb && face_nb && pctx->attr.ch_info[0].vipp == pctx->attr.ch_info[1].vipp)
    {
        pctx->use_g2d_flag = 1;
        alogd("both human and face are same vipp%d, use g2d scale.", pctx->attr.ch_info[0].vipp);
    }
    else
    {
        pctx->use_g2d_flag = 0;
    }

#ifndef AI_SERVICE_DEBUG_NPU_DET_DISABLE
    long start_time = get_time_in_us();
    awnn_init(mem_size);
    alogd("awnn_init cost %dms", (get_time_in_us() - start_time)/1000);
    alogd("total mem_size=%d, human model=%s, face model=%s", mem_size, human_nb, face_nb);
#endif

    for (int i = 0; i < NN_CHN_NUM_MAX; i++)
    {
        if (AWNN_DET_POST_HUMANOID_1 == pctx->attr.ch_info[i].nbg_type ||
            AWNN_DET_POST_HUMANOID_2 == pctx->attr.ch_info[i].nbg_type ||
            AWNN_DET_POST_HUMANOID_3 == pctx->attr.ch_info[i].nbg_type ||
            AWNN_DET_POST_FACE_1 == pctx->attr.ch_info[i].nbg_type)
        {
            pctx->region_info[i].region_hdl_base = pctx->attr.ch_info[i].region_hdl_base;
            mpp_helper_start_npu(pctx->attr.ch_info[i].isp,
                                 pctx->attr.ch_info[i].vipp,
                                 pctx->attr.ch_info[i].vi_buf_num,
                                 pctx->attr.ch_info[i].src_width,
                                 pctx->attr.ch_info[i].src_height,
                                 pctx->attr.ch_info[i].src_frame_rate,
                                 pctx->attr.ch_info[i].pixel_format);
            if (pctx->use_g2d_flag)
            {
                break;
            }
        }
    }
    pctx->vichannel_started = 1;

    for (int i = 0; i < NN_CHN_NUM_MAX; i++)
    {
        if (AWNN_DET_POST_HUMANOID_1 == pctx->attr.ch_info[i].nbg_type ||
            AWNN_DET_POST_HUMANOID_2 == pctx->attr.ch_info[i].nbg_type ||
            AWNN_DET_POST_HUMANOID_3 == pctx->attr.ch_info[i].nbg_type)
        {
            err = pthread_create(&pctx->detect_thread[i], NULL, npu_worker_thread_body, &pctx->attr.ch_info[i]);
            if (err != 0)
            {
                aloge("fatal error, create body ch%d detect worker thread failed.", i);
                return -1;
            }
            alogd("create body ch%d detect worker thread success.", i);
        }
        if (AWNN_DET_POST_FACE_1 == pctx->attr.ch_info[i].nbg_type)
        {
            err = pthread_create(&pctx->detect_thread[i], NULL, npu_worker_thread_face, &pctx->attr.ch_info[i]);
            if (err != 0)
            {
                aloge("fatal error, create face ch%d detect worker thread failed.", i);
                return -1;
            }
            alogd("create face ch%d detect worker thread success.", i);
        }
    }

    alogd("success");

    return err;
}

int aw_service_stop(void)
{
    unsigned long value;

    aialgo_context_t *pctx = get_aicontext();
    if (NULL == pctx)
    {
        aloge("fatal error! pctx is NULL");
        return -1;
    }

    pctx->aiservice_exit = 1;

    for (int i = 0; i < NN_CHN_NUM_MAX; i++)
    {
        if (pctx->detect_thread[i])
        {
            pthread_join(pctx->detect_thread[i], (void **)&value);
            pctx->detect_thread[i] = NULL;
        }
    }

    if (pctx->vichannel_started)
    {
        for (int i = 0; i < NN_CHN_NUM_MAX; i++)
        {
            if (AWNN_DET_POST_HUMANOID_1 == pctx->attr.ch_info[i].nbg_type ||
                AWNN_DET_POST_HUMANOID_2 == pctx->attr.ch_info[i].nbg_type ||
                AWNN_DET_POST_HUMANOID_3 == pctx->attr.ch_info[i].nbg_type ||
                AWNN_DET_POST_FACE_1 == pctx->attr.ch_info[i].nbg_type)
            {
                mpp_helper_stop_npu(pctx->attr.ch_info[i].isp, pctx->attr.ch_info[i].vipp);
                if (pctx->use_g2d_flag)
                {
                    break;
                }
            }
        }

#ifndef AI_SERVICE_DEBUG_NPU_DET_DISABLE
        long start_time = get_time_in_us();
        awnn_uninit();
        alogd("awnn_uninit cost %dms", (get_time_in_us() - start_time)/1000);
#endif
    }

    pctx->vichannel_started = 0;

    alogd("success");

    return 0;
}
