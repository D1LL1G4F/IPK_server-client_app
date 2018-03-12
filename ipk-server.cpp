/* PROJECT no.1 for IPK (client/server app)
*
* author: Matej Knazik
* login: xknazi00
*/


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

/* protocol for request from client */
struct requestMsg {
  int reqOpt;
  char login[BUFFSIZE-sizeof(int)];
};

/* protocol for response to client */
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

  struct requestMsg request; // create protocol for request
  memcpy(&request,buffer,sizeof(char)*BUFFSIZE); // map buffer to protocol
  memset(buffer,0,BUFFSIZE); // emppty buffer
  *login = request.login; // set recieved login
  *requestType = request.reqOpt; // set recieved option from client
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

bool checkBufferOverflow(int buffcnt) {
  return buffcnt >= (int)(BUFFSIZE-sizeof(int));
}

int main(int argc, char *argv[]) {

  /* program arguments parsing */
  string portArg;
  int errCheck = parseOptions(argc,argv,&portArg);
  if (errCheck) return errCheck;
  ///////////////////////////////

  /* setting up welcome socket */
  int welcomeSocket = socket(AF_INET, SOCK_STREAM,0);
  if (welcomeSocket <= 0) {
    cerr << "ERROR -3: socket creation failure\n";
    return -3;
  }
  /////////////////////////////

  int port = stoi(portArg,nullptr); // string port num to int

  /* binding welcome socket */
  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET; // adress family = internet
  serverAddr.sin_addr.s_addr = INADDR_ANY; // listen to any interface
  serverAddr.sin_port = htons(port);// port assigmnet

  if (bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
    cerr << "ERROR -4: server bind failure\n";
    close(welcomeSocket);
    return -4;
  }
  ////////////////////////////

  /* seting up listener */
  if ((listen(welcomeSocket, 10)) < 0) { // listen to max 10 connections
    cerr << "ERROR -5: server listen failure\n";
    close(welcomeSocket);
    return -5;
  }
  ////////////////////////

  while(true) {
    struct sockaddr_storage serverStorage;
    socklen_t storSize = sizeof(serverStorage);
    int connectSocket = accept(welcomeSocket,(struct sockaddr*)&serverStorage, &storSize); // wait for established connection
    if (connectSocket > 0) { // connection is established
      char sendBuff[BUFFSIZE];
      char recBuff[BUFFSIZE];

      int res = 0;

      /* read/write section */
      while (true) {

        // clear buffers
        memset(recBuff, 0, BUFFSIZE);
        memset(sendBuff,0,BUFFSIZE);


        res = recv(connectSocket, recBuff, BUFFSIZE,0);// recieve message from client
        if (res <= 0) break;

        string login;
        int requestType;

        decodeRequest(recBuff,&login,&requestType); // extract type of request and login from client request

        ifstream data( "/etc/passwd" );
        regex regExpr;
        struct responseMsg response;
        response.retVal = -1; // set error as default response value
        int buffcnt = 0; // buffer counter
        int col = 6; // col to extract for -n and -f options

        switch (requestType) { // serve client by his request type
          case 1:
            // -n option
            col = 5;
          case 2:
            // -f option
            regExpr = "^"+login+":.*";
            for( std::string line; getline( data, line ); ) {
              if (regex_match(line,regExpr)) {
                response.retVal=0;
                string userData = parseLine(line,col);
                int i = 0;
                char c;
                c = userData.at(i);
                while (true) {
                  if (checkBufferOverflow(buffcnt)) {
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
            // -l option
            regExpr = "^"+login+".*:.*";

            for( string line; getline( data, line ); ) {
              if (regex_match(line,regExpr)) {
                response.retVal=0;
                char c;
                int i = 0;
                c = line.at(i);
                while (c != ':') {
                  if (checkBufferOverflow(buffcnt)) {
                    flushFullBuffer(sendBuff,&response,&buffcnt);
                    send(connectSocket,sendBuff, BUFFSIZE, 0);
                    memset(sendBuff,0,BUFFSIZE);
                  }
                  response.msg[buffcnt] = c;
                  buffcnt++;
                  c = line.at(++i);
                }
                if (checkBufferOverflow(buffcnt)) {
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
