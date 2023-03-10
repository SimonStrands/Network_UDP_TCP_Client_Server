#include <bits/stdint-uintn.h>
#include <cstddef>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
// You will to add includes here */
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
// Included to get the support library
#include <calcLib.h>
#include "protocol.h"

#define PORT 4950

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


class Timer{
public:
  Timer(float time){
    this->time = time;
    this->t_start = std::chrono::steady_clock::now();
  };  
  bool checkTime(){
    this->t_end = std::chrono::steady_clock::now();
	  std::chrono::duration<double> runTime = this->t_end - this->t_start;
	  return this->time <= (float)runTime.count();
  };
  void initNew(float time){
    this->time = time;
    this->t_start = std::chrono::steady_clock::now();
  }
private:
  float time;
  std::chrono::steady_clock::time_point t_start;
	std::chrono::steady_clock::time_point t_end;
};

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
  if(e < 1){
    std::cout << errorMSG  << ":" << e << std::endl;
    char buffer[ 256 ];
    char * errorMsg = strerror_r( errno, buffer, 256 ); // GNU-specific version, Linux default
    printf("Error %s\n", errorMsg);
    exit(e);
  }
}

void calculate(calcProtocol &res, bool& intres ){
  switch (res.arith)
  {
  case 1:
    intres = true;
    printf("add %d %d", res.inValue1, res.inValue2);
    res.inResult = res.inValue1 + res.inValue2;
    break;
  case 2:
    intres = true;
    printf("sub %d %d", res.inValue1, res.inValue2);
    res.inResult = res.inValue1 - res.inValue2;
    break;
  case 3:
    intres = true;
    printf("mul %d %d", res.inValue1, res.inValue2);
    res.inResult = res.inValue1 * res.inValue2;
    break;
  case 4:
    intres = true;
    printf("div %d %d", res.inValue1, res.inValue2);
    res.inResult = res.inValue1 / res.inValue2;
    break;
  case 5:
    intres = false;
    printf("fadd %.6f %.6f", res.flValue1, res.flValue2);
    res.flResult = res.flValue1 + res.flValue2;
    break;
  case 6:
    intres = false;
    printf("fsub %.6f %.6f", res.flValue1, res.flValue2);
    res.flResult = res.flValue1 - res.flValue2;
    break;
  case 7:
    intres = false;
    printf("fmul %.6f %.6f", res.flValue1, res.flValue2);
    res.flResult = res.flValue1 * res.flValue2;
    break;
  case 8:
    intres = false;
    printf("fdiv %.6f %.6f", res.flValue1, res.flValue2);
    res.flResult = res.flValue1 / res.flValue2;
    break;
  default:
    std::cout << "didn't get the right arith" << std::endl;
    break;
  }
  printf("\n");
}

