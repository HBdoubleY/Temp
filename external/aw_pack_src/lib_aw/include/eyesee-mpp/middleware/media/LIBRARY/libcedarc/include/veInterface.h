/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : ve_interface.h
* Description :
* History :
*   Author  : xyliu <xyliu@allwinnertech.com>
*   Date    : 2016/04/13
*   Comment :
*
*
*/


#ifndef VE_INTERFACE_H
#define VE_INTERFACE_H

#include <stddef.h>
#include <stdint.h>
#include "../base/include/UserKernelAdapter.h"
#include "ve_common.h"
#ifdef __cplusplus
extern "C"
{
#endif

enum EVEOPSTYPE{
    VE_OPS_TYPE_NORMAL = 0,
    VE_OPS_TYPE_VP9    = 1,
};

enum RESET_VE_MODE
{
    RESET_VE_NORMAL = 0,
    RESET_VE_SPECIAL = 1,  // for dtmb, we should reset ve not reset decode
};

struct user_iommu_param {
    int            fd;
    unsigned int   iommu_addr;
};

/* the struct must be same with lichee/linux-xx/drivers/media/cedar-ve/cedar-ve.h*/
typedef struct ve_channel_proc_info
{
    unsigned char* base_info_data;
    unsigned int   base_info_size;
    unsigned char* advance_info_data;
    unsigned int   advance_info_size;
    unsigned int   channel_id;
}ve_channel_proc_info;

typedef struct CsiOnlineRelatedInfo
{
    unsigned int       csi_frame_start_cnt;
    unsigned int       csi_frame_done_cnt;
    unsigned int       csi_cur_frame_addr;
    unsigned int       csi_pre_frame_addr;
    unsigned int       csi_line_start_cnt;
    unsigned int       csi_line_done_cnt;
}CsiOnlineRelatedInfo;

typedef struct page_buf_info {
    /* total_size = header + data + ext */
    unsigned int header_size;
    unsigned int data_size;
    unsigned int ext_size;
    unsigned int phy_addr_0;
    unsigned int phy_addr_1;
    unsigned int buf_id;
    unsigned int vir_addr_0;
    unsigned int vir_addr_1;
}page_buf_info;

typedef struct VeOpsS
{
    void *(*init)(VeConfig*);
    void (*release)(void*);
    int (*lock)(void*);
    int (*unlock)(void*);
    void (*reset)(void*);
    void (*resetForce)(void*);
    int  (*waitInterrupt)(void*);

#if CEDARC_DEBUG
    void (*WriteValue)(void*, unsigned int);
    unsigned int (*ReadValue)(void*, unsigned int);
    void (*CleanValue)(void*);
#endif

    int          (*getChipId)(void*);
    uint64_t     (*getIcVeVersion)(void*);
    void*        (*getGroupRegAddr)(void*, int);
    int          (*getDramType)(void*);
    unsigned int (*getPhyOffset)(void*);

    void (*setDramType)(void*);
    void (*setDdrMode)(void*, int);
    int  (*setSpeed)(void*, unsigned int);
    void (*setEnableAfbcFlag)(void*, int);
    void (*setAdjustDramSpeedFlag)(void*, int);

    void (*enableVe)(void*);
    void (*disableVe)(void*);

    void (*initEncoderPerformance)(void*, int);
    void (*unInitEncoderPerformance)(void*, int);

    int  (*getIommuAddr)(void* , struct user_iommu_param *);
    int  (*freeIommuAddr)(void* , struct user_iommu_param *);

    int  (*setProcInfo)(void*, struct ve_channel_proc_info *);
    int  (*stopProcInfo)(void*, unsigned char);

    int  (*getCsiOnlineInfo)(void*, CsiOnlineRelatedInfo *);
    int  (*setLbcParameter)(void*, unsigned int, unsigned int);

    int  (*allocPageBuf)(void*, page_buf_info *);
    int  (*recPageBuf)(void*, page_buf_info *);
    int  (*freePageBuf)(void*, page_buf_info *);

    void (*setOnlineChannel)(void*, unsigned int);

	int (*setMode)(void *, enum VE_MODE);
}VeOpsS;

static inline void* CdcVeInit(VEOPS_STRUCT *veops, VeConfig* pVeConfig)
{
    return veops->init(pVeConfig);
}

static inline void CdcVeRelease(VEOPS_STRUCT *veops, void *p)
{
    veops->release(p);
}

static inline int CdcVeLock(VEOPS_STRUCT *veops, void *p)
{
    return veops->lock(p);
}

static inline int CdcVeUnLock(VEOPS_STRUCT *veops, void *p)
{
    return veops->unlock(p);
}

static inline void CdcVeReset(VEOPS_STRUCT *veops, void *p)
{
    veops->reset(p);
}

static inline void CdcVeResetForce(VEOPS_STRUCT *veops, void *p)
{
    veops->resetForce(p);
}

static inline int CdcVeWaitInterrupt(VEOPS_STRUCT *veops, void *p)
{
    return veops->waitInterrupt(p);
}

#if CEDARC_DEBUG
static inline void CdcVeWriteValue(VEOPS_STRUCT *veops, void *p, unsigned int value)
{
    return veops->WriteValue(p,value);
}
static inline unsigned int CdcVeReadValue(VEOPS_STRUCT *veops, void *p,unsigned int value)
{
    return veops->ReadValue(p,value);
}
static inline void CdcClearnValue(VEOPS_STRUCT *veops, void *p)
{
    return veops->CleanValue(p);
}
#endif

static inline int CdcVeGetChipId(VEOPS_STRUCT *veops, void *p)
{
    return veops->getChipId(p);
}

static inline uint64_t CdcVeGetIcVeVersion(VEOPS_STRUCT *veops, void *p)
{
    return veops->getIcVeVersion(p);
}

static inline void* CdcVeGetGroupRegAddr(VEOPS_STRUCT *veops, void *p, int nGroupId)
{
    return veops->getGroupRegAddr(p, nGroupId);
}

static inline int CdcVeGetDramType(VEOPS_STRUCT *veops, void *p)
{
    return veops->getDramType(p);
}

static inline unsigned int  CdcVeGetPhyOffset(VEOPS_STRUCT *veops, void *p)
{
    return veops->getPhyOffset(p);
}

static inline void CdcVeSetDramType(VEOPS_STRUCT *veops, void *p)
{
    veops->setDramType(p);
}

static inline void CdcVeSetDdrMode(VEOPS_STRUCT *veops, void *p, int ddr_mode)
{
    veops->setDdrMode(p, ddr_mode);
}

static inline int CdcVeSetSpeed(VEOPS_STRUCT *veops, void *p, int nSpeedMHz)
{
    return veops->setSpeed(p, nSpeedMHz);
}

static inline void CdcVeSetEnableAfbcFlag(VEOPS_STRUCT *veops, void *p, int bEnableFlag)
{
    veops->setEnableAfbcFlag(p, bEnableFlag);
}

static inline void CdcVeSetAdjustDramSpeedFlag(VEOPS_STRUCT *veops, void *p, int bEnableFlag)
{
    veops->setAdjustDramSpeedFlag(p, bEnableFlag);
}


static inline void CdcVeEnableVe(VEOPS_STRUCT *veops, void *p)
{
    veops->enableVe(p);
}

static inline void CdcVeDisableVe(VEOPS_STRUCT *veops, void *p)
{
    veops->disableVe(p);
}

static inline void CdcVeInitEncoderPerformance(VEOPS_STRUCT *veops, void *p, int nMode)
{
    veops->initEncoderPerformance(p, nMode);
}

static inline void CdcVeUnInitEncoderPerformance(VEOPS_STRUCT *veops, void *p, int nMode)
{
    veops->unInitEncoderPerformance(p, nMode);
}

static inline int CdcVeGetIommuAddr(VEOPS_STRUCT *veops, void *p, struct user_iommu_param *iommu_buffer)
{
    return veops->getIommuAddr(p, iommu_buffer);
}

static inline int CdcVeFreeIommuAddr(VEOPS_STRUCT *veops, void *p, struct user_iommu_param *iommu_buffer)
{
    return veops->freeIommuAddr(p, iommu_buffer);
}

static inline int CdcVeSetProcInfo(VEOPS_STRUCT *veops, void *p,struct ve_channel_proc_info *ch_proc_info)
{
    return veops->setProcInfo(p, ch_proc_info);
}

static inline int CdcVeStopProcInfo(VEOPS_STRUCT *veops, void *p, unsigned char cChannelNum)
{
    return veops->stopProcInfo(p, cChannelNum);
}

static inline int CdcGetCsiOnlineInfo(VEOPS_STRUCT *veops, void *p, CsiOnlineRelatedInfo *pCsiOnlineInfo)
{
    return veops->getCsiOnlineInfo(p, pCsiOnlineInfo);
}

static inline int CdcSetLbcParameter(VEOPS_STRUCT *veops, void *p, unsigned int nLbcMode, unsigned int nWidht)
{
    return veops->setLbcParameter(p, nLbcMode, nWidht);
}

static inline unsigned int CdcVeAllocPageBuf(VEOPS_STRUCT *veops, void *p, page_buf_info *page_buf)
{
    return veops->allocPageBuf(p, page_buf);
}

static inline unsigned int CdcVeRecPageBuf(VEOPS_STRUCT *veops, void *p, page_buf_info *page_buf)
{
    return veops->recPageBuf(p, page_buf);
}

static inline unsigned int CdcVeFreePageBuf(VEOPS_STRUCT *veops, void *p, page_buf_info *page_buf)
{
    return veops->freePageBuf(p, page_buf);
}

static inline void CdcVeSetOnlineChannel(VEOPS_STRUCT *veops, void *p, unsigned int bIsOnlineChannel)
{
    return veops->setOnlineChannel(p, bIsOnlineChannel);
}

static inline int CdcVeMode(VeOpsS *veops, void *p, enum VE_MODE mode)
{
	return veops->setMode(p, mode);
}

#ifdef __cplusplus
}
#endif

#endif

