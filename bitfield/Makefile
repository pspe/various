
#CC=gcc
CC=clang++
CFLAGS=-Wall -std=c++11 -O0 -ggdb
LDFLAGS=-ggdb
SOURCES=bitfield.cpp test_bitfield.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=test_bitfield
LIBS = -lstdc++ -lm

all: $(EXECUTABLE) 


$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@ 

clean:
	rm test_bitfield *.o
