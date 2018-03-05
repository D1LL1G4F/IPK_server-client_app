CC=g++
CFLAGS= -std=c++11 -I. 

ipk-client: ipk-client.o
	$(CC) -o ipk-client ipk-client.o $(CFLAGS)

ipk-server: ipk-server.o
	$(CC) -o ipk-server ipk-server.o $(CFLAGS)

clean:
	rm -f ipk-client.o ipk-server.o

testc: ipk-client
	./ipk-client

tests: ipk-server
	./ipk-server
