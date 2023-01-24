#include <thread>
#include <vector>
#include <functional>

class ThreadPool{
public:
  ThreadPool();
  virtual ~ThreadPool();
  void setJob(std::function<void(int)> job, int client){
    bool haveAThread = false;
    for(int i = 0; i < threads.size(); i++){
        if(threads[i]->joinable()){
            threads[i]->join();
            delete threads[i];
            threads[i] = new std::thread(job, client);
            return;
        }
    }

    if(!haveAThread){
        threads.push_back(new std::thread(job, client));
    }
  }

private:
    std::vector<std::thread*> threads;
};