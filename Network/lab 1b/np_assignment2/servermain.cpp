#include <cstddef>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

// You will to add includes here
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


using namespace std;
// Needs to be global, to be rechable by callback and main 
int loopCount=0;
int _terminate=0;


// Call back function, will be called when the SIGALRM is raised when the timer expires. 
void checkJobbList(int signum){
  // As anybody can call the handler, its good coding to check the signal number that called it.

  printf("Let me be, I want to sleep, loopCount = %d.\n", loopCount);

  if(loopCount>20){
    printf("I had enough.\n");
    _terminate=1;
  }
  
  return;
}





int main(int argc, char *argv[]){
  
  // Do more magic
  size_t sizeofRecv;
  sockaddr_in servaddr = {0};
  sockaddr_in clientaddr = {0};

  //create socket
  int sock = socket(PF_INET, SOCK_DGRAM, 0);
  if(sock == -1)
  {
    std::cout << "cannot create socket" << std::endl;
    return -1;
  }

  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(50001);
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//?

  //bind socket
  int rc = bind(sock, (sockaddr*)&servaddr, sizeof(servaddr));
  if(rc < 0){
    std::cout << "cannot build socket" << std::endl;
    close(sock);
    return rc;
  }

  calcMessage recvCalcMsg;
  socklen_t len = sizeof(clientaddr);
  sizeofRecv = recvfrom(sock, &recvCalcMsg, sizeof(calcMessage), 0, 
  (sockaddr*)&clientaddr, &len);

  if(sizeofRecv == sizeof(calcMessage)){
    std::cout << "got size of calcMessage" << std::endl;
    std::cout << recvCalcMsg.type << std::endl;
    std::cout << recvCalcMsg.message << std::endl;
    std::cout << recvCalcMsg.protocol << std::endl;
    std::cout << recvCalcMsg.major_version << std::endl;
    std::cout << recvCalcMsg.minor_version << std::endl;
    //very nice
  }
  else{
    std::cout << "error" << std::endl;
    close(sock);
    return -1;
  }
  recvCalcMsg.type = 21;
  recvCalcMsg.message = 1;
  recvCalcMsg.protocol = 17;
 
  char cIP[50];

  //inet_ntop(AF_INET, &clientaddr.sin_addr, cIP, 50);
  //std::cout << "client ip address: " << cIP << std::endl; 

  std::cout << "sending data back" << std::endl;
  sizeofRecv = sendto(sock, &recvCalcMsg, sizeof(recvCalcMsg), 0, (sockaddr*)&clientaddr, len);
  if(sizeofRecv < 0){
    std::cout << "error sending" << std::endl;
  }
  close(sock);
  return 0;


   
  // Prepare to setup a reoccurring event every 10s. If it_interval, or it_value is omitted, it will be a single alarm 10s after it has been set. 
  
  struct itimerval alarmTime;
  alarmTime.it_interval.tv_sec=10;
  alarmTime.it_interval.tv_usec=10;
  alarmTime.it_value.tv_sec=10;
  alarmTime.it_value.tv_usec=10;

  // Regiter a callback function, associated with the SIGALRM signal, which will be raised when the alarm goes of 
  signal(SIGALRM, checkJobbList);
  setitimer(ITIMER_REAL,&alarmTime,NULL); // Start/register the alarm. 

#ifdef DEBUG
  printf("DEBUGGER LINE ");
#endif
  
  
  while(_terminate==0){
    printf("This is the main loop, %d time.\n",loopCount);
    sleep(1);
    loopCount++;
  }

  printf("done.\n");
  return(0);
}
  
