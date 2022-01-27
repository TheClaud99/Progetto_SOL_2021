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
#include "statistics.h"
#include "utils.h"
#include "communication.h"
#include "file_manager.h"
#include "tpool.h"


#define MAX_EVENTS      32
#define BUF_SIZE        16
#define WELCOME_MESSAGE "Connessione avvenuta con successo"

struct epoll_event ev, events[MAX_EVENTS];

pthread_mutex_t lofgile_mtx = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t statitiche_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t singals_mtx = PTHREAD_MUTEX_INITIALIZER;

volatile sig_atomic_t sighup = 0; // termina immediatamente tutto
volatile sig_atomic_t sigquit = 0; // termina dopo la fine delle richieste client
volatile sig_atomic_t sigint = 0; // termina dopo la fine delle richieste client

int pipesegnali[2];
int clientspipe[2];

// La threadpool che gestirà le richieste dei client
tpool_t *tm;

// Variabili external
int print_debug = 1;
config_t config;
stats_t stats = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


void *signa_handler(void *argument) {
    sigset_t pset;
    int signum;
    int pfd = *(int *) argument; // Pipe sulla quale inviare la notifica

    //azzero la maschera puntata da pset
    ec_meno1(sigemptyset(&pset), "Azzeramento pset");

    //metto a 1 la posizione del segnale indicato come secondo parametro nella maschera pset
    ec_meno1(sigaddset(&pset, SIGINT), "sigaddset SIGINT");
    ec_meno1(sigaddset(&pset, SIGQUIT), "sigaddset SIGQUIT");
    ec_meno1(sigaddset(&pset, SIGHUP), "sigaddset SIGHUP");
    ec_meno1(sigaddset(&pset, SIGTSTP), "sigaddset SIGTSTP");
    ec_meno1(sigaddset(&pset, SIGTERM), "sigaddset SIGTERM");


    debug("Imposto la maschera")

    //applico la maschera pset
    ec_cond((errno = pthread_sigmask(SIG_SETMASK, &pset, NULL)) == 0, "pthread_sigmask");
    ec_cond((errno = sigwait(&pset, &signum)) == 0, "pthread_sigmask");

    debug("Ricevuto sengale %d", signum)

    switch (signum) {
        case SIGTSTP:
        case SIGTERM:
            exit(EXIT_FAILURE);
            break;
        case SIGQUIT:
            Pthread_mutex_lock(&singals_mtx);
            sigquit = 1;
            Pthread_mutex_unlock(&singals_mtx);
            break;
        case SIGINT:
            Pthread_mutex_lock(&singals_mtx);
            sigint = 1;
            Pthread_mutex_unlock(&singals_mtx);
            break;
        case SIGHUP:
            Pthread_mutex_lock(&singals_mtx);
            sighup = 1;
            Pthread_mutex_unlock(&singals_mtx);
            break;
        default:
            exit(EXIT_FAILURE);
    }

    debug("sigquit: %d, sigint: %d, sighup: %d", sigquit, sigint, sighup)

    write(pfd, &signum, sizeof(int));

    return (void *) NULL;
}

int should_exit() {
    int tmp = 0;
    Pthread_mutex_lock(&singals_mtx);
    if (sighup == 1) {
        tmp = 1;
    }
    Pthread_mutex_unlock(&singals_mtx);
    return tmp;
}

int should_force_exit() {
    int tmp = 0;
    Pthread_mutex_lock(&singals_mtx);
    if (sigint == 1 || sigquit == 1) {
        tmp = 1;
    }
    Pthread_mutex_unlock(&singals_mtx);
    return tmp;
}


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


int create_socket(struct sockaddr_un *addr) {
    int listen_sock;

    // controllo se il file socket esiste già, nel caso lo elimino
    if (access(config.socket_file_name, F_OK) == 0) {
        ec_meno1(remove(config.socket_file_name), "Delete socket file");
    }

    ec_meno1(listen_sock = socket(AF_UNIX, SOCK_STREAM, 0), "Socket");

    set_sockaddr(addr);
    ec_meno1(bind(listen_sock, (struct sockaddr *) addr, sizeof(*addr)), "Binding");

    ec_meno1(listen(listen_sock, config.max_connections), "Listen");

    return listen_sock;
}

void sendnfiles(int client_fd, int nfiles) {
    char *buf, file_name[MAX_FILE_NAME_LEN];
    int counter = 0;
    size_t size;
    request_t send_file_request;

    while (counter < nfiles && read_random_file(&buf, &size, file_name, 0) == 0) {
        send_file_request = prepare_request(REQ_SEND_FILE, size, file_name, 0);
        send_request(client_fd, send_file_request);

        response_t response = receive_response(client_fd);
        if (response == RESP_OK) {
            send_message(client_fd, buf, size);
        }
        counter++;
    }

    // Mando una richiesta vuota per far capire al client che ho terminato di mandare file
    send_file_request = prepare_request(REQ_NULL, 0, NULL, 0);
    send_request(client_fd, send_file_request);
}

