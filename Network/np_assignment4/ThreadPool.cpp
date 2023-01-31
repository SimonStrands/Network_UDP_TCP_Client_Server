#include "ThreadPool.h"
#include <iostream>
//////////////////
#include <mutex>

/*
ThreadPool::ThreadPool(){
    nrOfThreads = std::thread::hardware_concurrency();
    threads.resize(nrOfThreads);
    for (uint16_t i = 0; i < nrOfThreads; i++) {
        threads.at(i) = std::thread(&ThreadPool::ThreadLoop, this);
    }
    should_terminate = false;
}

ThreadPool::~ThreadPool(){
}


void ThreadPool::ThreadLoop(){
    while (true) {
        if(jobs.empty()){
            threadLock.lock();
            if(jobs.empty())//if job still empty
            {
                std::function<void()> job;
                job = jobs.front();
                jobs.pop();
                threadLock.unlock();
                job();

            }
            else{
                threadLock.unlock();
            }
        }
        if (should_terminate) {
            return;
        }

    }
}
*/
ThreadPool::ThreadPool(){
}
ThreadPool::~ThreadPool(){
    std::cout << "used " << threads.size() << std::endl;
    for(int i = 0; i < threads.size(); i++){
        if(threads[i] != nullptr){
            threads[i]->join();
            delete threads[i];
        }
    }
}


