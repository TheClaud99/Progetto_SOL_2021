/**
 * @file    api.c
 * @brief   Implementazione delle funzioni dell'api disponibili al client
**/

#include <string.h>
#include "api.h"

static void set_sockaddr(const char *sockname, struct sockaddr_un *addr) {

    // Controllo se il file socket esiste
    ec_meno1(access(sockname, F_OK), "File socket")

    ec_meno1(fd_socket = socket(AF_UNIX, SOCK_STREAM, 0), "Socket")

    memset(addr, 0, sizeof(*addr));
    addr->sun_family = AF_UNIX;
    strcpy(addr->sun_path, sockname);

}

// Aspetta un file dal socket e lo salva nella directory specificata
int read_and_save(const char *dirname, const char *file_name, size_t file_size) {
    FILE *f;
    char pathname[PATH_MAX];
    char *buf = cmalloc(file_size);

    // Aspetto che il server mandi i dati del file
    receive_message(fd_socket, buf, file_size);

    // Preparo il file in cui scrivere
    strcpy(pathname, dirname);
    if (pathname[PATH_MAX - 1] != '/') {
        strcat(pathname, "/");
    }
    strcat(pathname, file_name);
    ec_null(f = fopen(pathname, "wb"), "fopen read and save");

    // Scrivo nel file
    fwrite(buf, 1, file_size - 1, f); // size - 1 perché non scrivo il carattere di terminazione
    printf("Ricevuto file remoto '%s' (%ld bytes) salvato in '%s'", file_name, file_size, pathname);

    fclose(f);
    free(buf);

    return 0;
}

void receive_files(const char *dirname, int nfiles) {
    request_t server_request;
    int count = 0;
    server_request = receive_request(fd_socket);
    send_response(fd_socket, RESP_OK);
    // Ricevo file finché il server continua a mandare richeiste o
    // finché non si esaurisce il contatore.
    // se nfiles è 0 continuo a ricevere finché il server iniva file
    while (server_request.id == REQ_SEND_FILE && (nfiles <= 0 || count < nfiles)) {
        read_and_save(dirname, server_request.file_name, server_request.size);
        send_response(fd_socket, RESP_OK);
        server_request = receive_request(fd_socket);
        count++;
    }
}

void set_errno(response_t response) {
    switch (response) {
        case RESP_OK: {
            break;
        }

        case RESP_FILE_EXISTS: {
            errno = EEXIST;
            break;
        }

        case RESP_NO_PERMISSION: {
            errno = EACCES;
            break;
        }

        case RESP_FILE_NOT_EXISTS: {
            errno = ENOENT;
            break;
        }

        case RESP_ERROR: {
            errno = EREMOTEIO;
            break;
        }

        default: {
            errno = EBADRQC;
        }
    }
}

int openConnection(const char *sockname, int msec, const struct timespec abstime) {
    struct sockaddr_un sa;
    response_t response;

    long start = millis(), now, msabstime = toms(abstime);

    set_sockaddr(sockname, &sa);

    while (connect(fd_socket, (struct sockaddr *) &sa, sizeof(sa)) == -1) {

        if (errno == ENOENT) {
            ec_meno1(msleep(msec), "msleep");
        } else {
            PERROR("connect")
        }

        now = millis();
        if((now - start) > msabstime) break;
    }

    response = receive_response(fd_socket);
    if (response == RESP_SUCCES) {
        puts("Connessione avvenuta con successo");
    }

    return 0;
}


int closeConnection(const char *sockname) {
    ec_meno1(close(fd_socket), "Close");

    debug("Connessione chiusa con %s, fd: %d", sockname, fd_socket)

    return 0;
}


int openFile(const char *pathname, int flags) {

    response_t response;
    request_t request = prepare_request(REQ_OPEN, 0, pathname, flags);

    send_request(fd_socket, request);
    response = receive_response(fd_socket);

    if(response == RESP_SUCCES) {
        return 0;
    }


    set_errno(response);
    return -1;
}


