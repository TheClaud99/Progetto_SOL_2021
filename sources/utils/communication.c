/**
 * @file    communication.c
 * @brief   Contiene l'implementazione di alcune funzioni che permettono lo scambio di messaggi tra client e server.
**/

#include "communication.h"

int readn(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char *) buf;
    while (left > 0) {
        if ((r = (int) read((int) fd, bufptr, left)) == -1) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (r == 0) return 0;   // gestione chiusura socket
        left -= r;
        bufptr += r;
    }
    return (int) size;
}

int writen(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char *) buf;
    while (left > 0) {
        if ((r = write((int) fd, bufptr, left)) == -1) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (r == 0) return 0;
        left -= r;
        bufptr += r;
    }
    return 1;
}

int send_message(int fd, void *message) {

    long msg_size = sizeof(*message);

    // mando sempre tutto il messaggio (il body sarà vuoto)
    long ret = write(fd, message, msg_size);

    ec_cond(ret == msg_size, "send message")

    return 0;
}

void write_files(char *files) {
    while (files != NULL) {



        files++;
    }
}