#ifndef SYSDEBUG_H
#define SYSDEBUG_H
#include <syslog.h>
#include <apptypedef.h>

#include "debugcfg.h"

extern void debug_printf(const char *fmt, ...);
#define Debug(debug, x) do { if (IsDebugOn(debug)) { debug_printf x; } } while (0)

extern bool InitDebug();
extern bool SysInitDebug();

extern bool IsDebugOn(WORD wType);
extern bool IsDebugEnable();
extern void EnableDebug(bool fEnable);
extern void DebugFrm(char* pszHeader, BYTE* pbBuf, WORD wLen);
extern void DebugBuf(WORD debug, char* szHeadStr, BYTE* p, WORD wLen);

#endif //SYSDEBUG_H
