#include "stdafx.h"
#include <string.h>
#include <stdio.h>

#include "syscfg.h"
#include "sysarch.h"
#include "sysapi.h"
#include "ComAPI.h"
#include "sysfs.h"
#include "debugcfg.h"

#ifdef SYS_LINUX
#include <sys/statfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ftw.h>
#endif



//描述:
//		@iIntervV 递增或递减的时间间隔,取决于正负
bool AddIntervs(TTime& time, BYTE bIntervU, int iIntervV)
{
    int nYear, nMonth;
	int y;
    DWORD dwSec = TimeToSeconds(time);

    switch (bIntervU)
    {
    case TIME_UNIT_MINUTE:
        dwSec += (DWORD )60 * iIntervV;
        SecondsToTime(dwSec, &time);
        return true;

    case TIME_UNIT_HOUR:
        dwSec += (DWORD )60 * 60 * iIntervV;
        SecondsToTime(dwSec, &time);
        return true;

    case TIME_UNIT_DAY:
        dwSec += (DWORD )24 * 60 * 60 * iIntervV;
        SecondsToTime(dwSec, &time);
        return true;

    case TIME_UNIT_MONTH:
        nYear = iIntervV / 12;
        time.nYear = time.nYear + nYear;

        iIntervV = iIntervV % 12;
        nMonth = iIntervV + time.nMonth;       //先把month当有符号数看待
        if (nMonth > 12)
        {
            time.nYear++;
            nMonth -= 12;
        }
        else if (nMonth < 1)
        {
            time.nYear--;
            nMonth += 12;
        }

        time.nMonth = nMonth;

		//if (time.nDay > g_wDaysOfMonth[nMonth-1])
		//{
		//	time.nDay = g_wDaysOfMonth[nMonth-1];
		//}

		y = time.nYear;
		if (time.nMonth == 2) 
		{
			if ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0)   //闰年
			{
				if (time.nDay > 29) 
					time.nDay = 29;
			}
			else if (time.nDay > 28) 
				time.nDay = 28;
		}
		else if ((time.nMonth == 4) || (time.nMonth == 6) || (time.nMonth == 9) || (time.nMonth == 11))
		{
			if (time.nDay > 30)
				time.nDay = 30;
		}
		
        return true;

    default: return false;
    }

    return false;
}




bool IsAllAByte(const BYTE* p, BYTE b, WORD len) {
	for (WORD i = 0; i < len; i++) {
		if (*p++ != b) {
			return false;
		}
	}

	return true;
}

bool IsAllAVal32(const int* piVal32, int32 iDstVal32, WORD wNum) {
	for (WORD i = 0; i < wNum; i++) {
		if (piVal32[i] != iDstVal32) {

			return false;
		}
	}

	return true;
}

/*
bool IsAllAVal64(const int64* piVal64, int64 iDstVal64, WORD wNum) {
	for (WORD i = 0; i < wNum; i++) {
		if (piVal64[i] != iDstVal64) {

			return false;
		}
	}

	return true;
}
*/



BYTE CheckSum(BYTE* p, WORD wLen) {
	BYTE bSum = 0;

	for (; wLen > 0; wLen--) {
		bSum += *p++;
	}
	return bSum;
}
BYTE CRCCheck(BYTE bytDir, BYTE *abytCommOrder, WORD nStartPos, WORD nCheckLen) {
	static int16 MSBInfo;
	static WORD wCrcData;
	static WORD nIndexI, nIndexJ;

	wCrcData = 0xffff;
	for (nIndexI = nStartPos; nIndexI < (nCheckLen + nStartPos); nIndexI++) {
		wCrcData = wCrcData ^ abytCommOrder[nIndexI];
		for (nIndexJ = 0; nIndexJ < 8; nIndexJ++) {
			MSBInfo = wCrcData & 0x0001;
			wCrcData = wCrcData >> 1;
			if (MSBInfo != 0) {
				wCrcData = wCrcData ^ 0xa001;
			}
		}
	}
	if (bytDir == 0) {
		abytCommOrder[nIndexI] = wCrcData % 0x100;
		abytCommOrder[nIndexI + 1] = wCrcData / 0x100;
		return 0;
	}
	if (abytCommOrder[nIndexI] != (wCrcData % 0x100)) {
		return 1;
	}
	if (abytCommOrder[nIndexI + 1] != (wCrcData / 0x100)) {
		return 1;
	}
	return 0;
}

void GetCurTime(TTime* pTime) {
	GetSysTime(pTime);

	if (IsInvalidTime(*pTime)) {
		Debug(DEBUG_CRITICAL,
				("###error time %02d/%02d/%02d %02d::%02d::%02d.\r\n", pTime->nYear, pTime->nMonth, pTime->nDay, pTime->nHour, pTime->nMinute, pTime->nSecond));
	}
}

DWORD GetCurTime() {
	TTime now;

	GetCurTime(&now);

	return DaysFrom2000(now) * 60 * 60 * 24 + (DWORD) (now.nHour) * 60 * 60 + (DWORD) now.nMinute * 60 + now.nSecond;
}

