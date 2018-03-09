#include <sys/types.h>
#ifdef _WIN32
  #include <winsock2.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <iostream>
#include <string>
#include <map>
using namespace std;

#define BUFFSIZE 1024

bool alreadySet(bool flag,int opt) {
  if (flag) {
    cerr << "ERROR -2: argument: \"-" << opt << "\" can't be set multiple times\n";
    return true;
  }
  return false;
}

int parseOptions(int argc, char *argv[], string* port) {
  bool pFlag = false;

  int opt;
  while ((opt = getopt (argc, argv, "p:")) != -1) {
    switch (opt) {
      case 'p':
        if (alreadySet(pFlag,opt)) return -2;
        *port = optarg;
        pFlag = true;
        break;
      default:
        cerr << "ERROR -1: invalid option: \"-" << (char)optopt << "\" usage: ./ipk-server -p port\n";
        return -1;
      }
  }

  if (argc <= 1) {
    cerr << "ERROR -1: missing arguments, usage: ./ipk-server -p port\n";
    return -1;
  }

  if (!pFlag) {
    cerr << "ERROR -1: wrong arguments\n";
    return -1;
  }

  return 0;
}


int main(int argc, char *argv[]) {

  string portArg;
  int errCheck = parseOptions(argc,argv,&portArg);
  if (errCheck) return errCheck;

  char sendBuff[BUFFSIZE];
  char recBuff[BUFFSIZE];

  int wellcomeSocket = socket(AF_INET, SOCK_STREAM,0);
  if (wellcomeSocket <= 0) {
    cerr << "ERROR -3: socket creation failure\n";
    return -3;
  }

  int port = stoi(portArg,nullptr);

  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET; // adress family = internet
  serverAddr.sin_addr.s_addr = INADDR_ANY; // listen to any interface
  serverAddr.sin_port = htons(port);// port assigmnet

  if (bind(wellcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
    cerr << "ERROR -4: server bind failure\n";
    close(wellcomeSocket);
    return -4;
  }

  if ((listen(wellcomeSocket, 10)) < 0) { // listen to max 10 connections
    cerr << "ERROR -5: server listen failure\n";
    close(wellcomeSocket);
    return -5;
  }

  while(true) { // TODO
    int connectSocket = accept(wellcomeSocket,(struct sockaddr*)&sa_client, &sa_client_len);
    if (comm_socket > 0) {
      //
    }
  }








  return 0;
}
