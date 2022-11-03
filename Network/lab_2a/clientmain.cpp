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
// Enable if you want debugging to be printed, see examble below.
// Alternative, pass CFLAGS=-DDEBUG to make, make CFLAGS=-DDEBUG
#define DEBUG


// Included to get the support library
#include <calcLib.h>

void HandleError(int e, std::string errorMSG){
  if(e == -1){
    std::cout << errorMSG << std::endl; 
    exit(e);
  }
}

std::string calcMathQuestion(char* mathQ){
  char delim[]=" ";
  char *Operation=strtok(mathQ, delim);
  char *FirstNumber=strtok(NULL,delim);
  char *SecondNumber=strtok(NULL,delim);

  int intFN = std::atoi(FirstNumber);
  int intSN = std::atoi(SecondNumber);
  float floatFN = std::atof(FirstNumber);
  float floatSN = std::atof(SecondNumber);

  std::string theReturn = "";
  int iRes;
  double fRes;
  
  if(strcmp(Operation, "add") == 0){
    iRes = intFN + intSN;
    theReturn = std::to_string(iRes) + "\n";
  }
  else if (strcmp(Operation, "div") == 0) {
     iRes = intFN / intSN;
    theReturn = std::to_string(iRes) + "\n";
  }
  else if (strcmp(Operation, "mul") == 0) {
     iRes = intFN * intSN;
    theReturn = std::to_string(iRes) + "\n";
  }
  else if (strcmp(Operation, "sub") == 0) {
     iRes = intFN - intSN;
    theReturn = std::to_string(iRes) + "\n";
  }
  else if (strcmp(Operation, "fadd") == 0) {
    fRes = floatFN + floatSN;
    theReturn = std::to_string(fRes) + "\n";
  }
  else if (strcmp(Operation, "fdiv") == 0) {
    fRes = floatFN / floatSN;
    theReturn = std::to_string(fRes) + "\n";
  }
  else if (strcmp(Operation, "fmul") == 0) {
    fRes = floatFN * floatSN;
    theReturn = std::to_string(fRes) + "\n";
  }
  else if (strcmp(Operation, "fsub") == 0) {
    fRes = floatFN - floatSN;
    theReturn = std::to_string(fRes) + "\n";
  }
  else{
    std::cout << "error" << std::endl;
  }
  return theReturn;
}


int main(int argc, char *argv[]){
  int dataSize = 128;
  char bufferData[dataSize];
  memset(bufferData, 0, dataSize);
  int sizeOfRecv = 0;

  //get ip address and port number from argv //why he does it like this idk?
  char *ServerIP;
  char delim[]=":";
  if(argc < 2){
    //std::string a = "13.53.76.30:5000";
    std::string a = "127.0.0.1:5006";
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

  //connect to server

  HandleError(
    connect(sock, (sockaddr*)&hint, sizeof(hint)), 
    "can not connect to server"
    );

  #ifdef DEBUG 
  std::cout << "connected to server" << std::endl; 
  #endif


  //get response from server (should get 'TEXT TCP 1.0\n')
  while (true){
    memset(bufferData, 0, dataSize);
    sizeOfRecv = recv(sock, bufferData, dataSize, 0);
    std::cout << std::string(bufferData, sizeOfRecv) << std::endl;//got
    if(strcmp(&bufferData[strlen(bufferData) - 1], "\n") == 0){
      break;
    }
    
  }

  //send ok If we got it
  std::string ok = "OK\n";
  #ifdef DEBUG 
    std::cout << "sending ok to server" << std::endl;
  #endif
  
  HandleError(send(sock, ok.c_str(), ok.length(), 0), "error sending to server");//+1?

  //should recv a math question
  memset(bufferData, 0, dataSize);
  sizeOfRecv = recv(sock, bufferData, dataSize, 0);
  HandleError(sizeOfRecv, "didn't recv math question");
  #ifdef DEBUG
  std::cout << "got math question: ";//DEBUG
  #endif
  std::cout << std::string(bufferData, sizeOfRecv) << std::endl;  
  //do something with the math question (now we don't care)
  std::string MathAnswear = calcMathQuestion(bufferData); 

  //send back answear
  #ifdef DEBUG
  std::cout << "sending to server" << std::endl;
  #endif
  HandleError(send(sock, MathAnswear.c_str(), MathAnswear.length(), 0), "error sending to server");//+1?
  #ifdef DEBUG
  std::cout << "sent math answear to server : ";
  #endif
  std::cout << MathAnswear;

  //get if the answear was right or not
  #ifdef DEBUG
  std::cout << "trying to recv from server" << std::endl;
  #endif
  memset(bufferData, 0, dataSize);
  sizeOfRecv = recv(sock, bufferData, dataSize, 0);
  HandleError(sizeOfRecv, "didn't recv math question");
  #ifdef DEBUG
  std::cout << "Got from server: ";
  #endif
  std::cout << std::string(bufferData, sizeOfRecv) << std::endl;

  //close the socket
  close(sock);
  
  return 0;
}