void handle_request(int client_fd, request_t request) {
    switch (request.id) {
        case REQ_OPEN: {

            file_data_t *f = get_file(request.file_name);

            if (request.flags & O_CREATE) {

                if (check_memory_exced(request.size, CHECK_NFILES_EXCEDED)) { // Devo liberarmi di qualche file
                    send_response(client_fd, RESP_FULL);
                    // todo implementare espulsione file
                }

                if (f != NULL) {
                    send_response(client_fd, RESP_FILE_EXISTS);
                    return;
                }

                f = add_file(request.file_name, client_fd);

                debug("Creato file %s", request.file_name)
            }

            if (f == NULL) {
                send_response(client_fd, RESP_FILE_NOT_EXISTS);
                return;
            }

            if (request.flags & O_LOCK) {
                f->locked = 1;
            }

            f->opened = 1;
            send_response(client_fd, RESP_SUCCES);

            break;
        }

        case REQ_NULL:
            break;
        case REQ_READ: {
            char *buf;
            size_t size;

            read_file(request.file_name, &buf, &size);

            send_response(client_fd, RESP_OK);

            request_t send_file_request = prepare_request(REQ_SEND_FILE, size, request.file_name, 0);
            send_request(client_fd, send_file_request);

            response_t response = receive_response(client_fd);
            if (response == RESP_OK) {
                send_message(client_fd, buf, size);
            }

            free(buf);
            break;
        }
        case REQ_READ_N: {
            int nfiles = (int)request.size;
            send_response(client_fd, RESP_OK);
            sendnfiles(client_fd, nfiles);
            break;
        }
        case REQ_WRITE: {
            if (check_memory_exced(request.size, CHECK_MEMORY_EXCEDED)) { // Devo liberarmi di qualche file
                send_response(client_fd, RESP_FULL);
                // todo implementare espulsione file
            } else {
                send_response(client_fd, RESP_OK);
            }
            char *buf = cmalloc(request.size);
            receive_message(client_fd, buf, request.size);
            write_file(request.file_name, buf, request.size);
            free(buf);
            break;
        }
        case REQ_APPEND: {

            if (check_memory_exced(request.size, CHECK_MEMORY_EXCEDED)) { // Devo liberarmi di qualche file
                send_response(client_fd, RESP_FULL);
                // todo implementare espulsione file
            } else {
                send_response(client_fd, RESP_OK);
            }

            char *buf = cmalloc(request.size);
            receive_message(client_fd, buf, request.size);
            append_to_file(request.file_name, buf, request.size);
            free(buf);
            break;
        }

        case REQ_LOCK:
            break;
        case REQ_UNLOCK:
            break;
        case REQ_CLOSE:
            close_file(request.file_name);
            send_response(client_fd, RESP_SUCCES);
            break;
        case REQ_DELETE:
            break;
        case REQ_SEND_FILE:
            break;
    }
}

void worker(void *arg) {
    int *val = arg;
    int client_fd = *val;
    request_t request;
    char buf[BUF_SIZE];

    memset(buf, 0, sizeof(buf));
    request = receive_request(client_fd);

    handle_request(client_fd, request);

    tInfo("Eseguita richiesta %d del client %d", request.id, client_fd)

    // Comunico al master thread di ri-ascoltare nuovamente il descrittore
    write(clientspipe[1], &client_fd, sizeof(int));
}

/*
 * epoll echo server
 */
