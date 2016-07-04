
CC=g++
#CC=clang++
CFLAGS=-Wl,--no-as-needed -Wall -std=c++1y -O4 -ggdb -pthread
LDFLAGS=-ggdb -lpthread
SOURCES=test_enumClasses.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=exec_enumClasses
LIBS = -lstdc++ -lm

all: $(EXECUTABLE) #clangtest

#all: $(OBJECTS) #clangtest


.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@ 


$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)


clean:
	rm nn_1 nn_mainExe *.o clangtest *~ $(EXECUTABLE) threadtest ../mnist/*.o

