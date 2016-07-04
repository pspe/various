

FILES = out.test_ontology


all: $(FILES)


out.% : %.cpp
	g++ -ggdb -O0 -Wall $< -o  $@




coverage : test_ontology.cpp
	g++ -fprofile-arcs -ftest-coverage -ggdb -O0 -Wall test_ontology.cpp -o out.test_test_ontology
