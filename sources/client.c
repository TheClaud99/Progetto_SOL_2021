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

#define HELLO_MESSAGE   "Hello" // body dell'handshake iniziale HELLO
#define LOG_FOLDER_NAME "TestDirectory/output/Client/" // cartella dove caricare il file di log (DEVE terminare con un /)
#define LOG_FILE_NAME   "client_log.txt" // nome del file di log

FILE* fileLog; // puntatore al file di log del client

int main(int argc, char* argv[]) {


    return 0;
}