/**
 * @file    communication.h
 * @brief   Contiene l'header delle funzioni usate per scambiare messaggi tra client e server.
**/

#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>

#include "utils.h"
#include "statistics.h"

/**
 *  RICHIESTA
**/


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

    REQ_SEND_FILE, // Richiseta speciale che fa il server al client per espellere file
} request_id_t;

typedef struct {
    request_id_t id; // Id della richisesta
    int flags; // Id della richisesta
    size_t size;    // Dimensione dei dati che saranno inviati dopo questa richiesta
    size_t file_name_length;    // Lunghezza del nome del file
    char *file_name;    // File interessato dalla richiesta
} request_t;


#define O_OPEN      0
#define O_CREATE    1
#define O_LOCK      2

#define ec_response(c, fd)          \
    if ((c) == -1)                  \
    {                               \
        send_response_on_error(fd); \
        break;                      \
    }


/**
 *  RISPOSTA
**/

typedef enum {
    RESP_NULL,
    RESP_OK,
    RESP_SUCCES,
    RESP_NO_PERMISSION,
    RESP_ERROR,
    RESP_FILE_EXISTS,
    RESP_FILE_NOT_EXISTS,
    RESP_FILE_NOT_OPENED,
    RESP_FULL
} response_t;

extern int is_server;

int readn(long fd, void *buf, size_t size);

int writen(long fd, void *buf, size_t size);

request_t prepare_request(request_id_t id, size_t size, const char *file_path, int flags);

int send_message(int fd, void *message, size_t size);

int receive_message(int fd, void *buf, size_t size);

int send_response(int fd, response_t response);

response_t receive_response(int fd);

void send_response_on_error(int fd);

int send_request(int fd, request_t request);

request_t receive_request(int fd);

#endif /* COMMUNICATION_H_ */