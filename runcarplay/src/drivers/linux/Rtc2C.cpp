#include "Rtc.h"
#include <stdio.h>
#include "pthread.h"
extern "C" {
static pthread_rwlock_t rtc_rwlock = PTHREAD_RWLOCK_INITIALIZER;

    bool RtcGetTime2C(TTime* time) {
        if (time == nullptr) return false;
        pthread_rwlock_rdlock(&rtc_rwlock);
        bool ret = RtcGetTime(*time);
        pthread_rwlock_unlock(&rtc_rwlock);
        return ret;
    }
    

    bool RtcSetTime2C(const TTime* time) {
        if (time == nullptr) return false;
        if(time->nYear > 3000 || time->nYear <= 1900) return false;
        if(time->nMonth > 12 || time->nMonth <= 0) return false;
        if(time->nDay > 31 || time->nDay <= 0) return false;
        if(time->nHour > 24 || time->nHour < 0) return false;
        if(time->nMinute > 60 || time->nMinute < 0) return false;
        if(time->nSecond > 60 || time->nSecond < 0) return false;
        pthread_rwlock_wrlock(&rtc_rwlock);
        bool ret = RtcSetTime(*time);
        pthread_rwlock_unlock(&rtc_rwlock);
        
        return ret;
    }
}
