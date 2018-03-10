CC=g++
CFLAGS=-I.
CXXFLAGS= -std=c++11

all: ipk-client ipk-server

ipk-client: ipk-client.o
	$(CC) $(CXXFLAGS) -o ipk-client ipk-client.o $(CFLAGS)

ipk-server: ipk-server.o
	$(CC) $(CXXFLAGS) -o ipk-server ipk-server.o $(CFLAGS)

clean:
	rm -f ipk-client.o ipk-server.o

testc: ipk-client
	./ipk-client -h 127.0.0.1 -p 7891 -l 

tests: ipk-server
	./ipk-server -p 7891
