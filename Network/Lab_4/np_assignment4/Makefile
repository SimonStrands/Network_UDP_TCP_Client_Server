serverthread_SRC = serverthread.cpp
serverthread_SRC += HandleUser.cpp
serverthread_SRC += ThreadPool.cpp

serverfork_SRC = serverfork.cpp
serverfork_SRC += HandleUser.cpp

forkTest_SRC = forkTest.cpp
forkTest_OBJ := $(forkTest_SRC:.cpp=.o)

serverthread_OBJ := $(serverthread_SRC:.cpp=.o)
serverfork_OBJ := $(serverfork_SRC:.cpp=.o)

all: serverthread serverfork

makenew: clean2 serverthread serverfork

serverfork: $(serverfork_OBJ)
	$(CXX) -L./ -Wall -o serverfork $(serverfork_OBJ) -I.

serverthread: $(serverthread_OBJ)
	$(CXX) -L./ -Wall -o serverthread $(serverthread_OBJ) -lpthread -I.

forkTest: $(forkTest_OBJ)
	$(CXX) -L./ -Wall -o forkTest $(forkTest_OBJ) -I.

clean:
	rm *.o *.a perf_*.txt  tmp.* serverfork serverthread forkTest

clean2:
	rm *.o serverfork serverthread 
