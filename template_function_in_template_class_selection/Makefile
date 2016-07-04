

FILES = out.test_template_function_in_template_class_selection


all: $(FILES)


out.% : %.cpp
	g++ -fdump-tree-all -fdump-rtl-all -ggdb -O0 -Wall $< -o  $@

clean:
	rm out.* *.cpp.*


coverage : test_transformations.cpp
	g++ -fprofile-arcs -ftest-coverage -ggdb -O0 -Wall test_transformations.cpp -o out.test_test_transformations

