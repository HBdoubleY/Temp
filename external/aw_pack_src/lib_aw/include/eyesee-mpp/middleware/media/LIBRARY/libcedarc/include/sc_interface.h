/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : typedef.h
* Description :
* History :
*   Author  : xyliu <xyliu@allwinnertech.com>
*   Date    : 2016/04/13
*   Comment :
*
*
*/


#ifndef SC_INTERFACE_H
#define SC_INTERFACE_H

#include <stddef.h>
#include <stdint.h>
#include "../base/include/UserKernelAdapter.h"
#ifdef __cplusplus
extern "C"
{
#endif

MEMOPS_STRUCT
{
    int (*open)(void);

    void (*close)(void);

    int (*total_size)(void);

    void *(*palloc)(int /*size*/, void *, void *);

    void *(*palloc_no_cache)(int /*size*/, void *, void *);

    void  (*pfree)(void* /*mem*/, void *, void *);

    void (*flush_cache)(void * /*mem*/, int /*size*/);

    void *(*ve_get_phyaddr)(void * /*viraddr*/);

    void *(*ve_get_viraddr)(void * /*phyaddr*/);

    void *(*cpu_get_phyaddr)(void * /*viraddr*/);

    void *(*cpu_get_viraddr)(void * /*phyaddr*/);

    int (*mem_set)(void * /*s*/, int /*c*/, size_t /*n*/);

    int (*mem_cpy)(void * /*dest*/, void * /*src*/, size_t /*n*/);

    int (*mem_read)(void * /*dest */, void * /*src*/, size_t /*n*/);

    int (*mem_write)(void * /*dest*/, void * /*src*/, size_t /*n*/);

    int (*setup)(void);

    int (*shutdown)(void);

    //*the interface just for secureAlloc.c, not used by other function
    void *(*palloc_secure)(int /*size*/, void*, void*);

    unsigned int (*get_ve_addr_offset)(void);
};

static inline int CdcMemOpen(MEMOPS_STRUCT *memops)
{
    return memops->open();
}

//* close the memory adapter.
static inline void CdcMemClose(MEMOPS_STRUCT *memops)
{
    memops->close();
}

static inline int CdcMemTotalSize(MEMOPS_STRUCT *memops)
{
    return memops->total_size();
}

static inline void *CdcMemPalloc(MEMOPS_STRUCT *memops, int nSize, void *veOps, void *pVeopsSelf)
{
    return memops->palloc(nSize, veOps, pVeopsSelf);
}

static inline void CdcMemPfree(MEMOPS_STRUCT *memops, void* pMem, void *veOps, void *pVeopsSelf)
{
    memops->pfree(pMem, veOps, pVeopsSelf);
}

static inline void CdcMemFlushCache(MEMOPS_STRUCT *memops, void* pMem, int nSize)
{
    memops->flush_cache(pMem, nSize);
}

static inline void *CdcMemGetPhysicAddress(MEMOPS_STRUCT *memops, void* pVirtualAddress)
{
    return memops->ve_get_phyaddr(pVirtualAddress);
}

static inline void *CdcMemGetVirtualAddress(MEMOPS_STRUCT *memops, void* pPhysicAddress)
{
    return memops->ve_get_viraddr(pPhysicAddress);
}

static inline void CdcMemSet(MEMOPS_STRUCT *memops, void* pMem, int nValue, int nSize)
{
   memops->mem_set(pMem, nValue, nSize);
}

static inline void CdcMemCopy(MEMOPS_STRUCT *memops, void* pMemDst, void* pMemSrc, int nSize)
{
    memops->mem_cpy(pMemDst, pMemSrc, nSize);
}

static inline int CdcMemRead(MEMOPS_STRUCT *memops, void* pMemDst, void* pMemSrc, int nSize)
{
    memops->mem_read(pMemDst, pMemSrc, nSize);
    return 0;
}

static inline int CdcMemWrite(MEMOPS_STRUCT *memops,void* pMemDst,  void* pMemSrc, int nSize)
{
    (void)memops; /*not use memops */
    memops->mem_write(pMemDst, pMemSrc, nSize);
    return 0;
}

static inline void *CdcMemGetPhysicAddressCpu(MEMOPS_STRUCT *memops, void *virt)
{
    return memops->cpu_get_phyaddr(virt);
}

static inline void *CdcMemGetVirtualAddressCpu(MEMOPS_STRUCT *memops, void *phy)
{
    return memops->cpu_get_viraddr(phy);
}

static inline int CdcMemSetup(MEMOPS_STRUCT *memops)
{
    return memops->setup();
}

static inline int CdcMemShutdown(MEMOPS_STRUCT *memops)
{
    return memops->shutdown();
}

static inline void *CdcMemPallocSecure(MEMOPS_STRUCT *memops, int nSize,
                                              void *veOps, void *pVeopsSelf)
{
    return memops->palloc_secure(nSize,veOps, pVeopsSelf);
}

static inline unsigned int CdcMemPallocGetVeAddrOffset(MEMOPS_STRUCT *memops)
{
    return memops->get_ve_addr_offset();
}

#ifdef __cplusplus
}
#endif

#endif

