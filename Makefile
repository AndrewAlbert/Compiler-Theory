CC = g++

CFLAGS = -std=c++11

all: 
	$(CC) $(CFLAGS) compiler.cpp scanner.cpp parser.cpp scope.cpp scopeTracker.cpp codeGenerator.cpp -o compiler
