#ifndef UTILS_H_
#define UTILS_H_

#define _POSIX_C_SOURCE 200809L

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

#define ec_cond(c, s)       \
    if (!(c))        \
    {                       \
        perror(s);          \
        exit(EXIT_FAILURE); \
    }

#define PERROR(s)           \
    {                       \
        perror(s);          \
        exit(EXIT_FAILURE); \
    }

#define debug(s, ...) \
    if(print_debug == 1) { \
        char debug[80] = "DEBUG: \0"; \
        strncat(debug, s, 80); \
        fprintf(stdout, debug, ##__VA_ARGS__); \
        fprintf(stdout, "\n");             \
        fflush(stdout); \
    }

int print_debug;

void Pthread_mutex_lock(pthread_mutex_t *mtx);

void Pthread_mutex_unlock(pthread_mutex_t *mtx);

void Pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mtx);

void Pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime);

void Pthread_cond_signal(pthread_cond_t *cond);

void summstotimespec(struct timespec *ts, long msec);

/* msleep(): Sleep for the requested number of milliseconds. */
int msleep(long msec);

struct timespec get_abs_time_from_now(int seconds_from_now);

#endif /* UTILS_H_ */