#include <bits/stdint-uintn.h>
#include <cstdlib>
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

void HandleUserFork(int& socket){
  std::cout << "handling user" << std::endl;
  HandleUser client(socket);
  bool stop = false;
  //while(!stop){
  client.update();
  //}
  return;
}

int main(int argc, char *argv[]){
  bool gameOver = false;
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
  const int MAXNROFCONNECTIONS = 100;
  HandleError(listen(s_listen, MAXNROFCONNECTIONS), "cannot listen");

  socklen_t clientsSize = sizeof(sockaddr_in);

  int nrOfForks = 0;

  while(!gameOver){
    int ClientSocket;
    sockaddr_in clientaddr;
    if((ClientSocket = accept(s_listen, (struct sockaddr*)&(clientaddr),(socklen_t*)&clientsSize)) >= 0){
      pid_t fork_ID = -1;
      uint8_t tries = 0;
      std::cout << "got a connection" << std::endl;
      while(fork_ID < 0 && tries < 20){
        fork_ID = fork();
        if(fork_ID == 0){
          std::cout << "created new fork process C" << std::endl;
          HandleUserFork(ClientSocket);
          std::cout << "closing socket" << std::endl;
          shutdown(ClientSocket, SHUT_RDWR);
          close(ClientSocket);
          std::cout << "closed socket" << std::endl;
          exit(EXIT_SUCCESS);
          return 1;
        }
        else if(fork_ID > 0){
          nrOfForks++;
          std::cout << "created new fork process P" << std::endl;
        }
        else if(fork_ID < 0){
          std::cout << "couldn't create new fork: " << fork_ID << std::endl;
          tries += 1;
          usleep(500000);
          if(tries < 20){
            std::cout << "trying again" << std::endl;
          }
          else{
            std::cout << "nr of forks got to: " << nrOfForks << std::endl;
          }
        }
      }
    }
  }

  std::cout << "done" << std::endl;
  return(0);
}

