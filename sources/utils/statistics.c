/**
 * @file    statistics.c
 * @brief   Contiene l'implementazione della funzione print per le statistiche del server.
**/

#include "statistics.h"

#define BUFFER_STATS_SIZE 1024 // dimensione del buffer che contiene le statistiche da mettere nel file

pthread_mutex_t stats_mtx = PTHREAD_MUTEX_INITIALIZER;

int printStats(const char* pathname, int workers) {

    return 0;
}
