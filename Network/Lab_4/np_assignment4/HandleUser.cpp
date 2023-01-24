#include "HandleUser.h"
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <sys/socket.h>

static const std::string errorCode400 = "HTTP/x.x 400 Unknown Protocol\r\n\r\n";
static const std::string errorCode404 = " 404 Not Found\r\n\r\n";

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

HandleUser::HandleUser(int cSocket){
    this->cSocket = cSocket;

    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    
    if (setsockopt (cSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0){
        std::cout << "couldn't make socket have a recv timeout" << std::endl;
    }
    if (setsockopt (cSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0){
        std::cout << "couldn't make socket have a send timeout" << std::endl;
    }
}

bool HandleUser::update(){
    memset(bufferData, 0, dataSize);
    char fileName[50];
    memset(fileName, 0, 50);
    std::cout << "waiting for recv" << std::endl;
    if(recv(this->cSocket, &bufferData, sizeof(bufferData), 0) < 1){
        std::cout << bufferData << std::endl;
        std::cout << "recv from client failed" << std::endl;
        return false;
    }
    else{
        std::cout << "recv from server" << std::endl;
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

    for(int i = 0; i < calls.size(); i++){
        std::cout << "call " << i << ": " << calls[i] << std::endl;
    }

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
    

    return false;
}

bool HandleUser::handleGetCall(const std::string &getCall){
    int nrOfSlashes = 0;
    for(int i = 0; i < getCall.size(); i++){
        if(getCall[i] == '/'){
            nrOfSlashes++;
        }
    }
    if(nrOfSlashes > 3){
        std::cout << "to many slashes:" << nrOfSlashes << std::endl;
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
        std::cout << "error: file name: " << fileName << std::endl;
        std::cout << "error: http version: " << httpV << std::endl;
        sendError403();
        return false;
    }
    std::cout << "file name: " << fileName << std::endl;
    std::cout << "http version: " << httpV << std::endl;

    //std::ifstream inFile(fileName, std::ios::out | std::ios::binary);
    std::ifstream inFile(fileName, std::ios::out | std::ios::binary);
    if(!inFile){
        std::cout << "cannot open file" << std::endl;
        sendError404(fileName);
        return false;
    }
    send200(httpV);
    std::cout << "got file!" << std::endl;

    char bufferData[1500];
    std::string sendData = "";
    
    inFile.ignore( std::numeric_limits<std::streamsize>::max() );
    std::streamsize length = inFile.gcount();
    inFile.clear();   //  Since ignore will have set eof.
    inFile.seekg( 0, std::ios_base::beg );
    int fileSize = (int)length;
    int dataLeft = fileSize;
    inFile.clear();
    inFile.seekg(0);
    std::cout << "file size: " << fileSize << std::endl;
    
    //we don't read correctly
    while(dataLeft > 1500){
        memset(bufferData, 0, 1500);
        inFile.read(bufferData, 1500);
        sendData += bufferData;
        dataLeft -= 1500;
    }
    if(dataLeft > 0){
        char left[dataLeft];
        memset(left, 0, dataLeft);
        inFile.read(left, dataLeft);
        sendData += left;
    }
    inFile.close();
    sendData = sendData.substr(0, sendData.length() - 2);
    if(send(cSocket, sendData.c_str(), sendData.length(), 0) < 0){
        std::cout << "couldn't send data" << std::endl;
    }


    return true;
}

void HandleUser::send200(const std::string& httpV){
    std::cout << "send ok 200" << std::endl;
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