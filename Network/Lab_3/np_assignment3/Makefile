CXX=clang++
CC_FLAGS= -Wall -I. -glldb
LD_FLAGS= -Wall -L./ 


all: cchat cserverd

servermain.o: servermain.cpp
	$(CXX)  $(CC_FLAGS) $(CFLAGS) -c servermain.cpp 

clientmain.o: clientmain.cpp
	$(CXX) $(CC_FLAGS) $(CFLAGS) -c clientmain.cpp

cchat: clientmain.o

	$(CXX) $(LD_FLAGS) -o cchat clientmain.o -pthread

cserverd: servermain.o
	$(CXX) $(LD_FLAGS) -o cserverd servermain.o

clean:
	rm *.o cserverd cchat