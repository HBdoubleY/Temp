#ifndef SYSAPI_H
#define SYSAPI_H
#include "ComStruct.h"
// #include <net/if.h>
// #include <net/if_arp.h>
// #include <netinet/in.h>

#include "syscfg.h"
#include "ComAPI.h"

extern bool g_fAppExit;

bool GetSysTime(TTime* pTime);
bool SetSysTime(const TTime& t);
void SyncTimer();
DWORD GetClick();
DWORD GetTick();
uint64 GetTickUs();
void SysInit();
void SysCleanup();

bool IsAppExit();
bool IsAppExitDone();

void ResetApp();
int IfReadListProc(char *target);
DWORD GetLocalAddr(char* interface);
bool InitDir();
unsigned int freespace(char* disk);

int CloseConsole(void);
int OpenConsole(void);
bool IsConsoleClosed();
void SetConsoleVal(bool var);
#endif //SYSAPI_H
