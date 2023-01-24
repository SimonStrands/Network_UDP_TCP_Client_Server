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

void HandleUserThread(int socket){
  HandleUser client(socket);
  bool stop = false;
  while(!stop){
    stop = !client.update();
  }
  std::cout << "delete client" << std::endl;
  shutdown(socket, SHUT_RDWR);
  close(socket);
  std::cout << "deleted client" << std::endl;
  return;
}

int main(int argc, char *argv[]){
  bool gameOver = false;
  std::cout << "Threaded SERVER" << std::endl;
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

  //bind listen socket
  HandleError(bind(s_listen, addr->ai_addr, addr->ai_addrlen), "cannot bind socket");

  //start listening
  const int MAXNROFCONNECTIONS = 10;
  HandleError(listen(s_listen, MAXNROFCONNECTIONS), "cannot listen");

  socklen_t clientsSize = sizeof(sockaddr_in);

  std::vector<std::thread> threads;

  while(!gameOver){
    sockaddr_in clientaddr;
    int ClientSocket;
    if((ClientSocket = accept(s_listen, (struct sockaddr*)&(clientaddr),(socklen_t*)&clientsSize)) >= 0){
      std::cout << "got a new client" << std::endl;
      tPool.setJob(HandleUserThread, ClientSocket);
    }
  }

  close(s_listen);
  std::cout << "done" << std::endl;
  return(0);
}
