# Compiler Variable
CC = g++

CFLAGS = -std=c++11

all: 
	#$(CC) $(CFLAGS) testProg.cpp scanner.cpp scope.cpp parser.cpp scopeTracker.cpp -o compiler
	$(CC) $(CFLAGS) compiler.cpp scanner.cpp parser.cpp scope.cpp scopeTracker.cpp -o compiler
