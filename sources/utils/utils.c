#include "utils.h"

void Pthread_mutex_lock(pthread_mutex_t *mtx)
{
    int err;
    if ((err = pthread_mutex_lock(mtx)) != 0)
    {
        errno = err;
        perror("lock");
        pthread_exit((void *)&errno);
    }
}

void Pthread_mutex_unlock(pthread_mutex_t *mtx)
{
    int err;
    if ((err = pthread_mutex_unlock(mtx)) != 0)
    {
        errno = err;
        perror("unlock");
        pthread_exit((void *)&errno);
    }
}

void Pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mtx)
{
    int err;
    if ((err = pthread_cond_wait(cond, mtx)) != 0)
    {
        errno = err;
        perror("cond wait");
        pthread_exit((void *)&errno);
    }
}

void Pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime)
{
    int err;
    if ((err = pthread_cond_timedwait(cond, mutex, abstime)) != 0 && err != ETIMEDOUT)
    {
        errno = err;
        perror("cond wait");
        pthread_exit((void *)&errno);
    }
}

void Pthread_cond_signal(pthread_cond_t *cond)
{
    int err;
    if ((err = pthread_cond_signal(cond)) != 0)
    {
        errno = err;
        perror("cond signal");
        pthread_exit((void *)&errno);
    }
}

void summstotimespec(struct timespec *ts, long msec)
{
    int nano_sec = (ts->tv_nsec + (msec % 1000) * 1000000);
    ts->tv_sec +=  msec / 1000 + nano_sec / 1000000000;
    ts->tv_nsec = nano_sec % 1000000000;
}

/* msleep(): Sleep for the requested number of milliseconds. */
int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do
    {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

struct timespec get_abs_time_from_now(int seconds_from_now) {
    struct timespec abstime;
    time_t now = time(NULL);//Gestire errore
    abstime.tv_sec = now + seconds_from_now;
    return abstime;
}

int get_file_name(char *file_name, const char *pathname) {
    strcpy(file_name, basename(pathname));
    return (int)strlen(file_name);
}

void* cmalloc(size_t size) {
    void *ret = malloc(size);

    if(ret == NULL) {
        perror("Errore allocazione memoria");
        exit(EXIT_FAILURE);
    }

    memset(ret, 0, size);
    return ret;
}
