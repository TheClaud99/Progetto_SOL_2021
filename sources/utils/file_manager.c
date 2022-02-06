#include "file_manager.h"

hashtable_t *ht;
pthread_mutex_t ht_mtx = PTHREAD_MUTEX_INITIALIZER;

void init_file_manager() {
    ec_null(ht = ht_create(config.max_files), "hash table create");
}

int open_file(char *file_name, int lock, int client_fd) {
    file_data_t *f;

    Pthread_mutex_lock(&ht_mtx);                        // - LOCK LISTA
    if ((f = ht_get(ht, file_name)) == NULL) {
        Pthread_mutex_unlock(&ht_mtx);
        errno = ENOENT;
        return -1;
    }
    Pthread_mutex_lock(&f->mtx);                        // - LOCK FILE
    Pthread_mutex_unlock(&ht_mtx);                      // - UNLOCK LISTA

    FD_SET(client_fd, &f->opened_by);

    if (lock == 1) {
        while (f->locked_by > 0 && f->locked_by != client_fd) {
            Pthread_cond_wait(&f->lock_cond, &f->mtx);
        }
        f->locked_by = client_fd;
    }
    Pthread_mutex_unlock(&f->mtx);                        // - UNLOCK FILE

    return 0;
}

int file_exists(char *file_name) {
    file_data_t *f;

    if ((f = ht_get(ht, file_name)) == NULL) {
        return 0;
    }

    return 1;
}


int add_file(char *file_name, int lock, int author) {

    if (ht_get(ht, file_name) != NULL) {
        errno = EEXIST;
        return -1;
    }

    file_data_t *f = cmalloc(sizeof(file_data_t));
    f->author = author;
    f->update_date = time(NULL);
    f->length = 0;
    f->locked_by = lock == 1 ? author : -1;
    f->file = NULL;

    // Inizializzo la maschera dei client che hanno il file aperto
    FD_ZERO(&f->opened_by);
    FD_SET(author, &f->opened_by);

    // Inizializzo le variabili per la lock
    pthread_mutex_init(&f->mtx, NULL);
    pthread_cond_init(&f->lock_cond, NULL);

    ht_put(ht, file_name, f);

    return 0;
}

int remove_file(char *file_name, int client_fd) {
    Pthread_mutex_lock(&ht_mtx);
    file_data_t *f = get_file(file_name);

    if (f == NULL) {
        Pthread_mutex_unlock(&ht_mtx);
        errno = ENOENT;
        return -1;
    }

    ht_remove(ht, file_name);

    Pthread_mutex_lock(&f->mtx);
    Pthread_mutex_unlock(&ht_mtx);

    if (f->locked_by != client_fd) {
        Pthread_mutex_unlock(&f->mtx);
        errno = EACCES;
        return -1;
    }

    pthread_cond_destroy(&f->lock_cond);
    pthread_mutex_destroy(&f->mtx);
    free(f->file);
    free(f);

    return 0;
}


file_data_t *get_file(char *file_name) {
    return ht_get(ht, file_name);
}

int lockfile(char *file_name, int client_fd) {
    Pthread_mutex_lock(&ht_mtx);
    file_data_t *f = get_file(file_name);

    if (f == NULL) {
        Pthread_mutex_unlock(&ht_mtx);
        errno = ENOENT;
        return -1;
    }
    Pthread_mutex_lock(&f->mtx);
    Pthread_mutex_unlock(&ht_mtx);

    while (f->locked_by > 0 && f->locked_by != client_fd) {
        Pthread_cond_wait(&f->lock_cond, &f->mtx);
    }
    f->locked_by = client_fd;

    Pthread_mutex_unlock(&f->mtx);

    return 0;
}

int unlockfile(char *file_name, int client_fd) {
    Pthread_mutex_lock(&ht_mtx);
    file_data_t *f = get_file(file_name);

    if (f == NULL) {
        Pthread_mutex_unlock(&ht_mtx);
        errno = ENOENT;
        return -1;
    }
    Pthread_mutex_lock(&f->mtx);
    Pthread_mutex_unlock(&ht_mtx);

    if (f->locked_by != client_fd) {
        Pthread_mutex_unlock(&f->mtx);
        errno = EACCES;
        return -1;
    }

    f->locked_by = -1;

    // Segnalo a chi era in attesa della mutua esculsione sul file che adesso è disponibile
    Pthread_cond_signal(&f->lock_cond);

    Pthread_mutex_unlock(&f->mtx);
    return 0;
}

int write_file(char *file_name, char *data, size_t size, int client_fd) {
    Pthread_mutex_lock(&ht_mtx);
    file_data_t *f = get_file(file_name);

    if (f == NULL) {
        Pthread_mutex_unlock(&ht_mtx);
        errno = ENOENT;
        return -1;
    }
    Pthread_mutex_lock(&f->mtx);
    Pthread_mutex_unlock(&ht_mtx);

    if (!FD_ISSET(client_fd, &f->opened_by)) {
        Pthread_mutex_unlock(&f->mtx);
        errno = EPERM;
        return -1;
    }

    if (f->locked_by != client_fd) {
        Pthread_mutex_unlock(&f->mtx);
        errno = EACCES;
        return -1;
    }

    f->file = cmalloc(size);
    strncpy(f->file, data, size);
    f->length = size;

    Pthread_mutex_unlock(&f->mtx);

    return 0;
}

