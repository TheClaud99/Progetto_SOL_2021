#!/bin/bash
# FILE DI TEST N°2
#
# Autore: Claudio Mano
#

# avvio il server
valgrind --leak-check=full ./server -conf tests/test2/config_2.ini &

# mi salvo il pid ($! viene sostituito col PID del processo più recente avviato in background)
pid=$!

# attendo attivazione
sleep 2

# avvio il client
./client -f sock_file.sk -p -d tests/outputs/readed -D tests/outputs/deleted -W tests/a/b/foto.jpg,tests/a/b/test3.txt -w tests/a/b -R 3 -l "$PWD"/tests/a/b/foto.jpg -c "$PWD"/tests/a/b/foto.jpg -W tests/a/test.txt -r "$PWD"/tests/a/test.txt &
./client -f sock_file.sk -p -d tests/outputs/readed -D tests/outputs/deleted -W tests/a/b/foto.jpg,tests/a/b/test3.txt -w tests/a/c -R 3 -l "$PWD"/tests/a/c/test6.txt -c "$PWD"/tests/a/c/test6.txt -W tests/a/test.txt -r "$PWD"/tests/a/test.txt &
./client -f sock_file.sk -p -d tests/outputs/readed -D tests/outputs/deleted -W tests/a/test2.txt -l "$PWD"/tests/a/test2.txt -c "$PWD"/tests/a/test2.txt -W tests/a/test2.txt,tests/a/test.txt -R 0 -w tests/a/d/ &
./client -f sock_file.sk -p -d tests/outputs/readed -D tests/outputs/deleted -W tests/a/b/foto.jpg,tests/a/b/test3.txt -w tests/a/b -R 3 -l "$PWD"/tests/a/b/foto.jpg  -c "$PWD"/tests/a/b/foto.jpg -W tests/a/test.txt -r "$PWD"/tests/a/test.txt &
./client -f sock_file.sk -p -d tests/outputs/readed -D tests/outputs/deleted -W tests/a/test.txt -W tests/a/b/foto.jpg,tests/a/b/test3.txt -w tests/a/b -R 3 -l "$PWD"/tests/a/b/test3.txt -c "$PWD"/tests/a/b/test3.txt -r "$PWD"/tests/a/test.txt &

# aspetto il termine
sleep 2

# terminazione lenta al server
kill -s SIGHUP $pid

# aspetto il server
wait $pid

exit 0