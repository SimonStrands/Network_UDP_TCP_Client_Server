#include <bits/stdint-uintn.h>
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


// Included to get the support library
#include <calcLib.h>


#include "protocol.h"

#define PORT 4950

void HandleError(int e, std::string errorMSG){
  if(e < 1){
    std::cout << errorMSG << std::endl; 
    exit(e);
  }
}
//char *arith[]={"add","div","mul","sub","fadd","fdiv","fmul","fsub"};
struct results{
  int theOperator;
  double floatRes;
  int intRes;
  bool intResultat;
  int iValue1;
  int iValue2;
  double fValue1;
  double fValue2;
};
void calculate(results &res){
  switch (res.theOperator)
  {
  case 1:
    res.intResultat = true;
    res.intRes = res.iValue1 + res.iValue2;
    break;
  case 2:
    res.intResultat = true;
    res.intRes = res.iValue1 / res.iValue2;
    break;
  case 3:
    res.intResultat = true;
    res.intRes = res.iValue1 * res.iValue2;
    break;
  case 4:
    res.intResultat = true;
    res.intRes = res.iValue1 - res.iValue2;
    break;
  case 5:
    res.intResultat = false;
    res.floatRes = res.fValue1 + res.fValue2;
    break;
  case 6:
    res.intResultat = false;
    res.floatRes = res.fValue1 / res.fValue2;
    break;
  case 7:
    res.intResultat = false;
    res.floatRes = res.fValue1 * res.fValue2;
    break;
  case 8:
    res.intResultat = false;
    res.floatRes = res.fValue1 - res.fValue2;
    break;
  default:
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
  SendCalcMessage.type = htons(22);
  SendCalcMessage.message = htons(0);
  SendCalcMessage.protocol = htons(17);
  SendCalcMessage.major_version = htons(1);
  SendCalcMessage.minor_version = htons(0);

  std::cout << "sending data to server" << std::endl;
  sizeOfSend = sendto(sock, &SendCalcMessage, sizeof(SendCalcMessage), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
  HandleError(sizeOfSend, "we can't send");

  std::cout << "recv data from server" << std::endl;

  calcProtocol *recvCalcProtocol = new calcProtocol;
  calcMessage *recvCalcMessage = new calcMessage;
  void* p_struct = malloc(sizeof(calcProtocol));
  size_t size = sizeof(recvCalcProtocol);
  
  sizeOfRecv = recvfrom(sock, p_struct, sizeof(calcProtocol), 0, (sockaddr*)&serverAddr, &cLen);
  HandleError(sizeOfRecv, "we can't recv");

  if(sizeOfRecv == sizeof(calcProtocol)){
    std::cout << "got a calcProtocol LETS GOOO!!!" << std::endl;
  }
  else if(sizeOfRecv == sizeof(calcMessage)){
    std::cout << "got calcMessage" << std::endl;
    recvCalcMessage = (calcMessage*)p_struct;
    std::cout << "message says: " << ntohs(recvCalcMessage->message) << std::endl;
    std::cout << "type says: "<< ntohs(recvCalcMessage->type) << std::endl;
    return -1;
  }

  recvCalcProtocol = (calcProtocol*)p_struct;



    std::cout << recvCalcProtocol->inValue1 << recvCalcProtocol->flValue1 << recvCalcProtocol->arith <<
    recvCalcProtocol->inValue2 << recvCalcProtocol->flValue2 << std::endl;

  delete recvCalcProtocol;
  free(p_struct);

  close(sock);

  return 0;
}
