#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include "Rtc.h"
#include <sys/time.h>
#include <linux/rtc.h>
#include <time.h>
#include "sysapi.h"
#include "sysarch.h"

bool RtcGetTime(TTime& time) {
	int rtc_handle;
	int ret = 0;
	struct rtc_time rtc_tm;

	rtc_handle = open("/dev/rtc0",O_RDONLY);
	if (rtc_handle < 0) 
	{  
		printf("[get]open /dev/rtc0 fail\n"); 
		return  false;
	}
	memset(&rtc_tm,0,sizeof(rtc_tm));
	ret = ioctl(rtc_handle, RTC_RD_TIME, &rtc_tm); 
	if(ret < 0){
		printf("rtcGetTime fail\n");
		close(rtc_handle);
	  return false;
	}

	close(rtc_handle);

	time.nSecond = rtc_tm.tm_sec;  
	time.nMinute = rtc_tm.tm_min; 
	time.nHour = rtc_tm.tm_hour; 
	time.nDay = rtc_tm.tm_mday;  
	time.nMonth = rtc_tm.tm_mon + 1;  //lil 20201203
	time.nYear = rtc_tm.tm_year + 1900;  
	time.nWeek = rtc_tm.tm_wday; 
	// 	time->nSecond = rtc_tm.tm_sec;  
	// time->nMinute = rtc_tm.tm_min; 
	// time->nHour = rtc_tm.tm_hour; 
	// time->nDay = rtc_tm.tm_mday;  
	// time->nMonth = rtc_tm.tm_mon + 1;  //lil 20201203
	// time->nYear = rtc_tm.tm_year + 1900;  
	// time->nWeek = rtc_tm.tm_wday; 

	return true;
}

bool RtcSetTime(const TTime& time) {
	int rtc_handle = -1;
	int ret = 0;
	struct rtc_time rtc_tm;

	// for (WORD i = 0; i < 3; i++) {
		rtc_handle = open("/dev/rtc0",O_RDWR);
		if (rtc_handle < 0) 
		{  
			printf("open /dev/rtc0 fail\n"); 
			return false;
		}
		
		memset(&rtc_tm,0,sizeof(rtc_tm));
		rtc_tm.tm_sec   = time.nSecond;
		rtc_tm.tm_min   = time.nMinute;
		rtc_tm.tm_hour  = time.nHour;
		rtc_tm.tm_mday  = time.nDay;
		rtc_tm.tm_mon   = time.nMonth - 1;//lil 20201203
		rtc_tm.tm_year  = time.nYear - 1900;
		rtc_tm.tm_wday  = time.nWeek;

		// rtc_tm.tm_sec   = time->nSecond;
		// rtc_tm.tm_min   = time->nMinute;
		// rtc_tm.tm_hour  = time->nHour;
		// rtc_tm.tm_mday  = time->nDay;
		// rtc_tm.tm_mon   = time->nMonth - 1;//lil 20201203
		// rtc_tm.tm_year  = time->nYear - 1900;
		// rtc_tm.tm_wday  = time->nWeek;

//		rtc_tm.tm_yday  = time.tm_yday;
//		rtc_tm.tm_isdst = time.tm_isdst;	
		ret = ioctl(rtc_handle, RTC_SET_TIME, &rtc_tm);  
		if (ret < 0){  
		    printf("rtcSetTime fail\n");      
		} else {
			printf("rtc_set_time ok\n");
		}
		close(rtc_handle);
	
		Sleep(3);

		// TTime tmRead;
		// RtcGetTime(tmRead);
		// DWORD dwWrSec = TimeToSeconds(time);
		// DWORD dwRdSec = TimeToSeconds(tmRead);
		// if (dwRdSec >= dwWrSec && dwRdSec - dwWrSec < 10) {
		// 	return true;
		// }

		// Sleep(10);
	// }

	return false;
}
