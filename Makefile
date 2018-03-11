CC = g++
CXXFLAGS = -g -std=c++11 -Wall -Wextra -pedantic
LIBS   = -static-libstdc++

all: ipk-client ipk-server

ipk-client:
	$(CC) $(CXXFLAGS) $(LIBS) ipk-client.cpp -o ipk-client

ipk-server:
	$(CC) $(CXXFLAGS) $(LIBS) ipk-server.cpp -o ipk-server

clean:
	rm ipk-client
	rm ipk-server

testc: ipk-client
	./ipk-client -h 127.0.0.1 -p 7891 -l

tests: ipk-server
	./ipk-server -p 7891
