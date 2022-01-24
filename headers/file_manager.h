/**
 * @file    file_manager.h
 * @brief   Contiene l'header delle funzioni per caricare il config e la struttura che contiene il config.
**/


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
#include "hashtable.h"


typedef struct {
    int fd; // Proprietario del file
    char* file;
    size_t length;
    time_t update_date;
    int author;
    int locked;
    int last_action;
    int opened;
} file_data_t;

hashtable_t *ht;

void init_file_manager();

file_data_t* add_file(char* file_name, int author);

file_data_t* get_file(char *file_name);

void close_file_manager();

#endif /* FILE_MANAGER_H_ */