DWORD GetCurMinute() {
	TTime now;

	GetCurTime(&now);

	return DaysFrom2000(now) * 60 * 24 + (DWORD) (now.nHour) * 60 + now.nMinute;
}

WORD g_wDaysOfMonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
//1   2   3   4   5   6   7   8   9   10  11  12

DWORD DaysFrom2000(const TTime& time) {
	if (IsTimeEmpty(time)) {
		return 0;
	}

	int y, m, d, days;
	y = BASETIME;
	m = 1;
	d = 1;

	days = 0;
	while (y < time.nYear) {
		if ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0)   //???��?��???��??
				{
			days += 366;
		} else {
			days += 365;
		}

		y++;
	}

	while (m < time.nMonth) {
		if (m == 2) {
			if ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0)   //???��?��???��??
					{
				days += 29;
			} else {
				days += 28;
			}

		} else {
			days += g_wDaysOfMonth[m - 1];
		}

		m++;
	}

	days += time.nDay - d;

	return days;
}

DWORD MinutesFrom2000(const TTime& rPast) {
	DWORD dwPast = DaysFrom2000(rPast) * 60 * 24;

	dwPast += (DWORD) (rPast.nHour) * 60 + rPast.nMinute;

	return dwPast;
}

DWORD MonthFrom2000(const TTime& rPast) {
	return (rPast.nYear - BASETIME) * 12 + rPast.nMonth - 1;
}

DWORD MonthsPast(const TTime& rPast, const TTime& rNow) {
	DWORD dwPast = (rPast.nYear - BASETIME) * 12 + rPast.nMonth - 1;
	DWORD dwNow = (rNow.nYear - BASETIME) * 12 + rNow.nMonth - 1;

	if (dwNow <= dwPast) {
		return 0;
	} else {
		return dwNow - dwPast;
	}
}

int DaysPast(const TTime& rPast, const TTime& rNow) {
	int iPast = (int) DaysFrom2000(rPast);
	int iNow = (int) DaysFrom2000(rNow);

	return iNow - iPast;
}

DWORD HoursPast(const TTime& rPast, const TTime& rNow) {
	DWORD dwPast = DaysFrom2000(rPast) * 24;

	dwPast += rPast.nHour;

	DWORD dwNow = DaysFrom2000(rNow) * 24;
	dwNow += rNow.nHour;

	if (dwNow <= dwPast) {
		return 0;
	} else {
		return dwNow - dwPast;
	}
}

DWORD MinutesPast(const TTime& rPast, const TTime& rNow) {
	DWORD dwPast = DaysFrom2000(rPast) * 60 * 24;

	dwPast += (DWORD) (rPast.nHour) * 60 + rPast.nMinute;

	DWORD dwNow = DaysFrom2000(rNow) * 60 * 24;
	dwNow += (DWORD) (rNow.nHour) * 60 + rNow.nMinute;

	if (dwNow <= dwPast) {
		return 0;
	} else {
		return dwNow - dwPast;
	}
}

/*
 int MunitesSub(const TTime& time1, const TTime& time2)
 {
 int t1 = DaysFrom2000(time1) * 60 * 24;
 t1 += (int )(time1.nHour)*60 + time1.nMinute;

 int t2 = DaysFrom2000(time2) * 60 * 24;
 t2 += (int )(time2.nHour)*60 + time2.nMinute;

 return t1 - t2;
 }*/

DWORD SecondsPast(const TTime& rPast, const TTime& rNow) {
	DWORD dwPast = DaysFrom2000(rPast) * 60 * 60 * 24;

	dwPast += (DWORD) (rPast.nHour) * 60 * 60 + (DWORD) rPast.nMinute * 60 + rPast.nSecond;

	DWORD dwNow = DaysFrom2000(rNow) * 60 * 60 * 24;
	dwNow += (DWORD) (rNow.nHour) * 60 * 60 + (DWORD) rNow.nMinute * 60 + rNow.nSecond;

	if (dwNow <= dwPast) {
		return 0;
	} else {
		return dwNow - dwPast;
	}
}

bool IsInvalidTime(const TTime& time) {
	if (time.nYear < 2024 || time.nYear > 2100 || time.nMonth < 1 || time.nMonth > 12 || time.nDay < 1 || time.nDay > 31
			|| time.nHour > 23 || time.nMinute >= 60 || time.nSecond >= 60) {
		return true;
	} else {
		return false;
	}
}

