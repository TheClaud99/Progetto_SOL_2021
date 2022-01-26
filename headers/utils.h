#ifndef UTILS_H_
#define UTILS_H_

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <linux/limits.h>
#include <sched.h>
#include <string.h>

#define ec_meno1(c, s)      \
    if ((c) == -1)          \
    {                       \
        fprintf(stderr, "Errore: File %s Riga %d\n", __FILE__, __LINE__); \
        fflush(stderr);                    \
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
        strncat(debug, s, 80 - strlen(debug)); \
        fprintf(stdout, debug, ##__VA_ARGS__); \
        fprintf(stdout, "\n");             \
        fflush(stdout); \
    }

#define Info(s, ...) \
    { \
        char info[80] = "INFO: \0"; \
        strncat(info, s, 80 - strlen(info)); \
        fprintf(stdout, info, ##__VA_ARGS__); \
        fprintf(stdout, "\n");             \
        fflush(stdout); \
    }

#define tInfo(s, ...) \
    { \
        char info[80] = "THREAD %ld: \0"; \
        strncat(info, s, 80 - strlen(info)); \
        fprintf(stdout, info, pthread_self(), ##__VA_ARGS__); \
        fprintf(stdout, "\n");             \
        fflush(stdout); \
    }

#define warning_if(c, s, ...) \
    if(c) { \
        char warning[80] = "WARNING: \0"; \
        strncat(warning, s, 80 - strlen(warning)); \
        fprintf(stdout, warning, ##__VA_ARGS__); \
        fprintf(stdout, "\n");             \
        fflush(stdout); \
    }

extern int print_debug;

void Pthread_mutex_lock(pthread_mutex_t *mtx);

void Pthread_mutex_unlock(pthread_mutex_t *mtx);

void Pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mtx);

void Pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime);

void Pthread_cond_signal(pthread_cond_t *cond);

void summstotimespec(struct timespec *ts, long msec);

/* msleep(): Sleep for the requested number of milliseconds. */
int msleep(long msec);

struct timespec get_abs_time_from_now(int seconds_from_now);

int get_file_name(char *file_name, const char *pathname);

void* cmalloc(size_t size);

#endif /* UTILS_H_ */