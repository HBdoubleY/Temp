//#define LOG_NDEBUG 0
#define LOG_TAG "SampleG2dMem"

#include <string.h>
//#include <memoryAdapter.h>
//#include "sc_interface.h"
//#include "plat_log.h"
//#include <ion_memmanager.h>
#include "ion_mem_alloc.h"

struct SunxiMemOpsS *gpMemOps = NULL;
int g2d_MemOpen(void)
{
    gpMemOps = GetMemAdapterOpsS();
    return SunxiMemOpen(gpMemOps);
}

int g2d_MemClose(void)
{
    SunxiMemClose(gpMemOps);
    return 0;
}

unsigned char* g2d_allocMem(unsigned int size)
{
    IonAllocAttr allocAttr;
    memset(&allocAttr, 0, sizeof(IonAllocAttr));
    allocAttr.nLen = size;
    //allocAttr.mAlign = 0;
    allocAttr.eIonHeapType = IonHeapType_IOMMU;
    allocAttr.bSupportCache = true;
    return (unsigned char*)SunxiMemPallocExtend(gpMemOps, &allocAttr);
}

int g2d_freeMem(void *vir_ptr)
{
    SunxiMemPfree(gpMemOps, vir_ptr);
    return 0;
}

unsigned int g2d_getPhyAddrByVirAddr(void *vir_ptr)
{
    return (unsigned int)SunxiMemGetPhysicAddressCpu(gpMemOps, vir_ptr);
}

int g2d_flushCache(void *vir_ptr, unsigned int size)
{
    SunxiMemFlushCache(gpMemOps, vir_ptr, size);
    return 0;
}
