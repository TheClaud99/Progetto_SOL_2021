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

void write_file(char *file_name, char *data, size_t size) {
    file_data_t *f = get_file(file_name);
    f->file = cmalloc(size);
    strncpy(f->file, data, size);
    f->length = size;
}

void close_file_manager() {
    // todo liberare memeoria f->file per ogni elemento
    ht_clear(ht, 1);
    ht_destroy(ht);
}

int check_memory_exced(size_t data_to_insert) {
    Pthread_mutex_lock(&stats_mtx);

    if(stats.current_bytes_used + data_to_insert > config.max_memory_size)
        return 1;

    if(stats.current_files_saved > config.max_files) {
        return 1;
    }

    Pthread_mutex_unlock(&stats_mtx);

    return 0;
}