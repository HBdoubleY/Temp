#include "stdafx.h"
#include "syscfg.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include "sysdebug.h"
#include "Comm.h"
#include "sysfs.h"
#include "ComAPI.h"
#include "sysapi.h"
#ifdef SYS_LINUX
#include <syslog.h>
#endif

#define DEBUG_FILE_SIZE (1 * 1024 * 1024)
#define DB_BUF_LEN  (1024 * 16)


#define DebugOUT(s, len)   do { printf("%s", (char* )s); } while (0)

OS_SEM g_semDebug;
//BYTE* g_pbDbItem;
DWORD g_dwDebugClick;
bool g_fDbInited = false;
bool g_fDebugEnable = true;

BYTE g_bDbBuf[DB_BUF_LEN];

bool SysInitDebug() {
	g_semDebug = OSSemCreate(1);

	g_dwDebugClick = GetClick();

	return true;
}

bool InitDebug() {

	return true;
}

bool IsDebugOn(WORD wType) {
	if (!g_fDebugEnable) {
		return false;
	}

	wType = wType;

	return true;
}

bool DebugEnable(BYTE bType) {
	if (!g_fDebugEnable) {
		return false;
	}
	
	g_fDebugEnable = bType;
	return true;
}

void debug_printf(const char *fmt, ...) {
	WORD wOffset = 0;
	TTime tmTime;

	GetSysTime(&tmTime);

	OSSemPend(g_semDebug);

	sprintf((char*) g_bDbBuf, "%02d%02d%02d%02d ", (BYTE) tmTime.nDay, (BYTE) tmTime.nHour, (BYTE) tmTime.nMinute,
			(BYTE) tmTime.nSecond);
	wOffset = strlen((char*) g_bDbBuf);

	va_list varg;

	va_start(varg, fmt);
	vsprintf((char*) g_bDbBuf + wOffset, fmt, varg);
	va_end(varg);

	if (IsConsoleClosed()) {
		OSSemPost(g_semDebug);
		return;
	}

	DebugOUT((char* )g_bDbBuf, wLen);

	OSSemPost(g_semDebug);
}

void STRACE(WORD debug, char *s) {
	if (IsDebugOn(debug)) {
		if (IsConsoleClosed()) {
			return;
		}
		printf("%s", (char*) s);
	}
}

WORD PrintBuf(BYTE* out, BYTE* in, WORD wInLen) {
	for (int i = 0; i < wInLen; i++) {
		BYTE b = *in++;
		BYTE hi = b >> 4;
		BYTE lo = b & 0x0f;
		*out++ = ' ';
		if (hi >= 0x0a) {
			*out++ = hi - 0x0a + 'A';
		} else {
			*out++ = hi + '0';
		}

		if (lo >= 0x0a) {
			*out++ = lo - 0x0a + 'A';
		} else {
			*out++ = lo + '0';
		}
	}

	*out++ = 0;
	return wInLen * 3;
}

WORD PrintBuf(BYTE* out, WORD wOutLen, BYTE* in, WORD wInLen) {
	WORD i;

	for (i = 0; i < wInLen; i++) {
		if ((i + 1) * 3 > wOutLen) {
			return i;
		}

		BYTE b = *in++;
		BYTE hi = b >> 4;
		BYTE lo = b & 0x0f;
		*out++ = ' ';
		if (hi >= 0x0a) {
			*out++ = hi - 0x0a + 'A';
		} else {
			*out++ = hi + '0';
		}

		if (lo >= 0x0a) {
			*out++ = lo - 0x0a + 'A';
		} else {
			*out++ = lo + '0';
		}
	}

	return i;
}

void DebugBuf(WORD wSwitch, char* szHeadStr, BYTE* p, WORD wLen) {
	WORD wOffset = 0;
	TTime tmTime;

	GetSysTime(&tmTime);

	OSSemPend(g_semDebug);
	sprintf((char*) g_bDbBuf, "%02d%02d%02d%02d ", (BYTE) tmTime.nDay, (BYTE) tmTime.nHour, (BYTE) tmTime.nMinute,
			(BYTE) tmTime.nSecond);
	wOffset = strlen((char*) g_bDbBuf);

	WORD wStrLen = strlen(szHeadStr) + wOffset;
	memcpy(g_bDbBuf + wOffset, szHeadStr, wStrLen);

	for (WORD wPrinted = 0; wPrinted < wLen;) {
		WORD n = PrintBuf(&g_bDbBuf[wStrLen], 1000 - wStrLen, p, wLen - wPrinted);
		p += n;
		wPrinted += n;
		wStrLen += n * 3;
		if (n == 0 || wStrLen > 1000) {
			g_bDbBuf[wStrLen++] = '\r';
			g_bDbBuf[wStrLen++] = '\n';
			g_bDbBuf[wStrLen++] = 0;
			STRACE(wSwitch, (char*) g_bDbBuf);
			wStrLen = 0;
		}
	}

	if (wStrLen > 0) {
		g_bDbBuf[wStrLen++] = '\r';
		g_bDbBuf[wStrLen++] = '\n';
		g_bDbBuf[wStrLen++] = 0;
		STRACE(wSwitch, (char*) g_bDbBuf);
	}

	OSSemPost(g_semDebug);
}

void DebugFrm(char* pszHeader, BYTE* pbBuf, WORD wLen) {
	if (wLen > 0) {
		char szBuf[64];
		sprintf(szBuf, "%s %d\n", pszHeader, wLen);
		DebugBuf(DEBUG_CRITICAL, szBuf, pbBuf, wLen);
	}
}

void EnableDebug(bool fEnable) {
	g_fDebugEnable = fEnable;
}

bool IsDebugEnable() {
	return g_fDebugEnable;
}

