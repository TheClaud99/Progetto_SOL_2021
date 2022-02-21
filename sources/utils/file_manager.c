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

    // Aggiorno la data di utilizzo del file
    ec_meno1(clock_gettime(CLOCK_MONOTONIC, &f->last_use), "clock_gettime")

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
    ec_meno1(clock_gettime(CLOCK_MONOTONIC, &f->last_use), "clock_gettime")

    // Inizializzo la maschera dei client che hanno il file aperto
    FD_ZERO(&f->opened_by);
    FD_SET(author, &f->opened_by);

    // Inizializzo le variabili per la lock
    pthread_mutex_init(&f->mtx, NULL);
    pthread_cond_init(&f->lock_cond, NULL);

    // Inserisco il file nella lista
    Pthread_mutex_lock(&ht_mtx);
    ec_cond(ht_put(ht, file_name, f) != HT_ERROR, "ht_put");
    Pthread_mutex_unlock(&ht_mtx);
    debug("Aggiunto file %s", file_name)

    // Aggiorno il numero di file memorizzato
    increase_nfiles();
    return 0;
}

int remove_file(char *file_name, int client_fd) {
    Pthread_mutex_lock(&ht_mtx);
    file_data_t *f = get_file(file_name);
    size_t size = 0;

    if (f == NULL) {
        Pthread_mutex_unlock(&ht_mtx);
        errno = ENOENT;
        return -1;
    }


    Pthread_mutex_lock(&f->mtx);

    if (f->locked_by != client_fd) {
        Pthread_mutex_unlock(&f->mtx);
        Pthread_mutex_unlock(&ht_mtx);
        errno = EACCES;
        return -1;
    }

    ht_remove(ht, file_name);
    Pthread_mutex_unlock(&ht_mtx);

    size = f->length;

    // Risveglio tutti i thread in attesa del file, i quali vedranno che il file non è più nella lista
    // e quindi lo considereranno inesistente
    pthread_cond_broadcast(&f->lock_cond);

    // Secondo la documentazione posso fare una destroy se subito a seguito di una unlock
    Pthread_mutex_unlock(&f->mtx);
    pthread_mutex_destroy(&f->mtx);
    pthread_cond_destroy(&f->lock_cond);
    free(f->file);
    free(f);


    decrease_bytes_used((int) size); // Aggiorno la quantità di memoriza utilizzata
    decrease_nfiles();                      // Aggiorno il numero di file memorizzato

    return 0;
}

