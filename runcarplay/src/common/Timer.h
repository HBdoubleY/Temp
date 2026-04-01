#ifndef ___TIMER_H__
#define ___TIMER_H__

#include <sysarch.h>
#include <comm_sync.hpp>
#define MAX_TIMER_NUM			64

typedef struct {
	unsigned char state;
	void (*fun)(void *para);
	void *para;
	unsigned int time;
	unsigned char mode;

	unsigned int ctime;
	unsigned long tick;

	unsigned int cycle;
} TimerType;

class CommTimer {
private:
	static int refCnt;
	static CommMutex mutex;

public:
	CommTimer();
	~CommTimer();

	int add(void (*fun)(void *para), void *para, unsigned int timeMs, unsigned char mode);
	int del(void (*fun)(void *para));
	int delAll(void);
	int work();
};

extern int TimerService(void);

#endif