void SecondsToTime(DWORD dwSeconds, TTime* pTime) {
	int year = BASETIME, month = 1, day = 1, hour = 0, minute = 0, second = 0;
	DWORD delta;

	while (dwSeconds > 0) {
		if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)   //???��?��???��??
				{
			delta = 366 * 24 * 60 * 60;
		} else {
			delta = 365 * 24 * 60 * 60;
		}

		if (dwSeconds < delta) {
			break;
		} else {
			dwSeconds -= delta;
			year++;
		}
	}

	while (dwSeconds > 0) {
		if (month == 2) {
			if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)   //???��?��???��??
					{
				delta = 29 * 24 * 60 * 60;
			} else {
				delta = 28 * 24 * 60 * 60;
			}
		} else {
			delta = g_wDaysOfMonth[month - 1] * 24 * 60 * 60;
		}

		if (dwSeconds < delta) {
			break;
		} else {
			dwSeconds -= delta;
			month++;
		}
	}

	if (dwSeconds > 0) {
		day = dwSeconds / (24 * 60 * 60);

		dwSeconds -= day * 24 * 60 * 60;

		day++;

		if (dwSeconds > 0) {
			hour = dwSeconds / (60 * 60);

			dwSeconds -= hour * 60 * 60;

			if (dwSeconds > 0) {
				minute = dwSeconds / 60;
				second = dwSeconds - minute * 60;
			}
		}
	}

	pTime->nYear = year;
	pTime->nMonth = month;
	pTime->nDay = day;
	pTime->nHour = hour;
	pTime->nMinute = minute;
	pTime->nSecond = second;
//	pTime->nWeek = dayOfWeek(year, month, day) + 1;
	pTime->nWeek = DayOfWeek(*pTime);

}

void MinutesToTime(DWORD dwMins, TTime* pTime) {
	SecondsToTime(dwMins * 60, pTime);
}

void DaysToTime(DWORD dwDays, TTime* pTime) {
	SecondsToTime(dwDays * 24 * 3600, pTime);
}

void MonthsToTime(DWORD dwMonths, TTime* pTime) {
	pTime->nYear = (WORD) ((dwMonths / 12) + BASETIME);
	pTime->nMonth = (BYTE) ((dwMonths % 12) + 1);
	pTime->nDay = 1;
	pTime->nHour = 0;
	pTime->nMinute = 0;
	pTime->nSecond = 0;
}

/*
 int dayOfWeek(int year,int month,int day)
 {
 int _month[12]={31,0,31,30,31,30,31,31,30,31,30,31};
 if (year%4==0 && year%100!=0 || year%400==0)
 _month[1]=29;
 else _month[1]=28;
 int C=0;
 for (int i=0;i<month-1;++i)
 C+=_month[i];
 C+=day;
 int S=year-1+(year-1)/4-(year-1)/100+(year-1)/400+C;
 return S%7;
 }
 */

DWORD TimeToSeconds(const TTime& time) {
	return DaysFrom2000(time) * 60 * 60 * 24 + (DWORD) (time.nHour) * 60 * 60 + (DWORD) time.nMinute * 60 + time.nSecond;
}

DWORD TimeToMinutes(const TTime& time) {
	return DaysFrom2000(time) * 60 * 24 + (DWORD) (time.nHour) * 60 + time.nMinute;
}

bool IsTimeEmpty(const TTime& time) {
	if (time.nYear == 0 && time.nMonth == 0 && time.nDay == 0 && time.nHour == 0 && time.nMinute == 0
			&& time.nSecond == 0) {
		return true;
	} else {
		return false;
	}
}

bool IsDiffDay(const TTime& time1, const TTime& time2) {
	if (time1.nDay != time2.nDay || time1.nMonth != time2.nMonth || time1.nYear != time2.nYear) {
		return true;
	} else {
		return false;
	}
}

bool IsSameDay(const TTime& time1, const TTime& time2) {
	if (time1.nDay == time2.nDay && time1.nMonth == time2.nMonth && time1.nYear == time2.nYear) {
		return true;
	} else {
		return false;
	}
}

bool IsDiffHour(const TTime& time1, const TTime& time2) {
	if (time1.nHour != time2.nHour || time1.nDay != time2.nDay || time1.nMonth != time2.nMonth
			|| time1.nYear != time2.nYear) {
		return true;
	} else {
		return false;
	}
}


BYTE DayOfWeek(const TTime& time) {
	DWORD dwDays = DaysFrom2000(time);
	WORD nWeek = (WORD) (dwDays % 7);

	nWeek = (nWeek + BASEWEEK) % 7;
	return nWeek + 1;
}



BYTE DaysOfMonth(TTime time) {
	TTime tm = time;
	DWORD dwDays1, dwDays2;

	tm.nDay = 1;
	dwDays1 = DaysFrom2000(tm);

	if (tm.nMonth == 12) {
		tm.nYear++;
		tm.nMonth = 1;
	} else {
		tm.nMonth++;
	}
	dwDays2 = DaysFrom2000(tm);

	return (BYTE) (dwDays2 - dwDays1);
}



bool AutoLoopWriteFile(char* pszPathName, BYTE* pbData, DWORD dwLen, DWORD dwTotalLen) {
	bool fRet = true;
	int fp = open(pszPathName, O_CREAT | O_RDWR | O_BINARY, S_IREAD | S_IWRITE);

	if (fp < 0) {
		Debug(DEBUG_CRITICAL, ("WriteFile err!! fail to open %s.\r\n", pszPathName));
		return false;
	} else {
		//int nFileSize = fseek(fp, 0, SEEK_END);
		lseek(fp, 0, SEEK_SET);
		DWORD dwFileCurPos = 0;
		read(fp, (BYTE*) &dwFileCurPos, 4);

		if (dwFileCurPos > dwTotalLen) {
			dwFileCurPos = 4;
		}

		lseek(fp, dwFileCurPos, SEEK_SET);
		int retLen = write(fp, pbData, dwLen);
		if (retLen <= 0 || (DWORD) retLen != dwLen) {
			Debug(DEBUG_CRITICAL, ("WriteFile err!  fail to write %s .\r\n", pszPathName));
			fRet = false;
		}

		lseek(fp, 0, SEEK_SET);
		dwFileCurPos += dwLen;
		write(fp, (BYTE*) &dwFileCurPos, 4);
	}
	close(fp);

	return fRet;
}

