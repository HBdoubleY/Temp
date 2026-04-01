#include "stdafx.h"
#include "Monitor.h"

CMonitor g_Monitor;
///////////////////////////////////////////////////////////////////////////////////
//CMonitor

CMonitor::CMonitor() {
}

CMonitor::~CMonitor() {
}

bool CMonitor::Init() {
	m_semMonitor = OSSemCreate(1);
	memset(&m_dwRunClick, 0, sizeof(m_dwRunClick)); 
	memset(&m_dwUpdInterv, 0, sizeof(m_dwUpdInterv));  
	m_dwClick = 0;
	return true;
}

int CMonitor::ReqMonitor(char* pszName, DWORD dwUpdInterv) {
	if (dwUpdInterv == 0) {
		return -1;
	}

	int iID = -1;
	OSSemPend(m_semMonitor);
	for (WORD i = 0; i < THRD_MNTR_NUM; i++) {
		if (m_dwUpdInterv[i] == 0) 
				{
			m_dwRunClick[i] = m_dwClick;       
			m_dwUpdInterv[i] = dwUpdInterv;  
			iID = i;

			memset(m_szThreadName[iID], 0, THRD_NAME_LEN);
			if (pszName != NULL) {
				int iLen = strlen(pszName);
				if (iLen > THRD_NAME_LEN - 1) {
					iLen = THRD_NAME_LEN - 1;
				}

				iLen++; 
				memcpy(m_szThreadName[iID], pszName, iLen);
			}

			if (m_szThreadName[iID][0] == '\0') {
				memcpy(m_szThreadName[iID], "unknow-thrd", strlen("unknow-thrd") + 1);
			}

			m_szThreadName[iID][THRD_NAME_LEN - 1] = '\0';

			break;
		}
	}

	OSSemPost(m_semMonitor);
	return iID;
}

void CMonitor::FreeMonitor(int iID) {
	if (iID < 0 || iID >= THRD_MNTR_NUM) {
		return;
	}

	OSSemPend(m_semMonitor);

	m_dwUpdInterv[iID] = 0;
	m_dwRunClick[iID] = 0;

	OSSemPost(m_semMonitor);
}


void CMonitor::ClrMonitorWdg(int iID) {
	if (iID < 0 || iID >= THRD_MNTR_NUM) {
		return;
	}

	if (m_dwUpdInterv[iID] != 0) 
			{
		m_dwRunClick[iID] = m_dwClick;
	}
}


int CMonitor::RunMonitor() {
	m_dwClick++;
	for (WORD i = 0; i < THRD_MNTR_NUM; i++) {
		if (m_dwUpdInterv[i] != 0) 
				{
			if (m_dwClick - m_dwRunClick[i] > m_dwUpdInterv[i]) {
				return -(i + 1);
			}
		}
	}

	return 0;
}

bool CMonitor::GetName(int iID, char* pszName) {
	if (iID < 0 || iID >= THRD_MNTR_NUM) {
		memset(pszName, 0, THRD_NAME_LEN);
		return false;
	} else {
		memcpy(pszName, m_szThreadName[iID], THRD_NAME_LEN);
		return true;
	}
}
