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
    void *file;
    size_t length;
    time_t update_date;
    int author;
    int locked_by;
    int last_action;
    fd_set opened_by;           // Insieme dei file descriptor dei client che hanno il file aperto
    pthread_mutex_t mtx;        // Lock per accedere al file in maniera concorrente
    pthread_cond_t lock_cond;   // Condizione segnalata al rilascio di locked_by
} file_data_t;

typedef struct {
    char *name;
    void *file;
    size_t length;
} readn_ret_t;

extern hashtable_t *ht;

void init_file_manager();

int open_file(char *file_name, int lock, int client_fd);

int file_exists(char *file_name);

int add_file(char *file_name, int lock, int author);

int remove_file(char* file_name, int client_fd);

int remove_first_file(void** buf, size_t *size, char **file_name, int client_fd);

file_data_t *get_file(char *file_name);

int write_file(char *file_name, void *data, size_t size, int client_fd);

int read_file(char *file_name, void **buf, size_t *size, int client_fd);

int read_random_file(void **buf, size_t *size, char filename[], int remove);

int readn_files(readn_ret_t files[], int max_files);

int append_to_file(char *file_name, void *data, size_t size, int client_fd);

int lockfile(char *file_name, int client_fd);

int unlockfile(char *file_name, int client_fd);

int close_file(char *file_name, int client_fd);

void close_file_manager();

int check_memory_exced(size_t data_to_insert, int flags);

#endif /* FILE_MANAGER_H_ */