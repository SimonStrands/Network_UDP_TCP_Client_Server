#include <bits/stdint-uintn.h>
#include <cstddef>
#include <cstdlib>
#include <pthread.h>
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

void printCalcProto(calcProtocol& calcprotol){
  std::cout << "Type : " << calcprotol.type << std::endl
  << "major_version : " << calcprotol.major_version << std::endl
  << "minor_version : " << calcprotol.minor_version << std::endl
  << "id : " << calcprotol.id << std::endl
  << "arith : " << calcprotol.arith << std::endl
  << "inValue1 : " << calcprotol.inValue1 << std::endl
  << "inValue2 : " << calcprotol.inValue2 << std::endl
  << "inResult : " << calcprotol.inResult << std::endl
  << "flValue1 : " << calcprotol.flValue1 << std::endl
  << "flValue2 : " << calcprotol.flValue2 << std::endl
  << "flResult : " << calcprotol.flResult << std::endl;
}

void calcTheProtocol(calcProtocol& calcprotol)
{
  switch (calcprotol.arith)
  {
  case 1:
    printf("add %d %d", calcprotol.inValue1, calcprotol.inValue2);
    calcprotol.inResult = calcprotol.inValue1 + calcprotol.inValue2;
    break;
  case 2:
    printf("sub %d %d", calcprotol.inValue1, calcprotol.inValue2);
    calcprotol.inResult = calcprotol.inValue1 - calcprotol.inValue2;
    break;
  case 3:
    printf("mul %d %d", calcprotol.inValue1, calcprotol.inValue2);
    calcprotol.inResult = calcprotol.inValue1 * calcprotol.inValue2;
    break;
  case 4:
    printf("div %d %d", calcprotol.inValue1, calcprotol.inValue2);
    calcprotol.inResult = calcprotol.inValue1 / calcprotol.inValue2;
    break;
  case 5:
    printf("fadd %.6f %.6f", calcprotol.flValue1, calcprotol.flValue2);
    calcprotol.flResult = calcprotol.flValue1 + calcprotol.flValue2;
    break;
  case 6:
    printf("fsub %.6f %.6f", calcprotol.flValue1, calcprotol.flValue2);
    calcprotol.flResult = calcprotol.flValue1 - calcprotol.flValue2;
    break;
  case 7:
    printf("fmul %.6f %.6f", calcprotol.flValue1, calcprotol.flValue2);
    calcprotol.flResult = calcprotol.flValue1 * calcprotol.flValue2;
    break;
  case 8:
    printf("fdiv %.6f %.6f", calcprotol.flValue1, calcprotol.flValue2);
    calcprotol.flResult = calcprotol.flValue1 / calcprotol.flValue2;
    break;
  default:
    std::cout << "didn't get the right arith" << std::endl;
    break;
  }
  printf("\n");
}

bool checkAnswear(calcProtocol& serverProtocol, calcProtocol& clientProtocol){
  if(serverProtocol.arith < 5){
    //an int 
    return serverProtocol.inResult == clientProtocol.inResult;
  }
  else if(serverProtocol.arith < 9){
    //an float
    return abs(serverProtocol.flResult - clientProtocol.flResult) < 0.001f;
  }
  else{
    std::cout << "didn't get the right arith" << std::endl;
    return false;
  }
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
  bool ipv6;
  sockaddr clientSockAddr;
  calcProtocol theCalcProtocol;
  std::chrono::steady_clock::time_point startingLifePoint;
};

void createNewProtocol(calcProtocol& prot, uint32_t id){
  prot.type = 1;
  prot.major_version = 1;
  prot.minor_version = 0;
  prot.id = id;
  prot.arith = rand() % 8 + 1;
  prot.inValue1 = randomInt();
  prot.inValue2 = randomInt();
  //prot.inValue do not calc now
  prot.flValue1 = randomFloat();
  prot.flValue2 = randomFloat();
  //prot.flResult do not calc now
}

int userExist(std::vector<clientsStruct>& clients, sockaddr& aClient, uint32_t clientID)
{
  int theReturn = -1;
  for(size_t i = 0; i < clients.size(); i++){
    if(clients[i].ipv6 == (aClient.sa_family == AF_INET6)){
      if(!clients[i].ipv6){
        char ip[INET_ADDRSTRLEN];
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &((sockaddr_in*)&clients[i].clientSockAddr)->sin_addr, clientIP, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &((sockaddr_in*)&aClient)->sin_addr, ip, INET_ADDRSTRLEN);


        if(std::string(ip) == std::string(clientIP) && 
          ((sockaddr_in*)&aClient)->sin_port == ((sockaddr_in*)&clients[i].clientSockAddr)->sin_port &&
          clientID == clients[i].theCalcProtocol.id)
        {
          return i;
        }

      }
      else{
        char ip[INET6_ADDRSTRLEN];
        char clientIP[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET, &((sockaddr_in6*)&clients[i].clientSockAddr)->sin6_addr, clientIP, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &((sockaddr_in6*)&aClient)->sin6_addr, ip, INET6_ADDRSTRLEN);


        if(std::string(ip) == std::string(clientIP) && 
          ((sockaddr_in6*)&aClient)->sin6_port == ((sockaddr_in6*)&clients[i].clientSockAddr)->sin6_port &&
          clientID == clients[i].theCalcProtocol.id)
        {
          return i;
        }
      }
    }
  }
  return theReturn;
}

