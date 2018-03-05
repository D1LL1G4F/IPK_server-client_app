#include <sys/types.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <iostream>
#include <string>
#include <map>
using namespace std;

class Options {
  public:
    bool hFlag = false;
    bool pFlag = false;
    bool nFlag = false;
    bool fFlag = false;
    bool lFlag = false;
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

int parseOptions(int argc, char *argv[],Options option) {
  int opt;
  while ((opt = getopt (argc, argv, "h:p:nfl")) != -1) {
    switch (opt) {
      case 'h':
        if (alreadySet(option.hFlag,opt)) return -2;
        option.host = optarg;
        cout << "-h: " << option.host << "\n";
        break;
      case 'p':
        if (alreadySet(option.pFlag,opt)) return -2;
        option.port = optarg;
        cout << "-p: " << option.port << "\n";
        break;
      case 'n':
        if (alreadySet(option.nFlag,opt)) return -2;
        cout << "-n\n";
        break;
      case 'f':
        if (alreadySet(option.fFlag,opt)) return -2;
        cout << "-f\n";
        break;
      case 'l':
        if (alreadySet(option.lFlag,opt)) return -2;
        cout << "-l\n";
        break;
      default:
        cerr << "ERROR -1: invalid option: \"-" << (char)optopt << "\" \n";
        return -1;
      }
  }
  if (optind == argc - 1) {
    option.login = argv[optind];
    cout << "login:" << option.login << "\n";
  }
  return EXIT_SUCCESS;
}

int main(int argc,char *argv[]) {

  Options option;
  int invalidOpt = parseOptions(argc,argv,option);
  if (invalidOpt) return invalidOpt;

  
  return EXIT_SUCCESS;
}
