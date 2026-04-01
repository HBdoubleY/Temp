#ifndef __COMM_SYNC_H_
#define __COMM_SYNC_H_

#include <semaphore.h>
#include <pthread.h>

//----------------------------------------CSem Class---------------------------------------//
class CommSem {
private:
	sem_t sem;

public:
	CommSem();
	virtual ~CommSem();

	int post();
	int wait();
	int tryWait();
	int wait(int timeOut); //ms
	int get();
};

//-------------------------------------CThdMutex Class-------------------------------------//
class CommMutex {
private:
	pthread_mutex_t mutex;
	pthread_cond_t mCond;

public:
	CommMutex();
	virtual ~CommMutex();

	int lock();
	int lock(int timeOut); //ms
	int unlock();
	void wait();
	void running();

};

#endif /* SYNC_H_ */
