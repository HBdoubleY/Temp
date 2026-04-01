#include <time.h>
#include "sysarch.h"
#include "sysapi.h"
#include "sysdebug.h"
#include <errno.h>
#include <strings.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "ComAPI.h"
/*-----------------------------------------------------------------------------------*/
static struct linux_sem* sys_sem_new_m(unsigned int count, unsigned int max) {
	struct linux_sem *sem;

	sem = (struct linux_sem *) malloc(sizeof(struct linux_sem));
	sem->c = count;
	sem->max = max;

	pthread_cond_init(&(sem->cond), NULL);
	pthread_mutex_init(&(sem->mutex), NULL);

	return sem;
}

/*-----------------------------------------------------------------------------------*/
//����:��ʣ�µĵȴ�ʱ��,
//     ��timeout��Ϊ0ʱ,����0��ʾʧ�ܳ�ʱ;����>0ʱ�ڳ�ʱ����ǰ�ȴ����¼�
//     ��timeoutΪ0ʱ,����0��ʾ�ȴ����¼�
static unsigned long cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex, unsigned long timeout) {
	unsigned int tdiff;
	unsigned long sec, usec;
	struct timeval rtime1, rtime2;
	struct timespec ts;
	struct timezone tz;
	int retval;

	if (timeout > 0) {
		/* Get a timestamp and add the timeout value. */
		gettimeofday(&rtime1, &tz);
		sec = rtime1.tv_sec;
		usec = rtime1.tv_usec;
		usec += timeout % 1000 * 1000; //usec����timeout�ĺ��벿��
		sec += (int) (timeout / 1000) + (int) (usec / 1000000); //sec����timeout���벿�ּ�usec������벿��
		usec = usec % 1000000;
		ts.tv_nsec = usec * 1000;
		ts.tv_sec = sec;

		retval = pthread_cond_timedwait(cond, mutex, &ts);
		if (retval == ETIMEDOUT) {
			return 0; //��ʣ�µĵȴ�ʱ��Ϊ0,��ʾ�Ȳ�����
		} else {
			/* Calculate for how long we waited for the cond. */
			gettimeofday(&rtime2, &tz);
			tdiff = (rtime2.tv_sec - rtime1.tv_sec) * 1000 + (rtime2.tv_usec - rtime1.tv_usec) / 1000;
			if (tdiff == 0) {
				return 1; //���Ƿ���ETIMEDOUT,�ٸ�һ�λ��,����ʣ�µĵȴ�ʱ��Ϊ1
			}
			return tdiff; //����ʣ�µĵȴ�ʱ��
		}
	} else {
		pthread_cond_wait(cond, mutex); //��ʾ��Զ�ȴ�
		return 0;
	}
}

/*-----------------------------------------------------------------------------------*/
//����:�ȴ�sem->cond
//����:@timeout 0��ʾ��Զ�ȴ�
//����:�õ��ź�������1,����������0
unsigned long sys_arch_sem_wait(struct linux_sem *sem, unsigned long timeout) {
	unsigned long time = 1;

	pthread_mutex_lock(&(sem->mutex));
	while (sem->c <= 0) //�ȴ�sem->c��sys_sem_signal()��++
	{
		if (timeout > 0) {
			time = cond_wait(&(sem->cond), &(sem->mutex), timeout);
			if (time == 0) //��ʣ�µĵȴ�ʱ��Ϊ0,��ʾ�Ȳ�����
					{
				pthread_mutex_unlock(&(sem->mutex));
				return 0;
			}
			/*      pthread_mutex_unlock(&(sem->mutex));
			 return time;*/
		} else {
			cond_wait(&(sem->cond), &(sem->mutex), 0); //0��ʾ��Զ�ȴ�
		}
	}
	sem->c--; //�ݼ��ź�������
	pthread_mutex_unlock(&(sem->mutex));
	return time;
}

/*-----------------------------------------------------------------------------------*/
static void sys_sem_signal(struct linux_sem *sem) {
	pthread_mutex_lock(&(sem->mutex));
	sem->c++;
	if (sem->c > sem->max) //1
			{
		sem->c = sem->max; //1
	}
	pthread_cond_signal(&(sem->cond));
	pthread_mutex_unlock(&(sem->mutex));
}

/*-----------------------------------------------------------------------------------*/
static void sys_sem_free_(struct linux_sem *sem) {
	pthread_cond_destroy(&(sem->cond));
	pthread_mutex_destroy(&(sem->mutex));
	free(sem);
}

/*-----------------------------------------------------------------------------------*/
static void sys_sem_free(struct linux_sem *sem) {
	if (sem != NULL) {
		sys_sem_free_(sem);
	}
}
/*
 OS_SEM OSSemCreate(WORD count)
 {
 return sys_sem_new_m(count, 1);
 }
 */
OS_SEM OSSemCreate(WORD count, WORD timeout) {
	return sys_sem_new_m(count, timeout);
}

void OSSemDel(OS_SEM sem) {
	sys_sem_free(sem);
}
/*
 void OSSemPend(OS_SEM sem)
 {
 sys_arch_sem_wait(sem, 0);
 }
 */
WORD OSSemPend(OS_SEM sem, DWORD timeout) {
	if (sys_arch_sem_wait(sem, timeout) == 0) //�ȴ��źų�ʱ
			{
		return SYS_ERR_TIMEOUT;
	} else {
		return SYS_ERR_OK;
	}
}

void OSSemPost(OS_SEM sem) {
	sys_sem_signal(sem);
}

void Sleep(DWORD dwMilliseconds) {
	usleep(dwMilliseconds * 1000);
}

void NewThread(TThreadRet (*function)(void *arg), void *arg, DWORD nStackSize, TPrio prio) {
	pthread_t thread_id;
	int ret;

	ret = pthread_create(&thread_id, NULL, function, arg);
	if (ret) {
		Debug(0, ("######pthread_create err %s\n", strerror(ret)));
		Debug(0, ("######pthread_create err reset cpu...\n"));
		ResetApp();
	}

	pthread_detach(thread_id);
}

