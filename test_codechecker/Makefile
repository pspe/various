
CC=g++
#CC=clang++
CFLAGS=-Wall -Wno-attributes -std=c++14 -O0 -ggdb
#CFLAGS=-Wall -O0
LDFLAGS=-ggdb
SOURCES=test_codechecker.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=runTest
LIBS = -lstdc++ -lm
NOW := `date +'%Y%m%d_%H%M'`

all: $(EXECUTABLE)


$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

check:
	CodeChecker check -n test_codechecker_$(NOW) -b "make clean; make"

datetest:
	touch test_check_$(NOW)
	@echo $(NOW)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@ 

clean:
	rm *.o $(EXECUTABLE) *~