bool WriteFile(char* pszPathName, BYTE* pbData, DWORD dwLen) {
	bool fRet = true;
	int f = open(pszPathName, O_CREAT | O_RDWR | O_BINARY, S_IREAD | S_IWRITE);

	if (f < 0) {
		Debug(DEBUG_CRITICAL, ("WriteFile err!! fail to open %s.\r\n", pszPathName));
		return false;
	} else {
		int nFileSize = lseek(f, 0, SEEK_END);
		if (nFileSize > (int) dwLen) {
			close(f);
			unlink(pszPathName);   //?????��??

			if (f < 0) {
				return false;
			}
		} else {
			lseek(f, 0, SEEK_SET);
		}

		if (write(f, pbData, dwLen) != (int) dwLen) {
			Debug(DEBUG_CRITICAL, ("WriteFile err!  fail to write %s .\r\n", pszPathName));
			fRet = false;
		}
	}
	close(f);

	return fRet;
}

bool PartWriteFile(char* pszPathName, DWORD dwOff, BYTE* pbData, DWORD wLen) {
	bool fRet = true;

	int f = open(pszPathName, O_CREAT | O_RDWR | O_BINARY, S_IREAD | S_IWRITE);

	if (f < 0) {
		Debug(DEBUG_CRITICAL, ("SaveFile : error : fail to open %s.\r\n", pszPathName));
		return false;
	} else {
		lseek(f, dwOff, SEEK_SET);
		if (write(f, pbData, wLen) != (int) wLen) {
			Debug(DEBUG_CRITICAL, ("SaveFile : error:  fail to write %s .\r\n", pszPathName));
			fRet = false;
		}
	}
	close(f);

#ifdef SYS_WIN32
	FILE *p;
	char name[200];
	GetWin32FileName(pszPathName, name);
	if ( (p = fopen(name, "w+")) != NULL )
	{
		fseek(p, dwOff, SEEK_SET);
		fwrite(pbData, 1, wLen, p);
		fclose(p);
	}
#endif
	return fRet;
}

bool PartReadFile(char* pszPathName, DWORD dwOffset, BYTE *pbData, DWORD dwLen) {
	bool fRet = true;

	int f = open(pszPathName, O_RDWR | O_BINARY, S_IREAD | S_IWRITE);

	if (f < 0) {
		Debug(DEBUG_CRITICAL, ("PartReadFile : error : fail to open %s.\r\n", pszPathName));
		return false;
	} else {
		lseek(f, dwOffset, SEEK_SET);
		if (read(f, pbData, dwLen) != (int) dwLen) {
			Debug(DEBUG_CRITICAL, ("PartRaedFile : error:  fail to read %s .\r\n", pszPathName));
			fRet = false;
		}
	}
	close(f);

	return fRet;
}

bool DeleteFile(char* pszPathName) {
	int f = open(pszPathName, O_RDWR, S_IREAD | S_IWRITE);

	if (f < 0) {
		return true;
	}

	close(f);
	if (unlink(pszPathName) < 0) {
		return false;
	} else {
#ifdef SYS_WIN32
		FILE *p;
		char name[200];
		GetWin32FileName(pszPathName, name);
		//delete(name);
		if ( (p = fopen(name, "w")) != NULL )
		{
			fclose(p);
		}
#endif
		return true;
	}
}

bool ReadFile(char* pszPathName, BYTE* pbData, DWORD dwBytesToRead) {
	bool fRet = false;
	int nFileSize;
	int f = open(pszPathName, O_RDWR | O_BINARY, S_IREAD | S_IWRITE);  //,|O_BINARY S_IREAD|S_IWRITE

	if (f >= 0) {
		nFileSize = lseek(f, 0, SEEK_END);
		if (nFileSize > 0 && (DWORD) nFileSize == dwBytesToRead) {
			lseek(f, 0, SEEK_SET);
			int rdLen = read(f, pbData, dwBytesToRead);
			if (rdLen > 0 && (DWORD) rdLen == dwBytesToRead) {
				fRet = true;
			} else {
				Debug(DEBUG_CRITICAL, ("ReadFile : error : fail to read %s.\r\n", pszPathName));
			}
			close(f);
		} else {
			close(f);
			unlink(pszPathName);   //?????��??
		}
	}

	return fRet;
}

