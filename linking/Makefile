
CC=g++
#CC=clang++
CFLAGS=-Wall -std=c++11 -O4 -ggdb
LDFLAGS=-ggdb
SOURCES=prog.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=prog
LIBS = -lstdc++ -lm

all: $(EXECUTABLE)


#$(EXECUTABLE): $(OBJECTS)
#	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

.cpp.o:
	$(CC) $(CFLAGS) -c $< #-o $@ 

clean:
	rm *.o $(EXECUTABLE) *~
