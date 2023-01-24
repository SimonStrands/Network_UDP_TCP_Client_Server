#include "ThreadPool.h"
#include <iostream>

ThreadPool::ThreadPool(){}

ThreadPool::~ThreadPool(){
    std::cout << "used " << threads.size() << std::endl;
    for(int i = 0; i < threads.size(); i++){
        if(threads[i] != nullptr){
            threads[i]->join();
            delete threads[i];
        }
    }
}

