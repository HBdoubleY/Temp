#include "stdafx.h"

#include <Timer.h>
#include <sysarch.h>
#include <comm_sync.hpp>
#include "sysapi.h"
#include "ComAPI.h"

#define TIMER_NULL      0x45
#define TIMER_USED      0x54

static TimerType UsrTimer[MAX_TIMER_NUM];

CommMutex CommTimer::mutex;
int CommTimer::refCnt = 0;

TThreadRet TimerDaemo(void *pdata) {
	CommTimer timer;
	timer.work(); // while(1)...

	return THREAD_RET_OK;
}

CommTimer::CommTimer() {
	mutex.lock();

	if (refCnt == 0) {		
		for (int i = 0; i < MAX_TIMER_NUM; i++) {
			UsrTimer[i].state = TIMER_NULL;
			UsrTimer[i].fun = NULL;
		}
	}

	refCnt++;
	mutex.unlock();
}

CommTimer::~CommTimer() {
	mutex.lock();
	refCnt--;	
	mutex.unlock();
}

/*
 生成一个定时器
 Fun:回调函数
 Para:执行参数
 ms:间隔时间mS
 Mode:模式,0:1次,1:连续
 */
int CommTimer::add(void (*fun)(void *para), void *para, unsigned int ms, unsigned char mode) {
	del(fun);

	for (int i = 0; i < MAX_TIMER_NUM; i++) {
		mutex.lock();

		if (UsrTimer[i].state == TIMER_NULL) {
			UsrTimer[i].state = TIMER_USED;
			UsrTimer[i].fun = fun;
			UsrTimer[i].para = para;
			UsrTimer[i].time = ms / 40;
			UsrTimer[i].mode = mode;
			UsrTimer[i].ctime = 0;
			UsrTimer[i].cycle = ms;
			UsrTimer[i].tick = GetTick();

			mutex.unlock();
			return 0;
		}

		mutex.unlock();
	}

	return -1;
}

int CommTimer::del(void (*fun)(void *para)) {
	for (int i = 0; i < MAX_TIMER_NUM; i++) {
		mutex.lock();

		if (UsrTimer[i].fun == fun) {
			UsrTimer[i].state = TIMER_NULL;
		}

		mutex.unlock();
	}

	return -1;
}

int CommTimer::delAll(void) {
	for (int i = 0; i < MAX_TIMER_NUM; i++) {
		mutex.lock();

		if (UsrTimer[i].fun != NULL) {
			UsrTimer[i].state = TIMER_NULL;
		}

		mutex.unlock();
	}

	return -1;
}


int CommTimer::work(void) {
	while (1) {
		for (int i = 0; i < MAX_TIMER_NUM; i++) {
			mutex.lock();

			if (UsrTimer[i].state != TIMER_NULL) {
				//if (UsrTimer[i].ctime++ == (UsrTimer[i].time)) {
				unsigned long tick = GetTick();
				if((tick -UsrTimer[i].tick)>=UsrTimer[i].cycle) {
					if (UsrTimer[i].mode == 0) {
						UsrTimer[i].state = TIMER_NULL;  //一次
					} else {
						//UsrTimer[i].ctime = 0;           //连续
						UsrTimer[i].tick = tick;        //连续
					}

					void (*fun)(void *para) = UsrTimer[i].fun;
					mutex.unlock();
					fun(UsrTimer[i].para);  //执行
					mutex.lock();
				}
			}

			mutex.unlock();
		}
		Sleep(20);
	}

	return 0;
}

int TimerService(void) {	
	NewThread(TimerDaemo, NULL, 2048, THREAD_PRIORITY_NORMAL); 
	return 0;
}
