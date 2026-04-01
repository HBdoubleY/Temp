#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdarg.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <stddef.h>             /* offsetof */
#include <unistd.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/statfs.h>
#include <termios.h>
#include "fs.h"
#include "apptypedef.h"
#include "sysapi.h"
#include "sysarch.h"
#include "ComAPI.h"
#include "Rtc.h"
#include "board.h"


bool SetLinuxTime(const TTime& t);

static DWORD g_dwStartClick = 0;
static DWORD g_dwBaseClick = 0;
static OS_SEM g_semTime;

// S
DWORD GetClick() {
	return g_dwBaseClick + time(NULL) - g_dwStartClick;
}

// ms
DWORD GetTick() {
	struct timeval tv;
	struct timezone tz;
	unsigned long msec;

	gettimeofday(&tv, &tz);

	msec = tv.tv_usec % 1000000 / 1000;

	return GetClick() * 1000 + msec;
}

uint64 GetTickUs() {
	struct timeval tv;
	struct timezone tz;

	gettimeofday(&tv, &tz);

	return tv.tv_usec;
}

bool RTCInit() {
	TTime time;

	memset(&time, 0, sizeof(time));
	RtcGetTime(time);
	Debug(DEBUG_CRITICAL,
			("rtc:%d-%02d-%02d %02d:%02d:%02d.\r\n", time.nYear, time.nMonth, time.nDay, time.nHour, time.nMinute, time.nSecond));

	if (IsInvalidTime(time)) {
		Debug(DEBUG_CRITICAL, ("## rtc error!!!\r\n"));

		time.nYear = 2026;
		time.nMonth = 2;
		time.nDay = 24;
		time.nHour = 0;
		time.nMinute = 0;
		time.nSecond = 0;
		RtcSetTime(time);
	}

	return SetLinuxTime(time);
}

void SyncTimer() {
	static TTime tmLast = { 0 };
	TTime now;
	TTime tmOld;

	memset(&tmOld, 0, sizeof(tmOld));
	if (IsTimeEmpty(tmLast)) {
		GetCurTime(&tmLast);
	}
	GetCurTime(&now);
	if (HoursPast(tmLast, now) > 0 && now.nMinute == 57 && now.nSecond >= 45) //每锟斤拷小时锟斤拷57锟斤拷30锟斤拷同锟斤拷一锟斤拷时锟斤拷
			{
		int err = 0;

		for (WORD i = 0; i < 3; i++) {
			memset(&now, 0, sizeof(now));
			RtcGetTime(now);
			Debug(DEBUG_CRITICAL,
					("sync rtc: time%d %d-%02d-%02d %02d:%02d:%02d.\r\n", i, now.nYear, now.nMonth, now.nDay, now.nHour, now.nMinute, now.nSecond));

			if (!IsInvalidTime(now)) {
				if (now.nYear == tmOld.nYear && now.nMonth == tmOld.nMonth && now.nDay == tmOld.nDay
						&& now.nHour == tmOld.nHour) {
					if (SetLinuxTime(now)) //同锟斤拷一锟斤拷系统时锟斤拷
							{
						GetCurTime(&tmLast);
						return;
					}
				}
				tmOld = now;
			} else {
				memset(&tmOld, 0, sizeof(tmOld));
				Debug(DEBUG_CRITICAL, ("sync rtc: ############ invalid\r\n"));
				//if (now.nYear>2050 || now.nMonth>12 || now.nMonth>12)
				{
					err++;
				}
			}
			Sleep(1000);
		}

		GetCurTime(&tmLast);
		if (err > 2) {
			for (WORD i = 0; i < 10; i++) {
				RtcSetTime(tmLast);
				Debug(DEBUG_CRITICAL, ("###rtc: time to write back to RTC!!!\r\n"));

				memset(&now, 0, sizeof(now));
				RtcGetTime(now);
				Debug(DEBUG_CRITICAL,
						("rtc: time%d %d-%02d-%02d %02d:%02d:%02d.\r\n", i, now.nYear, now.nMonth, now.nDay, now.nHour, now.nMinute, now.nSecond));

				if (!IsInvalidTime(now)) {
					if (now.nYear == tmLast.nYear && now.nMonth == tmLast.nMonth && now.nDay == tmLast.nDay
							&& now.nHour == tmLast.nHour) {
						return;
					}
				}
				Sleep(150);
			}
		}
	}
}

