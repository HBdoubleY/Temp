#ifndef VE_COMMON_H
#define VE_COMMON_H

#include "typedef.h"

#define VE_ENCODER_REGISTRE_LEN (256)
typedef enum VE_REG_BUF_INDEX {
	REG_BUF_INDEX_ENCPP =      0,
	REG_BUF_INDEX_ENCODER =    1,
	REG_BUF_INDEX_NUM,
} ve_reg_buf_index_e;

typedef struct VeVcuCtlMgr {
	unsigned int       bEnableVcuFuncFlag;
	unsigned char      *pRegBuf;
	unsigned int       nRegBufSize;
} VeVcuCtlMgr;

enum VE_MODE {
	VE_MODE_NULL = -1,
	VE_MODE_ENCPP = 0,
	VE_MODE_ENC,
	VE_MODE_DEC,
	VE_MODE_VCUENC,
	VE_MODE_CNT,
};

enum VE_INTERRUPT_RESULT_TYPE {
	VE_INT_RESULT_TYPE_TIMEOUT   = 0,
	VE_INT_RESULT_TYPE_NORMAL    = 1,
	VE_INT_RESULT_TYPE_CSI_RESET = 2,
};

enum VE_WORK_MODE {
	VE_NORMAL_MODE  = 0,
	VE_DEC_MODE     = 1,
	VE_ENC_MODE     = 2,
	VE_JPG_DEC_MODE = 3,
};

enum DRAMTYPE
{
    DDRTYPE_DDR1_16BITS = 0,
    DDRTYPE_DDR1_32BITS = 1,
    DDRTYPE_DDR2_16BITS = 2,
    DDRTYPE_DDR2_32BITS = 3,
    DDRTYPE_DDR3_16BITS = 4,
    DDRTYPE_DDR3_32BITS = 5,
    DDRTYPE_DDR3_64BITS = 6,
    DDRTYPE_DDR4_16BITS = 7,
    DDRTYPE_DDR4_32BITS = 8,
    DDRTYPE_DDR4_64BITS = 9,
    DDRTYPE_LPDDR4      = 10,

    DDRTYPE_MIN = DDRTYPE_DDR1_16BITS,
    DDRTYPE_MAX = DDRTYPE_LPDDR4,
};

typedef struct VeConfig
{
    int nDecoderFlag;
    int nEncoderFlag;
    int nFormat;
    int nWidth;
    int nEnableAfbcFlag;
    int nResetVeMode;
    unsigned int nVeFreq;
    int bJustRunIspFlag;
	unsigned int bEnableVcuFuncFlag;
}VeConfig;

typedef struct CsiOnlineFrameCnt {
	int frame_cnt0;
	int frame_cnt1;
    unsigned int csi_sensor0_cur_frame_addr;
    unsigned int csi_sensor1_cur_frame_addr;
} CsiOnlineFrameCnt;


typedef enum VE_REGISTER_GROUP {
    REG_GROUP_VETOP             = 0,
    REG_GROUP_MPEG_DECODER      = 1,
    REG_GROUP_H264_DECODER      = 2,
    REG_GROUP_VC1_DECODER       = 3,
    REG_GROUP_RV_DECODER        = 4,
    REG_GROUP_H265_DECODER      = 5,
    REG_GROUP_JPEG_DECODER      = 6,
    REG_GROUP_AVS_DECODER       = 7,

	REG_GROUP_VCU_ENCODER       = 8,
	REG_GROUP_ENCPP_ENCODER     = 9,
	REG_GROUP_JPEG_ENCODER      = 10,
	REG_GROUP_H264_ENCODER      = 11,
	REG_GROUP_H265_ENCODER      = 12,
} ve_register_group_e;

typedef struct VeRegisterGroupMap {
	int  eGroupId;
	char  sGroupName[20];
	size_addr  nGroupRegAddrOffset;
} VeRegisterGroupMap;


#endif

