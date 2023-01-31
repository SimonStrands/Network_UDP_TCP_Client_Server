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
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <thread>
#include <vector>
#include <string>
#include <string.h>//For strtok
#include <iostream>
#include <fcntl.h>
#include <mutex>
#include <arpa/inet.h>
#include <iomanip>

void HandleError(int e, std::string errorMSG){
  if(e < 0){
    std::cout << errorMSG << std::endl; 
    exit(e);
  }
}

bool recver(int argc, char *argv[]){
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
  const int MAXNROFCONNECTIONS = 256;
  HandleError(listen(s_listen, MAXNROFCONNECTIONS), "cannot listen");

  socklen_t clientsSize = sizeof(sockaddr_in);
  bool gameOver = false;

  while(!gameOver){
    sockaddr_in clientaddr;
    int ClientSocket;
    if((ClientSocket = accept(s_listen, (struct sockaddr*)&(clientaddr),(socklen_t*)&clientsSize)) >= 0){
        timeval timeout;
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;
        static const size_t dataSize = 150;
        char bufferData[dataSize];
        memset(bufferData, 0, dataSize);
        int so;
        if ((so = setsockopt (ClientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout))) < 0){
            std::cout << "couldn't make socket have a recv timeout : " << so << std::endl;
        }
        if ((so = setsockopt (ClientSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout))) < 0){
            std::cout << "couldn't make socket have a send timeout : " << so << std::endl;
        }
        int recvSize = 0;
        recvSize = recv(ClientSocket, &bufferData, dataSize, 0);
        if(recvSize < 1){
            std::cout << "recv from client failed" << recvSize << std::endl;
            return false;
        }
        else{
            std::cout << bufferData << std::endl;
        }
        shutdown(ClientSocket, SHUT_RDWR);
        close(ClientSocket);
    }
  }

  close(s_listen);
  std::cout << "done" << std::endl;
  return true;
}

bool sender(){
  int sizeOfRecv = 0;

  //get ip address and port number from argv //why he does it like this idk?
  char *ServerIP;
  char delim[]=":";
  std::string a = "127.0.0.1:5000";
  char* ipandPort = new char[a.size() + 1];
  std::copy(a.begin(), a.end(), ipandPort);
  ServerIP=strtok(ipandPort,delim);
  
  char *ServerPort=strtok(NULL,delim);
  int port = atoi(ServerPort);

  //write server ip and host to console/terminal
  std::cout << "Server IP: " << ServerIP << " ServerPort: " << port << std::endl;

  //create socket
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock == -1)
  {
    std::cout << "socket cannot be created" << std::endl;
    return -1;
  }

  sockaddr_in hint;
  memset(&hint, 0, sizeof(hint));
  hint.sin_family = AF_INET;
  hint.sin_port = htons(port);//htons?
  inet_pton(AF_INET, ServerIP, &hint.sin_addr);

  HandleError(
    connect(sock, (sockaddr*)&hint, sizeof(hint)), 
    "can not connect to server"
    );

    std::string penisSend = "penis";

    //send(sock, penisSend.c_str(), penisSend.length(), 0);
    std::cout << "continue" << std::endl;
    std::cin >> penisSend;

    shutdown(sock, SHUT_RDWR);
    close(sock);

    return true;
}

int main(int argc, char *argv[]){
  bool gameOver = false;
    std::string senderOrRecver;
    std::cout << "s for sender/client, r for recv/server" << std::endl;
    std::cin >> senderOrRecver;

    if(senderOrRecver == "s"){
        sender();
    }
    else{
        recver(argc, argv);
    }

  return 0;
}