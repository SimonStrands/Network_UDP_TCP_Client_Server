#include "HandleUser.h"
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <sys/socket.h>

static const std::string errorCode400 = "HTTP/x.x 400 Unknown Protocol\r\n\r\n";
static const std::string errorCode404 = " 404 Not Found\r\n\r\n";

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

void printString(std::string str){
    for (const char* p = str.c_str(); *p != '\0'; ++p)
    {
    int c = (unsigned char) *p;

    switch (c)
    {
        case '\\':
            printf("\\\\");
            break;
        case '\n':
            printf("\\n");
            break;
        case '\r':
            printf("\\r");
            break;
        case '\t':
            printf("\\t");
            break;

        // TODO: Add other C character escapes here.  See:
        // <https://en.wikipedia.org/wiki/Escape_sequences_in_C#Table_of_escape_sequences>

        default:
            if (isprint(c))
            {
                putchar(c);
            }
            else
            {
                printf("\\x%X", c);
            }
            break;
    }
    }
    std::cout << "\n\n" << std::endl;
}

HandleUser::HandleUser(int Socket){
    this->cSocket = Socket;

    timeout.tv_sec = 10;
    timeout.tv_usec = 500000;
    int so;
    if ((so = setsockopt (cSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout))) < 0){
        DEBUG_MSG("couldn't make socket have a recv timeout : " << so);
    }
    if ((so = setsockopt (cSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout))) < 0){
        DEBUG_MSG("couldn't make socket have a send timeout : " << so);
    }
}

bool HandleUser::update(){
    memset(bufferData, 0, dataSize);
    char fileName[50];
    memset(fileName, 0, 50);
    DEBUG_MSG("waiting for recv");
    int recvSize = 0;
    recvSize = recv(this->cSocket, &bufferData, dataSize, 0);
    if(recvSize < 1){
        DEBUG_MSG(bufferData);
        DEBUG_MSG("recv from client failed" << recvSize);
        return false;
    }

    inData = std::string(bufferData);
    //cut them by calls
    char delimiter = '\n';
    std::vector<std::string> calls;
    size_t pos = 0;
    while ((pos = inData.find(delimiter)) != std::string::npos) {
        calls.push_back(inData.substr(0, pos));
        inData.erase(0, pos + 1);
    }
    calls.push_back(inData);

    for(int c = 0; c < calls.size(); c++){
        if(calls[c].size() < 1){
            continue;
        }
        if(calls[c].substr(0,3) == "GET"){
            if(!handleGetCall(calls[c])){
                return false;
            }
        }
    }
    

    return true;
}

bool HandleUser::handleGetCall(const std::string &getCall){
    int nrOfSlashes = 0;
    for(int i = 0; i < getCall.size(); i++){
        if(getCall[i] == '/'){
            nrOfSlashes++;
        }
    }
    if(nrOfSlashes > 3){
        DEBUG_MSG("to many slashes:" << nrOfSlashes);
        sendError400();
        return false;
    }
    std::string fileName = "";
    std::string httpV = "";
    bool http = false;
    for(int i = 5; i < getCall.size(); i++){
        if(getCall[i] == ' '){
            http = true;
        }
        else if(getCall[i] == '\\' || getCall[i] == '\n' ||getCall[i] == '\r' ||getCall[i] == '\t'){
            //NOTHING
        }
        else if(!http) {
            fileName += getCall[i];
        }
        else{
            httpV += getCall[i];
        }
    }
    
    if(httpV != "HTTP/1.1" && httpV != "HTTP/1.0"){
        DEBUG_MSG("error: file name: " << fileName);
        DEBUG_MSG("error: http version: " << httpV);
        sendError403();
        return false;
    }

    char bufferData[1500];
    std::string sendData = "";
    fileMutex.lock();
    std::ifstream inFile(fileName, std::ios::out | std::ios::binary);
    if(!inFile){
        DEBUG_MSG("cannot open file");
        sendError404(fileName);
        fileMutex.unlock();
        return false;
    }
    send200(httpV);
    DEBUG_MSG("got file!");
    
    inFile.ignore( std::numeric_limits<std::streamsize>::max() );
    std::streamsize length = inFile.gcount();
    inFile.clear();   //  Since ignore will have set eof.
    inFile.seekg( 0, std::ios_base::beg );
    int fileSize = (int)length;
    int dataLeft = fileSize;
    inFile.clear();
    inFile.seekg(0);

    DEBUG_MSG("GOT FILE SIZE");
    
    //we don't read correctly
    while(dataLeft > 1500){
        memset(bufferData, 0, 1500);
        inFile.read(bufferData, 1500);
        sendData += bufferData;
        dataLeft -= 1500;
    }
    DEBUG_MSG("Read first size of file");
    if(dataLeft > 0){
        char left[dataLeft];
        memset(left, 0, dataLeft);
        inFile.read(left, dataLeft);
        sendData += left;
    }
    DEBUG_MSG("read Second ");
    inFile.close();
    fileMutex.unlock();
    if(sendData.length() > 1){
        sendData = sendData.substr(0, sendData.length() - 1);
    }
    //MSG_NOSIGNAL
    if(sendData.length() == 0){
        sendError403();
    }
    if(send(cSocket, sendData.c_str(), sendData.length(), MSG_NOSIGNAL) < 0){
        DEBUG_MSG("couldn't send data");
    }
    DEBUG_MSG("sentdata");


    return true;
}

void HandleUser::send200(const std::string& httpV){
    DEBUG_MSG("send ok 200");
    std::string okSend = httpV + " 200 OK\r\n\r\n";
    send(cSocket, okSend.c_str(), okSend.length(), 0);
}
void HandleUser::sendError400(){
    std::cout << "send error 400" << std::endl;
    send(cSocket, errorCode400.c_str(), errorCode400.length(), 0);
}
void HandleUser::sendError403(){
    std::cout << "send error 403" << std::endl;
    static const std::string err = "403 Forbidden\r\n\r\n";
    send(cSocket, err.c_str(), err.length(), 0);
}
void HandleUser::sendError404(const std::string& filename){
    std::cout << "send error 404 : couldn't find " << filename << std::endl;
    std::string err = filename + errorCode404;
    send(cSocket, err.c_str(), err.length(), 0);
}