#include <cstdlib>
#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <sys/time.h>
#include <vector>
/* You will to add includes here */
#include "HandleUser.h"
#include "ThreadPool.h"

void HandleError(int e, std::string errorMSG){
  if(e < 0){
    std::cout << errorMSG << std::endl; 
    exit(e);
  }
}

void HandleWarnings(int e, std::string warningMSG){
  if(e < 0){
    std::cout << warningMSG << std::endl; 
  }
}

void HandleUserThread(int socket, int& tRet){
  HandleUser client(socket);
  bool stop = false;
  updateReturn r = client.update();
  if(r != updateReturn::AllGood){
    switch (r) {
    case RecvZero:
      std::cout << "recv 0" << std::endl;
    break;
    case RecvMinus:
      std::cout << "recv minus" << std::endl;
    break;
    case NoGet:
      std::cout << "didn't get anything" << std::endl;
    break;
    default:
      std::cout << "something went wrong" << std::endl;
    break;
    }
  }
  shutdown(socket, SHUT_RDWR);
  close(socket);
  DEBUG_MSG("deleted client");
  tRet = 2;
  return;
}

int main(int argc, char *argv[]){
  bool gameOver = false;
  std::cout << "Threaded SERVER 1.0" << std::endl;
  ThreadPool tPool;
  std::string PORT = "5000";  // 5000 is standard for this server
  std::string ipaddress = "0.0.0.0"; //take what is avalible

  if(argc > 1){
    char delim[]=":";
    char* serverIP = strtok(argv[1], delim); 
    ipaddress = serverIP;
    char* serverPort = strtok(NULL, delim);
    PORT = serverPort;
  }

  struct addrinfo hints = {};
  struct addrinfo *addr;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  if(getaddrinfo(
    ipaddress.c_str(), 
    PORT.c_str(), 
    &hints, &addr) != 0)
  {
    std::cout << "error" << std::endl;
    return -1;
  }

  //create listen socket
  int s_listen = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
  HandleError(s_listen, "cannot create listen");

  int y = 1;
  HandleError(setsockopt(s_listen, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int)), "couldn't setsockoptions");

  //bind listen socket
  HandleError(bind(s_listen, addr->ai_addr, addr->ai_addrlen), "cannot bind socket");

  //start listening
  const int MAXNROFCONNECTIONS = 61;
  HandleError(listen(s_listen, MAXNROFCONNECTIONS), "cannot listen");

  socklen_t clientsSize = sizeof(sockaddr_in);

  while(!gameOver){
    sockaddr_in clientaddr;

    //wait until we have a thread
    #if defined (NotPooled)
      while (!tPool.HaveThread()) {};
    #endif

    int ClientSocket = accept(s_listen, (struct sockaddr*)&(clientaddr),(socklen_t*)&clientsSize);
    if(ClientSocket >= 0){
      DEBUG_MSG("got a new client");
      #if defined (NotPooled)
        if(!tPool.setJob(HandleUserThread, ClientSocket)){
          std::cout << "couldn't set job" << std::endl;
          exit(-1);
        }
      #else
        tPool.setJob(HandleUserThread, ClientSocket);
      #endif
    }
    else{
      std::cout << "couldn't accept socket "  << ClientSocket << std::endl;
    }
  }

  close(s_listen);
  std::cout << "done" << std::endl;
  return(0);
}
