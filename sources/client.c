/**
 * @file    client.c
 * @brief   Contiene l'implementazione del client, tra cui la lettura dei parametri in ingresso ed esecuzione delle richieste verso il server.
**/

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <limits.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <getopt.h>

#include "api.h"
#include "communication.h"

#define HELLO_MESSAGE   "Hello" // body dell'handshake iniziale HELLO
#define LOG_FOLDER_NAME "TestDirectory/output/Client/" // cartella dove caricare il file di log (DEVE terminare con un /)
#define LOG_FILE_NAME   "client_log.txt" // nome del file di log
#define MAX_LINE        256
#define MAX_ACTIONS        256
#define MAX_PARAMS 256

FILE *fileLog; // puntatore al file di log del client

char socket_file_name[PATH_MAX];

typedef struct operation {
    int param;
    char arguments[MAX_LINE];
} operation_t;

operation_t ops[MAX_PARAMS];
char removed_file_dir[PATH_MAX];

/*
 * test clinet
 */
/*void client_run() {
    int n, c, sockfd;
    char buf[MAX_LINE];
    struct sockaddr_un srv_addr;

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    set_sockaddr(&srv_addr);

    if (connect(sockfd, (struct sockaddr *) &srv_addr, sizeof(srv_addr)) < 0) {
        perror("connect()");
        exit(1);
    }

    for (;;) {
        memset(buf, 0, sizeof(buf));
        strcpy(buf, "Ciao\0");
        c = (int) strlen(buf);
        write(sockfd, buf, c);

        memset(buf, 0, sizeof(buf));
        while (errno != EAGAIN && (n = read(sockfd, buf, sizeof(buf))) > 0) {
            printf("echo: %s\n", buf);
            memset(buf, 0, sizeof(buf));

            c -= n;
            if (c <= 0) {
                break;
            }
        }
    }
    close(sockfd);
}*/

static void help(void) {
    printf("Utilizzo:\n");
    printf("    -h                 \t aiuto\n");
    printf("    -f <filename>      \t specifica il nome del socket a cui connettersi\n");
    printf("    -w <dirname[,n=0]> \t invia i file nella cartella dirname; se n=0 manda tutti i file, altrimenti manda n file\n");
    printf("    -W <file1[,file2]> \t lista di file da inviare al server\n");
    printf("    -D <dirname>       \t cartella dove mettere i file in caso vengano espulsi dal server\n");
    printf("    -r <file1[,file2]> \t lista di file richiesti al server\n");
    printf("    -R [n=0]           \t legge n file dal server, se n non specificato li legge tutti\n");
    printf("    -d <dirname>       \t cartella dove mettere i file letti dal server\n");
    printf("    -t time            \t tempo (ms) che intercorre tra una richiesta di connessione e l'altra al server\n");
    printf("    -l <file1[,file2]> \t lista di file da bloccare\n");
    printf("    -u <file1[,file2]> \t lista di file da sbloccare\n");
    printf("    -c <file1[,file2]> \t lista di file da rimuovere dal server\n");
    printf("    -p                 \t abilita le stampe sull'stdout di ogni operazione\n");
}


int set_operations(int argc, char *argv[]) {
    int opt, count_ops = 0;
    print_debug = 0;

    while ((opt = getopt(argc, argv, ":hf:w:W:D:r:R:d:t:l:u:c:p")) != -1) {
        switch (opt) {
            case 'p': {  // attiva la modalità debug
                print_debug = 1;
                break;
            }
            case 'h': { // aiuto
                help();
                break;
            }
            case 'f': {// nome socket
                memset(socket_file_name, 0, PATH_MAX);
                strcpy(socket_file_name, optarg);
                break;
            }

            case 'D': { // Cartella dove memorizzare file rimossi dal server
                memset(removed_file_dir, 0, PATH_MAX);
                strcpy(removed_file_dir, optarg);
                break;
            }

            case '?': { // opzione non valida
                printf("> Argomento %c non riconosciuto", optopt);
                break;
            }

            default:
                // Memorizzo le operazioni che sono state richieste
                strcpy(ops[count_ops].arguments, optarg);
                ops[count_ops].param = opt;
                count_ops++;
                break;
        }
    }

    return count_ops;
}

int write_or_append(char *file_path) {
    // se il file non esiste lo creo, ci scrivo e lo chiudo
    if (openFile(file_path, O_CREATE) == 0) {

        ec_meno1(writeFile(file_path, removed_file_dir), "writeFile")
        ec_meno1(closeFile(file_path), "closeFile")

    } else if (openFile(file_path, 0) == 0) { // se il file esiste già allora lo apro, ci scrivo in append e lo chiudo
        FILE *file;
        ec_null(file = fopen(file_path, "rb"), "fopen")

        /*void *data = cmalloc(sizeof(char) * properties.st_size);

        if (fread(data, 1, properties.st_size, filePointer) == properties.st_size) { // se la lettura ha successo
            if (appendToFile(filePath, data, properties.st_size, ejectedDest) == 0) {
                if (closeFile(filePath) == 0) {
                    fclose(filePointer);
                    free(data);
                    return 1;
                }
            }
        }*/

        fclose(file);
    }

    return 0;
}

int send_files(char *files) {
    int writed_files = 0;
    char* save_ptr;

    char *file_path = strtok_r(files, ",", &save_ptr);
    while (file_path != NULL) {

        write_or_append(file_path);

        file_path = strtok_r(NULL, ",", &save_ptr);
        writed_files++;
    }

    return writed_files;
}


void execute_ops(int count_ops) {
    for (int i = 0; i < count_ops; i++) {
        switch (ops[i].param) {
            case 'w': {
                send_files(ops[i].arguments);
                break;
            }

            case 'r': { // lista di file da leggere dal server separati da virgola
                break;
            }

            case 'R': { // legge n file qualsiasi dal server (se n=0 vengono letti tutti)
                break;
            }

            case 'd': { // cartella dove salvare file letti con la r oppure R (INSIEME A r|R!)
                break;
            }

            case 't': { // specifica tempo tra una richiesta e l'altra in ms
                break;
            }

            case 'l': { // lista di nomi di cui acquisire mutex | NON IMPL.
                break;
            }

            case 'u': { // lista di nomi di cui rilasciare mutex | NON IMPL.
                break;
            }

            case 'c': { // lista di file da rimuovere dal server
                break;
            }

            case ':': { // manca un argomento
                switch (optopt) {
                    case 'R': { // può non avere argomenti (0 di default)
                        break;
                    }

                    default: {
                    }
                }
                break;
            }
        }
    }
}

void set_options(int argc, char *argv[]) {
    int count_ops;

    count_ops = set_operations(argc, argv);

    openConnection(socket_file_name, 1000, get_abs_time_from_now(20));
    execute_ops(count_ops);
    closeConnection(socket_file_name);
}

int main(int argc, char *argv[]) {

    set_options(argc, argv);

    return 0;
}