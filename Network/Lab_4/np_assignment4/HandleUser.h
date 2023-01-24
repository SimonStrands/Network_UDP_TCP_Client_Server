#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
/* You will to add includes here */
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <thread>
#include <vector>
#include <string>
#include <string.h>//For strtok
#include <iostream>
#include <fcntl.h>

class HandleUser{
public:
    HandleUser(int cSocket);
    bool update();
private:
    int cSocket;
    std::string inData;
    std::string outData;
    timeval timeout;  
    static const size_t dataSize = 150;
    static const size_t dataSendSize = 1500;
    char bufferData[dataSize];
    ssize_t dataRecvSize;
    
    bool handleGetCall(const std::string &getCall);

    //send ok
    void send200(const std::string& httpV);
    //errors
    void sendError400();
    void sendError403();
    void sendError404(const std::string& filename);
};