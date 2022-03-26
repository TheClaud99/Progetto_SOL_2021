/**
 * @file    config.h
 * @brief   Contiene l'header delle funzioni per caricare il config e la struttura che contiene il config.
**/

#ifndef CONFIG_H_
#define CONFIG_H_

#define  _POSIX_C_SOURCE 200809L

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <linux/limits.h>

#include "utils.h"
#include "ini.h"

/**
    @brief  Contiene i parametri di configurazione del server.
**/
typedef struct configurazione {
    int max_workers;
    int max_connections;
    int max_memory_size;
    int max_files;
    char socket_file_name[PATH_MAX];
    char logfile[PATH_MAX];
} config_t;

// Configurazione globale
extern config_t config;

// Carica le impostazioni di default se non viene specificato un file di configurazione
int load_defaults();

// Dato il nome del file di configurazione, carica la i parametri nella config globale
int load_config(char *filename);

#endif /* CONFIG_H_ */