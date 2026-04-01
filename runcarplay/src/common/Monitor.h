#ifndef THREADMONITRO_H_
#define THREADMONITRO_H_
#include "sysarch.h"
#include "sysapi.h"

#define THRD_MNTR_NUM           128
#define THRD_NAME_LEN           32

class CMonitor {
public:
	CMonitor();
	virtual ~CMonitor();

	bool Init();
	int ReqMonitor(char* pszName, DWORD dwUpdInterv);
	void FreeMonitor(int iID);
	void ClrMonitorWdg(int iID);
	int RunMonitor();
	bool GetName(int iID, char* pszName);

protected:
	DWORD m_dwClick;        
	OS_SEM m_semMonitor;   
	DWORD m_dwRunClick[THRD_MNTR_NUM];     
	DWORD m_dwUpdInterv[THRD_MNTR_NUM];    
	char m_szThreadName[THRD_MNTR_NUM][THRD_NAME_LEN];  
};

extern CMonitor g_Monitor;
inline bool InitMonitor() {
	return g_Monitor.Init();
}

inline int ReqMonitor(char* pszName, DWORD dwUdpInterv) {
	return g_Monitor.ReqMonitor(pszName, dwUdpInterv);
}

inline void FreeMonitor(int iID) {
	g_Monitor.FreeMonitor(iID);
}

inline void ClrMonitorWdg(int iID) {
	g_Monitor.ClrMonitorWdg(iID);
}

inline int RunMonitor() {
	return g_Monitor.RunMonitor();
}

inline bool GetMonitorName(int iID, char* pszName) {
	return g_Monitor.GetName(iID, pszName);
}

#endif //THREADMONITRO_H
