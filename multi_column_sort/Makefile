
CC=g++
#CC=clang++
CFLAGS=-Wall -std=c++14 -O0 -ggdb
#CFLAGS=-Wall -std=c++03 -O0 -ggdb
LDFLAGS=-ggdb
SOURCES=multi_column_sort.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=runTest
LIBS = -lstdc++ -lm

all: $(EXECUTABLE)


$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@ 

clean:
	rm -f *.o $(EXECUTABLE) *~
