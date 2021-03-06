#!/bin/bash
# FILE DI TEST N°1
#
# Autore: Claudio Mano
#

# avvio il server con valgrind
valgrind --leak-check=full ./server -conf tests/test1/config_1.ini > tests/outputs/Server/valgrind_output.txt 2>&1 &

# mi salvo il pid ($! viene sostituito col PID del processo più recente avviato in background)
pid=$!

# attendo attivazione valgrind per 3 secondi
sleep 2

# avvio il client
./client -f sock_file.sk -p -t 200 -d tests/outputs/readed -D tests/outputs/deleted -W tests/a/b/foto.jpg,tests/a/b/test3.txt -w tests/a/b -R 3 -l "$PWD"/tests/a/b/foto.jpg -c "$PWD"/tests/a/b/foto.jpg -W tests/a/test.txt -r "$PWD"/tests/a/test.txt &
./client -f sock_file.sk -p -t 200 -d tests/outputs/readed -W tests/a/b/foto.jpg,tests/a/b/test3.txt -w tests/a/c -R 3 -l "$PWD"/tests/a/c/test6.txt -c "$PWD"/tests/a/c/test6.txt -W tests/a/test.txt -r "$PWD"/tests/a/test.txt &
./client -f sock_file.sk -p -t 200 -d tests/outputs/readed -D tests/outputs/deleted -W tests/a/test2.txt -l "$PWD"/tests/a/test2.txt -c "$PWD"/tests/a/test2.txt -W tests/a/test2.txt,tests/a/test.txt -R 0 -w tests/a/d/ &
./client -f sock_file.sk -p -t 200 -d tests/outputs/readed -D tests/outputs/deleted -W tests/a/b/foto.jpg,tests/a/b/test3.txt -w tests/a/b -R 3 -W tests/a/test.txt -r "$PWD"/tests/a/test.txt &
./client -f sock_file.sk -p -t 200 -d tests/outputs/readed -D tests/outputs/deleted -W tests/a/b/foto.jpg,tests/a/b/test3.txt -w tests/a/b -R 3 -l "$PWD"/tests/a/b/test3.txt -c "$PWD"/tests/a/b/test3.txt -W tests/a/test.txt -r "$PWD"/tests/a/test.txt &

# aspetto il termine
sleep 2

# terminazione lenta al server
kill -s SIGHUP $pid

# aspetto il server
wait $pid

# ottengo il numero di errori
r=$(tail -10 tests/outputs/Server/valgrind_output.txt | grep "ERROR SUMMARY" | cut -d: -f 2 | cut -d" " -f 2)

# se il numero di errori è diverso da 0 allora non va bene
if [[ $r != 0 ]]; then
  echo "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
  echo "xxxxxxxxxxxxxx TEST 1 FALLITO xxxxxxxxxxxxxx"
  echo "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
  exit 1
fi

exit 0