bool GetSysTime(TTime* pTime) {
	struct tm *pCurTime;
	time_t curtime;

	OSSemPend(g_semTime);

	curtime = time(NULL); //lt1
	pCurTime = localtime(&curtime);
	pTime->nYear = pCurTime->tm_year + 1900;    //tm_year:
	pTime->nMonth = pCurTime->tm_mon + 1;   //tm_mon:  0-11
	pTime->nDay = pCurTime->tm_mday;        //tm_mday: 1-31
	pTime->nHour = pCurTime->tm_hour;       //tm_hour: 0-23
	pTime->nMinute = pCurTime->tm_min;      //tm_min:  0-59
	pTime->nSecond = pCurTime->tm_sec;      //tm_sec:  0-61
	pTime->nWeek = pCurTime->tm_wday + 1;     //tm_wday: 0-6
	if (pTime->nYear < 2000) {
		pTime->nYear = 2000;
	}

	OSSemPost(g_semTime);

	return true;
}

bool SetSysTime(const TTime& t) {
	if (IsInvalidTime(t)) {
		Debug(DEBUG_CRITICAL, ("#####set system time:  invalid\r\n"));
		return false;
	}

	Debug(DEBUG_CRITICAL,
				("set rtc:%d-%02d-%02d %02d:%02d:%02d.\r\n", t.nYear, t.nMonth, t.nDay, t.nHour, t.nMinute, t.nSecond));

	struct tm tmLinux;
	tmLinux.tm_sec = t.nSecond;  //int tm_sec;
	tmLinux.tm_min = t.nMinute;  //int tm_min
	tmLinux.tm_hour = t.nHour; //int tm_hour
	tmLinux.tm_mday = t.nDay;   //int tm_mday
	tmLinux.tm_mon = t.nMonth - 1;  //int tm_mon
	tmLinux.tm_year = t.nYear - 1900; //int tm_year

	OSSemPend(g_semTime);

	g_dwBaseClick += time(NULL) - g_dwStartClick;

	time_t timet = timegm(&tmLinux);
	int iRet = stime(&timet);

	g_dwStartClick = time(NULL);

	OSSemPost(g_semTime);

	RtcSetTime(t);

	return iRet == 0;
}

bool SetLinuxTime(const TTime& t) {
	struct tm tmLinux;

	tmLinux.tm_sec = t.nSecond;  //int tm_sec;
	tmLinux.tm_min = t.nMinute;  //int tm_min
	tmLinux.tm_hour = t.nHour; //int tm_hour
	tmLinux.tm_mday = t.nDay;   //int tm_mday
	tmLinux.tm_mon = t.nMonth - 1;  //int tm_mon
	tmLinux.tm_year = t.nYear - 1900; //int tm_year

	time_t timet = timegm(&tmLinux);
	int iRet = stime(&timet);
	return iRet == 0;
}

void SysInit() {
	g_semTime = OSSemCreate(1);
	RTCInit();
	g_dwBaseClick = 0;
	g_dwStartClick = time(NULL);
}

void SysCleanup() {
}

bool g_fAppExit = false;

bool IsAppExit() {
	return g_fAppExit;
}

bool IsAppExitDone() {
	return true;
}

void ResetCPU() {
	sync();
	system("sync");
	sleep(2);
	Debug(DEBUG_CRITICAL, ("reboot!\n"));
	system("reboot");
}

//extern int gWdtTime;
void ResetApp() {
//	gWdtTime = 3;
	ClearWDG();
	g_fAppExit = true;

	ResetCPU();
}

char *get_name(char *name, char *p) {
	int namestart = 0, nameend = 0;

	while (isspace(p[namestart])) {
		namestart++;
	}
	nameend = namestart;
	while (p[nameend] && p[nameend] != ':' && !isspace(p[nameend])) {
		nameend++;
	}
	if (p[nameend] == ':') {
		if ((nameend - namestart) < 16) {
			memcpy(name, &p[namestart], nameend - namestart);
			name[nameend - namestart] = '\0';
			p = &p[nameend];
		} else {
			name[0] = '\0';
		}
	} else {
		name[0] = '\0';
	}
	return p + 1;
}

