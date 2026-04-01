#ifndef COMAPI_H
#define COMAPI_H

#include <string.h>

#include "ComConst.h"
#include "ComStruct.h"
#include "apptypedef.h"
//#include <opencv2/opencv.hpp>

#define BASETIME          1999
#define BASEWEEK          5
bool AddIntervs(TTime& time, BYTE bIntervU, int iIntervV);
bool IsAllAByte(const BYTE* p, BYTE b, WORD len);

bool IsAllAVal32(const int* piVal32, int iDstVal32, WORD wNum);
//bool IsAllAVal64(const int64* piVal64, int64 iDstVal64, WORD wNum);

BYTE CheckSum(BYTE* p, WORD wLen);
BYTE CRCCheck(BYTE bytDir, BYTE *abytCommOrder, WORD nStartPos, WORD nCheckLen);

void GetCurTime(TTime* pTime);
DWORD GetCurTime();
DWORD GetCurMinute();

DWORD DaysFrom2000(const TTime& time);
DWORD MinutesFrom2000(const TTime& rPast);
DWORD MonthFrom2000(const TTime& rPast);
DWORD MonthsPast(const TTime& rPast, const TTime& rNow);
int DaysPast(const TTime& rPast, const TTime& rNow);
DWORD HoursPast(const TTime& rPast, const TTime& rNow);
DWORD MinutesPast(const TTime& rPast, const TTime& rNow);
DWORD SecondsPast(const TTime& rPast, const TTime& rNow);
bool IsInvalidTime(const TTime& time);
DWORD TimeToSeconds(const TTime& time);
DWORD TimeToMinutes(const TTime& time);
int dayOfWeek(int year, int month, int day);
void SecondsToTime(DWORD dwSeconds, TTime* pTime);
void MinutesToTime(DWORD dwMins, TTime* pTime);
void DaysToTime(DWORD dwDays, TTime* pTime);
void MonthsToTime(DWORD dwMonths, TTime* pTime);
bool IsTimeEmpty(const TTime& time);
bool IsDiffDay(const TTime& time1, const TTime& time2);
bool IsSameDay(const TTime& time1, const TTime& time2);
bool IsDiffHour(const TTime& time1, const TTime& time2);
int MunitesSub(const TTime& time1, const TTime& time2);

BYTE DayOfWeek(const TTime& time);
int DaysPast(int year, int month, int day);
bool AddIntervs(TTime& time, BYTE bIntervU, int iIntervV);
int IntervsPast(const TTime& tmPast, const TTime& tmNow, BYTE bIntervU, BYTE bIntervV);
BYTE DaysOfMonth(TTime time);

bool AutoLoopWriteFile(char* pszPathName, BYTE* pbData, DWORD dwLen, DWORD dwTotalLen);
bool ReadFile(char* pszPathName, BYTE* pbData, DWORD dwBytesToRead);
int readfile(const char* pszPathName, BYTE* pbData, DWORD dwMaxBytesToRead);
bool WriteFile(char* pszPathName, BYTE* pbData, DWORD dwLen);
bool PartWriteFile(char* pszPathName, DWORD dwOff, BYTE* pbData, DWORD wLen);
bool PartReadFile(char* pszPathName, DWORD dwOffset, BYTE *pbData, DWORD dwLen);
bool DeleteFile(char* pszPathName);
int GetFileLen(char* pszPathName);


WORD GetCrc16(WORD start, BYTE *p, int n);

char* TimeToStr(const TTime& time, char* psz);
char* TimeToStr(DWORD dwTime, char* psz);
char* MtrAddrToStr(const BYTE* pbAddr, char* psz);
BYTE* GetSubPos(BYTE *src, WORD wSrcLen, BYTE *dst, WORD wDstLen);
WORD CalcuBitNum(const BYTE* pbBuf, WORD wSize);
void AndTwoBuf(BYTE* p1, const BYTE* p2, WORD wSize);
void XorTwoBuf(BYTE* p1, const BYTE* p2, WORD wSize);
void OrTwoBuf(BYTE* p1, const BYTE* p2, WORD wSize);

void TestCommPort(WORD wPort, DWORD dwBaudRate);

#ifdef SYS_LINUX
bool IsMountedOK(char *str);
bool UsbUpdate(char* szPath);
#endif

int StatDirList(char* dirList, int iBufSize);
bool IsMountedOK(char *str);
int checkDirExist(char *ProcScsiUsbDir);
int CheckUDiskExist(void);
int FindFile(char *pPath, char *pName);
int GetFsSize(char *path, unsigned long long *freeSize, unsigned long long *totalSize);
int GetFolderSize(char *path, unsigned long long *totalSize);

int BinarySearch(int* arr, int len, int goal);
unsigned char CalcCrc8(unsigned char *BlockStart, unsigned char BlockLength);
#endif //COMAPI_H

