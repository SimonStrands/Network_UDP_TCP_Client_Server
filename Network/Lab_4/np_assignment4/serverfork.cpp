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
#include <sys/wait.h>


void HandleError(int e, std::string errorMSG){
  if(e < 0){
    std::cout << errorMSG << std::endl; 
    exit(e);
  }
}

void HandleWarnings(int e, std::string warningMSG){
  if(e < 0){
    DEBUG_MSG(warningMSG); 
  }
}

void WaitForFork(){
  int status = 0;
  pid_t wpid;
  while ((wpid = wait(&status)) > 0);
}

void HandleUserFork(int socket){
  DEBUG_MSG("handling user");
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
    break;
    }
  }
  return;
}


int main(int argc, char *argv[]){
  std::cout << "Forked server" << std::endl;
  bool gameOver = false;
  std::string PORT = "5000";  // 5000 is standard for this server
  std::string ipaddress = "0.0.0.0"; //take what is avalible

  if(argc > 1){
    std::string ipAndPort = argv[1];
    bool done = false;
    for(int i = ipAndPort.size(); i > 0 && !done; i--){
      if(ipAndPort[i] == ':'){
        done = true;
        ipaddress = ipAndPort.substr(0, i);
        PORT = ipAndPort.substr(i+1, ipAndPort.size() - i);
      }
    }
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

  while(!gameOver){
    int ClientSocket;
    sockaddr clientaddr;
    if((ClientSocket = accept(s_listen, &clientaddr,(socklen_t*)&clientsSize)) >= 0){
      pid_t fork_ID = -1;
      uint8_t tries = 0;
      DEBUG_MSG("got a connection");
      while(fork_ID < 0 && tries < 20){
        fork_ID = fork();
        if(fork_ID == 0){
          DEBUG_MSG("created new fork process C");
          HandleUserFork(ClientSocket);
          DEBUG_MSG("closing socket");
          shutdown(ClientSocket, SHUT_RDWR);
          close(ClientSocket);
          DEBUG_MSG("closed socket");
          exit(EXIT_SUCCESS);
          return 1;
        }
        else if(fork_ID > 0){
          close(ClientSocket);
          DEBUG_MSG("created new fork process P");
        }
        else if(fork_ID < 0){
          DEBUG_MSG("couldn't create new fork: " << fork_ID);
          tries += 1;
          WaitForFork();
          if(tries < 20){
            DEBUG_MSG("trying again");
          }
          else{
            DEBUG_MSG("nr of forks got to: ");
          }
        }
      }
    }
    else{
      std::cout << "cannot create socket" << std::endl;
      exit(-1);
    }
  }

  std::cout << "done" << std::endl;
  return(0);
}

