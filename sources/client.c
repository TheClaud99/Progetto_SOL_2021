/**
 * @file    client.c
 * @brief   Contiene l'implementazione del client, tra cui la lettura dei parametri in ingresso ed esecuzione delle richieste verso il server.
**/

#define _GNU_SOURCE

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
char readed_file_dir[PATH_MAX];

int print_debug = 0;
int is_server = 0;
int time_to_wait = 0;


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

            case 'd': { // Cartella dove memorizzare file inviati dal server
                memset(readed_file_dir, 0, PATH_MAX);
                strcpy(readed_file_dir, optarg);
                break;
            }

            case 'D': { // Cartella dove memorizzare file rimossi dal server
                memset(removed_file_dir, 0, PATH_MAX);
                strcpy(removed_file_dir, optarg);
                break;
            }

            case 't': { // specifica tempo tra una richiesta e l'altra in ms
                time_to_wait = (int) strtol(optarg, NULL, 10);
                break;
            }

            case '?': { // opzione non valida
                printf("> Argomento %c non riconosciuto", optopt);
                break;
            }

            default:

                // Memorizzo le operazioni che sono state richieste
                memset(ops[count_ops].arguments, 0, MAX_LINE);
                if (optarg != NULL) {
                    if (optarg[0] == '-') { // Ho come argomento una opt successiva
                        optind -= 1;
                    } else {
                        strcpy(ops[count_ops].arguments, optarg);
                    }
                }

                ops[count_ops].param = opt;
                count_ops++;
                break;
        }
    }

    return count_ops;
}

int write_or_append(char *file_path) {
    // se il file non esiste lo creo, ci scrivo e lo chiudo
    if (openFile(file_path, O_CREATE | O_LOCK) == 0) {

        ec_meno1(writeFile(file_path, removed_file_dir), "writeFile")
        ec_meno1(unlockFile(file_path), "unlockFile")
        ec_meno1(closeFile(file_path), "closeFile")

    } else if (openFile(file_path, 0) == 0) { // se il file esiste già allora lo apro, ci scrivo in append e lo chiudo
        FILE *file;
        size_t size;
        struct stat file_stat; // Informazioni sul file

        // Apro il file e prendo le informazioni su di esso
        ec_null(file = fopen(file_path, "rb"), "write_or_append fopen")
        ec_meno1(stat(file_path, &file_stat), "write_or_append stat")

        size = file_stat.st_size; // Imposto la dimensione di un byte in più per il carattere di terminazinoe '\0'

        // Leggo il contenuto del file in un buffer temporaneo
        void *buffer = cmalloc(size);
        ec_cond(file_stat.st_size == fread(buffer, 1, file_stat.st_size, file), "writeFile fread")
        fclose(file);

        ec_meno1(appendToFile(file_path, buffer, size, removed_file_dir), "appendToFile")

        free(buffer);

        ec_meno1(closeFile(file_path), "closeFile")
    }

    return 0;
}

int send_files(char *files) {
    int writed_files = 0;
    char *save_ptr;

    char *file_path = strtok_r(files, ",", &save_ptr);
    while (file_path != NULL) {

        write_or_append(file_path);

        file_path = strtok_r(NULL, ",", &save_ptr);
        writed_files++;
    }

    return writed_files;
}

int send_dir(char *dirname, int remaning_files, int completed) {
    if (remaning_files != 0) {
        DIR *folder;
        struct dirent *entry;

        if (!(folder = opendir(dirname))) return -1;

        // Scorro i file nella cartella
        while ((entry = readdir(folder)) != NULL && (remaning_files != 0)) {
            char path[PATH_MAX];
            if (dirname[strlen(dirname) - 1] != '/')
                snprintf(path, sizeof(path), "%s/%s", dirname, entry->d_name);
            else
                snprintf(path, sizeof(path), "%s%s", dirname, entry->d_name);

            struct stat properties;

            ec_meno1(stat(path, &properties), "send_dir")

            if (S_ISDIR(properties.st_mode)) { // se è una directory faccio una chiamata ricorsiva alla funzione
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                    continue;
                send_dir(path, remaning_files, completed);
            } else { // è un file
                completed += write_or_append(path);

                remaning_files--;
            }
        }

        closedir(folder);
    }

    return completed;
}

