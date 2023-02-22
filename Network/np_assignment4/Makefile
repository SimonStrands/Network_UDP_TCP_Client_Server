CXX=clang++
serverthread_SRC = serverthread.cpp
serverthread_SRC += HandleUser.cpp
serverthread_SRC += ThreadPool.cpp

serverfork_SRC = serverfork.cpp
serverfork_SRC += HandleUser.cpp

Test_SRC = Test.cpp
Test_OBJ := $(Test_SRC:.cpp=.o)

serverthread_OBJ := $(serverthread_SRC:.cpp=.o)
serverfork_OBJ := $(serverfork_SRC:.cpp=.o)

all: serverthread serverfork

test: Test

serverfork: $(serverfork_OBJ)
	$(CXX) -L./ -Wall -o serverfork $(serverfork_OBJ) -I.

serverthread: $(serverthread_OBJ)
	$(CXX) -L./ -Wall -o serverthread $(serverthread_OBJ) -lpthread -I.

Test: $(Test_OBJ)
	$(CXX) -L./ -Wall -o Test $(Test_OBJ) -lpthread -I.

clean:
	rm *.o *.a perf_*.txt  tmp.* serverfork serverthread

clean2:
	rm *.o Test

