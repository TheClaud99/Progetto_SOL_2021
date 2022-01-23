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
        if ((r = (int) write((int) fd, bufptr, left)) == -1) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (r == 0) return 0;
        left -= r;
        bufptr += r;
    }
    return 1;
}

int send_message(int fd, char *message) {

    long msg_size = sizeof(*message);
    long ret;

    ec_meno1(ret = writen(fd, message, msg_size), "send message");

    warning_if(ret == msg_size, "Inviati %ld id %ld del messaggio %s", ret, msg_size, message)

    debug("Sended message %s", message)

    return 0;
}

int receive_message(int fd, char *message) {

    long msg_size = sizeof(*message);
    long ret;

    ec_meno1(ret = writen(fd, message, msg_size), "send message");

    warning_if(ret == msg_size, "Inviati %ld id %ld del messaggio %s", ret, msg_size, message)

    debug("Sended message %s", message)

    return 0;
}


int send_request(int fd, request_t request) {

    long msg_size = sizeof(request);
    long ret;

    ec_meno1(ret = write(fd, &request, msg_size), "send request");

    if (request.file_name_length > 0) {
        ec_meno1(writen(fd, request.file_name, request.file_name_length), "writen")
    }

    warning_if(ret != msg_size, "Inviati %ld di %ld bytes della richiesta %d", ret, msg_size, request.id)

    if (request.file_name_length > 0) {
        debug("Sended request {id: %d, size: %d, file_name: %s}", request.id, request.size, request.file_name)
    } else {
        debug("Sended request {id: %d, size: %d}", request.id, request.size)
    }

    return 0;
}

request_t receive_request(int fd) {

    request_t request = {REQ_NULL, 0, 0};
    long msg_size = sizeof(request_t);
    long ret;

    ec_meno1(ret = read(fd, &request, msg_size), "receive request");

    if (request.file_name_length > 0) {
        ec_meno1(readn(fd, request.file_name, request.file_name_length), "writen")
        debug("Received request {id: %d, size: %d, file_name: %s}", request.id, request.size, request.file_name)
    } else {
        debug("Received request {id: %d, size: %d}", request.id, request.size)
    }


    warning_if(ret != msg_size, "Ricevuti %ld di %ld bytes della richiesta", ret, msg_size)


    return request;
}


int send_response(int fd, response_t response) {

    long msg_size = sizeof(response_t);
    long ret;

    ec_meno1(ret = write(fd, &response, msg_size), "send response");

    warning_if(ret != msg_size, "Inviati %ld di %ld bytes della riposta %d", ret, msg_size, response)

    debug("Writed response %d", response)

    return 0;
}

response_t receive_response(int fd) {

    response_t response = RESP_NULL;
    long msg_size = sizeof(response_t);
    long ret;

    ec_meno1(ret = read(fd, &response, msg_size), "receive response");

    warning_if(ret != msg_size, "Ricevuti %ld di %ld bytes della risposta", ret, msg_size)

    debug("Received response %d", response)

    return response;
}


void write_files(char *files) {
    while (files != NULL) {


        files++;
    }
}