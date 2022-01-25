
#include "config.h"

int load_defaults() {
    config.max_workers = 1;
    config.max_connections = 5;
    config.max_memory_size = 128000000;
    config.max_files = 10000;
    strcpy(config.socket_file_name, "sock_file.sk");

    return 0;
}

// Dato il nome del file di configurazione, carica la i parametri nella config globale
int load_config(char *filename) {

    ec_meno1(access(config.socket_file_name, F_OK), "caricamento file di configurazione")

    ini_t *config_file = ini_load(filename);

    config.max_workers = (int) strtol(ini_get(config_file, "settings", "max_workers"), NULL, 10);
    config.max_connections = (int) strtol(ini_get(config_file, "settings", "max_connections"), NULL, 10);
    config.max_memory_size = (int) strtol(ini_get(config_file, "settings", "max_memory_size"), NULL, 10);
    config.max_files = (int) strtol(ini_get(config_file, "settings", "max_files"), NULL, 10);
    strcpy(config.socket_file_name, ini_get(config_file, "settings", "socket_file_name"));
    ini_free(config_file);

    return 0;
}
