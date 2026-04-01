#ifndef COMSTRUCT_H
#define COMSTRUCT_H
#include "apptypedef.h"

typedef struct
{
    WORD nYear;
    BYTE nMonth;
    BYTE nDay;
    BYTE nHour;
    BYTE nMinute;
    BYTE nSecond;
    BYTE nWeek;
}TTime;


#endif //COMSTRUCT_H

