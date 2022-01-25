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