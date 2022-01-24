#include "file_manager.h"

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

void write_file(char *file_name, char *data) {
    file_data_t *f = get_file(file_name);
    strcpy(f->file, data);
    f->length = strlen(data);
}

void close_file_manager() {
    ht_destroy(ht);
}