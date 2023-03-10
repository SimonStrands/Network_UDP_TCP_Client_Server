#include <thread>
#include <math.h>
#include <iostream>

void stressTest(bool& done){
  while(!done){
    float result = 0;
    for(int i = 0; i < 10; i++){
      for(int b = 0; b < 10; b++){
        float f = sqrt(rand() % 2000);
        result += f;
      }
    }
  }
  return;
}

int main(int argc, char *argv[]){
  bool done = false;
  const int nrOfThreads = 8;
  std::thread threads[nrOfThreads];
  srand(time(NULL));

  for(int i = 0; i < nrOfThreads; i++){
    threads[i] = std::thread(stressTest, std::ref(done));
  }

  std::string a;
  std::cin >> a;
  done = true;
  std::cout << "done is " << done << std::endl;
  for(int i = 0; i < nrOfThreads; i++){
    threads[i].join();
  }


  return 0;
}