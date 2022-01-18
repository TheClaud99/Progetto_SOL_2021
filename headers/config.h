/**
 * @file    config.h
 * @brief   Contiene l'header delle funzioni per caricare il config e la struttura che contiene il config.
 * @author  Leonardo Pantani
**/

#ifndef CONFIG_H_
#define CONFIG_H_

#define  _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <linux/limits.h>

#include "utils.h"
#include "ini.h"

#define KEY "efhiauefrwhwafghaw2131guys" // chiave perché la configurazione venga letta dal server, è una specie di password

#define EXPECTED_CONF_VARS 8 // va considerata anche la chiave del server, quindi è +1

/**
    @brief  Contiene l'insieme di errori che possono verificarsi durante la lettura del config.
**/
enum ErroriConfig {
    ERR_FILEOPENING = -1,
    ERR_INVALIDKEY = -2,
    ERR_NEGVALUE = -3,
    ERR_EMPTYVALUE = -4,
    ERR_UNSETVALUES = -5,
    ERR_ILLEGAL = -6,
};

/**
    @brief  Contiene i parametri di configurazione del server.
**/
typedef struct configurazione {
    int max_workers;
    int max_connections;
    int max_memory_size;
    int max_files;
    char socket_file_name[PATH_MAX];
    /*char log_file_name[PATH_MAX];
    char stats_file_name[PATH_MAX];*/
} Config;

Config config;

// Dato il nome del file di configurazione, carica la i parametri nella config globale
int load_config(char *filename) {
    ini_t *config_file = ini_load(filename);

    config.max_workers = (int) strtol(ini_get(config_file, "settings", "max_workers"), NULL, 10);
    config.max_connections = (int) strtol(ini_get(config_file, "settings", "max_connections"), NULL, 10);
    config.max_memory_size = (int) strtol(ini_get(config_file, "settings", "max_memory_size"), NULL, 10);
    strcpy(config.socket_file_name, ini_get(config_file, "settings", "max_memory_size"));
    ini_free(config_file);

    return 0;
}

#endif /* CONFIG_H_ */