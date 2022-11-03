#include <bits/stdint-uintn.h>
#include <cstddef>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>

// You will to add includes here
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <iostream>
#include <chrono>
#include <fcntl.h>
#include <vector>

// Included to get the support library
#include <calcLib.h>

#include "protocol.h"

using namespace std;

// Needs to be global, to be rechable by callback and main 
void sendCalcProtocolConverter(calcProtocol& calcproto){
  calcproto.type = htons(calcproto.type);
  calcproto.major_version = htons(calcproto.major_version);
  calcproto.minor_version = htons(calcproto.minor_version);
  calcproto.id = htonl(calcproto.id);
  calcproto.arith = htonl(calcproto.arith);
  calcproto.inValue1 = htonl(calcproto.inValue1);
  calcproto.inValue2 = htonl(calcproto.inValue2);
  calcproto.inResult = htonl(calcproto.inResult);
}
void recvCalcProtocolConverter(calcProtocol& calcproto){
  calcproto.type = ntohs(calcproto.type);
  calcproto.major_version = ntohs(calcproto.major_version);
  calcproto.minor_version = ntohs(calcproto.minor_version);
  calcproto.id = ntohl(calcproto.id);
  calcproto.arith = ntohl(calcproto.arith);
  calcproto.inValue1 = ntohl(calcproto.inValue1);
  calcproto.inValue2 = ntohl(calcproto.inValue2);
  calcproto.inResult = ntohl(calcproto.inResult);
}
void recvCalcMessageConverter(calcMessage& calcMsg){
  calcMsg.type = ntohs(calcMsg.type);
  calcMsg.message = ntohl(calcMsg.message);
  calcMsg.protocol = ntohs(calcMsg.protocol);
  calcMsg.major_version = ntohs(calcMsg.major_version);
  calcMsg.minor_version = ntohs(calcMsg.minor_version);
}
void sendCalcMessageConverter(calcMessage& calcMsg){
  calcMsg.type = htons(calcMsg.type);
  calcMsg.message = htonl(calcMsg.message);
  calcMsg.protocol = htons(calcMsg.protocol);
  calcMsg.major_version = htons(calcMsg.major_version);
  calcMsg.minor_version = htons(calcMsg.minor_version);
}