int readFile(const char *pathname, void **buf, size_t *size) {

    response_t response;
    request_t request = prepare_request(REQ_READ, 0, pathname, 0);

    send_request(fd_socket, request);
    response = receive_response(fd_socket);

    if(response == RESP_OK) {

        request_t server_request;
        server_request = receive_request(fd_socket);

        send_response(fd_socket, RESP_OK);

        *size = server_request.size;
        *buf = cmalloc(*size);
        receive_message(fd_socket, *buf, *size);

        return 0;
    }


    set_errno(response);
    return -1;
}


int readNFiles(int N, const char *dirname) {

    response_t response;
    request_t request = prepare_request(REQ_READ_N, N, NULL, 0);

    send_request(fd_socket, request);
    response = receive_response(fd_socket);

    if(response == RESP_OK) {
        receive_files(dirname, N);
    }

    return 0;
}


int writeFile(const char *pathname, const char *dirname) {

    request_t request;
    response_t response;
    FILE *file;
    size_t size;
    struct stat file_stat; // Informazioni sul file

    // Apro il file e prendo le informazioni su di esso
    ec_null(file = fopen(pathname, "rb"), "writeFile fopen")
    ec_meno1(stat(pathname, &file_stat), "writeFile stat")

    size = file_stat.st_size + 1; // Imposto la dimensione di un byte in più per il carattere di terminazinoe '\0'

    // Leggo il contenuto del file in un buffer temporaneo
    char *buffer = cmalloc(size);
    ec_cond(file_stat.st_size == fread(buffer, 1, file_stat.st_size, file), "writeFile fread")
    fclose(file);

    // Mando la richiesta per porter scrivere il file
    request = prepare_request(REQ_WRITE, size, pathname, 0);
    send_request(fd_socket, request);

    // Aspetto una risposta dal server
    response = receive_response(fd_socket);

    if (response == RESP_FULL) { // Il server è pieno e deve espellere dei file
        receive_files(dirname, 0);
        response = RESP_OK;
    }

    if (response == RESP_OK) { // Il server ha dato l'ok
        send_message(fd_socket, buffer, size);
        free(buffer);
        return 0;
    }

    free(buffer);
    set_errno(response);

    return -1;
}


int appendToFile(const char *pathname, void *buf, size_t size, const char *dirname) {
    response_t response;
    request_t request = prepare_request(REQ_APPEND, size, pathname, 0);
    send_request(fd_socket, request);

    response = receive_response(fd_socket);

    if (response == RESP_FULL) { // Il server è pieno e deve espellere dei file
        receive_files(dirname, 0);
        response = RESP_OK;
    }

    if (response == RESP_OK) {
        send_message(fd_socket, buf, size);
        return 0;
    }

    set_errno(response);

    return -1;
}


int lockFile(const char *pathname) {
    response_t response;
    request_t request = prepare_request(REQ_LOCK, 0, pathname, 0);
    send_request(fd_socket, request);

    response = receive_response(fd_socket);
    if (response == RESP_SUCCES) {
        return 0;
    }

    set_errno(response);

    return -1;
}


int unlockFile(const char *pathname) {
    response_t response;
    request_t request = prepare_request(REQ_UNLOCK, 0, pathname, 0);
    send_request(fd_socket, request);

    response = receive_response(fd_socket);
    if (response == RESP_SUCCES) {
        return 0;
    }

    set_errno(response);

    return -1;
}


int closeFile(const char *pathname) {
    response_t response;
    request_t request = prepare_request(REQ_CLOSE, 0, pathname, 0);
    send_request(fd_socket, request);

    response = receive_response(fd_socket);
    if (response == RESP_SUCCES) {
        return 0;
    }

    set_errno(response);

    return -1;
}


int removeFile(const char *pathname) {
    response_t response;
    request_t request = prepare_request(REQ_DELETE, 0, pathname, 0);
    send_request(fd_socket, request);

    response = receive_response(fd_socket);
    if (response == RESP_SUCCES) {
        return 0;
    }

    set_errno(response);

    return -1;
}