#include <mutex>
#include <thread>
#include <vector>
#include <functional>
////////////////
#include <queue>


class ThreadPool{
public:
  ThreadPool();
  virtual ~ThreadPool();
  template <class _Fn, class... _Args>
  void setJob(_Fn&& func,  _Args&&... args){
    bool haveAThread = false;
    for(int i = 0; i < threads.size(); i++){
        if(threads[i]->joinable()){
            threads[i]->join();
            delete threads[i];
            threads[i] = new std::thread(func, args...);
            return;
        }
    }

    if(!haveAThread){
        threads.push_back(new std::thread(func, args...));
    }
  }

private:
    std::vector<std::thread*> threads;
};
/*
class ThreadPool{
public:
  ThreadPool();
  virtual ~ThreadPool();
  void setJob(std::function<void()>& func){
    jobs.push(func);
  }

private:
  std::mutex threadLock;
  void ThreadLoop();
  std::vector<std::thread> threads;
  std::queue<std::function<void()>> jobs;
  uint16_t nrOfThreads;
  bool should_terminate;
};
*/