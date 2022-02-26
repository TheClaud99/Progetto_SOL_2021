/**
 * @file    statistics.c
 * @brief   Contiene l'implementazione della funzione print per le statistiche del server.
**/

#include "statistics.h"

char stats_path[] = "tests/outputs/Server/statistics.txt";
pthread_mutex_t stats_mtx = PTHREAD_MUTEX_INITIALIZER;
stats_t stats = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void print_stats() {
    float avg_bytes_read = 0, avg_bytes_written = 0;
    char *stat_string;
    FILE *f;


    if (stats.n_read != 0) avg_bytes_read = (float) stats.bytes_read / (float) stats.n_read;
    if (stats.n_write != 0) avg_bytes_written = (float) stats.bytes_written / (float) stats.n_write;

    ec_meno1(
            asprintf(
                    &stat_string,
                    "----- STATISTICHE SERVER -----\n"
                    "Media bytes letti: %g\n"
                    "Media bytes scritti: %g\n"
                    "Letture totali: %d\n"
                    "Scritture totali: %d\n"
                    "Lock totali: %d\n"
                    "Aperture totali: %d\n"
                    "Aperture&Create totali: %d\n"
                    "Unlock totali: %d\n"
                    "Cancellazioni totali: %d\n"
                    "Chiusure totali: %d\n"
                    "\nDimensione max file totale raggiunta: %fMB / %fMB\n"
                    "Numero massimo di file raggiunto: %d/%d\n"
                    "Numero di applicazioni dell'algoritmo di rimpiazzamento: %d\n"
                    "Numero di connessioni concorrenti totali: %d\n"
                    "Numero di connessioni bloccate: %d\n",
                    avg_bytes_read, avg_bytes_written, stats.n_read, stats.n_write, stats.n_lock, stats.n_open,
                    stats.n_opencreate, stats.n_unlock, stats.n_delete, stats.n_close,
                    (stats.max_size_reached / (double) 1000000), (config.max_memory_size / (double) 1000000),
                    stats.max_file_number_reached,
                    config.max_files, stats.n_replace_applied, stats.max_concurrent_connections,
                    stats.blocked_connections
            ),
            "asprintf"
    )

    // Stampo le statistiche sullo standard output
    puts(stat_string);
    fflush(stdout);

    // Scrivo le statistiche nel file dedicato
    ec_null(f = fopen(stats_path, "w"), "fopen")
    ec_cond(fputs(stat_string, f) != EOF, "fputs")
    ec_cond(fclose(f) != EOF, "fclose")

    free(stat_string);
}

void increase_nfiles() {
    Pthread_mutex_lock(&stats_mtx);

    stats.current_files_saved++;

    if (stats.max_file_number_reached < stats.current_files_saved) {
        stats.max_file_number_reached = stats.current_files_saved;
    }

    Pthread_mutex_unlock(&stats_mtx);
}

void decrease_nfiles() {
    Pthread_mutex_lock(&stats_mtx);
    stats.current_files_saved--;
    Pthread_mutex_unlock(&stats_mtx);
}

void increase_bytes_used(int nbytes) {
    Pthread_mutex_lock(&stats_mtx);

    stats.current_bytes_used += nbytes;

    if (stats.max_size_reached < stats.current_bytes_used) {
        stats.max_size_reached = stats.current_bytes_used;
    }

    Pthread_mutex_unlock(&stats_mtx);
}

void decrease_bytes_used(int nbytes) {
    Pthread_mutex_lock(&stats_mtx);
    stats.current_bytes_used -= nbytes;
    Pthread_mutex_unlock(&stats_mtx);
}

void increase_connections() {
    // Non c'è bisogno della lock perché questa statistica viene modificata solo dal thread master
    stats.current_connections++;

    if (stats.max_concurrent_connections < stats.current_connections) {
        stats.max_concurrent_connections = stats.current_connections;
    }

}

void decrease_connections() {
    // Non c'è bisogno della lock perché questa statistica viene modificata solo dal thread master
    stats.current_connections--;
}
