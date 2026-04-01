#ifndef _MAINTREAD_H_
#define _MAINTREAD_H_
#include "sysfs.h"
#include "sysarch.h"
#include "sysapi.h"
#include "board.h"
#include "sysdebug.h"
#include "Monitor.h"

extern TThreadRet MainThread(void* pvPara);
//extern unsigned char HDMI_WriteI2C_Byte( unsigned char reg_addr, unsigned char reg_value);


#endif
