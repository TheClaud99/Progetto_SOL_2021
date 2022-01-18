/**
 * @file    api.h
 * @brief   Contiene l'header dei metodi utilizzati dal client per comunicare col server.
**/

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libgen.h>
#include <linux/limits.h>
#include <time.h>


/**
 * @brief Apre connessione AF_UNIX e ripete più volte la richiesta fino ad un certo tempo.
 * 
 * @param sockname nome del socket su cui aprire la connessione
 * @param msec     tempo in millisecondi tra un tentativo di connessione e l'altro
 * @param abstime  tempo dopo il quale la connessione si considera fallita
 * 
 * @returns 0 in caso di successo, -1 in caso di fallimento (setta errno)
**/
int openConnection(const char* sockname, int msec, const struct timespec abstime);


/**
 * @brief Chiude connessione AF_UNIX.
 * 
 * @param sockname nome del socket a cui è associata la connessione da chiudere.
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
 *                 O_OPEN   -> dice di aprire il file (non necessario, usabile per chiarezza)
 *                 O_CREATE -> dice di creare il file (se c'è già dà errore)
 *                 O_LOCK   -> dice che il file può essere scritto o letto solo dal processo che lo ha creato (non impl.)
 * 
 * @returns 0 in caso di successo, -1 in caso di fallimento (setta errno)
**/
int openFile(const char* pathname, int flags);


/**
 * @brief Legge il contenuto del file dal server (se esiste).
 * 
 * @param pathname percorso relativo al file che si vuole leggere
 * @param buf      puntatore sull'heap del file ottenuto
 * @param size     dimensione del buffer dati ottenuto
 * 
 * @returns 0 in caso di successo, -1 in caso di fallimento (setta errno)
**/
int readFile(const char* pathname, void** buf, size_t* size);


/**
 * @brief Richiede al server di leggere N files qualunque e salvarli in una cartella.
 * 
 * @param N         Numero di file da richiedere al server (se ne ha meno li invia tutti), se <= 0 vengono letti tutti i file
 * @param dirname   Cartella lato client in cui salvare i file
 * 
 * @returns 0 in caso di successo (letti N file), -1 in caso di fallimento (setta errno)
**/
int readNFiles(int N, const char* dirname);


/**
 * @brief Scrive tutto il file nel server.
 * 
 * @param pathname  percorso relativo al file da inviare
 * @param dirname   specifica comportamento server
 *                  != NULL -> se un file viene espulso dal server allora viene salvato in dirname
 *                  =  NULL -> se un file viene espulso dal server allora viene ignorato
 * 
 * @returns 0 in caso di successo (solo se la precedente operazione è stata una openFile con O_CREATE|O_LOCK), 0 in caso di fallimento (setta errno)
**/
int writeFile(const char* pathname, const char* dirname);


/**
 * @brief Richiede di aggiungere ad un file del contenuto.
 * 
 * @param pathname  percorso relativo al file su cui appendere
 * @param buf       puntatore sull'heap dei dati da appendere al file del server
 * @param size      dimensione del buffer dati inviato
 * @param dirname   specifica comportamento server
 *                  != NULL -> se un file viene espulso dal server allora viene salvato in dirname
 *                  =  NULL -> se un file viene espulso dal server allora viene ignorato
 * 
 * @returns 0 in caso di successo, -1 in caso di fallimento (setta errno)
**/
int appendToFile(const char* pathname, void* buf, size_t size, const char* dirname);


/**
 * @brief Imposta il file come accedibile solo dal processo che ha chiamato questo metodo.
 * 
 * @param pathname  percorso relativo al file da bloccare
 * 
 * @returns 0 in caso di successo, -1 in caso di fallimento (setta errno)
**/
int lockFile(const char* pathname);


/**
 * @brief Imposta il file come accedibile da tutti.
 * 
 * @param pathname  percorso relativo al file da sbloccare
 * 
 * @returns 0 in caso di successo (solo se l'owner del lock è il processo che chiama questo metodo), -1 in caso di fallimento (setta errno)
**/
int unlockFile(const char* pathname);


/**
 * @brief Chiude il file puntato da pathname. Tutte le operazioni dopo questo metodo falliranno.
 * 
 * @param pathname  percorso relativo al file da chiudere
 * 
 * @returns 0 in caso di successo, -1 in caso di fallimento (setta errno)
**/
int closeFile(const char* pathname);


/**
 * @brief Elimina il file puntato da pathname. Fallisce se non è in stato locked o è locked da un client diverso.
 * 
 * @param pathname  percorso relativo al file da chiudere
 * 
 * @returns 0 in caso di successo, -1 in caso di fallimento (setta errno)
**/
int removeFile(const char* pathname);