void updateUserTime(std::vector<clientsStruct>& clients)
{
  for(size_t i = 0; i < clients.size(); i++){
    std::chrono::duration<double> runTime = std::chrono::steady_clock::now() - clients[i].startingLifePoint;
    if((float)runTime.count() >= 10.0f){
      clients.erase(clients.begin() + i);
      i--;
    }
  }
}

void userAnsweard(clientsStruct &client){
  client.startingLifePoint = std::chrono::steady_clock::now();
}

int main(int argc, char *argv[]){

  initCalcLib();
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

  calcMessage errorMsg;
  errorMsg.type = 2;
  errorMsg.message = 2;
  errorMsg.major_version = 1;
  errorMsg.minor_version = 0;
  errorMsg.protocol = 17;
  sendCalcMessageConverter(errorMsg);

  //set ip address and port of server
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
  while (!gameOver){
    socklen_t len = sizeof(struct sockaddr_in6);
    sockaddr tempClientAddr;

    sizeofRecv = recvfrom(
    sock, 
    p_struct, 
    sizeof(calcProtocol), 
    0, 
    &tempClientAddr,
    &len
    );
    if(tempClientAddr.sa_family == AF_INET){
      len = sizeof(struct sockaddr_in);
    }
    else{
      len = sizeof(struct sockaddr_in6);
    }

    updateUserTime(clients);
    
    int currentUser = -1;
    //check what type we got
    if(sizeofRecv == sizeof(calcProtocol)){
      recvCalcProtocol = *(calcProtocol*)p_struct;
      recvCalcProtocolConverter(recvCalcProtocol);
      currentUser = userExist(clients, tempClientAddr, recvCalcProtocol.id);
      if(currentUser == -1){
        //send error
        std::cout << "sending error user didn't exist" << std::endl;
        sizeofSend = sendto(
          sock, 
          &errorMsg, 
          sizeof(calcMessage), 0, 
          //(sockaddr*)&tempClientAddr,
          &tempClientAddr,
          len
        );
      }
      else{
        //if we have user check what so the user send the right answear
        if(checkAnswear(clients[currentUser].theCalcProtocol, recvCalcProtocol)){
          //send ok
          std::cout << "OK" << std::endl;
          sendCalcMsg.type = 2;
          sendCalcMsg.message = 1;
          sendCalcMsg.protocol = 6;
          sendCalcMessageConverter(sendCalcMsg);
          sizeofSend = sendto(
              sock, 
              &sendCalcMsg, 
              sizeof(calcMessage), 0, 
              //(sockaddr*)&tempClientAddr,
              &tempClientAddr,
              len
            );
        }
        else{
          //send error
          std::cout << "send error" << std::endl;
          sizeofSend = sendto(
              sock, 
              &errorMsg, 
              sizeof(calcMessage), 0, 
              //(sockaddr*)&tempClientAddr,
              &tempClientAddr,
              len
            );
        }

      }
    }
    else if(sizeofRecv == sizeof(calcMessage)){
      //check if we have this client before
      currentUser = userExist(clients, tempClientAddr, -1);
      bool creatingNewUser = false;
      if(currentUser == -1){
        updateUserTime(clients);
        creatingNewUser = true;
        //check so that calc message have the right things
        recvCalcMsg = *(calcMessage*)p_struct;
        recvCalcMessageConverter(recvCalcMsg);
        if(recvCalcMsg.type != 22 ||
          recvCalcMsg.message != 0 || 
          recvCalcMsg.protocol != 17 || 
          recvCalcMsg.major_version != 1 ||
          recvCalcMsg.minor_version != 0 
          ){
            creatingNewUser = false;
            //send error
            sizeofSend = sendto(
              sock, 
              &errorMsg, 
              sizeof(calcMessage), 0, 
              //(sockaddr*)&tempClientAddr,
              &tempClientAddr,
              len
            );
            //return to while loop
            continue;
          }
          //create user
          clients.push_back(clientsStruct());
          currentUser = clients.size() - 1;
          clients[currentUser].ipv6 = tempClientAddr.sa_family == 10;//check if user uses ipv6 or ipv4
          clients[currentUser].clientSockAddr = tempClientAddr;
          clients[currentUser].startingLifePoint = std::chrono::steady_clock::now();

          //create protocol to user
          createNewProtocol(clients[currentUser].theCalcProtocol, id++);

      }
      sendCalcProtocolConverter(clients[currentUser].theCalcProtocol);
        sizeofSend = sendto(
        sock, 
        &clients[currentUser].theCalcProtocol, 
        sizeof(calcProtocol), 0, 
        &tempClientAddr,
        len
        );

      std::cout << "sent data to user : " << sizeofSend << std::endl;
      std::cout << sizeof(clients[currentUser].theCalcProtocol) << std::endl;
      std::cout << sizeof(tempClientAddr) << std::endl;
      if(sizeofSend < 0){
        std::cout << "couldn't send" << std::endl;
      }
      recvCalcProtocolConverter(clients[currentUser].theCalcProtocol);

      if(creatingNewUser){
        //calc the protocol
        calcTheProtocol(clients[currentUser].theCalcProtocol);
      }

      if(sizeofSend < 1){
        std::cout << "error sending" << std::endl;
      }
    }
    else{
      std::cout << "error" << std::endl;
    }
  }

  close(sock);
  return 0;
}
  
