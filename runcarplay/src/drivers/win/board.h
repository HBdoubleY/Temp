#ifndef  _BOARD_H_
#define  _BOARD_H_

#include "apptypedef.h"
#include "rtc.h"


#define 	ISDMAUSER
#define T707_PLATFORM
//#define BOARD_T707_CORE


#define GPRS_MODULE_NONE     0
#define GPRS_MODULE_ACKAT    1
#define GPRS_MODULE_NOTACKAT 2
#define GPRS_MODULE_RESERVE  3

#define MODULE_TYPE_SOCKET      0x00
#define MODULE_TYPE_GPRS		0x01
#define MODULE_TYPE_WCDMA		0x02
#define MODULE_TYPE_TDDCDMA		0x04
#define MODULE_TYPE_CDMA2000	0x08
#define MODULE_TYPE_CDMAEVDO	0x10
#define MODULE_TYPE_LTE			0x20
#define MODULE_TYPE_PSTH		0x40



void ClearWDG();
void InitGPIO();

bool InitDrivers(WORD wHwType);

#endif
