#ifndef INFO_H
#define INFO_H

#include "apptypedef.h"

#define  INFO_APP_RST			0
#define  INFO_GPRS_TXRX         1
#define  INFO_ETHER_TXRX        2
#define  INFO_ACTIVE			3

void InitInfo();
void SetInfo(WORD wID, bool fInfo = true);
bool GetInfo(WORD wID);
void SetDelayInfo(WORD wID);

#endif //INFO_H
