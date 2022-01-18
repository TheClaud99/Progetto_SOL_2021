/**
 * @file    server_so.c
 * @brief   Contiene l'implementazione del server, tra cui caricamento dei dati config, salvataggio statistiche ed esecuzione delle richieste.
**/

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "config.h"
#include "file.h"
#include "ini.h"
#include "statistics.h"
#include "utils.h"

#define WELCOME_MESSAGE "Benvenuto, sono in attesa di tue richieste."

FILE *logfile;  // puntatore al file di log del client
pthread_mutex_t lofgile_mtx = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t statitiche_mtx = PTHREAD_MUTEX_INITIALIZER;

volatile sig_atomic_t chiusuraForte = 0; // termina immediatamente tutto
volatile sig_atomic_t chiusuraDebole = 0; // termina dopo la fine delle richieste client
pthread_mutex_t mutexChiusura = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]) {

    char *config_filename = NULL;

    if (argc >= 3 && strcmp(argv[1], "-conf") == 0)
    {
        config_filename = argv[2];
        load_config(config_filename);
    }


    printf("Max workers: %d", config.max_workers);
    return 0;
}