#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <iostream>
#include <string>
#include <fstream>
#include <regex>
using namespace std;

#define BUFFSIZE 1024

struct requestMsg {
  int reqOpt;
  char login[BUFFSIZE-sizeof(int)];
};

struct responseMsg {
  int retVal;
  char msg[BUFFSIZE-sizeof(int)];
};

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
  if (buffer == NULL) {
    cerr << "ERROR -99: internal error occured while decoding client request\n";
    exit(-99);
  }

  struct requestMsg request;
  memcpy(&request,buffer,sizeof(char)*BUFFSIZE);
  memset(buffer,0,BUFFSIZE);
  *login = request.login;
  *requestType = request.reqOpt;
}

void flushFullBuffer(char *sendBuff,struct responseMsg* response,int *buffcnt) {
  response->retVal = 1;
  memset(sendBuff,0,BUFFSIZE);
  memcpy(sendBuff,response,sizeof(struct responseMsg));
  memset(response->msg,0,BUFFSIZE-sizeof(int));
  *buffcnt = 0;
}

string parseLine(string line,unsigned col) {
  string result;
  unsigned delimCnt = 0;
  int i = 0;
  while(delimCnt<col-1) {
    if(line.at(i++)==':') {
      delimCnt++;
    }
  }
  int j = 0;
  int start = i;

  while (line.at(i) != ':') {
    i++;
    j++;
  }
  result = line.substr(start,j);
  result = result + "\n";
  return result;
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
        struct responseMsg response;
        response.retVal = -1;
        int buffcnt = 0;

        switch (requestType) {
          case 1:
            // -n
            regExpr = "^"+login+":.*";
            for( std::string line; getline( data, line ); ) {
              if (regex_match(line,regExpr)) {
                response.retVal=0;
                string userData = parseLine(line,5);
                int i = 0;
                char c;
                c = userData.at(i);
                while (true) {
                  if (buffcnt >= (int)(BUFFSIZE-sizeof(int))) {
                    flushFullBuffer(sendBuff,&response,&buffcnt);
                    send(connectSocket,sendBuff, BUFFSIZE, 0);
                    memset(sendBuff,0,BUFFSIZE);
                  }
                  response.msg[buffcnt] = c;
                  buffcnt++;
                  if (c == '\n') {
                    break;
                  } else {
                    c = userData.at(++i);
                  }
                }
              }
            }

            if (response.retVal < 0) strcpy(response.msg, "SERVER ERROR: login not found\n");
            memcpy(&sendBuff,&response,sizeof(response));
            send(connectSocket, sendBuff, BUFFSIZE, 0);
            memset(response.msg,0,BUFFSIZE-sizeof(int));
            memset(sendBuff,0,BUFFSIZE);
            break;
          case 2:
            // -f
            regExpr = "^"+login+":.*";
            for( std::string line; getline( data, line ); ) {
              if (regex_match(line,regExpr)) {
                response.retVal=0;
                string userData = parseLine(line,6);
                int i = 0;
                char c;
                c = userData.at(i);
                while (true) {
                  if (buffcnt >= (int)(BUFFSIZE-sizeof(int))) {
                    flushFullBuffer(sendBuff,&response,&buffcnt);
                    send(connectSocket,sendBuff, BUFFSIZE, 0);
                    memset(sendBuff,0,BUFFSIZE);
                  }
                  response.msg[buffcnt] = c;
                  buffcnt++;
                  if (c == '\n') {
                    break;
                  } else {
                    c = userData.at(++i);
                  }
                }
              }
            }

            if (response.retVal < 0) strcpy(response.msg, "SERVER ERROR: login not found\n");
            memcpy(&sendBuff,&response,sizeof(response));
            send(connectSocket, sendBuff, BUFFSIZE, 0);
            memset(response.msg,0,BUFFSIZE-sizeof(int));
            memset(sendBuff,0,BUFFSIZE);
            break;
          case 3:
            // -l
            regExpr = "^"+login+".*:.*";

            for( string line; getline( data, line ); ) {
              if (regex_match(line,regExpr)) {
                response.retVal=0;
                char c;
                int i = 0;
                c = line.at(i);
                while (c != ':') {
                  if (buffcnt >= (int)(BUFFSIZE-sizeof(int))) {
                    flushFullBuffer(sendBuff,&response,&buffcnt);
                    send(connectSocket,sendBuff, BUFFSIZE, 0);
                    memset(sendBuff,0,BUFFSIZE);
                  }
                  response.msg[buffcnt] = c;
                  buffcnt++;
                  c = line.at(++i);
                }
                if (buffcnt >= (int)(BUFFSIZE-sizeof(int))) {
                  flushFullBuffer(sendBuff,&response,&buffcnt);
                  send(connectSocket,sendBuff, BUFFSIZE, 0);
                  memset(sendBuff,0,BUFFSIZE);
                }
                response.msg[buffcnt] = '\n';
                buffcnt++;
              }
            }

            if (response.retVal < 0) strcpy(response.msg, "SERVER ERROR: login not found\n");
            memcpy(&sendBuff,&response,sizeof(response));
            send(connectSocket, sendBuff, BUFFSIZE, 0);
            memset(response.msg,0,BUFFSIZE-sizeof(int));
            memset(sendBuff,0,BUFFSIZE);


            break;
          default:
            response.retVal = -1;
            strcpy(response.msg, "SERVER ERROR: invalid request from client\n");
            memcpy(&sendBuff,&response,sizeof(response));
            send(connectSocket, sendBuff, BUFFSIZE, 0);
            memset(sendBuff,0,BUFFSIZE);
            break;
        }
        data.close();
      }
      close(connectSocket);
    }
  }






  close(welcomeSocket);
  return 0;
}
