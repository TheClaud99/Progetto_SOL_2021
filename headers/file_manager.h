/**
 * @file    file_manager.h
 * @brief   Contiene l'header delle funzioni per caricare il config e la struttura che contiene il config.
**/


#ifndef FILE_MANAGER_H_
#define FILE_MANAGER_H_

#define _GNU_SOURCE

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
#include "hashtable.h"
#include "statistics.h"

#define CHECK_MEMORY_EXCEDED    1
#define CHECK_NFILES_EXCEDED     2

typedef struct {
    int fd; // Proprietario del file
    char *file;
    size_t length;
    time_t update_date;
    int author;
    int locked;
    int last_action;
    int opened;
} file_data_t;

extern hashtable_t *ht;

void init_file_manager();

file_data_t *add_file(char *file_name, int author);

file_data_t *get_file(char *file_name);

void write_file(char *file_name, char *data, size_t size);

void append_to_file(char *file_name, char *data, size_t size);

int lockfile(char *file_name);

int unlockfile(char *file_name);

int close_file(char *file_name);

void close_file_manager();

int check_memory_exced(size_t data_to_insert, int flags);

#endif /* FILE_MANAGER_H_ */