#ifndef USERKERNELADAPTER_h
#define USERKERNELADAPTER_h

#ifdef CONFIG_VIDEO_RT_MEDIA
//#include "ve_interface.h"
#include <cdc_log.h>
#include <linux/mutex.h>

int kernel_write_file(unsigned char *data0, int size0, unsigned char *data1, int size1, char *file_name);
int kernel_mutex_init(struct mutex *p_mutex);

#define PRINTF printk

#define MUTEX_STRUCT struct mutex
#define SEM_STRUCT struct semaphore
#define THREAD_STRUCT struct task_struct*
#define MALLOC(size) kmalloc(size, GFP_KERNEL)
#define MALLOC_BIG(size) vmalloc(size)
#define CALLOC(num, size)  kcalloc(num, size, GFP_KERNEL)
#define FREE(poiter) if (poiter) {\
    kfree(poiter);\
    poiter = NULL;\
}
#define FREE_BIG(poiter) if (poiter) {\
    vfree(poiter);\
    poiter = NULL;\
}

#define THREAD_CREATE(p_thread, attr, func, arg, name)\
do { \
    p_thread = kthread_run(func, (void*)arg, name); \
} while(0)

#define THREAD_JOIN(thread, pp_ret) kthread_stop(thread)
#define THREAD_SETPARAM(thread, sched, param) sched_setscheduler(thread, sched, param)

#define DEF_STATIC_MUTEX(Mutex) DEFINE_MUTEX(Mutex)
#define MUTEX_INIT(p_mutex, p_mutex_attr)  kernel_mutex_init(p_mutex)
#if 1
#define MUTEX_LOCK(p_mutex) mutex_lock(p_mutex)
#else
#define MUTEX_LOCK(p_mutex)\
do { \
	logw("begin lock %px", p_mutex);\
	if (p_mutex)\
		mutex_lock(p_mutex);\
	else\
		loge("p_mutex is null");\
	logw("lock end %px", p_mutex);\
} while(0)
#endif
#define MUTEX_UNLOCK(p_mutex) mutex_unlock(p_mutex)
#define MUTEX_DESTROY(p_mutex) mutex_destroy(p_mutex)

#define SEM_INIT(p_sem, shared, value) sema_init(p_sem, value)
#define SEM_POST(p_sem) up(p_sem)
#define SEM_WAIT(p_sem) down(p_sem)
#define SEM_DESTROY(p_sem)

#define MEMOPS_STRUCT struct mem_interface
#define VEOPS_STRUCT struct ve_interface

#define FILE_STRUCT struct file
#define WRITE_FILE(data0, size0, data1, size1, file_name) kernel_write_file(data0, size0, data1, size1, file_name)
#define WRITE_FILE_FD(data0, size0, data1, size1, fd)

#define POPCOUNT(p_addr) popcount(p_addr)
#define USLEEP(us) udelay(us)

#ifndef VENC_SUPPORT_EXT_PARAM
#define VENC_SUPPORT_EXT_PARAM 0
#endif

#else//not CONFIG_VIDEO_RT_MEDIA
#include <pthread.h>
#include <stdio.h>

#define PRINTF printf
#define MUTEX_STRUCT pthread_mutex_t
#define SEM_STRUCT sem_t
#define THREAD_STRUCT pthread_t
#define MALLOC(size) malloc(size)
#define MALLOC_BIG(size) malloc(size)
#define CALLOC(num, size)  calloc(num, size)
#define FREE(poiter) if (poiter) {\
    free(poiter);\
    poiter = NULL;\
}

#define FREE_BIG(poiter) if (poiter) {\
    free(poiter);\
    poiter = NULL;\
}

#define THREAD_CREATE(thread, attr, func, arg, name) pthread_create(&thread, attr, func, (void*)arg)
#define THREAD_JOIN(thread, pp_ret) pthread_join(thread, pp_ret)
#define THREAD_SETPARAM(thread, sched, param) pthread_setschedparam(thread, sched, param)

#define DEF_STATIC_MUTEX(Mutex) static MUTEX_STRUCT Mutex = PTHREAD_MUTEX_INITIALIZER
#define MUTEX_INIT(p_mutex, p_mutex_attr)  pthread_mutex_init(p_mutex, p_mutex_attr)
#define MUTEX_LOCK(p_mutex) pthread_mutex_lock(p_mutex)
#define MUTEX_UNLOCK(p_mutex) pthread_mutex_unlock(p_mutex)

#define MUTEX_DESTROY(p_mutex) pthread_mutex_destroy(p_mutex)

#define SEM_INIT(p_sem, shared, value) sem_init(p_sem, shared, value)
#define SEM_POST(p_sem) sem_post(p_sem)
#define SEM_WAIT(p_sem) sem_wait(p_sem)
#define SEM_DESTROY(p_sem) sem_destroy(p_sem)

#define MEMOPS_STRUCT struct ScMemOpsS
#define VEOPS_STRUCT VeOpsS

#define FILE_STRUCT FILE
#define WRITE_FILE(data0, size0, data1, size1, file_name) user_write_file(data0, size0, data1, size1, file_name)
#define WRITE_FILE_FD(data0, size0, data1, size1, fd) user_write_file_fd(data0, size0, data1, size1, fd)

#define POPCOUNT(p_addr) __builtin_popcount(p_addr)
#define USLEEP(us) usleep(us)

int user_write_file(unsigned char *data0, int size0, unsigned char *data1, int size1, char *file_name);
int user_write_file_fd(unsigned char *data0, int size0, unsigned char *data1, int size1, FILE_STRUCT *fp);
#endif//CONFIG_VIDEO_RT_MEDIA


#define ALIGN_XXB(y, x) (((x) + ((y)-1)) & ~((y)-1))

int get_random_number(void);

#endif//USERKERNELADAPTER_h