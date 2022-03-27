/**
 * @file    file_manager.h
 * @brief   Fornisce un'interfaccia per una gestione thread safe dei file
**/


#ifndef FILE_MANAGER_H_
#define FILE_MANAGER_H_

#define  _POSIX_C_SOURCE 200809L

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
    int waiters;                // Thread in attesa di impostare il locked_by
    int delete;                 // Se 1, vuol dire che il file è in attesa di essere eliminato
    fd_set opened_by;           // Insieme dei file descriptor dei client che hanno il file aperto
    pthread_mutex_t mtx;        // Lock per accedere al file in maniera concorrente
    pthread_cond_t lock_cond;   // Condizione segnalata al rilascio di locked_by
    struct timespec last_use;   // Ultima data di utilizzo del file per aprtura/lettura/scrittura
} file_data_t;

typedef struct {
    char *name;
    void *file;
    size_t length;
} readn_ret_t;

// Hash table utilizzata per contenere i file
extern hashtable_t *ht;

/**
 * @brief Inizializza i parametri del file manager e crea la hashtable
**/
void init_file_manager();

/**
 * @brief Apre il file specificato
 *
 * @param file_name     nome del file da aprire
 * @param lock          se impostato, apre il file e ne acquisice la lock
 * @param client_fd     fd del client che sta effettuando l'operazione
 *
 * @returns 0 in caso di successo, -1 altrimenti
**/
int open_file(char *file_name, int lock, int client_fd);

/**
 * @brief Aggiunge un file al file manager
 *
 * @param file_name     nome del file da aggiungere
 * @param lock          se impostato aggiunge il file già con la lock acquisita
 * @param client_fd     fd del client che sta effettuando l'operazione
 *
 * @returns 0 in caso di successo, -1 altrimenti
**/
int add_file(char *file_name, int lock, int author);

/**
 * @brief Rimuove un file
 *
 * @param file_name     nome del file da rimuovere
 * @param client_fd     fd del client che sta effettuando l'operazione
 *
 * @returns 0 in caso di successo, -1 altrimenti
**/
int remove_file(char* file_name, int client_fd);

/**
 * @brief Applica l'algoritmo LRU per scegliere un file da rimuovere e lo espelle dentro buf
 *
 * @param buf           buffer in cui saranno salvati i dati del file rimosso
 * @param size          dimensione del file rimosso
 * @param file_name     nome del file rimosso
 * @param client_fd     fd del client che sta effettuando l'operazione
 *
 * @returns 0 in caso di successo, -1 altrimenti
**/
int remove_LRU(void** buf, size_t *size, char **file_name, int client_fd);

/**
 * @brief Scrive i dati dentro un file
 *
 * @param file_name     nome del file in cui scrivere
 * @param data          buffer dei dati da scrivere
 * @param size          dimensione dei dati da scrivere
 * @param client_fd     fd del client che sta effettuando l'operazione
 *
 * @returns 0 in caso di successo, -1 altrimenti
**/
int write_file(char *file_name, void *data, size_t size, int client_fd);

/**
 * @brief Legge i dati da un file
 *
 * @param file_name     nome del file da cui leggere
 * @param buf           buffer in cui saranno salvati i dati letti dal file
 * @param size          dimensione dei dati letti
 * @param client_fd     fd del client che sta effettuando l'operazione
 *
 * @returns 0 in caso di successo, -1 altrimenti
**/
int read_file(char *file_name, void **buf, size_t *size, int client_fd);

/**
 * @brief Legge randomicamente un massimo di max_files di file salvati.
 *
 * @param files         array in cui saranno ritornati i file letti insieme al loro nome e alla loro dimensione
 * @param max_files     numero massimo di file da leggere. Se max_files <= 0 allora legge tutti i file presenti
 *
 * @returns 0 in caso di successo, -1 altrimenti
**/
int readn_files(readn_ret_t files[], int max_files);

/**
 * @brief Scrive in append dentro al file specificato da file_name
 *
 * @param file_name     nome del file in cui scrivere
 * @param data          buffer dei dati da scrivere
 * @param size          dimensione dei dati da scrivere
 * @param client_fd     fd del client che sta effettuando l'operazione
 *
 * @returns 0 in caso di successo, -1 altrimenti
**/
int append_to_file(char *file_name, void *data, size_t size, int client_fd);

/**
 * @brief Acquisice la lock sul file specificato da file name
 *
 * @param file_name     nome del file di cui acquisire la lock
 * @param client_fd     fd del client che sta effettuando l'operazione
 *
 * @returns 0 in caso di successo, -1 altrimenti
**/
int lockfile(char *file_name, int client_fd);

/**
 * @brief Rilascia la lock sul file specificato da file name
 *
 * @param file_name     nome del file di cui rilasciare la lock
 * @param client_fd     fd del client che sta effettuando l'operazione
 *
 * @returns 0 in caso di successo, -1 altrimenti
**/
int unlockfile(char *file_name, int client_fd);

/**
 * @brief Imposta a 1 il flag di chiusura del file manager e risveglia tutti i thread che sono in attesa di
 * acquisire la lock
**/
void wakeup_threads();

/**
 * @brief Chiude il file specificato
 *
 * @param file_name     nome del file da chiudere
 * @param client_fd     fd del client che sta effettuando l'operazione
 *
 * @returns 0 in caso di successo, -1 altrimenti
**/
int close_file(char *file_name, int client_fd);

/**
 * @brief Chiude il file manager facendo la free di tutti i file presenti
**/
void close_file_manager();

/**
 * @brief Controlla se inserendo una certa quantità di dati ed un file in più, il file manager oltrepaserà
 * il limite di spazio consentito
 *
 * @param data_to_insert    dimensione dei dati che si sta provando ad inserire
 * @param flags             flag aggiuntivi. Se impostato CHECK_MEMORY_EXCEDED viene controllata la quantità di bytes,
 * se impostato CHECK_NFILES_EXCEDED viene controllato il numero di file presenti
 *
 * @returns 0 in caso si sia sempre entro i limit, 1 altrimenti
**/
int check_memory_exced(size_t data_to_insert, int flags);

#endif /* FILE_MANAGER_H_ */