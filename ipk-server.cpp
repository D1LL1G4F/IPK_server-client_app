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
#include <fstream>
#include <regex>
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

void decodeRequest(char* buffer,string* login,int* requestType) {
  string message = buffer;
  std::string::size_type sz;   // alias of size_t
  *requestType = stoi(message,&sz);
  *login = message.substr(sz);
}

int main(int argc, char *argv[]) {

  string portArg;
  int errCheck = parseOptions(argc,argv,&portArg);
  if (errCheck) return errCheck;

  int welcomeSocket = socket(AF_INET, SOCK_STREAM,0);
  if (welcomeSocket <= 0) {
    cerr << "ERROR -3: socket creation failure\n";
    return -3;
  }

  int port = stoi(portArg,nullptr);

  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET; // adress family = internet
  serverAddr.sin_addr.s_addr = INADDR_ANY; // listen to any interface
  serverAddr.sin_port = htons(port);// port assigmnet

  if (bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
    cerr << "ERROR -4: server bind failure\n";
    close(welcomeSocket);
    return -4;
  }

  if ((listen(welcomeSocket, 10)) < 0) { // listen to max 10 connections
    cerr << "ERROR -5: server listen failure\n";
    close(welcomeSocket);
    return -5;
  }

  while(true) {
    struct sockaddr_storage serverStorage;
    socklen_t storSize = sizeof(serverStorage);
    int connectSocket = accept(welcomeSocket,(struct sockaddr*)&serverStorage, &storSize);
    if (connectSocket > 0) {
      char sendBuff[BUFFSIZE];
      char recBuff[BUFFSIZE];

      int res = 0;
      while (true) {
        memset(recBuff, 0, BUFFSIZE);
        memset(sendBuff,0,BUFFSIZE);
        res = recv(connectSocket, recBuff, BUFFSIZE,0);
        if (res <= 0) break;

        string login;
        int requestType;

        decodeRequest(recBuff,&login,&requestType);
        ifstream data( "/etc/passwd" );
        regex regExpr;

        switch (requestType) {
          case 1:
            // -n
            for( std::string line; getline( data, line ); ) {

            }
            break;
          case 2:
            // -f
            break;
          case 3:
            // -l
            strcpy(sendBuff,"0");
            regExpr = "^"+login+".*:";
            for( std::string line; getline( data, line ); ) {
              if (regex_match(line,regExpr)) {
                //TODO
              }
            }
            break;
          default:
            strcpy(sendBuff, "1SERVER ERROR: invalid request from client\n");
            break;
        }
        data.close();

        send(connectSocket, sendBuff, BUFFSIZE, 0);
      }
      close(connectSocket);
    }
  }






  close(welcomeSocket);
  return 0;
}