int readfile(const char* pszPathName, BYTE* pbData, DWORD dwMaxBytesToRead) {
	int iRet = -1;
	int nFileSize;
	int f = open(pszPathName, O_RDWR | O_BINARY, S_IREAD | S_IWRITE);  //,|O_BINARY S_IREAD|S_IWRITE

	if (f >= 0) {
		nFileSize = lseek(f, 0, SEEK_END);
		if (nFileSize <= (int) dwMaxBytesToRead) {
			lseek(f, 0, SEEK_SET);
			if (read(f, pbData, nFileSize) == nFileSize) {
				iRet = nFileSize;
			} else {
				Debug(DEBUG_CRITICAL, ("ReadFile : error : fail to read %s.\r\n", pszPathName));
			}
			close(f);
		} else {
			iRet = -2;
			close(f);
			unlink(pszPathName);   //?????��??
		}
	}

	return iRet;
}

int GetFileLen(char* pszPathName) {
	int iFileSize = -1;
	int f = open(pszPathName, O_RDWR, S_IREAD | S_IWRITE); //, S_IREAD|S_IWRITE

	if (f >= 0) {
		iFileSize = lseek(f, 0, SEEK_END);
		close(f);
	}

	return iFileSize;
}

#ifndef SYS_VDK

/*
 * FCS lookup table as calculated by genfcstab.
 */
static const u_short fcstab[256] = { 0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf, 0x8c48, 0x9dc1,
		0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7, 0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
		0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876, 0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af,
		0x4434, 0x55bd, 0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5, 0x3183, 0x200a, 0x1291, 0x0318,
		0x77a7, 0x662e, 0x54b5, 0x453c, 0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974, 0x4204, 0x538d,
		0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb, 0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
		0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a, 0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960,
		0xbbfb, 0xaa72, 0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9, 0xef4e, 0xfec7, 0xcc5c, 0xddd5,
		0xa96a, 0xb8e3, 0x8a78, 0x9bf1, 0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738, 0xffcf, 0xee46,
		0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70, 0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
		0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff, 0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324,
		0xf1bf, 0xe036, 0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e, 0xa50a, 0xb483, 0x8618, 0x9791,
		0xe32e, 0xf2a7, 0xc03c, 0xd1b5, 0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd, 0xb58b, 0xa402,
		0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134, 0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
		0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3, 0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9,
		0x2f72, 0x3efb, 0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232, 0x5ac5, 0x4b4c, 0x79d7, 0x685e,
		0x1ce1, 0x0d68, 0x3ff3, 0x2e7a, 0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1, 0x6b46, 0x7acf,
		0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9, 0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
		0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78 };

WORD pppfcs16(WORD fcs, unsigned char * cp, int len) {
	while (len--) {
		fcs = (fcs >> 8) ^ fcstab[(fcs ^ *cp++) & 0xff];
	}
	return fcs;
}

#endif //SYS_VDK

static unsigned short crc_16_table[16] = { 0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401, 0xA001,
		0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400 };

WORD GetCrc16(WORD start, BYTE *p, int n) {
	WORD crc = start;
	register WORD r;

	/* while there is more data to process */
	while (n-- > 0) {
		/* compute checksum of lower four bits of *p */
		r = crc_16_table[crc & 0xF];
		crc = (crc >> 4) & 0x0FFF;
		crc = crc ^ r ^ crc_16_table[*p & 0xF];

		/* now compute checksum of upper four bits of *p */
		r = crc_16_table[crc & 0xF];
		crc = (crc >> 4) & 0x0FFF;
		crc = crc ^ r ^ crc_16_table[(*p >> 4) & 0xF];

		/* next... */
		p++;
	}
	return crc;
}

#define CRC_CODE ((unsigned long)0x04C11DB7)
DWORD sub_cal_crc32(unsigned char * data, unsigned int data_len) {
	unsigned long crc = 0x00000000;
	unsigned int i, j;
	unsigned char dat;

	for (i = 0; i < data_len; i++) {
		dat = data[i];
		crc ^= ((unsigned long) dat) << 24;
		for (j = 0; j < 8; j++) {
			if (crc & 0x80000000) {
				crc <<= 1;
				crc ^= CRC_CODE;
			} else {
				crc <<= 1;
			}
		}
	}
	return crc;
}

char* TimeToStr(const TTime& time, char* psz) {
	sprintf(psz, "%04d-%02d-%02d %02d:%02d:%02d", time.nYear, time.nMonth, time.nDay, time.nHour, time.nMinute,
			time.nSecond);

	return psz;
}

char* TimeToStr(DWORD dwTime, char* psz) {
	TTime tmTime;

	SecondsToTime(dwTime, &tmTime);

	return TimeToStr(tmTime, psz);
}

char* MtrAddrToStr(const BYTE* pbAddr, char* psz) {
	sprintf(psz, "%02x%02x%02x%02x%02x%02x", pbAddr[5], pbAddr[4], pbAddr[3], pbAddr[2], pbAddr[1], pbAddr[0]);

	return psz;
}

BYTE* GetSubPos(BYTE *src, WORD wSrcLen, BYTE *dst, WORD wDstLen) {
	int i, j;

	for (i = 0; i <= wSrcLen - wDstLen; i++) {
		if (src[i] == dst[0]) {
			j = 1;
			while (src[i + j] == dst[j] && j < wDstLen) {
				j++;
			}
			if (j == wDstLen) {
				return src + i;
			}
		}
	}
	return NULL;
}

