#include "stdio.h"

#include "comm_sig.hpp"

CommMutex CommSig::mutex;
int CommSig::runTimes = 0;
int CommSig::lastSig = SIGUNUSED;

void ListenSig(int sig) {
	CommSig::setSig(sig);
}

void CommSig::setSig(int sig) {
	mutex.lock();
	lastSig = sig;
	mutex.unlock();
}

int CommSig::getSig() {
	return lastSig;
}

void CommSig::stop() {
	mutex.lock();
	lastSig = SIGKILL;
	mutex.unlock();
}

int CommSig::init() {
	mutex.lock();

	if (runTimes > 0) {
		mutex.unlock();
		return -1;
	}

	runTimes++;
	signal(SIGTERM, ListenSig);
	signal(SIGKILL, ListenSig);
	signal(SIGINT, ListenSig);

	mutex.unlock();

	return 0;
}