int IfReadListProc(char *target) {
	struct ifreq ifr;
	int fd;
	int err;
	unsigned long LocalIP, RemoteIP;

	strcpy(ifr.ifr_name, target);
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	err = ioctl(fd, SIOCGIFADDR, &ifr);
	if (err) {
		//perror("ioctl");
		close(fd);
		return 0;
	}
	memcpy(&LocalIP, ifr.ifr_addr.sa_data + 2, 4);

	err = ioctl(fd, SIOCGIFDSTADDR, &ifr);
	if (err) {
		//perror("ioctl");
		close(fd);
		return 0;
	}
	memcpy(&RemoteIP, ifr.ifr_addr.sa_data + 2, 4);
	close(fd);

	if (LocalIP != 0) {
		if ((RemoteIP & 0xff) == 192) {
			system("ifconfig eth0 193.168.13.8 up");
		}
		system("cp /etc/ppp/resolv.conf /etc/resolv.conf");
		return 1;
	} else {
		return 0;
	}
}

unsigned int freespace(char* disk) {
	/*
	 int i;
	 struct statfs disk_statfs;
	 printf("%d %d %d\r\n", sizeof(disk_statfs), sizeof(disk_statfs.f_blocks),
	 sizeof(disk_statfs.f_ffree), sizeof(disk_statfs.f_fsid));
	 printf("Get: %d\r\n", statfs("/opt/data", &disk_statfs));
	 printf("f_type: %d\r\n", disk_statfs.f_type);
	 printf("f_bsize: %d\r\n", disk_statfs.f_bsize);// 每锟斤拷拇锟叫�(锟街斤拷锟斤拷) bytes
	 printf("f_blocks: %d\r\n", disk_statfs.f_blocks);
	 printf("f_bfree: %d\r\n", disk_statfs.f_bfree);
	 printf("f_bavail: %d\r\n", disk_statfs.f_bavail);// 锟斤拷锟矫的匡拷锟斤拷 allbytes = f_bavail*f_bsize
	 printf("f_files: %d\r\n", disk_statfs.f_files);
	 printf("f_ffree: %d\r\n", disk_statfs.f_ffree);
	 */
	struct statfs disk_statfs;

	statfs(disk, &disk_statfs);
	return disk_statfs.f_bavail * disk_statfs.f_bsize;
}

DWORD GetLocalAddr(char* interface) {
	struct ifreq ifr;
	int fd;
	int err;
	unsigned long ip;

	strcpy(ifr.ifr_name, interface);
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	err = ioctl(fd, SIOCGIFADDR, &ifr);
	if (err) {
		//perror("ioctl");
		close(fd);
		return 0;
	}
	close(fd);
	memcpy(&ip, ifr.ifr_addr.sa_data + 2, 4);
	return ip;
}

bool InitDir() {
	DIR* dir = opendir(USER_PATH "data");
	if (dir == NULL) {
		if (mkdir(USER_PATH "data", 0) == -1) {
			return false;
		}
	} else {
		closedir(dir);
	}

	return true;
}

static bool g_fConsoleClosed = false;

int CloseConsole(void) {
	int fd;

	if (g_fConsoleClosed) {
		return 0;
	}
	fd = open("/dev/ttyS0", O_RDONLY);
	ioctl(fd, TIOCCONS);
	close(fd);
	g_fConsoleClosed = true;

	return 1;
}

int OpenConsole(void) {
	int fd;

	if (!g_fConsoleClosed) {
		return 0;
	}
	fd = open("/dev/console", O_RDONLY); // 锟侥憋拷console
	ioctl(fd, TIOCCONS);
	close(fd);
	system("stty -parenb cs8 speed 115200 rows 24");
	g_fConsoleClosed = false;
	return 1;
}

bool IsConsoleClosed() {
	return g_fConsoleClosed;
}

void SetConsoleVal(bool var) {
	g_fConsoleClosed = var;
}
