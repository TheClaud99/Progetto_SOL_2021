/**
 * @file    api.c
 * @brief   Implementazione delle funzioni dell'api disponibili al client
**/

#include "api.h"
#include "communication.h"

static void set_sockaddr(const char *sockname, struct sockaddr_un *addr) {

    // Controllo se il file socket esiste
    ec_meno1(access(sockname, F_OK), "File socket")

    ec_meno1(fd_socket = socket(AF_UNIX, SOCK_STREAM, 0), "Socket")

    memset(addr, 0, sizeof(*addr));
    addr->sun_family = AF_UNIX;
    strcpy(addr->sun_path, sockname);

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
    return 0;
}


int openFile(const char *pathname, int flags) {
    return 0;
}


int readFile(const char *pathname, void **buf, size_t *size) {
    return 0;
}


int readNFiles(int N, const char *dirname) {
    return 0;
}


int writeFile(const char *pathname, const char *dirname) {
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