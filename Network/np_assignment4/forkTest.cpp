#include <bits/stdint-uintn.h>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <sys/time.h>
#include <iostream>

int main(){
    int numberOfForks = 0;
    pid_t fork_ID = 1;
    while (fork_ID > 0) {
        fork_ID = fork();
        if(fork_ID == 0){
            sleep(10);
            exit(EXIT_SUCCESS);
        }
        else if(fork_ID > 0){
            numberOfForks++;
            std::cout << "I am still here" << std::endl;
            sleep(1);
        }
        else if(fork_ID < 0){
            std::cout << "cannot create more forks nr of Forks : " << numberOfForks << std::endl;
        }
    }
    
    std::cout << "nr of forks = " << numberOfForks << std::endl;
    return 0;
}