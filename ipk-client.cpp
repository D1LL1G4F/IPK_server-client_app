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

struct requestMsg {
  int reqOpt;
  char login[BUFFSIZE-sizeof(int)];
};

struct responseMsg {
  int retVal;
  char msg[BUFFSIZE-sizeof(int)];
};

struct Options {
    bool hFlag;
    bool pFlag;
    bool nFlag;
    bool fFlag;
    bool lFlag;
    string host;
    string port;
    string login;
};

bool alreadySet(bool flag,int opt) {
  if (flag) {
    cerr << "ERROR -2: argument: \"-" << opt << "\" can't be set multiple times\n";
    return true;
  }
  return false;
}

int parseOptions(int argc, char *argv[],Options *option) {
  option->hFlag = false;
  option->pFlag = false;
  option->nFlag = false;
  option->fFlag = false;
  option->lFlag = false;

  int opt;
  while ((opt = getopt (argc, argv, "h:p:nfl")) != -1) {
    switch (opt) {
      case 'h':
        if (alreadySet(option->hFlag,opt)) return -2;
        option->host = optarg;
        option->hFlag = true;
        break;
      case 'p':
        if (alreadySet(option->pFlag,opt)) return -2;
        option->port = optarg;
        option->pFlag = true;
        break;
      case 'n':
        if (alreadySet(option->nFlag,opt)) return -2;
        if (option->fFlag || option->lFlag) {
          cerr << "ERROR -2: there can be selected only one option of [-n -f -l]\n";
          return -2;
        }
        option->nFlag = true;
        break;
      case 'f':
        if (alreadySet(option->fFlag,opt)) return -2;
        if (option->nFlag || option->lFlag) {
          cerr << "ERROR -2: there can be selected only one option of [-n -f -l]\n";
          return -2;
        }
        option->fFlag = true;
        break;
      case 'l':
        if (alreadySet(option->lFlag,opt)) return -2;
        if (option->fFlag || option->nFlag) {
          cerr << "ERROR -2: there can be selected only one option of [-n -f -l]\n";
          return -2;
        }
        option->lFlag = true;
        break;
      default:
        cerr << "ERROR -1: invalid option: \"-" << (char)optopt << "\" usage: ./ipk-client -h host -p port [-n|-f|-l] login \n";
        return -1;
      }
  }

  if (!option->hFlag || !option->pFlag) {
    cerr << "ERROR -1: wrong arguments\n";
    return -1;
  }

  if (option->nFlag) {
    if (optind+1==argc) {
      option->login = argv[optind];
      return 1;
    } else {
      cerr << "ERROR -2: missing login argument\n";
      return -2;
    }
  }
  if (option->fFlag) {
    if (optind+1==argc) {
      option->login = argv[optind];
      return 2;
    } else {
      cerr << "ERROR -2: missing login argument\n";
      return -2;
    }
  }
  if (option->lFlag) {
    if (optind+1==argc) {
      option->login = argv[optind];
    }
    return 3;
  }

  if (optind+1==argc) {
    option->login = argv[optind];
  } else {
    cerr << "ERROR -2: missing login argument\n";
    return -2;
  }

  cerr << "ERROR -1: missing option, usage: ./ipk-client -h host -p port [-n|-f|-l] login \n";
  return -1;
}

void createRequest(char *buffer,int reqOpt, string login) {
  if (buffer == NULL) {
    cerr << "ERROR -99: internal error occured while creating request message\n";
    exit(-99);
  }
  if (sizeof(login)+sizeof(int) > BUFFSIZE) {
    cerr << "ERROR -98: login is too big!\n";
    exit(-98);
  }
  memset(buffer, 0, BUFFSIZE);
  struct requestMsg request;
  request.reqOpt = reqOpt;
  if (!login.empty()) {
    strcpy(request.login,login.c_str());
  } else {
    strcpy(request.login,"");
  }
  memcpy(buffer,&request,sizeof(request));
}

int decodeResponse(char* buffer) {
  if (buffer == NULL) {
    cerr << "ERROR -99: internal error occured while decoding client request\n";
    exit(-99);
  }

  struct responseMsg response;
  memcpy(&response,buffer,sizeof(response));
  memset(buffer, 0, BUFFSIZE);
  if (response.retVal < 0) {
    cerr << response.msg;
    return response.retVal;
  } else {
    cout << response.msg << flush;
  }
  return response.retVal;
}

int main(int argc,char *argv[]) {

  struct Options options;
  int opt = parseOptions(argc,argv,&options);
  if (opt < 0) return opt;

  char sendBuff[BUFFSIZE];
  char recBuff[BUFFSIZE];
  int clientSocket = socket(AF_INET, SOCK_STREAM,0);
  if (clientSocket <= 0) {
    cerr << "ERROR -3: socket creation failure\n";
    return -3;
  }

  int port = stoi(options.port,nullptr);

  

  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET; // adress family = internet
  serverAddr.sin_port = htons(port);; // port assigmnet
  //serverAddr.sin_addr.s_addr = inet_addr(options.host.c_str()); // set adress to host

  if (connect(clientSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
    cerr << "ERROR -4: connection failure\n";
    close(clientSocket);
    return -4;
  }

  createRequest(sendBuff,opt,options.login);

  if (send(clientSocket, sendBuff, BUFFSIZE, 0) < 0) {
    cerr << "ERROR -5: sending data failure\n";
    close(clientSocket);
    return -5;
  }

  if(recv(clientSocket, recBuff, BUFFSIZE, 0) < 0) {
    cerr << "ERROR -6: recieving data failure\n";
    close(clientSocket);
    return -6;
  }

  int retVal = decodeResponse(recBuff);
  while (retVal) {
    if (retVal < 0) break;

    if(recv(clientSocket, recBuff, BUFFSIZE, 0) < 0) {
      cerr << "ERROR -6: recieving data failure\n";
      close(clientSocket);
      retVal =-6;
      break;
    }
    retVal = decodeResponse(recBuff);
  }

  close(clientSocket);
  return retVal;
}
