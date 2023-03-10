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
#include <regex>
// Enable if you want debugging to be printed, see examble below.
// Alternative, pass CFLAGS=-DDEBUG to make, make CFLAGS=-DDEBUG
#define DEBUG

void HandleError(int e, std::string errorMSG){
  if(e == -1){
    std::cout << errorMSG << std::endl; 
    exit(e);
  }
}

//copied from stackoverflow https://stackoverflow.com/questions/31004997/how-to-make-non-blocking-openssl-connection
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
  if(argc < 3){
    std::cout << "please provide IP:PORT and username" << std::endl;
    std::cout << argc << std::endl;
    return -1;
  }

  std::string ServerIP;
  std::string ServerPort;
  std::string userName;

  std::string ipAndPort = argv[1];
  //get last ':' in string and that is the port
  bool done = false;
  for(int i = ipAndPort.size(); i > 0 && !done; i--){
    if(ipAndPort[i] == ':'){
      done = true;
      ServerIP = ipAndPort.substr(0, i);
      ServerPort = ipAndPort.substr(i+1, ipAndPort.size() - i);
    }
  }
  userName = argv[2];
  

  int port = std::stoi(ServerPort);

  //CHECK NAME
  //int illegal[] = {
  //  96,94,93,91
  //};
  if(userName.length() > 12){
    std::cout << "name can only be max of 12 characters" << std::endl;
    return -1;
  }
  std::regex reg("[^A-Za-z0-9\_]");
  std::smatch mathes;
  std::smatch matches;
  while(std::regex_search(userName, matches, reg)){
      if(matches.size() > 0){
          std::cout << userName << " is not a valid name" << std::endl << matches.suffix().str() << std::endl;
          return -1;
      }
  }


  ////////////

  std::cout << "Server IP: " << ServerIP << " ServerPort: " << port << " UserName: " << userName << std::endl;
  ////////////////////////////////////////////

  struct addrinfo hints = {}, *res;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  if(getaddrinfo(ServerIP.c_str(), ServerPort.c_str(), &hints, &res) != 0){
    return -1;
  }

  ////CREATE SOCKET /////////////////////////
  int sock = socket(res->ai_family, res->ai_socktype, 0);
  if(sock == -1)
  {
    std::cout << "socket cannot be created" << std::endl;
    return -1;
  }
  

  if(res->ai_family == AF_INET){
    
    sockaddr_in* hint2 = (sockaddr_in*)res->ai_addr;
    inet_pton(res->ai_family, ServerIP.c_str(), &hint2->sin_addr);
    HandleError(
    connect(sock, (sockaddr*)hint2, sizeof(*hint2)), 
      "cannot connect to server"
    );
  }
  else if(res->ai_family == AF_INET6){
    sockaddr_in6* hint2 = (sockaddr_in6*)res->ai_addr;
    inet_pton(res->ai_family, ServerIP.c_str(), &hint2->sin6_addr);
    HandleError(
    connect(sock, (sockaddr*)hint2, sizeof(*hint2)), 
      "cannot connect to server"
    );
  }
  //SEND NICK
  std::string SendString = "NICK " + userName;
  HandleError(send(sock, SendString.c_str(), SendString.length(), 0), "error sending nick to server");

  #ifdef DEBUG 
  std::cout << "connected to server" << std::endl; 
  #endif
  //get protocol
  memset(bufferData, 0, dataSize);
  sizeOfRecv = recv(sock, bufferData, dataSize, 0);
  if(sizeOfRecv > 0){
    std::string a(bufferData);
    if((a.substr(0,5) == "Hello" || a.substr(0,5) == "HELLO")&& (a.substr(6,2) == "1\n" || a.substr(6,4) == "1.0\n" || a.substr(6,1) == "1" || a.substr(6,3) == "1.0")){
      std::cout << "Supporting server version" << std::endl;
    }
    else{
      std::cout << "Don't support the version" << std::endl;
      std::cout << a << std::endl;
      return -2;
    }
  } 
  else{
    std::cout << "error server didn't respond" << std::endl;
  }

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
      std::string serverMessage(bufferData);
      if(serverMessage.substr(0, 3) == "MSG"){
        std::cout << serverMessage.substr(4) << std::endl;
      }
      else if(serverMessage.substr(0, 3) == "DISC"){
        gameOver = true;
        InputThread.join();
      }
      else if(serverMessage.substr(0,3) == "OK\n"){
        std::cout << "Name accepted!" << std::endl;
      }
      else {
        std::cout << "server sent :" << bufferData << std::endl;
      }
    }
    if(sendMessage){
      if(SendString == "QUIT"){
        gameOver = true;
        InputThread.join();
      }
      else{
        fullSendString = "MSG " + SendString;
        send(sock, fullSendString.c_str(), fullSendString.length(), 0);
        sendMessage = false;
      } 
    }
  }
  close(sock);
  
  return 0;
}
