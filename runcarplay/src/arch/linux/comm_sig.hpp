#ifndef __COMM_SIG_H_
#define __COMM_SIG_H_

#include <signal.h>
#include "comm_sync.hpp"

enum {
	COMM_SIG_UNUSED = SIGUNUSED, COMM_SIG_TERM = SIGTERM, COMM_SIG_INT = SIGINT, COMM_SIG_KILL = SIGKILL
};

//----------------------------------------CSem Class---------------------------------------//
class CommSig {
private:
	static CommMutex mutex;
	static int runTimes;
	static int lastSig;

public:
	static void setSig(int sig);
	static int getSig();
	static void stop();
	int init();
};

#endif /* SYNC_H_ */
