# Compiler Variable
CC = g++

CFLAGS = -std=c++11

all: 
	$(CC) $(CFLAGS) compiler.cpp scanner.cpp parser.cpp scope.cpp scopeTracker.cpp -o compiler
