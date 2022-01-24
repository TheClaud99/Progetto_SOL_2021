/**
 * @file    communication.h
 * @brief   Contiene l'header delle funzioni usate per scambiare messaggi tra client e server.
**/

#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>

#include "utils.h"

/**
 *  RICHIESTA
**/

#define MAX_FILE_NAME_LEN 16

typedef enum {
    REQ_NULL = 0,
    REQ_OPEN = 12, // openFile
    REQ_READ = 13,
    REQ_READ_N = 14,
    REQ_WRITE = 15,
    REQ_APPEND = 16,
    REQ_LOCK = 17,
    REQ_UNLOCK = 18,
    REQ_CLOSE = 19,
    REQ_DELETE = 20,
} request_id_t;

typedef struct {
    request_id_t id; // Id della richisesta
    size_t size;    // Dimensione dei dati che saranno inviati dopo questa richiesta
    size_t file_name_length;    // Lunghezza del nome del file
    char file_name[MAX_FILE_NAME_LEN];    // File interessato dalla richiesta
} request_t;


/**
 *  RISPOSTA
**/

typedef enum {
    RESP_NULL,
    RESP_OK,
    RESP_SUCCES,
    RESP_FILE_EXITS,
    RESP_NO_PERMISSION,
    RESP_ERROR,
    RESP_FILE_EXISTS,
    RESP_FILE_NOT_EXISTS,
} response_t;

typedef enum {
    /**
     *  RISPOSTE SERVER -> CLIENT
    **/
    // generiche
    ANS_ERROR = -1,
    ANS_UNKNOWN = 0,
    ANS_NO_PERMISSION = 1,
    ANS_MAX_CONN_REACHED = 2,
    ANS_OK = 3,
    ANS_BAD_RQST = 4,

    // handshake server-client
    ANS_WELCOME = 5,
    ANS_HELLO = 6,

    // relative a file
    ANS_FILE_EXISTS = 7,
    ANS_FILE_NOT_EXISTS = 8,
    ANS_STREAM_START = 9,
    ANS_STREAM_FILE = 10,
    ANS_STREAM_END = 11,

    /**
     * AZIONI DEL CLIENT (servono per identificare l'operazione da svolgere)
    **/
    ACT_WRITE_DIR = 1, // -w
    ACT_WRITE_LIST = 2, // -W
    ACT_READ_LIST = 3, // -r
    ACT_READ_N_FILES = 4, // -R
    ACT_MUTEX_LOCK = 5, // -l
    ACT_MUTEX_UNLOCK = 6, // -u
    ACT_DELETE_LIST = 7, // -c
} action_t;

/**
    @brief  Struttura che descrive i messaggi che il server e i client si spediscono.

    @struct  action      Codice dell'azione riferita al messaggio
    @struct  flags       Intero che definisce il tipo di flag impostato (utile in alcuni casi)
    @struct  path        Array di caratteri che descrive il percorso riferito ad un file
    @struct  data_length Intero che definisce la lunghezza del dato da spedire
    @struct  data        Il dato da spedire
**/
/*typedef struct {
    ActionType action;
    int flags;

    char path[PATH_MAX];

    int data_length;
    char *data;
} Message;*/

int readn(long fd, void *buf, size_t size);

int writen(long fd, void *buf, size_t size);

request_t prepare_request(request_id_t id, size_t size, const char *file_path, int flags);

void write_files(char *files);

int send_message(int fd, char *message);

int recive_message(int fd, void *buf);

int send_response(int fd, response_t response);

response_t receive_response(int fd);

int send_request(int fd, request_t request);

request_t receive_request(int fd);

#endif /* COMMUNICATION_H_ */