
CC=g++
#CC=clang++
CFLAGS=-Wall -std=c++14 -O4 -ggdb
LDFLAGS=-ggdb
SOURCES=test_hundred_best.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=runTest
LIBS = -lstdc++ -lm

all: $(EXECUTABLE)


$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@ 

clean:
	rm *.o $(EXECUTABLE) *~
