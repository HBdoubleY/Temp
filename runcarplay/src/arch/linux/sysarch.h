#ifndef _SYSARCH_H_
#define _SYSARCH_H_

#include <strings.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>
#include "apptypedef.h"
#include "ComStruct.h"
#include "sysdebug.h"

#define INFINITE    0x7ffffff0
#define NOWAIT      0

#define SYS_ERR_OK          0       //�޴���
#define SYS_ERR_INVALID     1       //�������ݷǷ�
#define SYS_ERR_TIMEOUT     0x20    //��ʱ
#define SYS_ERR_SYS         3       //ϵͳ����

struct linux_sem {
	unsigned int c;
	unsigned int max;
	pthread_cond_t cond; /*  */
	pthread_mutex_t mutex;
};

typedef linux_sem* OS_SEM;

#ifndef TPrio
typedef unsigned int TPrio;
#define THREAD_PRIORITY_IDLE            (0)
#define THREAD_PRIORITY_LOWEST          (1)
#define THREAD_PRIORITY_BELOW_NORMAL    (2)
#define THREAD_PRIORITY_NORMAL          (3)
#define THREAD_PRIORITY_ABOVE_NORMAL    (4)
#define THREAD_PRIORITY_HIGHEST         (5)
#define THREAD_PRIORITY_TIME_CRITICAL   (6)
#endif

#define THREAD_RET_OK   NULL
#define THREAD_RET_ERR  NULL

typedef void* TThreadRet;

//extern OS_SEM OSSemCreate(WORD count);
extern OS_SEM OSSemCreate(WORD count, WORD timeout = 1);
extern void OSSemDel(OS_SEM sem);
//extern void OSSemPend(OS_SEM sem);
extern WORD OSSemPend(OS_SEM sem, DWORD timeout = 0);
extern void OSSemPost(OS_SEM sem);
extern void NewThread(TThreadRet (*function)(void *arg), void *arg, DWORD nStackSize = 0, TPrio prio =
THREAD_PRIORITY_NORMAL);

extern void Sleep(DWORD dwMilliseconds);

extern void InitArch();

#endif //_SYSARCH_H_

