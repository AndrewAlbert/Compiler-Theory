# C: GCC compiler and flags
CC = gcc
CFLAGS = -std=c90

# C++: G++ compiler and flags
CXX = g++
CXXFLAGS = -std=c++11

# Relevant Directory Structure
Source = testPgms/correct/
fSource = testPgms/incorrect/
Des = Executables/
lib = RuntimeLibrary/runtime.c
Code = SourceCode/

# Leave this intentionally blank unless you want debug mode
# I do not recommend running make all with debug mode
# debug mode can be set as --d or --debug
debug = 

compiler_files = $(Code)scanner.cpp $(Code)parser.cpp $(Code)scope.cpp\
	$(Code)scopeTracker.cpp $(Code)codeGenerator.cpp $(Code)compiler.cpp
compilerName = compiler
compilerProg = ./$(compilerName)

compiler:
	$(CXX) $(CXXFLAGS) $(compiler_files) -o compiler

all: compiler success_tests fail_tests

#Run all Tests for Correct Programs
success_tests: compiler test1 test1orig test1b test1borig test2 test_heap test_program_minimal test_program_array

test1: compiler
	$(compilerProg) $(debug) $(Source)test1.src
	$(CC) $(Source)test1.src.c $(lib) -o $(Des)test1

test1orig: compiler
	$(compilerProg) $(debug) $(Source)test1.src.orig
	$(CC) $(Source)test1.src.orig.c $(lib) -o $(Des)test1orig

test1b: compiler
	$(compilerProg) $(debug) $(Source)test1b.src
	$(CC) $(Source)test1b.src.c $(lib) -o $(Des)test1b

test1borig: compiler
	$(compilerProg) $(debug) $(Source)test1b.src.orig
	$(CC) $(Source)test1b.src.orig.c $(lib) -o $(Des)test1borig

test2: compiler
	$(compilerProg) $(debug) $(Source)test2.src
	$(CC) $(Source)test2.src.c $(lib) -o $(Des)test2

test_heap: compiler
	$(compilerProg) $(debug) $(Source)test_heap.src
	$(CC) $(Source)test_heap.src.c $(lib) -o $(Des)test_heap

test_program_minimal: compiler
	$(compilerProg) $(debug) $(Source)test_program_minimal.src
	$(CC) $(Source)test_program_minimal.src.c $(lib) -o $(Des)test_program_minimal

test_program_array: compiler
	$(compilerProg) $(debug) $(Source)test_program_array.src
	$(CC) $(Source)test_program_array.src.c $(lib) -o $(Des)test_program_array


#Run all Tests for Incorrect Programs
fail_tests: fail_test1 fail_test2 fail_test3

fail_test1:
	$(compilerProg) $(debug) $(fSource)test1.src
fail_test2:
	$(compilerProg) $(debug) $(fSource)test2.src
fail_test3:
	$(compilerProg) $(debug) $(fSource)test3.src

clean: clean_executables clean_cfiles clean_compiler

# remove all created executables
clean_executables:
	rm $(Des)*

# remove all generated c files
clean_cfiles:
	rm $(Source)*.c

clean_compiler:
	rm compiler
