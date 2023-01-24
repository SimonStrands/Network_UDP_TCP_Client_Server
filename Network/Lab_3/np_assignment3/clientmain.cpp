#include <cstdlib>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>//why?
#include <stdlib.h>
// You will to add includes here 
#include <sys/socket.h>
#include <sys/types.h>
#include <string>
#include <iostream>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <iomanip>
#include <sstream>
#include <vector>
#include <math.h>
#include <fcntl.h>
#include <thread>
// Enable if you want debugging to be printed, see examble below.
// Alternative, pass CFLAGS=-DDEBUG to make, make CFLAGS=-DDEBUG
#define DEBUG

void HandleError(int e, std::string errorMSG){
  if(e == -1){
    std::cout << errorMSG << std::endl; 
    exit(e);
  }
}

bool SetSocketBlockingEnabled(int fd, bool blocking)
{
   if (fd < 0) return false;

#ifdef _WIN32
   unsigned long mode = blocking ? 0 : 1;
   return (ioctlsocket(fd, FIONBIO, &mode) == 0) ? true : false;
#else
   int flags = fcntl(fd, F_GETFL, 0);
   if (flags == -1) return false;
   flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
   return (fcntl(fd, F_SETFL, flags) == 0) ? true : false;
#endif
}

void getInput(std::string &answear, bool& sent, bool& quit)
{
  while(answear != "QUIT" && !quit){
    while(sent)//apperently in clang++ when std::getline is called "answear" is wiped
    {
    }
    std::getline(std::cin, answear);
    if(answear.length() > 251){
      answear = "";
      std::cout << "Too long of a message to send" << std::endl;
      continue;
    }
    sent = true;
  }
  return;
}

int main(int argc, char *argv[]){
  int dataSize = 255;
  char bufferData[dataSize];
  memset(bufferData, 0, dataSize);
  int sizeOfRecv = 0;

  ////Getting  IP : PORT : USERNAME//////////
  char delim[]=":";
  if(argc < 3){
    std::cout << "please provide IP:PORT and username" << std::endl;
    return -1;
  }
  
  std::string ServerIP = strtok(argv[1],delim);
  std::string ServerPort = strtok(NULL,delim);
  int port = std::stoi(ServerPort);
  std::string userName = argv[2];

  //CHECK NAME
  int illegal[] = {
    96,94,93,91
  };
  if(userName.length() > 12){
    std::cout << "name can only be max of 12 characters" << std::endl;
    return -1;
  }
  for(int i =  0; i < userName.length(); i++){
    if((userName[i] < 48 || userName[i] > 122) || (userName[i] > 57 && userName[i] < 65)){
      std::cout << "cannot use " << userName[i] << " in name" << std::endl;
      return -1;
    }
    for(int x = 0; x < 4; x++){
      if(userName[i] == illegal[x]){
        std::cout << "cannot use " << userName[i] << " in name" << std::endl;
        return -1;
      }
    }
  }
  ////////////

  std::cout << "Server IP: " << ServerIP << " ServerPort: " << port << " UserName: " << userName << std::endl;
  ////////////////////////////////////////////

  struct addrinfo hints = {}, *res;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  if(getaddrinfo(ServerIP.c_str(), NULL, &hints, &res) != 0){
    return -1;
  }

  ////CREATE SOCKET /////////////////////////
  int sock = socket(res->ai_family, res->ai_socktype, 0);
  if(sock == -1)
  {
    std::cout << "socket cannot be created" << std::endl;
    return -1;
  }
  
  sockaddr_in hint;
  memset(&hint, 0, sizeof(hint));
  hint.sin_family = res->ai_family;
  hint.sin_port = htons(port);//htons?
  inet_pton(res->ai_family, ServerIP.c_str(), &hint.sin_addr);

  //connect to server
  HandleError(
    connect(sock, (sockaddr*)&hint, sizeof(hint)), 
    "can not connect to server"
    );
  //SEND NICK
  std::string SendString = "NICK " + userName;
  HandleError(send(sock, SendString.c_str(), SendString.length(), 0), "error sending nick to server");

  #ifdef DEBUG 
  std::cout << "connected to server" << std::endl; 
  #endif
  SetSocketBlockingEnabled(sock, false);
  
  bool gameOver = false;
  bool sendMessage = false;
  std::string fullSendString;
  std::thread InputThread(getInput, std::ref(SendString), std::ref(sendMessage), std::ref(gameOver));
  
  while(!gameOver){
    memset(bufferData, 0, dataSize);
    sizeOfRecv = recv(sock, bufferData, dataSize, 0);
    if(sizeOfRecv > 0){
      //handle message
      std::cout << bufferData << std::endl;
    }
    if(sendMessage){
      if(SendString == "QUIT"){
        gameOver = true;
        InputThread.join();
      }
      else{
        sendMessage = false;
        fullSendString = "MSG " + SendString;
        send(sock, fullSendString.c_str(), fullSendString.length(), 0);
      } 
    }
  }
  close(sock);
  
  return 0;
}