/*int read_random_file(char **buf, size_t *size, char filename[], int remove) {
    hash_elem_it it = HT_ITERATOR(ht);
    char *k = ht_iterate_keys(&it);
    if (k != NULL) {
        read_file(k, buf, size);
        strcpy(filename, k);
        if (remove == 1) {
            return remove_file(k);
        }

        return 0;
    }

    return -1;
}*/

int readn_files(readn_ret_t files[], int max_files) {
    char *k;
    int count = 0;
    file_data_t *f;

    Pthread_mutex_lock(&ht_mtx);

    hash_elem_it it = HT_ITERATOR(ht);
    k = ht_iterate_keys(&it);
    while ((max_files <= 0 || count < max_files) && k != NULL) {
        files[count].name = cmalloc(strlen(k) + 1);
        strcpy(files[count].name, k);

        f = ht_get(ht, k);

        Pthread_mutex_lock(&f->mtx);
        files[count].length = f->length;
        files[count].file = cmalloc(f->length);
        strncpy(files[count].file, f->file, f->length);
        Pthread_mutex_unlock(&f->mtx);

        k = ht_iterate_keys(&it);
        count++;
    }

    Pthread_mutex_unlock(&ht_mtx);

    return count;
}

int read_file(char *file_name, char **buf, size_t *size, int client_fd) {
    Pthread_mutex_lock(&ht_mtx);
    file_data_t *f = get_file(file_name);

    if (f == NULL) {
        Pthread_mutex_unlock(&ht_mtx);
        errno = ENOENT;
        return -1;
    }
    Pthread_mutex_lock(&f->mtx);
    Pthread_mutex_unlock(&ht_mtx);

    if (!FD_ISSET(client_fd, &f->opened_by)) {
        Pthread_mutex_unlock(&f->mtx);
        errno = EPERM;
        return -1;
    }

    *size = f->length;
    *buf = cmalloc(*size);
    strncpy(*buf, f->file, *size);

    Pthread_mutex_unlock(&f->mtx);

    return 0;
}

int append_to_file(char *file_name, char *data, size_t size, int client_fd) {
    Pthread_mutex_lock(&ht_mtx);
    file_data_t *f = get_file(file_name);

    if (f == NULL) {
        Pthread_mutex_unlock(&ht_mtx);
        errno = ENOENT;
        return -1;
    }
    Pthread_mutex_lock(&f->mtx);
    Pthread_mutex_unlock(&ht_mtx);

    if (!FD_ISSET(client_fd, &f->opened_by)) {
        Pthread_mutex_unlock(&f->mtx);
        errno = EPERM;
        return -1;
    }

    size_t total_size = f->length + size;
    char *new_file = cmalloc(total_size);

    strncpy(new_file, f->file, f->length);
    strncat(new_file, data, size);

    free(f->file);

    f->file = new_file;
    f->length = total_size;

    Pthread_mutex_unlock(&f->mtx);

    return 0;
}

int close_file(char *file_name, int client_fd) {
    Pthread_mutex_lock(&ht_mtx);
    file_data_t *f = get_file(file_name);

    if (f == NULL) {
        Pthread_mutex_unlock(&ht_mtx);
        errno = ENOENT;
        return -1;
    }
    Pthread_mutex_lock(&f->mtx);
    Pthread_mutex_unlock(&ht_mtx);

    FD_CLR(client_fd, &f->opened_by);

    Pthread_mutex_unlock(&f->mtx);
    return 0;
}

void close_file_manager() {
    file_data_t *f;

    Pthread_mutex_lock(&ht_mtx);

    // Non ho bisogno di acquisire la lock sui file perché in teoria quando arrivo qui
    // tutti i thread dovrebbero essere terminati
    hash_elem_it it = HT_ITERATOR(ht);
    char *k = ht_iterate_keys(&it);
    // Faccio la free di tutti i file
    while (k != NULL) {
        f = ht_remove(ht, k);
        free(f->file);
        free(f);
        k = ht_iterate_keys(&it);
    }

    ht_destroy(ht);

    pthread_mutex_destroy(&ht_mtx);
}

int check_memory_exced(size_t data_to_insert, int flags) {
    Pthread_mutex_lock(&stats_mtx);

    if (flags & CHECK_MEMORY_EXCEDED && stats.current_bytes_used + data_to_insert > config.max_memory_size)
        return 1;

    if (flags & CHECK_NFILES_EXCEDED && stats.current_files_saved >= config.max_files) {
        return 1;
    }

    Pthread_mutex_unlock(&stats_mtx);

    return 0;
}