

#CC=clang++
CC=g++
CFLAGS=-ggdb -O0 -Wall
FILES = out.test_copy_vector


all: $(FILES)


out.% : %.cpp
	$(CC) $(CFLAGS) $< -o  $@




coverage : test_transformations.cpp
	$(CC) -fprofile-arcs -ftest-coverage $(CFLAGS) test_transformations.cpp -o out.test_test_transformations

