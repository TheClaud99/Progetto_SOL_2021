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
    FILE* f;
    char pathname[PATH_MAX];
    char *buf = cmalloc(file_size);

    // Aspetto che il server mandi i dati del file
    receive_message(fd_socket, buf, file_size);

    // Preparo il file in cui scrivere
    strcpy(pathname, dirname);
    if(pathname[PATH_MAX - 1] != '/') {
        strcat(pathname, "/");
    }
    strcat(pathname, file_name);
    ec_null(f = fopen(pathname, "wb"), "fopen read and save");

    // Scrivo nel file
    fwrite(buf, 1, file_size, f);
    printf("Ricevuto file remoto '%s' (%ld bytes) salvato in '%s'", file_name, file_size, pathname);

    fclose(f);
    free(buf);

    return 0;
}

int openConnection(const char *sockname, int msec, const struct timespec abstime) {
    char buf[BUFSIZ];
    struct sockaddr_un sa;
    int n;
    int elapsed_time = 0;
    response_t response;

    set_sockaddr(sockname, &sa);

    while (connect(fd_socket, (struct sockaddr *) &sa, sizeof(sa)) == -1) {
        if (errno == ENOENT) {
            ec_meno1(msleep(msec), "msleep");
        } else {
            PERROR("connect")
        }
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

    // le flag passate non sono valide (0 -> open | 1 -> open & create)
    if (flags != O_CREATE && flags != O_LOCK) {
        errno = EINVAL;
        return -1;
    }

    request_t request = prepare_request(REQ_OPEN, 0, pathname, flags);

    send_request(fd_socket, request);
    response = receive_response(fd_socket);

    switch (response) {
        case RESP_OK: {
            debug("Inizia trasferimento file")
            break;
        }

        case RESP_FILE_EXISTS: {
            debug("Il file %s, esiste gia'", pathname)
            errno = EEXIST;
            return -1;
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


    return 0;
}


int readFile(const char *pathname, void **buf, size_t *size) {
    return 0;
}


int readNFiles(int N, const char *dirname) {
    return 0;
}


int writeFile(const char *pathname, const char *dirname) {

    request_t request;
    response_t response;
    FILE* file;
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

    if(response == RESP_FULL) { // Il server è pieno e deve espellere dei file
        request_t server_request;
        server_request = receive_request(fd_socket);
        while(server_request.id == REQ_SEND_FILE) {
            read_and_save(dirname, server_request.file_name, server_request.size);
            send_response(fd_socket, RESP_OK);
            server_request = receive_request(fd_socket);
        }
    }

    if(response == RESP_OK) { // Il server ha dato l'ok
        send_message(fd_socket, buffer, size);
    }

    free(buffer);

    return 0;
}


int appendToFile(const char *pathname, void *buf, size_t size, const char *dirname) {
    return 0;
}


int lockFile(const char *pathname) {
    return 0;
}


int unlockFile(const char *pathname) {
    return 0;
}


int closeFile(const char *pathname) {
    return 0;
}


int removeFile(const char *pathname) {
    return 0;
}