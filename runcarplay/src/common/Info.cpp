#include "stdafx.h"
#include "ComConst.h"
#include "Info.h"
#include "sysarch.h"
#include "sysapi.h"

#define INFO_NUM    128

static DWORD g_dwInfoClick;
static OS_SEM g_semInfo;
static bool g_fInfo[INFO_NUM];

void InitInfo() {
	g_dwInfoClick = 0;
	g_semInfo = OSSemCreate(1);
	memset(&g_fInfo, 0, sizeof(g_fInfo));
}

void SetInfo(WORD wID, bool fInfo) {
	OSSemPend(g_semInfo);

	if (wID < INFO_NUM) {
		g_fInfo[wID / 8] |= 1 << (wID % 8);
	}

	OSSemPost(g_semInfo);
}

bool GetInfo(WORD wID) {
	bool fRet = false;

	if (wID < INFO_NUM) {
		OSSemPend(g_semInfo);

		WORD wIndex = wID / 8;
		BYTE bFlg = 1 << (wID % 8);

		if (g_fInfo[wIndex] & bFlg) {
			fRet = true;
			g_fInfo[wIndex] &= ~bFlg;
		}
		OSSemPost(g_semInfo);
	}

	return fRet;
}