void printCalcProto(calcProtocol& calcproto){
  std::cout << "Type : " << calcproto.type << std::endl
  << "major_version : " << calcproto.major_version << std::endl
  << "minor_version : " << calcproto.minor_version << std::endl
  << "id : " << calcproto.id << std::endl
  << "arith : " << calcproto.arith << std::endl
  << "inValue1 : " << calcproto.inValue1 << std::endl
  << "inValue2 : " << calcproto.inValue2 << std::endl
  << "inResult : " << calcproto.inResult << std::endl
  << "flValue1 : " << calcproto.flValue1 << std::endl
  << "flValue2 : " << calcproto.flValue2 << std::endl
  << "flResult : " << calcproto.flResult << std::endl;
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

struct clientsStruct{
  sockaddr_in clientSockAddr;
  calcProtocol theCalcProtocol;
  float time;//can I change this to std::chrome something?
};

void createNewProtocol(calcProtocol& prot, uint32_t id){
  prot.type = 1;
  prot.major_version = 1;
  prot.minor_version = 0;
  prot.id = id;
  prot.arith = rand() % 8 + 1;
  prot.inValue1 = rand() % 500;
  prot.inValue2 = rand() % 500;
  //prot.inValue do not calc now
  prot.flValue1 = rand() % 500;
  prot.flValue2 = rand() % 500;
  //prot.flResult do not calc now
}

int userExist(std::vector<clientsStruct> clients, sockaddr_in aClient)
{
  int theReturn = -1;
  for(size_t i = 0; i < clients.size(); i++){
    //check if user should still exist?
    if(aClient.sin_addr.s_addr == clients[i].clientSockAddr.sin_addr.s_addr && 
      aClient.sin_port == clients[i].clientSockAddr.sin_port)
    {
      return i;
    }
  }
  return theReturn;
}


int main(int argc, char *argv[]){

  bool gameOver = false;
  std::vector<clientsStruct> clients;
  uint32_t id = 0;

  //set up data
  void* p_struct = malloc(sizeof(calcProtocol));
  size_t sizeofRecv = 0;
  size_t sizeofSend = 0;
  calcMessage recvCalcMsg;
  calcMessage sendCalcMsg;
  calcProtocol recvCalcProtocol;
  calcProtocol sendCalcProtocol;

  calcMessage errorMsg;
  errorMsg.type = 2;
  errorMsg.message = 2;
  errorMsg.major_version = 1;
  errorMsg.minor_version = 0;
  errorMsg.protocol = 17;
  sendCalcMessageConverter(errorMsg);

  sockaddr_in tempClientAddr = {0};
  //sockaddr_in serverAddr = {0};

  //set ip address and port of server
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
  hints.ai_socktype = SOCK_DGRAM;
  if(getaddrinfo(
    ipaddress.c_str(), 
    PORT.c_str(), 
    &hints, &addr) != 0)
  {
    std::cout << "error" << std::endl;
    return -1;
  }
  std::cout << ipaddress << ":" << PORT << std::endl;
  //create socket
  std::cout << addr->ai_family << " == " << AF_INET << std::endl;
  std::cout << addr->ai_socktype << " == " << SOCK_DGRAM << std::endl;
  int sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
  if(sock == -1)
  {
    std::cout << "cannot create socket" << std::endl;
    return -1;
  }

  //bind socket
  int rc = bind(sock, addr->ai_addr, addr->ai_addrlen);
  if(rc < 0){
    std::cout << "cannot build socket" << std::endl;
    close(sock);
    return rc;
  }
  std::cout << "created and binded socket" << std::endl;

  while (!gameOver){
    socklen_t len;

    //recv from client
    std::cout << "waiting to recv from client" << std::endl;
    sizeofRecv = recvfrom(
    sock, 
    p_struct, 
    sizeof(calcProtocol), 
    0, 
    (sockaddr*)&tempClientAddr, 
    &len
    );
    std::cout << "got from client" << std::endl;
    int currentUser = -1;
    //check what type we got
    if(sizeofRecv == sizeof(calcProtocol)){
      //check if this user exist
      std::cout << "user sent calcProtocol" << std::endl;
      currentUser = userExist(clients, tempClientAddr);
      if(currentUser == -1){
        //send error
        sizeofSend = sendto(
          sock, 
          &errorMsg, 
          sizeof(calcMessage), 0, 
          (sockaddr*)&tempClientAddr,
          len
        );
      }
      else{
        //if we have user check what so the user send the right answear
      }
    }
    else if(sizeofRecv == sizeof(calcMessage)){
      //check if we have this client before
      std::cout << "user sent calcMessage" << std::endl;
      currentUser = userExist(clients, tempClientAddr);
      if(currentUser == -1){
        //check so that calc message have the right things
        recvCalcMsg = *(calcMessage*)p_struct;
        recvCalcMessageConverter(recvCalcMsg);
        if(recvCalcMsg.type != 22 ||
          recvCalcMsg.message != 0 || 
          recvCalcMsg.protocol != 17 || 
          recvCalcMsg.major_version != 1 ||
          recvCalcMsg.minor_version != 0
          ){
            //send error
            sizeofSend = sendto(
              sock, 
              &errorMsg, 
              sizeof(calcMessage), 0, 
              (sockaddr*)&tempClientAddr,
              len
            );
            std::cout << "sent error" << std::endl;
            //return to while loop
            continue;
          }
          std::cout << "creating user" << std::endl;
          //create user
          clients.push_back(clientsStruct());
          currentUser = clients.size() - 1;
          clients[currentUser].clientSockAddr = tempClientAddr;
          clients[currentUser].time = 0;

          //create protocol to user
          createNewProtocol(clients[currentUser].theCalcProtocol, id++);
          std::cout << "Created protocol" << std::endl;
      }
      //send protocol to user
      std::cout << "sending protocol" << std::endl;
      sendCalcProtocolConverter(clients[currentUser].theCalcProtocol);
      sizeofSend = sendto(
      sock, 
      &clients[currentUser].theCalcProtocol, 
      sizeof(calcProtocol), 0, 
      (sockaddr*)&clients[currentUser].clientSockAddr,
      len
      );
      recvCalcProtocolConverter(clients[currentUser].theCalcProtocol);

      if(sizeofSend < 1){
        std::cout << "error sending" << std::endl;
      }
    }
    else{
      std::cout << "error" << std::endl;
    }
  }

#ifdef DEBUG
  printf("DEBUGGER LINE ");
#endif

  close(sock);
  return 0;
}
  
