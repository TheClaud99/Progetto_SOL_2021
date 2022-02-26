/**
 * @file    statistics.h
 * @brief   Contiene l'implementazione della struct statistics per le statistiche del server.
**/

#ifndef STATISTICS_H_
#define STATISTICS_H_

#define _GNU_SOURCE

#include <stdio.h>

#include "config.h"

/**
 * @brief   Struttura che definisce l'insieme di statistiche salvate dal server.
*/
typedef struct {
    int n_read;
    int n_write;
    int n_lock;
    int n_open;
    int n_opencreate;
    int n_unlock;
    int n_delete;
    int n_close;

    int max_size_reached;
    int max_file_number_reached;
    int n_replace_applied;

    int max_concurrent_connections;

    int blocked_connections;
    int current_connections;
    int current_bytes_used;
    int current_files_saved;

    int bytes_read;
    int bytes_written;

    int *workerRequests; // contiene l'array contenente le richieste servite da ogni worker
} stats_t;

extern stats_t stats;
extern pthread_mutex_t stats_mtx;

void print_stats();

void increase_nfiles();

void decrease_nfiles();

void increase_bytes_used(int nbytes);

void decrease_bytes_used(int nbytes);

void increase_connections();

void decrease_connections();

#endif /* STATISTICS_H_ */