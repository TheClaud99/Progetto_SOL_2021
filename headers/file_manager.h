#ifndef FILE_MANAGER_H_
#define FILE_MANAGER_H_

#define _POSIX_C_SOURCE 200809L

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

#include "config.h"


typedef struct {
    int fd; // Proprietario del file
    FILE* file;
    int locked;
} file_data_t;

file_data_t *files_data;

void init_file_manager();

void close_file_manager();

#endif /* FILE_MANAGER_H_ */