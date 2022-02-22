#!/bin/bash
# FILE DI TEST N°3
#
# Autore: Claudio Mano
#

# avvio il server
valgrind --leak-check=full ./server -conf tests/test3/config_3.ini > tests/outputs/Server/valgrind_output.txt 2>&1 &

# mi salvo il pid ($! viene sostituito col PID del processo più recente avviato in background)
pid=$!

# attendo attivazione
sleep 2

# Per uscire dal ciclo di creazione dei client
# uso la variabile speciale SECONDS e imposto un
# timer di 30 secondi prima di uccidere il server
breakLoop=$((SECONDS+30))
echo "$1"
index=1
while [ $SECONDS -le $breakLoop ]; do
    ./client -f ./sock_file.sk \
    -d tests/outputs/readed  \
    -D tests/outputs/deleted \
    -W "tests/a/d/test - Copia (${index}).txt"  \
    -l "$PWD/tests/a/d/test - Copia (${index}).txt" \
    -R \
    -c "$PWD/tests/a/d/test - Copia (${index}).txt" &
    index=$((index+1))
  if [ $((index%20)) = 1 ]; then
      index=1
      sleep 0.5
  fi
done

# terminazione immediata al server
kill -s SIGINT $pid

# aspetto il server
wait $pid

exit 0