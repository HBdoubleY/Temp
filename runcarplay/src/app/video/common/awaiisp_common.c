#include <mpi_isp.h>
#include "awaiisp_common.h"

//#define AWAIISP_COMMON_DUMP_TDM_DATA

#ifdef AWAIISP_COMMON_DUMP_TDM_DATA
#define AWAIISP_COMMON_DUMP_TDM_FRAME_CNT_MIN     (50)
#define AWAIISP_COMMON_DUMP_TDM_FRAME_CNT_MAX     (51)
#endif

/* print level */
#define awaiisp_common_err_print(fmt, arg...)      printf("[AWAIISP_COMMON_ERR]%s:%d: " fmt "\n", __FUNCTION__, __LINE__, ##arg)
#define awaiisp_common_warn_print(fmt, arg...)     printf("[AWAIISP_COMMON_WRN]%s:%d: " fmt "\n", __FUNCTION__, __LINE__, ##arg)
#define awaiisp_common_dbg_print(fmt, arg...)      printf("[AWAIISP_COMMON_DBG]%s:%d: " fmt "\n", __FUNCTION__, __LINE__, ##arg)
#define awaiisp_common_ver_print(fmt, arg...)      //printf("[AWAIISP_COMMON_VER]%s:%u: " fmt "\n", __FUNCTION__, __LINE__, ##arg)


#define AWAIISP_COMMON_NUM_MAX  (4)

#define AWAIISP_COMMON_GAMMA_TYPE (3)

typedef struct awaiisp_common_context {
    int isp_id;
    unsigned int tdm_frm_cnt;
    unsigned int tdm_frm_head_len;
    unsigned int tdm_frm_fill_len;
    int npu_lut_enable;
    int aiisp_enable;
} awaiisp_common_context;

static awaiisp_common_context gAiIspCommonContext[AWAIISP_COMMON_NUM_MAX];

static struct awaiisp_common_context *awaiisp_get_common_context(int isp)
{
    if (isp >= AWAIISP_COMMON_NUM_MAX)
    {
        awaiisp_common_err_print("fatal error! invalid ch %d >= %d", isp, AWAIISP_COMMON_NUM_MAX);
        return NULL;
    }
    return &gAiIspCommonContext[isp];
}

#ifdef AWAIISP_COMMON_DUMP_TDM_DATA
void awaiisp_dump_tdm_data_once_callback(int isp, int buf_id, unsigned int buf_size, char *flag)
{
    struct awaiisp_common_context *pContext = awaiisp_get_common_context(isp);

    if (AWAIISP_COMMON_DUMP_TDM_FRAME_CNT_MIN <= pContext->tdm_frm_cnt && pContext->tdm_frm_cnt <= AWAIISP_COMMON_DUMP_TDM_FRAME_CNT_MAX)
    {
        struct vin_isp_tdm_data data;
        memset(&data, 0, sizeof(struct vin_isp_tdm_data));
        data.buf = malloc(buf_size);
        if (data.buf == NULL)
        {
            awaiisp_common_err_print("ch%d malloc size is %d error!", isp, buf_size);
            return;
        }
        data.buf_size = buf_size;
        data.req_buf_id = buf_id;
        AW_MPI_ISP_GetTdmData(isp, &data);
        if (flag)
        {
            char fdstr[128];
            FILE *fp = NULL;
            snprintf(fdstr, 128, "/mnt/extsd/tdm_test/%s_%d.bin", flag, pContext->tdm_frm_cnt);
            fp = fopen(fdstr, "w");
            fwrite(data.buf + pContext->tdm_frm_fill_len + pContext->tdm_frm_head_len, data.buf_size - pContext->tdm_frm_fill_len - pContext->tdm_frm_head_len, 1, fp);
            awaiisp_common_ver_print("ch%d save file %s success", isp, flag);
            fclose(fp);
        }
        if (data.buf)
        {
            free(data.buf);
            data.buf = NULL;
            data.buf_size = 0;
        }
    }
}
#endif

void awaiisp_return_tdm_buffer_callback(struct vin_isp_tdm_event_status *status)
{
    int id = status->dev_id;
    struct awaiisp_common_context *pContext = awaiisp_get_common_context(id);
    pContext->isp_id = id;
    pContext->tdm_frm_fill_len = status->fill_len;
    pContext->tdm_frm_head_len = status->head_len;
    pContext->tdm_frm_cnt++;
    AW_MPI_ISP_ReturnTdmBuf(id, status);
}

