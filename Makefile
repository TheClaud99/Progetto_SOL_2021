# ------------ CARTELLE ------------
# Cartella dei file oggetto
OBJECT_FOLDER = build/objects
# Cartella delle librerie
LIB_FOLDER = build/libs
# Cartella delle api
API_FOLDER = sources/api
# Cartella delle strutture dati
DS_FOLDER = sources/data_structures
# Cartella delle utils
UTL_FOLDER = sources/utils
# Cartella degli headers
HEADERS_FOLDER = headers


# ------------- ALTRO --------------
# Estensione del file socket da rimuovere da ovunque
SOCKET_EXTENSION = sk
# Bersagli phony (non sono nomi file ma una ricetta esplicita da eseguire)
.PHONY: all clean test1 test2 test3


# --------- COMPILAZIONE -----------
CC = gcc
CFLAGS += -g -std=c99 -Wall
CXXFLAGS += -DDEBUG
# CXXFLAGS += -DDEBUG -DDEBUG_VERBOSE # stampa informazioni relative all'invio e ricezione dei messaggi
THREAD_FLAGS = -pthread
INCLUDES = -I./$(HEADERS_FOLDER)

all: server client

# --------- CLIENTE E SERVER -----------

server: libs/libserver.so libs/libcomm.so
	$(CC) $(INCLUDES) $(CFLAGS) $(CXXFLAGS) $(THREAD_FLAGS) sources/server.c -o server -Wl,-rpath,./build/libs -L ./build/libs -lserver -lcomm -lm

client: libs/libcomm.so libs/libapi.so
	$(CC) $(INCLUDES) $(CFLAGS) $(CXXFLAGS) sources/client.c -o client -Wl,-rpath,./build/libs -L ./build/libs -lapi -lcomm -lm


# --------- LIBRERIE -----------

libs/libcomm.so: $(OBJECT_FOLDER)/communication.o $(OBJECT_FOLDER)/utils.o
	$(CC) -shared -o $(LIB_FOLDER)/libcomm.so $^

libs/libserver.so: $(OBJECT_FOLDER)/config.o $(OBJECT_FOLDER)/statistics.o $(OBJECT_FOLDER)/ini.o $(OBJECT_FOLDER)/utils.o $(OBJECT_FOLDER)/file_manager.o $(OBJECT_FOLDER)/hashtable.o $(OBJECT_FOLDER)/threadpool.o
	$(CC) -shared -o $(LIB_FOLDER)/libserver.so $^

libs/libapi.so: $(OBJECT_FOLDER)/communication.o $(OBJECT_FOLDER)/api.o $(OBJECT_FOLDER)/utils.o
	$(CC) -shared -o $(LIB_FOLDER)/libapi.so $^

# --------- FILE OGGETTO -----------

$(OBJECT_FOLDER)/api.o: $(API_FOLDER)/api.c $(HEADERS_FOLDER)/api.h
	$(CC) $(INCLUDES) $(CFLAGS) $(CXXFLAGS) $(API_FOLDER)/api.c -c -fPIC -o $@

$(OBJECT_FOLDER)/statistics.o: $(UTL_FOLDER)/statistics.c $(HEADERS_FOLDER)/statistics.h
	$(CC) $(INCLUDES) $(CFLAGS) $(CXXFLAGS) $(UTL_FOLDER)/statistics.c -c -fPIC -o $@

$(OBJECT_FOLDER)/config.o: $(UTL_FOLDER)/config.c $(HEADERS_FOLDER)/config.h
	$(CC) $(INCLUDES) $(CFLAGS) $(CXXFLAGS) $(UTL_FOLDER)/config.c -c -fPIC -o $@

$(OBJECT_FOLDER)/communication.o: $(UTL_FOLDER)/communication.c $(HEADERS_FOLDER)/communication.h
	$(CC) $(INCLUDES) $(CFLAGS) $(CXXFLAGS) $(UTL_FOLDER)/communication.c -c -fPIC -o $@

$(OBJECT_FOLDER)/utils.o: $(UTL_FOLDER)/utils.c $(HEADERS_FOLDER)/utils.h
	$(CC) $(INCLUDES) $(CFLAGS) $(CXXFLAGS) $(UTL_FOLDER)/utils.c -c -fPIC -o $@

$(OBJECT_FOLDER)/ini.o: $(UTL_FOLDER)/ini.c $(HEADERS_FOLDER)/ini.h
	$(CC) $(INCLUDES) $(CFLAGS) $(CXXFLAGS) $(UTL_FOLDER)/ini.c -c -fPIC -o $@

$(OBJECT_FOLDER)/file_manager.o: $(UTL_FOLDER)/file_manager.c $(HEADERS_FOLDER)/file_manager.h
	$(CC) $(INCLUDES) $(CFLAGS) $(CXXFLAGS) $(UTL_FOLDER)/file_manager.c -c -fPIC -o $@

$(OBJECT_FOLDER)/hashtable.o: $(UTL_FOLDER)/hashtable.c $(HEADERS_FOLDER)/hashtable.h
	$(CC) $(INCLUDES) $(CFLAGS) $(CXXFLAGS) $(UTL_FOLDER)/hashtable.c -c -fPIC -o $@

$(OBJECT_FOLDER)/threadpool.o: $(UTL_FOLDER)/threadpool.c $(HEADERS_FOLDER)/threadpool.h
	$(CC) $(INCLUDES) $(CFLAGS) $(CXXFLAGS) $(UTL_FOLDER)/threadpool.c -c -fPIC -o $@

# ------------- TARGET PHONY --------------
clean:
	rm -f -rf build
	rm -f -rf tests/outputs
	rm -f *.$(SOCKET_EXTENSION)
	rm -f server
	rm -f client
	mkdir build
	mkdir $(OBJECT_FOLDER)
	mkdir $(LIB_FOLDER)
	mkdir tests/outputs
	mkdir tests/outputs/Server
	mkdir tests/outputs/deleted
	mkdir tests/outputs/readed


test1:
	@$(MAKE) -s clean
	@$(MAKE) -s all
	@echo "*********************************************"
	@echo "*************** AVVIO TEST 1 ****************"
	@echo "*********************************************"
	@rm -f valgrind_output.txt
	@bash scripts/test1.sh
	@echo "*********************************************"
	@echo "************** TEST 1 SUPERATO **************"
	@echo "*********************************************"

test2:
	@$(MAKE) -s clean
	@$(MAKE) -s all
	@echo "*********************************************"
	@echo "*************** AVVIO TEST 2 ****************"
	@echo "*********************************************"
	@bash scripts/test2.sh
	@echo "*********************************************"
	@echo "************** TEST 2 SUPERATO **************"
	@echo "*********************************************"

test3:
	@$(MAKE) -s clean
	@$(MAKE) -s all
	@echo "*********************************************"
	@echo "********** TEST 3 NON IMPLEMENTATO **********"
	@echo "*********************************************"