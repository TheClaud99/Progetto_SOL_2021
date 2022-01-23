#include "file_manager.h"

void init_file_manager() {
    files_data = calloc(config.max_files, sizeof(file_data_t));
}

void close_file_manager() {
    free(files_data);
}