int awaiisp_common_enable(int isp, awaiisp_common_config_param *param)
{
    int ret = 0;

    struct awaiisp_common_context *pContext = awaiisp_get_common_context(isp);
    if (1 == pContext->aiisp_enable)
    {
        awaiisp_common_warn_print("ch%d aiisp repeatedly enable, please check it.", isp);
        return 0;
    }

    memset(pContext, 0, sizeof(awaiisp_common_context));

    pContext->aiisp_enable = 1;

    param->config.ion_mem_open = 1;

    ret |= awaiisp_open(isp, &param->config);

    ret |= awaiisp_start(isp);

    ret |= awaiisp_register_return_tdm_buffer_callback(isp, &awaiisp_return_tdm_buffer_callback);

#ifdef AWAIISP_COMMON_DUMP_TDM_DATA
    ret |= awaiisp_register_dump_tdm_data_once_callback(isp, &awaiisp_dump_tdm_data_once_callback);
    awaiisp_set_dump_g2d_data_once_path(isp, "/mnt/extsd/tdm_test");
#endif

    isp_ai_isp_info ai_isp_info;
    memset(&ai_isp_info, 0, sizeof(isp_ai_isp_info));
    if (strlen(param->config.lut_model_file) != 0)
    {
        ai_isp_info.ai_isp_en = AWAIISP_COMMON_GAMMA_TYPE;
        pContext->npu_lut_enable = 1;
    }
    else
    {
        awaiisp_common_dbg_print("ch%d user set lut nbg file path is NULL, set isp ai_isp_en=0", isp);
        ai_isp_info.ai_isp_en = 0;
        pContext->npu_lut_enable = 0;
    }
    ret |= AW_MPI_ISP_SetAiIsp(isp, &ai_isp_info);

    if (param->isp_cfg_bin_path && strlen(param->isp_cfg_bin_path) != 0)
    {
        if (0 != access(param->isp_cfg_bin_path, F_OK))
        {
            awaiisp_common_err_print("fatal error! ch%d isp_cfg_bin_path %s is not exist!", isp, param->isp_cfg_bin_path);
        }
        awaiisp_common_dbg_print("ch%d user set isp cfg bin file %s", isp, param->isp_cfg_bin_path);
        ISP_CFG_BIN_MODE mode_flag = LINEAR_COLOR_CFG;
        ret |= AW_MPI_ISP_ReadIspBin(isp, mode_flag, param->isp_cfg_bin_path);
    }

    ret |= AW_MPI_ISP_RegisterTdmBufDoneCallback(isp, &awaiisp_tdm_buffer_process_callback);

    return ret;
}

int awaiisp_common_disable(int isp)
{
    int ret = 0;
    struct awaiisp_common_context *pContext = awaiisp_get_common_context(isp);

    if (0 == pContext->aiisp_enable)
    {
        awaiisp_common_warn_print("ch%d aiisp is not enable or repeatedly disable.", isp);
        return 0;
    }

    ret |= AW_MPI_ISP_RegisterTdmBufDoneCallback(isp, NULL);

    ret |= awaiisp_stop(isp);

    ret |= awaiisp_close(isp);

    pContext->aiisp_enable = 0;

    return ret;
}

int awaiisp_common_switch_mode(int isp, awaiisp_common_switch_param *param)
{
    int ret = 0;
    struct awaiisp_common_context *pContext = awaiisp_get_common_context(isp);
    if (0 == pContext->aiisp_enable)
    {
        awaiisp_common_warn_print("ch%d aiisp is not enable, please check it.", isp);
        return -1;
    }

    if ((NULL == param->isp_cfg_bin_path) || (param->isp_cfg_bin_path && strlen(param->isp_cfg_bin_path) == 0))
    {
        awaiisp_common_err_print("fatal error! ch%d invalid isp cfg bin path %p", isp, param->isp_cfg_bin_path);
        return -1;
    }

    /**
      awaiisp_switch_mode is a block api that needs to be switched before configuring isp.
    */
    ret |= awaiisp_switch_mode(isp, &param->config);

    isp_ai_isp_info ai_isp_info;
    memset(&ai_isp_info, 0, sizeof(isp_ai_isp_info));
    if (0 == pContext->npu_lut_enable)
    {
        ai_isp_info.ai_isp_en = 0;
        awaiisp_common_dbg_print("ch%d user disable npu lut, so set ai_isp_en = 0", isp);
    }
    else
    {
        if (AWAIISP_MODE_NPU == param->config.mode || AWAIISP_MODE_NORMAL_GAMMA == param->config.mode)
            ai_isp_info.ai_isp_en = AWAIISP_COMMON_GAMMA_TYPE;
        else
            ai_isp_info.ai_isp_en = 0;
    }
    ret |= AW_MPI_ISP_SetAiIsp(isp, &ai_isp_info);

    ISP_CFG_BIN_MODE mode_flag = LINEAR_COLOR_CFG;
    ret |= AW_MPI_ISP_ReadIspBin(isp, mode_flag, param->isp_cfg_bin_path);

    return ret;
}
