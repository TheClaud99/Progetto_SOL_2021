#!/bin/bash
# FILE DI TEST N°2
#
# Autore: Leonardo Pantani
#

# avvio il server
valgrind --leak-check=full ./server -conf tests/test2/config_2.ini > tests/outputs/Server/valgrind_output.txt 2>&1 &

# mi salvo il pid ($! viene sostituito col PID del processo più recente avviato in background)
pid=$!

# attendo attivazione
sleep 2

# avvio il client
./client -f sock_file.sk -p -t 1000 -d tests/outputs/readed -D tests/outputs/deleted -W tests/a/b/foto.jpg,tests/a/b/test3.txt -w tests/a/b -R 3 -c tests/a/b/foto.jpg -W tests/a/test.txt -r tests/a/test.txt &
./client -f sock_file.sk -p -t 1250 -d tests/outputs/readed -W tests/a/b/foto.jpg,TestDirectory/tests/a/b/test3.txt -w tests/a/c -R 3 -c tests/a/c/test6.txt -W tests/a/test.txt -r tests/a/test.txt &
./client -f sock_file.sk -p -t 1000 -d tests/outputs/readed -D tests/outputs/deleted -W tests/a/test2.txt -c tests/a/test2.txt -W tests/a/test2.txt,tests/a/test.txt -R 0 -w tests/a/d/ &
./client -f sock_file.sk -p -t 1250 -d tests/outputs/readed -D tests/outputs/deleted -W tests/a/b/foto.jpg,tests/a/b/test3.txt -w tests/a/b -R 3 -c tests/a/b/foto.jpg -W tests/a/test.txt -r tests/a/test.txt &
./client -f sock_file.sk -p -t 1000 -d tests/outputs/readed -D tests/outputs/deleted -W tests/a/test.txt -W tests/a/b/foto.jpg,tests/a/b/test3.txt -w tests/a/b -R 3 -c tests/a/b/foto.jpg -r tests/a/test.txt &

# aspetto il termine
sleep 2

# terminazione lenta al server
kill -s SIGHUP $pid

# aspetto il server
wait $pid

exit 0