#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* You will to add includes here */
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
#include <string.h>

// Included to get the support library
#include <calcLib.h>


// Enable if you want debugging to be printed, see examble below.
// Alternative, pass CFLAGS=-DDEBUG to make, make CFLAGS=-DDEBUG
#define DEBUG


class Timer{
public:
  Timer(float time){
    this->time = time;
    this->t_start = std::chrono::steady_clock::now();
  };  
  bool checkTime(){//checks if the time as NOT run out return true
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
  sockaddr_in clientaddr;
  bool newConnection = true;
  bool connected = false;
};

const std::string strOperators[8] = {
  "add",
  "div",
  "mul",
  "sub",
  "fadd",
  "fdiv",
  "fmul",
  "fsub"
};

bool aFloatAnswear = false;
float fTheAnswear = -1;
int iTheAnswear = -1; 

std::random_device rd;

std::string makeMathQuestion(){
  std::string theReturn = "";
  int iLH;
  int iRH;
  float fLH;
  float fRH;
    //make a math question send it to client
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist(0, 1000);
    int op = rand() % 8;
    if(op > 7){
      std::cout << "error didn't get the right operator" << std::endl;
      theReturn += "add";
    }
    else{
      theReturn += strOperators[op];
    }
    fLH = dist(gen);
    fRH = dist(gen);
    if(op < 4){
      //int type
      aFloatAnswear = false;
      iLH = fLH;
      iRH = fRH;
      theReturn += " " + std::to_string(iLH);
      theReturn += " " + std::to_string(iRH);
      if(op == 0){
        iTheAnswear = iLH + iRH;
      }
      else if(op == 1){
        iTheAnswear = iLH / iRH;
      }
      else if(op == 2){
        iTheAnswear = iLH * iRH;
      }
      else if(op == 3){
        iTheAnswear = iLH - iRH;
      }
    }
    else{
      //float type
      aFloatAnswear = true;
      theReturn += " " + std::to_string(fLH);
      theReturn += " " + std::to_string(fRH);
      if(op == 4){
        fTheAnswear = fLH + fRH;
      }
      else if(op == 5){
        fTheAnswear = fLH / fRH;
      }
      else if(op == 6){
        fTheAnswear = fLH * fRH;
      }
      else if(op == 7){
        fTheAnswear = fLH - fRH;
      }
    }
    return theReturn;
}

bool checkAnswear(std::string answear){
  float theFloat = std::atof(answear.c_str());
  int theInt = std::atoi(answear.c_str());
  if(aFloatAnswear){
    return abs(theFloat - fTheAnswear) < 0.0001f;
  }
  else{
    return abs(theInt - iTheAnswear) < 0.0001f;
  }
}

int main(int argc, char *argv[]){
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

  //data
  ssize_t dataRecvSize = 0;
  size_t dataSize = 128;
  bool gameOver = false;
  bool newClient = false;
  std::string ServerToClient;
  char ClientToServer[dataSize]; memset(ClientToServer, 0, dataSize);

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

  //create socket
  int s_listen = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
  std::cout << addr->ai_family << " == " << AF_INET << std::endl;
  HandleError(s_listen, "cannot create listen");

  //bind socket
  HandleError(bind(s_listen, addr->ai_addr, addr->ai_addrlen), "cannot bind socket");

  //start listening
  const int MAXNROFCONNECTIONS = 5;
  HandleError(listen(s_listen, MAXNROFCONNECTIONS), "cannot listen");

  socklen_t clientsSize = sizeof(sockaddr_in);

  clientStruct clients[MAXNROFCONNECTIONS];

  SetSocketBlockingEnabled(s_listen, false);
  //set client socket to non blocking
  for(int i = 0; i < MAXNROFCONNECTIONS; i++){
      SetSocketBlockingEnabled(clients[i].clientSocket, false);
  }

  int currentClientSocket = 0;
  int takeinClientSockets = 0;
  int currentNumberOfConnectedSockets = 0;
  Timer time(5.0f);

  while(!gameOver){
    std::this_thread::sleep_for(std::chrono::milliseconds(50));//sleep so we use less of the computers power
    
    #ifdef DEBUG 
    //std::cout << "update" << std::endl;
    #endif

    //get new clients
    if(currentNumberOfConnectedSockets < MAXNROFCONNECTIONS){
      clients[takeinClientSockets].clientSocket = accept(s_listen, (sockaddr*)&clients[takeinClientSockets], &clientsSize);
      if(clients[takeinClientSockets].clientSocket > -1){
        time.initNew(5.0f);
        clients[takeinClientSockets].connected = true;
        clients[takeinClientSockets].newConnection = true;
        SetSocketBlockingEnabled(clients[takeinClientSockets].clientSocket, false);
        takeinClientSockets++;
        takeinClientSockets %= MAXNROFCONNECTIONS;
        currentNumberOfConnectedSockets++;

        #ifdef DEBUG 
        std::cout << "got a new client" << std::endl; 
        #endif
        
      }
    }

    if(clients[currentClientSocket].connected){
      //if its a new connection send 'TCP 1.0' or something like that
      if(clients[currentClientSocket].newConnection){
        clients[currentClientSocket].newConnection = false;

        #ifdef DEBUG 
        std::cout << "new Connection from client" << std::endl;
        #endif

        //send tcp 1.0
        ServerToClient = "TCP 1.0\n\n";
        send(clients[currentClientSocket].clientSocket, ServerToClient.c_str(), ServerToClient.length(), 0);

        #ifdef DEBUG 
        std::cout << "sent TCP 1.0 to client" << std::endl;
        #endif
      }
      else{
        //recv from client
        memset(ClientToServer, 0, dataSize);
        dataRecvSize = recv(clients[currentClientSocket].clientSocket, ClientToServer, dataSize, 0);
        if(dataRecvSize > 0){

          #ifdef DEBUG 
          std::cout << "got message from client, size: " << dataRecvSize << std::endl;
          #endif

          time.initNew(5.0f);
          //get message and send back message
          if(strcmp(ClientToServer, "OK\n") == 0){
            std::string mathQ = makeMathQuestion();
            std::cout << mathQ << std::endl;
            send(clients[currentClientSocket].clientSocket, mathQ.c_str(), mathQ.length(), 0);
          }
          else if(strcmp(ClientToServer, "Q") == 0 || strcmp(ClientToServer, "Q\n") == 0 ){
            //it's time to shut down server
            gameOver = true;
            std::cout << "shutting down server" << std::endl;
          }
          else{
            //hoping its a int or float else error
            std::string theRecv(ClientToServer);
            if(checkAnswear(theRecv)){
              //send back OK\n
              std::string ok = "OK\n";
              send(clients[currentClientSocket].clientSocket, ok.c_str(), ok.length(), 0);
              std::cout << "OK" << std::endl;
            }
            else{
              //send back error
              std::string er = "ERROR\n";
              send(clients[currentClientSocket].clientSocket, er.c_str(), er.length(), 0);
              std::cout << er;
            }
            #ifdef DEBUG 
            std::cout << "Start new user" << std::endl;
            #endif

            //remove the client
            clients[currentClientSocket].newConnection = true;
            clients[currentClientSocket].connected = false;

            close(clients[currentClientSocket].clientSocket);
            //get to the new client
            currentClientSocket++;
            currentClientSocket %= MAXNROFCONNECTIONS;
            currentNumberOfConnectedSockets--;
            //reset the answear for next user
            fTheAnswear = -1;
            iTheAnswear = -1; 
          }
        }
        else{
          #ifdef DEBUG 
          std::cout << "we didn't get msg from client" << std::endl;
          #endif
        }
        if(time.checkTime()){

          #ifdef DEBUG 
          std::cout << "time has run out for user" << std::endl;
          #endif

          //remove the client
          clients[currentClientSocket].newConnection = true;
          clients[currentClientSocket].connected = false;
          //send ERROR to client
          std::string errorStr = "ERROR TO\n";
          std::cout << errorStr;
          send(clients[currentClientSocket].clientSocket, errorStr.c_str(), errorStr.length(), 0);

          close(clients[currentClientSocket].clientSocket);
          //get to the new client
          currentClientSocket++;
          currentClientSocket %= MAXNROFCONNECTIONS;
          currentNumberOfConnectedSockets--;
          //reset the answear for next user
          fTheAnswear = -1;
          iTheAnswear = -1; 
        }
      }
    }
  }

  close(s_listen);
  //for loop here
  for(int i = 0; i < MAXNROFCONNECTIONS; i++){
    close(clients[i].clientSocket);
  }
  
  return 0;
}
