#ifndef _RTC_H
#define _RTC_H

#include <apptypedef.h>
#include <ComStruct.h>
#ifdef __cplusplus
extern "C" {
#endif


bool RtcGetTime(TTime& time);
bool RtcSetTime(const TTime& time);
#ifdef __cplusplus
}
#endif


#endif //DRIVER_H
