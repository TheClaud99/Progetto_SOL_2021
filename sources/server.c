/**
 * @file    server_so.c
 * @brief   Contiene l'implementazione del server, tra cui caricamento dei dati config, salvataggio statistiche ed esecuzione delle richieste.
**/

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <strings.h>


#include "config.h"
#include "file.h"
#include "statistics.h"
#include "utils.h"


#define MAX_EVENTS      32
#define BUF_SIZE        16
#define WELCOME_MESSAGE "Benvenuto, sono in attesa di tue richieste."

struct epoll_event ev, events[MAX_EVENTS];

FILE *logfile;  // puntatore al file di log del client
pthread_mutex_t lofgile_mtx = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t statitiche_mtx = PTHREAD_MUTEX_INITIALIZER;

volatile sig_atomic_t chiusuraForte = 0; // termina immediatamente tutto
volatile sig_atomic_t chiusuraDebole = 0; // termina dopo la fine delle richieste client
pthread_mutex_t mutexChiusura = PTHREAD_MUTEX_INITIALIZER;

/*
 * register events of fd to epfd
 */
static void epoll_ctl_add(int epfd, int fd, uint32_t events) {
    ev.events = events;
    ev.data.fd = fd;
    ec_meno1(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev), "Epoll ctl");
}

static void set_sockaddr(struct sockaddr_un *addr) {
    memset(addr, 0, sizeof(*addr));
    addr->sun_family = AF_UNIX;
    strcpy(addr->sun_path, config.socket_file_name);
}


int create_socket(struct sockaddr_un* addr) {
    int listen_sock;

    // controllo se il file socket esiste già, nel caso lo elimino
    if( access( config.socket_file_name, F_OK ) == 0 ) {
        ec_meno1(remove(config.socket_file_name), "Delete socket file");
    }

    ec_meno1(listen_sock = socket(AF_UNIX, SOCK_STREAM, 0), "Socket");

    set_sockaddr(addr);
    ec_meno1(bind(listen_sock, (struct sockaddr *) addr, sizeof(*addr)), "Binding");

    ec_meno1(listen(listen_sock, config.max_connections), "Listen");

    return listen_sock;
}

/*
 * epoll echo server
 */
void server_run() {
    int i;
    int n;
    int epfd;
    int nfds;
    int listen_sock;
    int conn_sock;
    socklen_t socklen;
    char buf[BUF_SIZE];
    struct sockaddr_un srv_addr;
    struct sockaddr_un cli_addr;

    listen_sock = create_socket(&srv_addr);

    epfd = epoll_create1(0);
    epoll_ctl_add(epfd, listen_sock, EPOLLIN);

    socklen = sizeof(cli_addr);

    for (;;) {
        nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (i = 0; i < nfds; i++) {
            if (events[i].data.fd == listen_sock) {
                /* handle new connection */
                conn_sock = accept(listen_sock, (struct sockaddr *) &cli_addr, &socklen);

                epoll_ctl_add(epfd, conn_sock,EPOLLIN);
            } else if (events[i].events & EPOLLIN) {
                /* handle EPOLLIN event */
                for (;;) {
                    memset(buf, 0, sizeof(buf));
                    n = (int) read(events[i].data.fd, buf,
                             sizeof(buf));
                    if (n <= 0 /* || errno == EAGAIN */ ) {
                        break;
                    } else {
                        printf("[+] data: %s\n", buf);
                        write(events[i].data.fd, buf,
                              strlen(buf));
                    }
                }
            } else {
                printf("[+] unexpected\n");
            }
            /* check if the connection is closing */
            if (events[i].events & (EPOLLRDHUP | EPOLLHUP)) {
                printf("[+] connection closed\n");
                epoll_ctl(epfd, EPOLL_CTL_DEL,
                          events[i].data.fd, NULL);
                close(events[i].data.fd);
                continue;
            }
        }
    }

}

int main(int argc, char *argv[]) {

    if (argc >= 3 && strcmp(argv[1], "-conf") == 0) {
        char *config_filename = NULL;
        config_filename = argv[2];
        load_config(config_filename);
    } else {
        load_defaults();
    }

    server_run();

    printf("Max workers: %d", config.max_workers);
    return 0;
}