
CC=g++
#CC=clang++
CFLAGS=-Wall -std=c++17 -O4 -ggdb
LDFLAGS=-ggdb
SOURCES=opengl_test.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=opengl_test
LIBS = -lstdc++ -lm -lsfml-graphics -lsfml-window -lsfml-system

all: $(EXECUTABLE)


$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@ 

clean:
	rm *.o $(EXECUTABLE) *~
