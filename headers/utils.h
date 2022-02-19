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
#include <math.h>

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

#define check_empty_string(c, s)       \
    if (strlen(c) == 0)        \
    {                       \
        fprintf(stderr, "ERRORE: %s \n", s); \
        fflush(stderr); \
        exit(EXIT_FAILURE); \
    } \

#define PERROR(s)           \
    {                       \
        perror(s);          \
        exit(EXIT_FAILURE); \
    }

#define debug(s, ...) \
    if(print_debug == 1) { \
        char debug[1024]; \
        time_t timer; \
        struct tm* tm_info; \
        timer = time(NULL); \
        tm_info = localtime(&timer); \
        strftime(debug, 1024, "%Y-%m-%d %H:%M:%S DEBUG: ", tm_info);  \
        strncat(debug, s, 1024 - strlen(debug)); \
        fprintf(stdout, debug, ##__VA_ARGS__); \
        fprintf(stdout, "\n");             \
        fflush(stdout); \
    }

#define Info(s, ...) \
    { \
        char info[1024]; \
        time_t timer; \
        struct tm* tm_info; \
        timer = time(NULL); \
        tm_info = localtime(&timer); \
        strftime(info, 1024, "%Y-%m-%d %H:%M:%S INFO: ", tm_info);  \
        strncat(info, s, 1024 - strlen(info)); \
        fprintf(stdout, info, ##__VA_ARGS__); \
        fprintf(stdout, "\n");             \
        fflush(stdout); \
    }

#define Error(s, ...) \
    { \
        char info[1024] = "ERROR: \0"; \
        strncat(info, s, 1024 - strlen(info)); \
        fprintf(stderr, info, ##__VA_ARGS__); \
        fprintf(stderr, "\n");             \
        fflush(stderr); \
    }

#define tInfo(s, ...) \
    {                 \
        char info[1024]; \
        time_t timer; \
        struct tm* tm_info; \
        timer = time(NULL); \
        tm_info = localtime(&timer); \
        strftime(info, 1024, "%Y-%m-%d %H:%M:%S THREAD %%ld: ", tm_info); \
        strncat(info, s, 1024 - strlen(info)); \
        fprintf(stdout, info, pthread_self(), ##__VA_ARGS__); \
        fprintf(stdout, "\n");             \
        fflush(stdout); \
    }

#define warning_if(c, s, ...) \
    if(c) { \
        char warning[1024] = "WARNING: \0"; \
        strncat(warning, s, 1024 - strlen(warning)); \
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

int get_file_name(char **file_name, const char *pathname);

void *cmalloc(size_t size);

long toms(struct timespec _t);

long millis();

int tscmp(struct timespec a, struct timespec b);

#endif /* UTILS_H_ */