WORD CalcuBitNum(const BYTE* pbBuf, WORD wSize) {
	static WORD wBitNumArr[16] = { 0, 1, 1, 2, 1, 2, 2, 3, //0~7
			1, 2, 2, 3, 2, 3, 3, 4 }; //8~15

	WORD wBitNum = 0;
	BYTE b;

	for (WORD i = 0; i < wSize; i++) {
		b = *pbBuf++;
		wBitNum += wBitNumArr[b & 0x0f];
		wBitNum += wBitNumArr[(b >> 4) & 0x0f];
	}

	return wBitNum;
}

void AndTwoBuf(BYTE* p1, const BYTE* p2, WORD wSize) {
	for (WORD i = 0; i < wSize; i++, p1++, p2++) {
		*p1 = *p1 & *p2;
	}
}

void XorTwoBuf(BYTE* p1, const BYTE* p2, WORD wSize) {
	for (WORD i = 0; i < wSize; i++, p1++, p2++) {
		*p1 = *p1 ^ *p2;
	}
}

void OrTwoBuf(BYTE* p1, const BYTE* p2, WORD wSize) {
	for (WORD i = 0; i < wSize; i++, p1++, p2++) {
		*p1 = *p1 | *p2;
	}
}

bool IsMountedOK(char *str) {
#ifdef SYS_LINUX
	DIR *dir = opendir(str); //str====/opt/usb

	if (dir == NULL) {
//		Debug(DEBUG_CRITICAL, ("%s no exit\r\n", str));
		return false;
	}
	closedir(dir);

	char str2[64];
	sprintf(str2, "%s/..", str);
	struct stat s1;
	struct stat s2;
	lstat(str, &s1);
	lstat(str2, &s2);
	if (s1.st_dev != s2.st_dev) {
		return true;
	}
	if (s1.st_ino == s2.st_ino) {
		return true;
	}
#endif
	return false;
}


const unsigned char HexData[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
unsigned char asctohex(unsigned char asciidata) {
	unsigned int i;

	for (i = 0; i < 16; i++) {
		if (HexData[i] == asciidata) {
			return i;
		}
	}

	return 0;
}

void ASCIIToHex(unsigned char * pAsciiData, unsigned short len, unsigned char * pHexBuff) {
	unsigned int i;

	if (len == 0) {
		return;
	}
	if (len % 2) {
		len += 1;
	}
	for (i = 0; i < len / 2; i++) {
		pHexBuff[i] = asctohex(pAsciiData[i * 2]);
		pHexBuff[i] = (pHexBuff[i] << 4) & 0xf0;
		pHexBuff[i] += asctohex(pAsciiData[i * 2 + 1]);
	}
}

void HexToASCII(unsigned char * pHexBuff, unsigned short len, unsigned char * pAsciiData) {
	unsigned int i;

	if (len == 0) {
		return;
	}
	for (i = 0; i < len; i++) {
		pAsciiData[i * 2] = HexData[((pHexBuff[i] >> 4) & 0x0f)];
		pAsciiData[i * 2 + 1] = HexData[(pHexBuff[i] & 0x0f)];
		pAsciiData[i * 2 + 2] = 0;
	}
}

unsigned short WordToBcd(unsigned short num) {
	unsigned short bcd;

	num = num % 10000;
	bcd = num / 1000;
	num = num % 1000;
	bcd <<= 4;
	bcd += num / 100;
	num = num % 100;
	bcd <<= 4;
	bcd += num / 10;
	bcd <<= 4;
	bcd += num % 10;
	return bcd;
}

void Anticpy(BYTE *bDstAddr, const BYTE *bSrcAddr, WORD wLen) {
	for (int i = 0; i < wLen; i++) {
		bDstAddr[i] = bSrcAddr[wLen - i - 1];
	}
}

void SearchStrcNum(char** ppStart, char* pEnd, char * cNumber) //10???��?��???��??
		{
	bool fGetFirst = false;
	char *pStart = *ppStart;
	BYTE i = 0;
	while (pStart < pEnd) {
		char c = *pStart++;
		if (!fGetFirst) {
			if (c >= '0' && c <= '9') {
				fGetFirst = true;
			}
		}

		if (fGetFirst) {
			if (c >= '0' && c <= '9' && i < 20) {
				i++;
				*cNumber++ = c;
				Debug(DEBUG_COMMON, ("Gprs: get number = %c!!!!!!!!.\r\n", *(cNumber-1)));
			} else {
				break;
			}
		}

	}
	*ppStart = pStart;
}

WORD SearchStrValHex(char** ppStart, char* pEnd) //16???��?��???��??
		{
	bool fGetFirst = false;
	WORD val = 0;
	char *pStart = *ppStart;
	while (pStart < pEnd) {
		char c = *pStart++;
		if (!fGetFirst) {
			if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
				fGetFirst = true;
			}
		}

		if (fGetFirst) {
			if (c >= '0' && c <= '9') {
				val = val * 16 + c - '0';
			} else if (c >= 'A' && c <= 'F') {
				val = val * 16 + c - 'A' + 10;
			} else if (c >= 'a' && c <= 'f') {
				val = val * 16 + c - 'a' + 10;
			} else {
				break;
			}
		}

	}

	*ppStart = pStart;

	return val;
}


