
CC=g++
#CC=clang++
#CFLAGS=-Wl,--no-as-needed -Wall -Wno-unused -Wno-reorder -std=c++1y -O0 -ggdb -pthread
CFLAGS=-Wl,--no-as-needed -Wall -std=c++1y -O4 -ggdb -pthread
LDFLAGS=-ggdb -lpthread
SOURCES=speedTest.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=speedTest
LIBS = -lstdc++ -lm

all: $(EXECUTABLE) #speed #clangtest


speed: speedTest.cpp
	$(CC) $(CFLAGS) speedTest.cpp -o speedTest 


$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@ 

clean:
	rm -f speedTest