int read_files(char *files) {
    int readed_files = 0;
    char *save_ptr;
    void *buf;
    char save_path[PATH_MAX];
    char *file_name;
    size_t size;
    FILE *file;

    memset(save_path, 1, PATH_MAX);

    char *file_path = strtok_r(files, ",", &save_ptr);
    while (file_path != NULL) {

        ec_meno1(openFile(file_path, 0), "openFile")
        ec_meno1(readFile(file_path, &buf, &size), "readFile")
        ec_meno1(closeFile(file_path), "closeFile")

        file_name = basename(file_path);
        strcpy(save_path, readed_file_dir);
        if(strlen(readed_file_dir) > 0) {
            if (save_path[PATH_MAX - 1] != '/') {
                strcat(save_path, "/");
            }
            strcat(save_path, file_name);

            if((file = fopen(save_path, "wb")) != NULL) {
                fwrite(buf, 1, size, file);
            }

            debug("File '%s' (%ld bytes) salavato in '%s'", file_name, size, save_path)

            fclose(file);
        } else {
            debug("Ricevuto e scartato file '%s' (%ld bytes)", file_name, size)
        }

        free(buf);
        file_path = strtok_r(NULL, ",", &save_ptr);
        readed_files++;
    }

    return readed_files;
}

int delete_files(char *files) {
    int deleted_files = 0;
    char *save_ptr;

    char *file_path = strtok_r(files, ",", &save_ptr);
    while (file_path != NULL) {

        ec_meno1(removeFile(file_path), "removeFile")

        debug("Rimosso file '%s'", file_path)

        file_path = strtok_r(NULL, ",", &save_ptr);
        deleted_files++;
    }

    return deleted_files;
}

void execute_ops(int count_ops) {
    for (int i = 0; i < count_ops; i++) {
        switch (ops[i].param) {
            case 'w': {
                check_empty_string(ops[i].arguments, "Specifica la cartella da scrivere sul server")
                int max_files = -1;
                char *dirnmame = strtok(ops[i].arguments, ",");
                char *temp;

                if ((temp = strtok(NULL, ",")) != NULL) {
                    max_files = (int) strtol(temp, NULL, 10);
                    if (max_files == 0) max_files -= 1; // se n=0 lo considero come -1
                }

                send_dir(dirnmame, max_files, 0);

                break;
            }

            case 'W': {
                send_files(ops[i].arguments);
                break;
            }

            case 'r': { // lista di file da leggere dal server separati da virgola
                read_files(ops[i].arguments);
                break;
            }

            case 'R': { // legge n file qualsiasi dal server (se n=0 vengono letti tutti)
                int nfiles = strlen(ops[i].arguments) > 0 ? (int) strtol(ops[i].arguments, NULL, 10) : 0;
                debug("Leggo %d file qualsiasi dal server (se n=0 vengono letti tutti)", nfiles)
                readNFiles(nfiles, readed_file_dir);
                break;
            }

            case 'l': { // lista di nomi di cui acquisire mutex
                lockFile(ops[i].arguments);
                break;
            }

            case 'u': { // lista di nomi di cui rilasciare mutex
                unlockFile(ops[i].arguments);
                break;
            }

            case 'c': { // lista di file da rimuovere dal server
                delete_files(ops[i].arguments);
                break;
            }

            case ':': { // manca un argomento
                switch (optopt) {
                    case 'R': { // può non avere argomenti (0 di default)
                        debug("Leggo tutti i file del server")
                        readNFiles(0, readed_file_dir);
                        break;
                    }

                    default: {
                    }
                }
                break;
            }
        }

        // Attendo il tempo specificato per mandare la richiesta successiva
        msleep(time_to_wait);
    }
}

void set_options(int argc, char *argv[]) {
    int count_ops;
    struct timespec max_timeout = {5, 0};

    // File log
    char charValue[MAXNAMLEN];
    sprintf(charValue, "tests/outputs/client_log/client_%d.log", getpid());

    // Imposto le operazioni da fare
    count_ops = set_operations(argc, argv);

    // Se il debug è impostato (-p) allora creo un logfile
    if (print_debug == 1) ec_null(logfile = fopen(charValue, "w"), "fopen")

    // Apro la connessione ed eseguo le operazioni
    openConnection(socket_file_name, 1000, max_timeout);
    execute_ops(count_ops);
    closeConnection(socket_file_name);

    // Se il debug è impostato (-p) chiudo il logfile
    if (print_debug == 1) ec_cond(fclose(logfile) != EOF, "fclose")
}

int main(int argc, char *argv[]) {

    set_options(argc, argv);

    return 0;
}