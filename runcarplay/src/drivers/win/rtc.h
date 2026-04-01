#ifndef _RTC_H
#define _RTC_H

#include "apptypedef.h"
#include "ComStruct.h"

bool RtcGetTime(TTime& time);
bool RtcSetTime(const TTime& time);

#endif //DRIVER_H
