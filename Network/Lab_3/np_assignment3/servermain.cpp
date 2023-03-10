#include <bits/stdint-uintn.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <string>
#include <iostream>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string>
#include <chrono>
#include <fcntl.h>
#include <random>
#include <thread>//just for sleep
#include <string.h>//For strtok
#include <vector>
#include <regex>


// Enable if you want debugging to be printed, see examble below.
// Alternative, pass CFLAGS=-DDEBUG to make, make CFLAGS=-DDEBUG
#define DEBUG

static std::string errorReason;

bool checkIllegalName(std::string name){
  static const std::regex reg("[^A-Za-z0-9\_]");
  std::smatch mathes;
  std::smatch matches;
  while(std::regex_search(name, matches, reg)){
      if(matches.size() > 0){
          std::cout << name << " is not a valid name" << std::endl << matches.suffix().str() << std::endl;
          return false;
      }
  }
  return true;
}

//this function is taken from stackoverflow
//https://stackoverflow.com/questions/1543466/how-do-i-change-a-tcp-socket-to-be-non-blocking
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

struct clientStruct{
  int clientSocket;
  sockaddr clientaddr;
  std::string name;
  clientStruct(){
    clientSocket = -1;
    name = "Ä";//setting this to an Illegal character so we know it should not be this.
  }
};

int main(int argc, char *argv[]){
  //set ip address and port of server
  std::string PORT = "5000";  // 5000 is standard for this server
  std::string ipaddress = "0.0.0.0"; //take what is avalible

  if(argc > 1){
    std::string ipAndPort = argv[1];

    //get last ':' in string and that is the port
    bool done = false;
      for(int i = ipAndPort.size(); i > 0 && !done; i--){
        if(ipAndPort[i] == ':'){
        done = true;
        ipaddress = ipAndPort.substr(0, i);
        PORT = ipAndPort.substr(i+1, ipAndPort.size() - i);
      }
    }
  }

  //data
  ssize_t dataRecvSize = 0;
  size_t dataSize = 255;
  bool gameOver = false;
  char bufferData[dataSize];
  std::string serverToClient;
  const std::string version = "Hello 1.0\n";
  const std::string OkSend = "OK\n";
  const std::string ErrorSend = "ERR\n";

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

  std::vector<clientStruct> clients;
  clients.push_back(clientStruct());

  SetSocketBlockingEnabled(s_listen, false);
  //set client socket to non blocking
  for(int i = 0; i < clients.size(); i++){
    SetSocketBlockingEnabled(clients[i].clientSocket, false);
  }

  while(!gameOver){
    //error handleing
    if(clients.size() == 0){
      clients.push_back(clientStruct());
      std::cout << "an error would have happend" << std::endl;
    }
    //listen to new connection
    if((clients[clients.size() - 1].clientSocket = accept(
      s_listen, 
      (struct sockaddr*)&(clients[clients.size() - 1].clientaddr), 
      (socklen_t*)&clientsSize)) >= 0){
        clientsSize = sizeof(sockaddr_in);
        //add new connection
        if(clients[clients.size()-  1].clientaddr.sa_family == AF_INET){
          //ipv4
          char str[INET_ADDRSTRLEN];
          uint16_t port = htons(((sockaddr_in*)&clients[clients.size()-  1].clientaddr)->sin_port);
          inet_ntop( AF_INET, &((sockaddr_in*)&clients[clients.size()-  1].clientaddr)->sin_addr, str, INET_ADDRSTRLEN );
          std::cout << "Client connected from " << str << ":" << port << std::endl;
        }
        else if(clients[clients.size()-  1].clientaddr.sa_family == AF_INET6){
          //ipv6
          char str[INET6_ADDRSTRLEN];
          uint16_t port = htons(((sockaddr_in6*)&clients[clients.size()-  1].clientaddr)->sin6_port);
          inet_ntop(AF_INET6, &((sockaddr_in6*)&clients[clients.size()-  1].clientaddr)->sin6_addr, str, INET6_ADDRSTRLEN );
          std::cout << "Client connected from " << str << ":" << port << std::endl;
        }
        else{
          std::cout << "not ipv4 or ipv6" << std::endl;
        }
        SetSocketBlockingEnabled(clients[clients.size() - 1].clientSocket, false);
        //send hello and version
        send(clients[clients.size() - 1].clientSocket, version.c_str(), version.length(), 0);
        std::cout << clients[clients.size() - 1].name << std::endl;

        clients.push_back(clientStruct());
        
      }
    
    //check if clients have sent something
    for(int i = 0; i < clients.size() - 1; i++){
      dataRecvSize = 0;
      memset(bufferData, 0, dataSize);
      dataRecvSize = recv(clients[i].clientSocket, bufferData, dataSize, 0);
      
      
      if(dataRecvSize > 0){
        std::cout << "recv: " << bufferData << std::endl;
        std::string buffdataStr(bufferData);
        //std::cout << "recved: " << bufferData << std::endl;
        //check if it's a new client with nick
        if(buffdataStr.substr(0,4) == "NICK"){
          if(clients[i].name == "Ä"){
            std::string name = buffdataStr.substr(5,buffdataStr.length() - 5);
            if(checkIllegalName(name)){
              //set client name and send ok to client
              clients[i].name = name;
              send(clients[i].clientSocket, OkSend.c_str(), OkSend.length(), 0);
            }
            else{
              //send error and remove client
              std::cout << "d2.2" << std::endl;
              std::string errorMSG = ErrorSend + errorReason;
              send(clients[i].clientSocket, errorMSG.c_str(), errorMSG.length(), 0);
              std::cout << "removing user " <<  clients[i].name << std::endl;
              close(clients[i].clientSocket);
              clients.erase(clients.begin() + i);
              i--;
            }
          }
        }
        else if(buffdataStr.substr(0,3) == "MSG")
        {
          //check if nick is usable
          if(clients[i].name == "Ä"){
            //send err beacuse client has not been set before nick is set
            std::string errorMSG = ErrorSend + ", No name is set";
            send(clients[i].clientSocket, errorMSG.c_str(), errorMSG.length(), 0);
            std::cout << "removing user " <<  clients[i].name << std::endl;
            close(clients[i].clientSocket);
            clients.erase(clients.begin() + i);
            i--;
            continue;
          }
          std::string sendData = "MSG " + clients[i].name + " " + buffdataStr.substr(4,buffdataStr.length() - 4);
          std::cout << "sending: " << sendData << std::endl;
          for(int c = 0; c < clients.size() - 1; c++){
            if(c != i){
              //check if this works?
              send(clients[c].clientSocket, sendData.c_str(), sendData.length(), MSG_DONTWAIT);
            }
          }
        }
        else{
          send(clients[i].clientSocket, ErrorSend.c_str(), ErrorSend.length(), 0);
        }
      }
      else if(dataRecvSize == 0){
        //remove client
        std::cout << "removing user " <<  clients[i].name << std::endl;
        close(clients[i].clientSocket);
        clients.erase(clients.begin() + i);
      }   
    }
  }
  std::cout << "closing" << std::endl;
  close(s_listen);
  //for loop here
  for(int i = 0; i < MAXNROFCONNECTIONS; i++){
    close(clients[i].clientSocket);
  }
  
  return 0;
}