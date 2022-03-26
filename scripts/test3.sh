#!/bin/bash
# FILE DI TEST N°3
#
# Autore: Claudio Mano
#

# avvio il server
./server -conf tests/test3/config_3.ini &

# mi salvo il pid ($! viene sostituito col PID del processo più recente avviato in background)
pid=$!

# attendo attivazione
sleep 2

# Per uscire dal ciclo di creazione dei client
# uso la variabile speciale SECONDS e imposto un
# timer di 30 secondi prima di uccidere il server
breakLoop=$((SECONDS+30))
echo "$1"
#index=1
#while [ $SECONDS -le $breakLoop ]; do
#    ./client -f ./sock_file.sk \
#    -d tests/outputs/readed  \
#    -D tests/outputs/deleted \
#    -W "tests/a/d/test - Copia (${index}).txt"  \
#    -w "tests/a/d/" 3  \
#    -r "tests/a/d/test - Copia (${index}).txt"  \
#    -l "$PWD/tests/a/d/test - Copia (${index}).txt" \
#    -R 3 \
#    -c "$PWD/tests/a/d/test - Copia (${index}).txt" &
#    index=$((index+1))
#  if [ $((index%10)) = 1 ]; then
#      index=1
#      sleep 0.5
#  fi
#done

# Avvio 10 loop che per 30 secondi lanciano client consecutivamente
CLS=()
for i in {1..10}; do
    ./scripts/start_client.sh &
    CLS+=($!)
done

# Aspetto che terminino
for i in "${CLS[@]}"; do
    wait ${i}
done

# terminazione immediata al server
kill -s SIGINT $pid

# aspetto il server
wait $pid

exit 0