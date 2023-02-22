#include "ThreadPool.h"
#include <iostream>
//////////////////
#include <mutex>

#if defined (NotPooled) 
ThreadPool::ThreadPool(){
    nrOfThreads = std::thread::hardware_concurrency() + 2;
    threads.resize(nrOfThreads);
    runningThreads.resize(nrOfThreads, 0);
    std::cout << "nr of Threads:" << nrOfThreads << std::endl;
}

ThreadPool::~ThreadPool(){
  for(int i = 0; i < runningThreads.size(); i++){
    if(runningThreads[i] == 2 || runningThreads[i] == 1){
      threads[i].join();
    }
  }
}

bool ThreadPool::HaveThread(){
  for(int i = 0; i < runningThreads.size(); i++){
    if(runningThreads[i] == 2){
      threads[i].join();
      runningThreads[i] = 0;
    }
  }
  for(int i = 0; i < runningThreads.size(); i++){
    if(runningThreads[i] == 0){
      return true;
    }
  }
  return false;
}
#else

ThreadPool::ThreadPool() {
    const uint32_t num_threads = std::thread::hardware_concurrency() + 2; // Max # of threads the system supports
    threads.resize(num_threads);
    for (uint32_t i = 0; i < num_threads; i++) {
        threads.at(i) = std::thread(&ThreadPool::ThreadLoop, this);
    }
}

ThreadPool::~ThreadPool(){
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    should_terminate = true;
  }
  mutex_condition.notify_all();
  for (std::thread& active_thread : threads) {
    active_thread.join();
  }
  threads.clear();
}

void ThreadPool::ThreadLoop(){
  while (true) {
        
            std::unique_lock<std::mutex> lock(queue_mutex);
            mutex_condition.wait(lock, [this] {
                return !jobs.empty() || should_terminate;
            });
            if (should_terminate) {
                return;
            }
            jobStruct job = jobs.front();
            jobs.pop();
        job.job(job.sock, job.ret);
    }
}

void ThreadPool::setJob(const std::function<void(int, int&)>& job, int sock) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        jobs.push(jobStruct(job, sock));
    }
    mutex_condition.notify_one();
}

bool ThreadPool::busy() {
    bool poolbusy;
    std::unique_lock<std::mutex> lock(queue_mutex);
    poolbusy = jobs.empty();
    return poolbusy;
}
#endif