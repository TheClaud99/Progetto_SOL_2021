#include "utils.h"

FILE *logfile = NULL;

void Pthread_mutex_lock(pthread_mutex_t *mtx) {
    int err;
    if ((err = pthread_mutex_lock(mtx)) != 0) {
        errno = err;
        perror("lock");
        pthread_exit((void *) &errno);
    }
}

void Pthread_mutex_unlock(pthread_mutex_t *mtx) {
    int err;
    if ((err = pthread_mutex_unlock(mtx)) != 0) {
        errno = err;
        perror("unlock");
        pthread_exit((void *) &errno);
    }
}

void Pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mtx) {
    int err;
    if ((err = pthread_cond_wait(cond, mtx)) != 0) {
        errno = err;
        perror("cond wait");
        pthread_exit((void *) &errno);
    }
}

void Pthread_cond_signal(pthread_cond_t *cond) {
    int err;
    if ((err = pthread_cond_signal(cond)) != 0) {
        errno = err;
        perror("cond signal");
        pthread_exit((void *) &errno);
    }
}


/* msleep(): Sleep for the requested number of milliseconds. */
int msleep(long msec) {
    struct timespec ts;
    int res;

    if (msec < 0) {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

int get_file_name(char **file_name, const char *pathname) {
    // todo cambiare in percorso assoluto
    *file_name = basename(pathname);
    return (int) strlen(*file_name) + 1;    // Il +1 server per l'allocazione corretta delle stringhe
}

int relative2absolute(char **absolute, const char *relative) {
    int len;
    ec_null(*absolute = realpath(relative, NULL), "Percorso non valido")
    len = (int) strlen(*absolute) + 1;
    return len;
}

void *cmalloc(size_t size) {
    void *ret = malloc(size);

    if (ret == NULL) {
        perror("Errore allocazione memoria");
        exit(EXIT_FAILURE);
    }

    memset(ret, 0, size);
    return ret;
}

long toms(struct timespec _t) {
    return _t.tv_sec * 1000 + lround((double) _t.tv_nsec / 1e6);
}

long millis() {
    struct timespec _t;
    clock_gettime(CLOCK_REALTIME, &_t);
    return toms(_t);
}

int tscmp(struct timespec a, struct timespec b) {
    if (a.tv_sec == b.tv_sec) {
        if (a.tv_nsec == b.tv_nsec) return 0;
        return a.tv_nsec > b.tv_nsec ? 1 : -1;
    }
    else {
        return a.tv_sec > b.tv_sec ? 1 : -1;
    }
}