
CC=g++
#CC=clang++
#CFLAGS=-Wall -Wno-attributes -std=c++11 -O0 -ggdb -flto=4
CFLAGS=-Wall -Wno-attributes -std=c++11 -O0 -ggdb -flto -Iinc
#CFLAGS=-Wall -O0
LDFLAGS=-ggdb
#SOURCES=test_codechecker.cpp
SOURCES=test_small.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=runTest
LIBS = -lstdc++ -lm
NOW := `date +'%Y%m%d_%H%M'`

all: $(EXECUTABLE)


$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

codechecker:
	CodeChecker check -n test_check_$(NOW) -b "make clean; make"

datetest:
	touch test_check_$(NOW)
	@echo $(NOW)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@ 

clean:
	rm -f *.o $(EXECUTABLE) "*~"
