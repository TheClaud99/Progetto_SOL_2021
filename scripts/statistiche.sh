#!/bin/bash
# FILE CHE STAMPA A SCHERMO LE STATISTICHE FORMATTATE
#
# Autore: Claudio Mano
#
NOMESCRIPT=${0##*/}

# se non ci sono argomenti ($# contiene il numero di argomenti passati allo script bash)
if [ $# = 0 ]; then
  echo -e "${NOMESCRIPT} > Fornisci anche il nome di un file." 1>&2
  exit 1
fi

# scorro gli argomenti ricevuti
for argomento; do
  # controllo se il file esiste (-f restituisce VERO se il file esiste ed è un file regolare, -r restituisce VERO se il file è leggibile)
  if [ -f "$argomento" ] && [ -r "$argomento" ]; then
    FILESTATS=$argomento
  else # mando messaggio di errore trasferendo l'stdout sull'stderr
    echo "${NOMESCRIPT} > File \"$argomento\" inesistente o non hai i permessi di lettura." 1>&2
    exit 1
  fi
done

# apro in lettura lo statfile nel descrittore 4
exec 4<"$FILESTATS"

awk '/STATISTICHE SERVER/,0' "$FILESTATS"

echo ''
