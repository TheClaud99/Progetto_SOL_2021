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

request_t prepare_request(request_id_t id, size_t size, const char *file_path, int flags) {
    request_t request;
    memset(&request, 0, sizeof(request_t));

    request.id = id;
    request.size = size;
    request.flags = flags;
    request.file_name_length = 0;
    request.file_name = NULL;

    if (file_path != NULL) {
        if (is_server != 1) {
            request.file_name_length = relative2absolute(&request.file_name, file_path);
        } else {
            request.file_name_length = get_file_name(&request.file_name, file_path);
        }
    }
    return request;
}

int send_message(int fd, void *message, size_t size) {
    ec_meno1(writen(fd, message, size), "send message")

    debug("Sended message to %d", fd)

    Pthread_mutex_lock(&stats_mtx);
    stats.n_write++;
    stats.bytes_written += (int) size;
    Pthread_mutex_unlock(&stats_mtx);

    return 0;
}

int receive_message(int fd, void *message, size_t size) {

    ec_meno1(readn(fd, message, size), "receive message")

    debug("Received message from %d", fd)

    Pthread_mutex_lock(&stats_mtx);
    stats.n_read++;
    stats.bytes_read += (int) size;
    Pthread_mutex_unlock(&stats_mtx);

    return 0;
}


int send_request(int fd, request_t request) {

    long msg_size = sizeof(request);
    long ret;

    ec_meno1(ret = write(fd, &request, msg_size), "send request")

    if (request.file_name_length > 0) {
        ec_meno1(writen(fd, request.file_name, request.file_name_length), "writen")
        ret += (long) request.file_name_length;
    }

    warning_if(ret < msg_size, "Inviati %ld di %ld bytes della richiesta %d", ret, msg_size, request.id)

    if (request.file_name_length > 0) {
        debug("Sended request {id: %d, size: %d, file_name: %s} to %d", request.id, request.size, request.file_name, fd)
    } else {
        debug("Sended request {id: %d, size: %d} to %d", request.id, request.size, fd)
    }

    Pthread_mutex_lock(&stats_mtx);
    stats.n_write++;
    stats.bytes_written += (int) ret;
    Pthread_mutex_unlock(&stats_mtx);

    return 0;
}

request_t receive_request(int fd) {

    request_t request = {REQ_NULL, 0, 0};
    long msg_size = sizeof(request_t);
    long ret;

    ec_meno1(ret = read(fd, &request, msg_size), "receive request")

    if (request.file_name_length > 0) {
        request.file_name = cmalloc(request.file_name_length);
        ec_meno1(readn(fd, request.file_name, request.file_name_length), "readn")
        debug("Received request {id: %d, size: %d, file_name: %s} from %d", request.id, request.size, request.file_name,
              fd)
        ret += (long) request.file_name_length;
    } else {
        debug("Received request {id: %d, size: %d} from %d", request.id, request.size, fd)
    }


    warning_if(ret < msg_size, "Ricevuti %ld di %ld bytes della richiesta", ret, msg_size)

    Pthread_mutex_lock(&stats_mtx);
    stats.n_read++;
    stats.bytes_read += (int) ret;
    Pthread_mutex_unlock(&stats_mtx);

    return request;
}


int send_response(int fd, response_t response) {

    long msg_size = sizeof(response_t);
    long ret;

    ec_meno1(ret = write(fd, &response, msg_size), "send response")

    warning_if(ret < msg_size, "Inviati %ld di %ld bytes della riposta %d", ret, msg_size, response)

    debug("Writed response %d", response)

    Pthread_mutex_lock(&stats_mtx);
    stats.n_write++;
    stats.bytes_written += (int) ret;
    Pthread_mutex_unlock(&stats_mtx);

    return 0;
}

void send_response_on_error(int fd) {
    switch (errno) {
        case ENOENT: {
            Info("Il client %d ha provato ad accedere ad un file inesistente", fd)
            send_response(fd, RESP_FILE_NOT_EXISTS);
            break;
        }
        case EEXIST: {
            Info("Il client %d ha provato a creare un file già esistente", fd)
            send_response(fd, RESP_FILE_EXISTS);
            break;
        }
        case EPERM: {
            Info("Il client %d ha provato ad accedere ad un file senza prima aprirlo", fd)
            send_response(fd, RESP_FILE_NOT_OPENED);
            break;
        }
        case EACCES: {
            Info("Il client %d ha provato ad eseguire un'operazione per cui è richiesta la lock, senza possderla", fd)
            send_response(fd, RESP_NO_PERMISSION);
            break;
        }
        default: {
            Info("Il client %d ha riscontrato un errore generico", fd)
            send_response(fd, RESP_ERROR);
        }
    }
}

response_t receive_response(int fd) {

    response_t response = RESP_NULL;
    long msg_size = sizeof(response_t);
    long ret;

    ec_meno1(ret = read(fd, &response, msg_size), "receive response")

    warning_if(ret < msg_size, "Ricevuti %ld di %ld bytes della risposta", ret, msg_size)

    debug("Received response %d", response)

    Pthread_mutex_lock(&stats_mtx);
    stats.n_read++;
    stats.bytes_read += (int) ret;
    Pthread_mutex_unlock(&stats_mtx);


    return response;
}