int StatDirList(char* dirList, int iBufSize) {
	int iCnt = 0;
#ifdef SYS_LINUX
	DIR *dir;
	struct dirent *de;
	struct stat s;
	const char *dname = "/opt/usb";
	char fdir[] = ".";
	char gdir[] = "..";
	char rdir[] = "recycler";

	char str[100];
	dir = opendir(dname);     // "/opt/usb",
	if (dir != NULL) {
		while ((de = readdir(dir)) != NULL) {
			sprintf(str, "%s/%s", dname, de->d_name);
			lstat(str, &s);
			if ((s.st_mode & S_IFMT) == S_IFDIR) {

				if ((memcmp(de->d_name, fdir, sizeof(fdir)) != 0) && (memcmp(de->d_name, gdir, sizeof(gdir)) != 0)
						&& (memcmp(de->d_name, rdir, sizeof(rdir)) != 0)) {
					strcpy(dirList + iCnt * 64, de->d_name);
					iCnt++;
					if (iCnt >= iBufSize) {
						closedir(dir);
						return iCnt;
					}
					continue;
				}
			}
		}
		closedir(dir);
	}
#endif
	return iCnt;
}

int checkDirExist(char *ProcScsiUsbDir) {
#ifdef SYS_LINUX
	DIR *dir = NULL;

	if ((dir = opendir(ProcScsiUsbDir)) == NULL) {
		return -1;
	}
	closedir(dir);

	char pathname[128];

	sprintf(pathname, "%s/0", ProcScsiUsbDir);
	if (access(pathname, F_OK) == 0) {
		return 0;
	}

	sprintf(pathname, "%s/1", ProcScsiUsbDir);
	if (access(pathname, F_OK) == 0) {
		return 0;
	}

	sprintf(pathname, "%s/2", ProcScsiUsbDir);
	if (access(pathname, F_OK) == 0) {
		return 0;
	}

	sprintf(pathname, "%s/3", ProcScsiUsbDir);
	if (access(pathname, F_OK) == 0) {
		return 0;
	}

	sprintf(pathname, "%s/4", ProcScsiUsbDir);
	if (access(pathname, F_OK) == 0) {
		return 0;
	}

	sprintf(pathname, "%s/5", ProcScsiUsbDir);
	if (access(pathname, F_OK) == 0) {
		return 0;
	}
#endif
	return -1;
}

#ifdef SYS_LINUX
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/usbdevice_fs.h>
#include <dirent.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
int access(const char *pathname, int mode);
#endif

static int is_dir_exist(const char*dir_path) {
#ifdef SYS_LINUX
	if (dir_path == NULL) {
		return -1;
	}

	DIR *dp = opendir(dir_path);
	if (dp == NULL) {
		return -1;
	}
	closedir(dp);
#endif
	return 0;
}

int getUsbCount() {
	int count = 0;

#ifdef SYS_LINUX
	DIR *dp = opendir("/proc/scsi/usb-storage");

	struct dirent *entry;
	struct stat statbuf;

	if (dp == NULL) {
		return 0;
	}
	while ((entry = readdir(dp)) != NULL) {
		lstat(entry->d_name, &statbuf);

		if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
			continue;

		count++;

	}

	closedir(dp);
#endif

	return count;
}

void restUsbPort(const char *strpath) {
#ifdef SYS_LINUX
	char dir[PATH_MAX] = { 0 };
	char childpath[PATH_MAX] = { 0 };

	DIR *dp;

	struct dirent *entry;
	struct stat statbuf;
	strcpy(dir, strpath);

	if (is_dir_exist(strpath) != 0) {
		int fd = open(strpath, O_WRONLY);
		ioctl(fd, USBDEVFS_RESET, 0);
		close(fd);

		return;
	} else if ((dp = opendir(dir)) == NULL) {
		return;
	}

	while ((entry = readdir(dp)) != NULL) {
		lstat(entry->d_name, &statbuf);

		if (S_IFDIR & statbuf.st_mode) {

			if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
				continue;

			memset(childpath, 0x00, PATH_MAX);

			sprintf(childpath, "%s/%s", strpath, entry->d_name);

			restUsbPort(childpath);

		} else {

		}
	}
	closedir(dp);
#endif
}