int main(int argc, char *argv[]){

  int sizeOfRecv = 0;
  int sizeOfSend = 0;

  std::string ServerIP;
  std::string ServerPort;
  if(argc < 2){
    std::string ipAndPort = "13.53.76.30:5000";
    bool done = false;
    for(int i = ipAndPort.size(); i > 0 && !done; i--){
      if(ipAndPort[i] == ':'){
        done = true;
        ServerIP = ipAndPort.substr(0, i);
        ServerPort = ipAndPort.substr(i+1, ipAndPort.size() - i);
      }
    }
  }
  else{
    std::string ipAndPort = argv[1];
    bool done = false;
    for(int i = ipAndPort.size(); i > 0 && !done; i--){
      if(ipAndPort[i] == ':'){
        done = true;
        ServerIP = ipAndPort.substr(0, i);
        ServerPort = ipAndPort.substr(i+1, ipAndPort.size() - i);
      }
    }
  }

  //write server ip and host to console/terminal
  std::cout << "Host " << ServerIP << ", and port: " << ServerPort << std::endl;

  struct addrinfo hints = {}, *res;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;

  if(getaddrinfo(ServerIP.c_str(), ServerPort.c_str(), &hints, &res) != 0){
    return -1;
  }

  int sock = socket(res->ai_family, res->ai_socktype, 0);
  #ifdef DEBUG 
  std::cout << "created socket" << std::endl;
  #endif
  HandleError(sock, "cannot create socket");

  void* serverAddr;
  socklen_t cLen;
  socklen_t sLen;
  if(res->ai_family == AF_INET){
    serverAddr = (sockaddr_in*)res->ai_addr;
    cLen = sizeof(sockaddr_in);
    sLen = sizeof(sockaddr_in);
    inet_pton(res->ai_family, ServerIP.c_str(), &((sockaddr_in*)serverAddr)->sin_addr);
  }
  else if(res->ai_family == AF_INET6){
    serverAddr = (sockaddr_in6*)res->ai_addr;
    cLen = sizeof(sockaddr_in6);
    sLen = sizeof(sockaddr_in6);
    inet_pton(res->ai_family, ServerIP.c_str(), &((sockaddr_in6*)serverAddr)->sin6_addr);
  }

  //give buffer data som data
  calcMessage SendCalcMessage;
  SendCalcMessage.type = 22;
  SendCalcMessage.message = 0;
  SendCalcMessage.protocol = 17;
  SendCalcMessage.major_version = 1;
  SendCalcMessage.minor_version = 0;
  sendCalcMessageConverter(SendCalcMessage);


  calcProtocol *recvCalcProtocol = new calcProtocol;
  calcMessage *recvCalcMessage = new calcMessage;
  void* p_struct = malloc(sizeof(calcProtocol)* 2);

  //get message
  Timer time(2.0f);
  int timesSent = 0;
  bool done = false;
  SetSocketBlockingEnabled(sock, false);
  while (timesSent < 3 && !done)
  {
    
    sizeOfSend = sendto(sock, &SendCalcMessage, sizeof(calcMessage), 0, (sockaddr*)serverAddr, sLen);
    HandleError(sizeOfSend, "we can't send");
    time.initNew(2.0f);
    timesSent++;
    
    while (!time.checkTime() && !done)
    {
      sizeOfRecv = recvfrom(sock, recvCalcProtocol, sizeof(calcProtocol), 0, (sockaddr*)serverAddr, &cLen);
      if(sizeOfRecv != 0 && sizeOfRecv != -1)
      {
        done = true;
      }
    }
  }

  if(sizeOfRecv == sizeof(calcMessage)){
    std::cout << "Error got calcMessage expected calcprotocol" << std::endl;
    close(sock);
    return -1;
  }
  else if(sizeOfRecv == sizeof(calcProtocol)){
    //continue
  }
  else{
    close(sock);
    HandleError(sizeOfRecv, "Error didn't get any data from server");
    return -1;
  }

  sizeOfRecv = 0;
  bool intres;
  
  recvCalcProtocolConverter(*recvCalcProtocol);

  calculate(*recvCalcProtocol, intres);
  float fTheResult;
  int iTheResult;
  if(intres){
    #ifdef DEBUG 
    std::cout << "Calculated the result to " << recvCalcProtocol->inResult << std::endl;
    #endif
    iTheResult = recvCalcProtocol->inResult;
  }
  else{
    #ifdef DEBUG 
    std::cout << "Calculated the result to " << recvCalcProtocol->flResult << std::endl;
    #endif
    fTheResult = recvCalcProtocol->flResult;
  }
  recvCalcProtocol->type = 2;

  //printCalcProto(*recvCalcProtocol);

  sendCalcProtocolConverter(*recvCalcProtocol);

  free(p_struct);
  p_struct = malloc(sizeof(calcMessage));

  timesSent = 0;
  done = false;
  while (timesSent < 3 && !done)
  {
    sizeOfSend = sendto(sock, recvCalcProtocol, sizeof(calcProtocol), 0, (sockaddr*)serverAddr, sLen);
    HandleError(sizeOfSend, "can't send data to server" + std::to_string(sizeOfSend));

    time.initNew(2.0f);
    timesSent++;
    
    while (!time.checkTime() && !done)
    {
      sizeOfRecv = recvfrom(sock, recvCalcMessage, sizeof(calcMessage), 0, (sockaddr*)serverAddr, &cLen);
      if(sizeOfRecv != 0 && sizeOfRecv != -1)
      {
        done = true;
      }
    }
  }
  if(sizeOfRecv == sizeof(calcProtocol)){
    std::cout << "error got calcprotocol waited for calcMessage" << std::endl;
    return -1;
  }
  else if(sizeOfRecv == sizeof(calcMessage)){
    recvCalcMessageConverter(*recvCalcMessage);
    if(recvCalcMessage->message == 1){
      std::cout << "OK ";
      std::cout << "(myresult=";
      if(intres){
        std::cout << iTheResult << ")"<< std::endl;
      }
      else{
        std::cout << fTheResult << ")" << std::endl;
      }
    }
    else if(recvCalcMessage->message == 2){
      std::cout << "NOT OK" << std::endl;
    }
    else {
      std::cout << "error message was not right" << std::endl;
    }
  }

  delete recvCalcProtocol;
  free(p_struct);
  close(sock);

  return 0;
}