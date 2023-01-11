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
    std::cout << errorMSG << std::endl; 
    exit(e);
  }
}

void calculate(calcProtocol &res, bool& intres ){
  switch (res.arith)
  {
  case 1:
    intres = true;
    res.inResult = res.inValue1 + res.inValue2;
    std::cout << res.inValue1 << + " + " << res.inValue2 << " = " << res.inResult << std::endl;
    break;
  case 2:
    intres = true;
    res.inResult = res.inValue1 - res.inValue2;
    std::cout << res.inValue1 << + " - " << res.inValue2 << " = " << res.inResult << std::endl;
    break;
  case 3:
    intres = true;
    res.inResult = res.inValue1 * res.inValue2;
    std::cout << res.inValue1 << + " * " << res.inValue2 << " = " << res.inResult << std::endl;
    break;
  case 4:
    intres = true;
    res.inResult = res.inValue1 / res.inValue2;
    std::cout << res.inValue1 << + " / " << res.inValue2 << " = " << res.inResult << std::endl;
    break;
  case 5:
    intres = false;
    res.flResult = res.flValue1 + res.flValue2;
    std::cout << res.flValue1 << + " + " << res.flValue2 << " = " << res.flResult << std::endl;
    break;
  case 6:
    intres = false;
    res.flResult = res.flValue1 - res.flValue2;
    std::cout << res.flValue1 << + " - " << res.flValue2 << " = " << res.flResult << std::endl;
    break;
  case 7:
    intres = false;
    res.flResult = res.flValue1 * res.flValue2;
    std::cout << res.flValue1 << + " * " << res.flValue2 << " = " << res.flResult << std::endl;
    break;
  case 8:
    intres = false;
    res.flResult = res.flValue1 / res.flValue2;
    std::cout << res.flValue1 << + " / " << res.flValue2 << " = " << res.flResult << std::endl;
    break;
  default:
    std::cout << "didn't get the right arith" << std::endl;
    break;
  }
}

int main(int argc, char *argv[]){

  size_t sizeOfBuffer = 256;
  char dataBuffer[sizeOfBuffer];
  int sizeOfRecv = 0;
  int sizeOfSend = 0;

  //get ip address and port number from argv
  char *ServerIP;
  char delim[]=":";
  if(argc < 2){
    std::string a = "13.53.76.30:5000";
    char* ipandPort = new char[a.size() + 1];
    std::copy(a.begin(), a.end(), ipandPort);
    ServerIP=strtok(ipandPort,delim);
  }
  else{
    ServerIP=strtok(argv[1],delim);
  }
  
  char *ServerPort=strtok(NULL,delim);
  int port = atoi(ServerPort);

  //write server ip and host to console/terminal
  std::cout << "Server IP: " << ServerIP << " ServerPort: " << port << std::endl;

  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  std::cout << "created socket" << std::endl;
  HandleError(sock, "cannot create socket");

  sockaddr_in clientAddr;
  sockaddr_in serverAddr;

  socklen_t cLen = sizeof(clientAddr);

  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);
  inet_pton(AF_INET, ServerIP, &serverAddr.sin_addr);

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
  size_t size = sizeof(recvCalcProtocol);

  //get message
  Timer time(2.0f);
  int timesSent = 0;
  bool done = false;
  SetSocketBlockingEnabled(sock, false);
  while (timesSent < 3 && !done)
  {
    std::cout << "sending data to server" << std::endl;
    sizeOfSend = sendto(sock, &SendCalcMessage, sizeof(calcMessage), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
    HandleError(sizeOfSend, "we can't send");

    time.initNew(2.0f);
    timesSent++;
    
    while (!time.checkTime() && !done)
    {
      sizeOfRecv = recvfrom(sock, recvCalcProtocol, sizeof(calcProtocol), 0, (sockaddr*)&serverAddr, &cLen);
      if(sizeOfRecv != 0 && sizeOfRecv != -1)
      {
        done = true;
        std::cout << "got data from server" << std::endl;
      }
    }
  }
  
  if(sizeOfRecv == sizeof(calcMessage)){
    std::cout << "got calcMessage sad" << std::endl;
    close(sock);
    return -1;
  }
  else if(sizeOfRecv == sizeof(calcProtocol)){
    std::cout << "got a calcProtocol LETS GOOO!!!" << std::endl;
  }
  else{
    std::cout << "didn't get any data from server" << std::endl;
    return -1;
  }

  sizeOfRecv = 0;
  bool intres;
  
  recvCalcProtocolConverter(*recvCalcProtocol);

  calculate(*recvCalcProtocol, intres);
  if(intres){
    std::cout << "sending: i " << recvCalcProtocol->inResult << std::endl;
  }
  else{
    std::cout << "sending: f " << recvCalcProtocol->flResult << std::endl;
  }
  recvCalcProtocol->type = 2;

  printCalcProto(*recvCalcProtocol);

  sendCalcProtocolConverter(*recvCalcProtocol);

  free(p_struct);
  p_struct = malloc(sizeof(calcMessage));

  timesSent = 0;
  done = false;
  while (timesSent < 3 && !done)
  {
    std::cout << "sending data to server" << std::endl;
    sizeOfSend = sendto(sock, recvCalcProtocol, sizeof(calcProtocol), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
    HandleError(sizeOfSend, "we can't send");

    time.initNew(2.0f);
    timesSent++;
    
    while (!time.checkTime() && !done)
    {
      sizeOfRecv = recvfrom(sock, recvCalcMessage, sizeof(calcMessage), 0, (sockaddr*)&serverAddr, &cLen);
      if(sizeOfRecv != 0 && sizeOfRecv != -1)
      {
        done = true;
        std::cout << "got data from server" << std::endl;
      }
    }
  }
  if(sizeOfRecv == sizeof(calcProtocol)){
    std::cout << "error got calcprotocol waited for calcMessage" << std::endl;
  }
  else if(sizeOfRecv == sizeof(calcMessage)){
    recvCalcMessageConverter(*recvCalcMessage);
    if(recvCalcMessage->message == 1){
      std::cout << "OK" << std::endl;
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