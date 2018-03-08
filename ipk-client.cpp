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

  return 0;
}

int main(int argc,char *argv[]) {

  struct Options options;
  int opt = parseOptions(argc,argv,&options);
  if (opt < 0) return opt;

  int clientSocket = socket(AF_INET, SOCK_STREAM,0);
  if (clientSocket <= 0) {
    cerr << "ERROR -3: socket creation failure\n";
    return -3;
  }

  sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET; // adress family = internet
  int port = stoi(options.port,nullptr);
  serverAddr.sin_port = htons(port); // port assigmnet
  serverAddr.sin_addr.s_addr = inet_addr(options.host.c_str()); // set adress to host
  






  return EXIT_SUCCESS;
}