int remove_LRU(void** buf, size_t *size, char **file_name, int client_fd) {
    char *k;
    char *selected_file_name;
    file_data_t *f = NULL;
    size_t file_name_len;
    struct timespec min_last_use;

    Pthread_mutex_lock(&ht_mtx);

    hash_elem_it it = HT_ITERATOR(ht);

    // Inizializzo l'ultimo utilizzo più lontano a quello del primo file della lista
    k = ht_iterate_keys(&it);
    if (k != NULL) {
        f = ht_get(ht, k);
        Pthread_mutex_lock(&f->mtx);
        min_last_use = f->last_use;
        Pthread_mutex_unlock(&f->mtx);
        selected_file_name = k;
    }

    k = ht_iterate_keys(&it);
    while (k != NULL) {

        file_data_t *f_temp = ht_get(ht, k);

        Pthread_mutex_lock(&f_temp->mtx);
        // Controllo se il file è stato usato meno recentemente rispetto a quello che ho già selezionato.
        // f->file != NULL server per evitare di espellere un file che è stato creato ma sul quale non è stata
        // ancora fatta la write
        if (tscmp(f_temp->last_use, min_last_use) < 0 && f_temp->file != NULL) {
            f = f_temp;
            min_last_use = f_temp->last_use;
            selected_file_name = k;
        }
        Pthread_mutex_unlock(&f_temp->mtx);

        k = ht_iterate_keys(&it);
    }

    if (f == NULL) {
        Pthread_mutex_unlock(&ht_mtx);
        tInfo("Nessun file trovato")
        errno = ENOENT;
        return -1;
    }

    tInfo("Selezionato per l'espulsione verso %d il file %s, lockato da %d", client_fd, selected_file_name, f->locked_by)

    Pthread_mutex_lock(&f->mtx);

    file_name_len = strlen(selected_file_name) + 1;
    *file_name = cmalloc(file_name_len * sizeof (char));
    strncpy(*file_name, selected_file_name, file_name_len);

    // Devo temporaneamente rilasciare la lock sulla lista per aspettare che il file sia sbloccato
    // dall'utente che lo sta bloccando
    Pthread_mutex_unlock(&ht_mtx);

    while (f->locked_by > 0 && f->locked_by != client_fd) {
        Pthread_cond_wait(&f->lock_cond, &f->mtx);

        // Controllo se il file è sempre nella hashtable, altrimenti vuol dire che è stato rimosso
        // (unlock(&f->mtx) potrebbe dare un'errore valgrind perché è gia stata fatta la free di f, ma non c'è modo di
        // evitarlo)
        Pthread_mutex_unlock(&f->mtx);
        Pthread_mutex_lock(&ht_mtx);
        f = ht_get(ht, selected_file_name);
        Pthread_mutex_unlock(&ht_mtx);

        if (f == NULL) {
            free(*file_name);
            errno = ENOENT;
            return -1;
        }

        Pthread_mutex_lock(&f->mtx);
    }

    f->locked_by = client_fd;

    Info("Lock sul file acquisita")

    // Prendo i dati sul file letto
    *size = f->length;
    *buf = f->file;
    *buf = cmalloc(f->length);
    memcpy(*buf, f->file, f->length);
    Pthread_mutex_unlock(&f->mtx);

    tInfo("Rimuovo file %s", selected_file_name)
    remove_file(selected_file_name, client_fd);

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

        // Controllo se il file è sempre nella hashtable, altrimenti vuol dire che è stato rimosso
        // (unlock(&f->mtx) potrebbe dare un'errore valgrind perché è gia stata fatta la free di f, ma non c'è modo di
        // evitarlo)
        Pthread_mutex_unlock(&f->mtx);
        Pthread_mutex_lock(&ht_mtx);
        f = ht_get(ht, file_name);
        Pthread_mutex_unlock(&ht_mtx);

        if (f == NULL) {
            errno = ENOENT;
            return -1;
        }

        Pthread_mutex_lock(&f->mtx);
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

int write_file(char *file_name, void *data, size_t size, int client_fd) {
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
    memcpy(f->file, data, size);
    f->length = size;

    // Aggiorno la data di utilizzo del file
    ec_meno1(clock_gettime(CLOCK_MONOTONIC, &f->last_use), "clock_gettime")

    Pthread_mutex_unlock(&f->mtx);

    // Aggiorno la quantità di memoriza utilizzata
    increase_bytes_used((int) size);

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
        memcpy(files[count].file, f->file, f->length);
        Pthread_mutex_unlock(&f->mtx);

        k = ht_iterate_keys(&it);
        count++;
    }

    Pthread_mutex_unlock(&ht_mtx);

    return count;
}

int read_file(char *file_name, void **buf, size_t *size, int client_fd) {
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
    memcpy(*buf, f->file, *size);

    // Aggiorno la data di utilizzo del file
    ec_meno1(clock_gettime(CLOCK_MONOTONIC, &f->last_use), "clock_gettime")

    Pthread_mutex_unlock(&f->mtx);

    return 0;
}

int append_to_file(char *file_name, void *data, size_t size, int client_fd) {
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
    void *new_file = cmalloc(total_size);

    memcpy(new_file, f->file, f->length);
    memcpy((char *) new_file + f->length, data, size);

    free(f->file);

    f->file = new_file;
    f->length = total_size;

    // Aggiorno la data di utilizzo del file
    ec_meno1(clock_gettime(CLOCK_MONOTONIC, &f->last_use), "clock_gettime")

    Pthread_mutex_unlock(&f->mtx);

    // Aggiorno la quantità di memoriza utilizzata
    increase_bytes_used((int) size);
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
        debug("Free file %s", k)
        f = ht_remove(ht, k);
        free(f->file);
        free(f);
        k = ht_iterate_keys(&it);
    }

    ht_destroy(ht);

    pthread_mutex_destroy(&ht_mtx);
}

int check_memory_exced(size_t data_to_insert, int flags) {
    int ret = 0;
    Pthread_mutex_lock(&stats_mtx);

    if (flags & CHECK_MEMORY_EXCEDED && stats.current_bytes_used + data_to_insert > config.max_memory_size) {
        ret =  1;
        Info("Oltrepassato limite di byte utilizzati con %dB", stats.current_bytes_used + data_to_insert)
    }

    if (flags & CHECK_NFILES_EXCEDED && stats.current_files_saved >= config.max_files) {
        ret = 1;
        Info("Oltrepassato limite di file con %d files", stats.current_files_saved)
    }

    Pthread_mutex_unlock(&stats_mtx);

    return ret;
}