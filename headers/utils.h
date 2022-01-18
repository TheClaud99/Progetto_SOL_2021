#ifndef UTILS_H_
#define UTILS_H_

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <linux/limits.h>
#include <sched.h>

#define ec_meno1(c, s)      \
    if ((c) == -1)          \
    {                       \
        perror(s);          \
        exit(EXIT_FAILURE); \
    }
#define ec_null(c, s)       \
    if ((c) == NULL)        \
    {                       \
        perror(s);          \
        exit(EXIT_FAILURE); \
    }
#define PERROR(s)           \
    {                       \
        perror(s);          \
        exit(EXIT_FAILURE); \
    }

void Pthread_mutex_lock(pthread_mutex_t *mtx);

void Pthread_mutex_unlock(pthread_mutex_t *mtx);

void Pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mtx);

void Pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime);

void Pthread_cond_signal(pthread_cond_t *cond);

void summstotimespec(struct timespec *ts, long msec);

/* msleep(): Sleep for the requested number of milliseconds. */
/*int msleep(long msec);*/

#endif /* UTILS_H_ */