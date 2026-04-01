#ifndef _RTC_H
#define _RTC_H

#include <apptypedef.h>
#include <ComStruct.h>
#ifdef __cplusplus
extern "C" {
#endif

bool RtcGetTime2C(TTime* time);
bool RtcSetTime2C(TTime* time);

#ifdef __cplusplus
}
#endif


#endif //DRIVER_H