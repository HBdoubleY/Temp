#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#include <comm_sync.hpp>
#include "sysarch.h"

//-------------------------------------------------------------------------------------------------------------------------
CommSem::CommSem() {
	sem_init(&sem, 0, 0);
}

CommSem::~CommSem() {
	sem_destroy(&sem);
}

int CommSem::post() {
	return sem_post(&sem);
}

int CommSem::wait() {
	return sem_wait(&sem);
}

int CommSem::tryWait() {
	return sem_trywait(&sem);
}

int CommSem::wait(int timeOut) {
	struct timeval now;
	struct timespec to;

	gettimeofday(&now, NULL);

	int sec = timeOut / 1000;
	int usec = (timeOut % 1000) * 1000;

	to.tv_sec = now.tv_sec + sec;
	to.tv_nsec = now.tv_usec + usec;

	return sem_timedwait(&sem, &to);
}

int CommSem::get() {
	int sem_value;

	if (sem_getvalue(&sem, &sem_value) == true) {
		return sem_value;
	} else {
		return -1;
	}
}

//-------------------------------------------------------------------------------------------
CommMutex::CommMutex() {
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&mCond, NULL);
}

CommMutex::~CommMutex() {
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&mCond);
}

int CommMutex::lock() {
	return pthread_mutex_lock(&mutex);
}

int CommMutex::lock(int timeOut) {
	while (timeOut > 0) {
		if (pthread_mutex_trylock(&mutex) == true) {
			return 0;
		} else {
			timeOut -= 10;
			Sleep(20);
		}
	}

	return -1;
}

int CommMutex::unlock() {
	return pthread_mutex_unlock(&mutex);
}

void CommMutex::wait() {
	pthread_cond_wait(&mCond, &mutex);
}

void CommMutex::running() {
	pthread_cond_signal(&mCond);
}