void server_run() {
    int i;
    int epfd;
    int nfds;
    int listen_sock;
    int conn_sock;
    socklen_t socklen;
    char buf[BUF_SIZE];
    struct sockaddr_un srv_addr;
    struct sockaddr_un cli_addr;
    int exit = 0;
    int signum = 0;

    init_file_manager();

    listen_sock = create_socket(&srv_addr);

    epfd = epoll_create1(0);
    epoll_ctl_add(epfd, listen_sock, EPOLLIN);

    // Registro la pipe dei segnali
    epoll_ctl_add(epfd, pipesegnali[0], EPOLLIN);

    // Registro la pipe dei client
    epoll_ctl_add(epfd, clientspipe[0], EPOLLIN);

    socklen = sizeof(cli_addr);

    while (exit != 1) {
        nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (i = 0; i < nfds; i++) {
            if (events[i].data.fd == listen_sock) {
                /* handle new connection */
                conn_sock = accept(listen_sock, (struct sockaddr *) &cli_addr, &socklen);
                epoll_ctl_add(epfd, conn_sock, EPOLLIN);

                printf("[+] New client connected, fd: %d\n", conn_sock);

                send_response(conn_sock, RESP_SUCCES);
            } else if (events[i].data.fd == pipesegnali[0]) { // è arrivato un segnale dalla pipe
                read(events[i].data.fd, &signum, sizeof(int));
                debug("Arrvivato segnale %d da pipe", signum)
                break;
            } else if (events[i].data.fd == clientspipe[0]) { // un thread ha eseguito la richiesta di un client
                int client_fd;
                read(events[i].data.fd, &client_fd, sizeof(int));
                Info("Eseguita richiesta del client %d, lo rimetto in ascolto", client_fd)
                epoll_ctl_add(epfd, client_fd, EPOLLIN);
            } else if (events[i].events & EPOLLIN) { // ci sono dati da leggere da un client

                if (events[i].events & (EPOLLRDHUP | EPOLLHUP)) {
                    // Se l'evento è scatenato da una disconnessione, non eseguo nessuna richiesta
                    memset(buf, 0, sizeof(buf));
                    receive_request(events[i].data.fd);
                } else {
                    // La richiesta del client sarà letta ed eseguita da un worker
                    int client_fd = events[i].data.fd;
                    tpool_add_work(tm, worker, &client_fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, NULL);
                }

            } else {
                printf("[+] unexpected\n");
            }
            /* check if the connection is closing */
            if (events[i].events & (EPOLLRDHUP | EPOLLHUP)) {
                printf("[+] connection closed\n");
                fflush(stdout);
                epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                close(events[i].data.fd);
                continue;
            }
        }

        if (should_exit() == 1 || should_force_exit() == 1) {
            exit = 1;
        }
    }

    close_file_manager();

}

// Imosta che i segnali vengano ignorati per il thread che la richiama
void singlas_ignore() {
    sigset_t pset;
    ec_meno1(sigfillset(&pset), "inizializzazione set segnali");
    ec_meno1(pthread_sigmask(SIG_SETMASK, &pset, NULL), "mascheramento iniziale segnali");

    // Ignoro il sengale SIGPIPE
    struct sigaction ignoro;
    memset(&ignoro, 0, sizeof(ignoro));
    ignoro.sa_handler = SIG_IGN;
    ec_meno1(sigaction(SIGPIPE, &ignoro, NULL), "SC per ignorare segnale SIGPIPE");


    // tolgo la maschera inserita all'inizio, la lascio solo per i segnali da gestire con thread
    ec_meno1(sigemptyset(&pset), "settaggio a 0 di tutte le flag di insieme");
    ec_meno1(sigaddset(&pset, SIGINT), "settaggio a 1 della flag per SIGINT");
    ec_meno1(sigaddset(&pset, SIGTERM), "settaggio a 1 della flag per SIGTERM"); // windows
    ec_meno1(sigaddset(&pset, SIGQUIT), "settaggio a 1 della flag per SIGQUIT");
    ec_meno1(sigaddset(&pset, SIGHUP), "settaggio a 1 della flag per SIGHUP");
    ec_meno1(sigaddset(&pset, SIGUSR1), "settaggio a 1 della flag per SIGUSR1");
    ec_meno1(sigaddset(&pset, SIGUSR2), "settaggio a 1 della flag per SIGUSR2");
    ec_meno1(pthread_sigmask(SIG_SETMASK, &pset, NULL),
             "rimozione mascheramento iniziale dei segnali ignorati o gestiti in modo custom");
}

int main(int argc, char *argv[]) {

    pthread_t singnal_handler_thread;

    if (argc >= 3 && strcmp(argv[1], "-conf") == 0) {
        char *config_filename = NULL;
        config_filename = argv[2];
        load_config(config_filename);
    } else {
        load_defaults();
    }

    /*========= SEGNALI =========*/

    // Ignoro i sengali per poterli gestire con il thread
    singlas_ignore();

    // inizializzo la pipe su cui la epoll riceverà i segnali
    ec_meno1(pipe(pipesegnali), "pipe segnali")

    ec_meno1(pthread_create(&singnal_handler_thread, NULL, signa_handler, (pipesegnali + 1)),
             "pthread_create singal handler")

    /*========= PIPE DEI CLIENT =========*/
    ec_meno1(pipe(clientspipe), "pipe client")

    /*========= THREAD WORKERS =========*/
    tm = tpool_create(config.max_workers);


    /*========= ESECUZIONE SERVER =========*/
    server_run();

    /*========= CHIUSURA =========*/
    tpool_destroy(tm);
    ec_meno1(pthread_join(singnal_handler_thread, NULL), "pthread_join singal handler")

    // Chiudo le pipe
    close(pipesegnali[0]);
    close(pipesegnali[1]);
    close(clientspipe[0]);
    close(clientspipe[1]);

    printf("Max workers: %d", config.max_workers);
    return 0;
}