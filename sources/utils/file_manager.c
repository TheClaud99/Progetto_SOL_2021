#include "file_manager.h"

hashtable_t *ht;

void init_file_manager() {
    ec_null(ht = ht_create(config.max_files), "hash table create");
}

file_data_t* add_file(char* file_name, int author) {
    file_data_t* f = cmalloc(sizeof(file_data_t));
    f->author = author;
    f->update_date = time(NULL);
    f->length = 0;

    ht_put(ht, file_name, f);

    return f;
}

int remove_file(char* file_name) {
    file_data_t* f;

    f = ht_remove(ht, file_name);

    if(f != NULL) {
        free(f->file);
        free(f);

        return 0;
    }

    return -1;
}

file_data_t* get_file(char *file_name) {
    return ht_get(ht, file_name);
}

int lockfile(char *file_name) {
    file_data_t *f = get_file(file_name);

    if(f == NULL) return -1;

    f->locked = 1;
    return 0;
}

int unlockfile(char *file_name) {
    file_data_t *f = get_file(file_name);

    if(f == NULL) return -1;

    f->locked = 0;
    return 0;
}

void write_file(char *file_name, char *data, size_t size) {
    file_data_t *f = get_file(file_name);
    f->file = cmalloc(size);
    strncpy(f->file, data, size);
    f->length = size;
}

int read_random_file(char **buf, size_t *size, char filename[], int remove) {
    hash_elem_it it = HT_ITERATOR(ht);
    char* k = ht_iterate_keys(&it);
    if(k != NULL) {
        read_file(k, buf, size);
        strcpy(filename, k);
        if(remove == 1) {
            return remove_file(k);
        }

        return 0;
    }

    return -1;
}

int readn_files(readn_ret_t files[], int max_files) {
    hash_elem_it it = HT_ITERATOR(ht);
    char* k;
    int count = 0;
    file_data_t *f;

    k = ht_iterate_keys(&it);
    while((max_files <= 0 || count < max_files) && k != NULL) {
        f = ht_get(ht, k);

        files[count].name = cmalloc(strlen(k) + 1);
        strcpy(files[count].name, k);
        files[count].length = f->length;
        files[count].file = cmalloc(f->length);
        strcpy(files[count].file, f->file);

        k = ht_iterate_keys(&it);
        count++;
    }

    return count;
}

int read_file(char *file_name, char **buf, size_t *size) {
    file_data_t *f = get_file(file_name);
    *size = f->length;
    *buf = cmalloc(*size);
    strncpy(*buf, f->file, *size);

    return 0;
}

void append_to_file(char *file_name, char *data, size_t size) {
    file_data_t *f = get_file(file_name);
    size_t total_size = f->length + size;
    char *new_file = cmalloc(total_size);

    strncpy(new_file, f->file, f->length);
    strncat(new_file, data, size);

    free(f->file);

    f->file = new_file;
    f->length = total_size;
}

int close_file(char *file_name) {
    file_data_t *f = get_file(file_name);

    if(f == NULL) return -1;

    f->opened = 0;
    return 0;
}

void close_file_manager() {
    hash_elem_it it = HT_ITERATOR(ht);
    char* k = ht_iterate_keys(&it);
    file_data_t *f;

    // Faccio la free di tutti i file
    while(k != NULL)
    {
        f = ht_remove(ht, k);
        free(f->file);
        free(f);
        k = ht_iterate_keys(&it);
    }

    ht_destroy(ht);
}

int check_memory_exced(size_t data_to_insert, int flags) {
    Pthread_mutex_lock(&stats_mtx);

    if(flags & CHECK_MEMORY_EXCEDED && stats.current_bytes_used + data_to_insert > config.max_memory_size)
        return 1;

    if(flags & CHECK_NFILES_EXCEDED && stats.current_files_saved >= config.max_files) {
        return 1;
    }

    Pthread_mutex_unlock(&stats_mtx);

    return 0;
}