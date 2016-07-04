

FILES = out.test_negate_sizet out.test_template_default_parameters out.test_iterator_to_const_iterator


all: $(FILES)


out.% : %.cpp
	g++ -ggdb -O0 -Wall $< -o  $@




coverage : test_transformations.cpp
	g++ -fprofile-arcs -ftest-coverage -ggdb -O0 -Wall test_transformations.cpp -o out.test_test_transformations