int CheckUDiskExist(void) {
#ifdef SYS_LINUX
	char usbDirMount[] = "/opt/usb/usb1";
	if (access("/opt/usb/usb1", F_OK) != 0) {
		system("mkdir /opt/usb/usb1");
	}
	for (int i = 0; i < 2; i++) {
		if (IsMountedOK(usbDirMount)) {   

			Debug(DEBUG_CRITICAL, ("USB mounted OK %d.\n", i));
			return 0;
		}

		char buff[128];
		for (int i = 0; i < 5; i++) {
			char path[128];
			sprintf(path, "/dev/sd%c", 'a' + i);
			if (access(path, F_OK) == 0) {
				sprintf(buff, "mount -t vfat /dev/sd%c /opt/usb/usb1", 'a' + i);
				if (system(buff) == 0) {
					break;
				}
			}

			sprintf(path, "/dev/sd%c1", 'a' + i);
			if (access(path, F_OK) == 0) {
				sprintf(buff, "mount -t vfat /dev/sd%c1 /opt/usb/usb1", 'a' + i);
				if (system(buff) == 0) {
					break;
				}
			}
		}

		Sleep(50);
		continue;
	}

	if(getUsbCount() > 0){
		restUsbPort("/dev/bus/usb");
		return -2;
	}

	Debug(DEBUG_CRITICAL, ("No usb disk.\r\n"));
#endif
	return -1;
}


int CheckUDiskExist0(void) {
#ifdef SYS_LINUX
	char usbDirMount[] = "/opt/usb/usb0";
//	if (access("/opt/usb", F_OK) != 0) {
	system("mkdir /opt/usb/usb0");
//	}
	for (int i = 0; i < 2; i++) {

		char buff[128];
		for (int i = 0; i < 1; i++) {
			char path[128];
			sprintf(path, "/dev/sd%c", 'a' + i);
			if (access(path, F_OK) == 0) {
				sprintf(buff, "mount -t vfat /dev/sd%c /opt/usb/usb0", 'a' + i);
				if (system(buff) == 0) {
					Debug(DEBUG_CRITICAL, ("USB mounted OK %d.\n", i));
					return 0;
				}
			}

			sprintf(path, "/dev/sd%c1", 'a' + i);
			if (access(path, F_OK) == 0) {
				sprintf(buff, "mount -t vfat /dev/sd%c1 /opt/usb/usb0", 'a' + i);
				if (system(buff) == 0) {
					Debug(DEBUG_CRITICAL, ("USB mounted OK %d.\n", i));
					return 0;
				}
			}
		}

//		Sleep(500);
		continue;
	}

	if (getUsbCount() > 0) {

		return -2;
	}

	Debug(DEBUG_CRITICAL, ("No usb disk.\r\n"));
#endif
	return -1;
}

int findFile(char *pPath, char *pName) {
	char str[128];
	sprintf(str, "find %s -name %s", pPath, pName);
	if (system(str) > 0) {
		return 0;
	} else {
		return -1;
	}
}


int GetFsSize(char *path, unsigned long long *freeSize, unsigned long long *totalSize) {
#ifdef SYS_LINUX
	struct statfs64 diskInfo;

	*freeSize = *totalSize = 0;
	if (statfs64(path, &diskInfo) == -1) {
		return -1;
	}

	unsigned long long blocksize = diskInfo.f_bsize;
	unsigned long long totalsize = blocksize * diskInfo.f_blocks;
	printf("Total_size = %llu B = %llu KB = %llu MB = %llu GB\n", totalsize, totalsize >> 10, totalsize >> 20,
			totalsize >> 30);

	unsigned long long freeDisk = diskInfo.f_bfree * blocksize;
	unsigned long long availableDisk = diskInfo.f_bavail * blocksize;
	printf("Disk_free = %llu MB = %llu GB\nDisk_available = %llu MB = %llu GB\n", freeDisk >> 20, freeDisk >> 30,
			availableDisk >> 20, availableDisk >> 30);


	*freeSize = freeDisk;
	*totalSize = totalsize;
#endif
	return 0;
}

unsigned long long total = 0;

int FileSum(const char *fpath, const struct stat *sb, int typeflag) {
	total += (unsigned long long) sb->st_size;
	return 0;
}

int GetFolderSize(char *path, unsigned long long *totalSize) {
#ifdef SYS_LINUX
	if (!path || !totalSize || access(path, R_OK)) {
		return -1;
	}
	*totalSize = 0;

	if (ftw(path, &FileSum, 1)) {
		perror("ftw");
		return -1;
	}

	*totalSize = total;
	printf("%s,  size=%llu\n", path, *totalSize);
#endif
	return 0;
}

int BinarySearch(int* arr, int len, int goal) {
	int low = 0;
	int high = len - 1;
	while (low <= high) {
		int middle = (high - low) / 2 + low;
		if (arr[middle] == goal)
			return middle;

		else if (arr[middle] > goal)
			high = middle - 1;
		else
			low = middle + 1;
	}

	return -1;
}

#define POLYNOMIAL 0x2F
#define CRC8_INIT  0xAA

unsigned char CalcCrc8(unsigned char *BlockStart, unsigned char BlockLength) {
	unsigned char i, j;
	unsigned char CRC_VAL = CRC8_INIT;

	for (j = 0; j < BlockLength; j++) {
		CRC_VAL = *(BlockStart + j) ^ CRC_VAL;
		for (i = 0; i < 8; i++) {
			if (CRC_VAL & 0x80) {
				CRC_VAL <<= 1;
				CRC_VAL ^= POLYNOMIAL;
			} else {
				CRC_VAL <<= 1;
			}
		}
	}

	return CRC_VAL;
}

