/**
 * @file    api.h
 * @brief   Contiene l'header dei metodi utilizzati dal client per comunicare col server.
**/

#ifndef API_H_
#define API_H_

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <time.h>

// Socket con il quale comunicare col server
int fd_socket;

/**
 * @brief Apre connessione AF_UNIX e ripete più volte la richiesta fino ad un certo tempo.
 * 
 * @param sockname nome del file socket su cui aprire la connessione
 * @param msec     tempo in millisecondi tra un tentativo di connessione e l'altro
 * @param abstime  tempo entro il quale il metodo ritorna con errore se la connessione non è ancora avvenuta
 * 
 * @returns 0 in caso di successo, -1 in caso di fallimento (setta errno)
**/
int openConnection(const char* sockname, int msec, const struct timespec abstime);


/**
 * @brief Chiude la connessione con il server.
 * 
 * @param sockname nome del file socket.
 *  
 * @returns 0 in caso di successo, -1 in caso di fallimento (setta errno)
**/
int closeConnection(const char* sockname);


/**
 * @brief Manda una richiesta di apertura o creazione di un file.
 *        Il funzionamento cambia in base ai flags impostati.
 * 
 * @param pathname percorso relativo al file di cui mandare la richiesta
 * @param flags    imposta una o più flags
 *                 O_CREATE -> se settato, crea il file o ritorna errore se il file è già esistente
 *                 O_LOCK   -> nell'aprire il file acquisice al contempo la lock
 * 
 * @returns 0 in caso di successo, -1 in caso di fallimento (setta errno)
**/
int openFile(const char* pathname, int flags);


/**
 * @brief Legge il contenuto del file specificato da pathname.
 * 
 * @param pathname percorso del file che si vuole leggere
 * @param buf      al ritorno del metodo conterrà il file letto
 * @param size     dimensione dei dati letti
 * 
 * @returns 0 in caso di successo, -1 in caso di fallimento (setta errno)
**/
int readFile(const char* pathname, void** buf, size_t* size);


/**
 * @brief Richiede al server di leggere N files qualsisasi e salvarli in una cartella.
 * 
 * @param N         numero di file da leggere al server (se ne ha meno li invia tutti), se <= 0 vengono letti tutti i file
 * @param dirname   cartella in cui salvare i file letti, se NULL i file verranno scartati
 * 
 * @returns 0 in caso di successo , -1 in caso di fallimento (setta errno)
**/
int readNFiles(int N, const char* dirname);


/**
 * @brief Scrive un file sul server
 * 
 * @param pathname  percorso del file da inviare
 * @param dirname   cartella in cui salvare i file eventualmente espulsi dal server. Se NULL, i file verrano scartati
 * 
 * @returns 0 in caso di successo, 0 in caso di fallimento (setta errno)
**/
int writeFile(const char* pathname, const char* dirname);


/**
 * @brief Richiede di scrivere in append su un file.
 * 
 * @param pathname  percorso del file su cui scrivere
 * @param buf       buffer dei dati da scrivere
 * @param size      dimensione del buffer dei dati
 * @param dirname   cartella in cui salvare i file eventualmente espulsi dal server. Se NULL, i file verrano scartati
 * 
 * @returns 0 in caso di successo, -1 in caso di fallimento (setta errno)
**/
int appendToFile(const char* pathname, void* buf, size_t size, const char* dirname);


/**
 * @brief Acquisice la lock su questo file
 * 
 * @param pathname  percorso relativo del file da bloccare
 * 
 * @returns 0 in caso di successo, -1 in caso di fallimento (setta errno)
**/
int lockFile(const char* pathname);


/**
 * @brief Rilascia la lock su un file
 * 
 * @param pathname  percorso del file da sbloccare
 * 
 * @returns 0 in caso di successo, -1 in caso di fallimento (setta errno)
**/
int unlockFile(const char* pathname);


/**
 * @brief Chiude il file specificato in pathname.
 * 
 * @param pathname  percorso del file da chiudere
 * 
 * @returns 0 in caso di successo, -1 in caso di fallimento (setta errno)
**/
int closeFile(const char* pathname);


/**
 * @brief Elimina il file specificato in pathname
 * 
 * @param pathname  percorso del file da chiudere
 * 
 * @returns 0 in caso di successo, -1 in caso di fallimento (setta errno)
**/
int removeFile(const char* pathname);

#endif /* API_H_ */