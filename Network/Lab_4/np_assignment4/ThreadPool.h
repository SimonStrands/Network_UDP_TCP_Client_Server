#include <mutex>
#include <thread>
#include <vector>
#include <functional>
////////////////
#include <queue>
#include <condition_variable>

//comment this out or in based if the server should be threadpooling or not
//#define NotPooled true

#if defined (NotPooled) 
class ThreadPool{
public:
  ThreadPool();
  virtual ~ThreadPool();
  template <class _Fn, class... _Args>
  bool setJob(_Fn&& func,  _Args&&... args){
    //look for a thread that is NotRunning
    bool done = false;
    for(int i = 0; i < runningThreads.size() && !done; i++){
      if(runningThreads[i] == 0){
        runningThreads[i] = 1;
        threads[i] = std::thread(func, args..., std::ref(runningThreads[i]));
        return true;
      }
    }
    return false;
  }

  bool HaveThread();

private:
    std::vector<std::thread> threads;
    std::vector<int> runningThreads;//0 = NotRunning, 1 = running, 2 = ready to join
    int nrOfThreads;
};
#else

struct jobStruct{
  std::function<void(int, int&)> job;
  int sock;
  int ret;
  jobStruct(const std::function<void(int, int&)> &job, int sock){
    this->job = job;
    this->sock = sock;
    this->ret = 0;
  }
  jobStruct();
};

class ThreadPool {
public:
    ThreadPool();
    ~ThreadPool();
    void setJob(const std::function<void(int, int&)>& job, int sock);
    void Stop();
    bool busy();

private:
    void ThreadLoop();

    bool should_terminate = false;           // Tells threads to stop looking for jobs
    std::mutex queue_mutex;                  // Prevents data races to the job queue
    std::condition_variable mutex_condition; // Allows threads to wait on new jobs or termination 
    std::vector<std::thread> threads;
    std::queue<jobStruct> jobs;
};
#endif