#ifndef CARPLAY_THREAD_PRIO_H
#define CARPLAY_THREAD_PRIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>

/**
 * Try to raise current thread to the maximum SCHED_FIFO priority.
 * If RT scheduling is not permitted (EPERM), it only logs and keeps working.
 */
static inline void carplay_set_self_sched_fifo_max(const char *tag)
{
	int max_prio = sched_get_priority_max(SCHED_FIFO);
	if (max_prio < 0) {
		printf("[carplay_thread_prio] %s: sched_get_priority_max failed\n", tag ? tag : "unknown");
		return;
	}

	struct sched_param sp;
	memset(&sp, 0, sizeof(sp));
	sp.sched_priority = max_prio;

	pthread_t self = pthread_self();
	int rc = pthread_setschedparam(self, SCHED_FIFO, &sp);
	if (rc != 0) {
		/* pthread_setschedparam returns error code directly */
		printf("[carplay_thread_prio] %s: setschedparam(SCHED_FIFO,%d) failed: %s\n",
		       tag ? tag : "unknown", max_prio, strerror(rc));
		return;
	}
}

#ifdef __cplusplus
}
#endif

#endif /* CARPLAY_THREAD_PRIO_H */

