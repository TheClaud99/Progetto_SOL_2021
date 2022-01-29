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
#include <sys/select.h>

#include "config.h"
#include "hashtable.h"
#include "statistics.h"
#include "communication.h"

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
    fd_set opened_by;   // Insieme dei file descriptor dei client che hanno il file aperto
} file_data_t;

typedef struct {
    char *name;
    char *file;
    size_t length;
} readn_ret_t;

extern hashtable_t *ht;

void init_file_manager();

int open_file(char *file_name, int client_fd);

int file_exists(char *file_name);

int add_file(char *file_name, int author);

file_data_t *get_file(char *file_name);

void write_file(char *file_name, char *data, size_t size);

int read_file(char *file_name, char **buf, size_t *size);

int read_random_file(char **buf, size_t *size, char filename[], int remove);

int readn_files(readn_ret_t files[], int max_files);

void append_to_file(char *file_name, char *data, size_t size);

int lockfile(char *file_name);

int unlockfile(char *file_name);

int close_file(char *file_name, int client_fd);

void close_file_manager();

int check_memory_exced(size_t data_to_insert, int flags);

#endif /* FILE_MANAGER_H_ */