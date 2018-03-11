#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include <iostream>
#include <string>
#include <map>
using namespace std;

#define BUFFSIZE 1024

/* protocol for request to server */
struct requestMsg {
  int reqOpt;
  char login[BUFFSIZE-sizeof(int)];
};

/* protocol for response from server */
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
  if (sizeof(login)+sizeof(int) > BUFFSIZE) { // check login size
    cerr << "ERROR -98: login is too big!\n";
    exit(-98);
  }
  memset(buffer, 0, BUFFSIZE); // clear buffer
  struct requestMsg request; // create protocol for request
  request.reqOpt = reqOpt; // setup request type
  if (!login.empty()) { // if login is set setup login
    strcpy(request.login,login.c_str());
  } else {
    strcpy(request.login,"");
  }
  memcpy(buffer,&request,sizeof(request)); // map protocol to buffer
}

int decodeResponse(char* buffer) {
  if (buffer == NULL) {
    cerr << "ERROR -99: internal error occured while decoding response from server\n";
    exit(-99);
  }

  struct responseMsg response; // create protocol for response
  memcpy(&response,buffer,sizeof(response)); // map buffer to response protocol
  memset(buffer, 0, BUFFSIZE); // clear buffer
  if (response.retVal < 0) { // if error uccured read message to  stderr
    cerr << response.msg;
    return response.retVal; // return error
  } else {
    cout << response.msg << flush; // read message to stdout
  }
  return response.retVal; // (0 if session ended successfuly and 1 if more messages are expected)
}

int main(int argc,char *argv[]) {

  /* input argument parsing */
  struct Options options;
  int opt = parseOptions(argc,argv,&options);
  if (opt < 0) return opt;
  ////////////////////////////

  /* creating buffers and socket */
  char sendBuff[BUFFSIZE]; // sending buffer
  char recBuff[BUFFSIZE]; // recieving buffer
  int clientSocket = socket(AF_INET, SOCK_STREAM,0);
  if (clientSocket <= 0) {
    cerr << "ERROR -3: socket creation failure\n";
    return -3;
  }
  ////////////////////////////////////////////

  int port = stoi(options.port,nullptr); // string port num to int

  /* connecting to server */
  struct hostent *server;
  if ((server = gethostbyname(options.host.c_str())) == NULL) {
    cerr << "ERROR -7: no such host as " << options.host << "\n";
    return -7;
  }

  struct sockaddr_in serverAddr;
  bzero((char *) &serverAddr, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET; // adress family = internet
  serverAddr.sin_port = htons(port);; // port assigmnet
  bcopy((char *)server->h_addr, (char *)&serverAddr.sin_addr.s_addr,server->h_length);

  if (connect(clientSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
    cerr << "ERROR -4: connection failure\n";
    close(clientSocket);
    return -4;
  }
  //////////////////////////

  /* filling sending buffer with request */
  createRequest(sendBuff,opt,options.login);
  /////////////////////////////////////////

  /* sending buffer with request to server */
  if (send(clientSocket, sendBuff, BUFFSIZE, 0) < 0) {
    cerr << "ERROR -5: sending data failure\n";
    close(clientSocket);
    return -5;
  }
  ///////////////////////////////////////////

  /* recieving response from server */
  if(recv(clientSocket, recBuff, BUFFSIZE, 0) < 0) {
    cerr << "ERROR -6: recieving data failure\n";
    close(clientSocket);
    return -6;
  }
  ///////////////////////////////////

  /* decoding resposne from server */
  int retVal = decodeResponse(recBuff); // if response response is full and OK 0 is returned
  while (retVal) {
    if (retVal < 0) break; // if error occured from server negativ value is returned

    // if more response is expected positive valie is returned
    if(recv(clientSocket, recBuff, BUFFSIZE, 0) < 0) { // recieving aditional response
      cerr << "ERROR -6: recieving data failure\n";
      close(clientSocket);
      retVal =-6;
      break;
    }
    retVal = decodeResponse(recBuff);
  }
  ////////////////////////////////////

  close(clientSocket);
  return retVal;
}
