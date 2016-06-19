#include "scanner.h"
#include "parser.h"
#include "macro.h"
#include "scope.h"
#include "scopeTracker.h"
#include<string>
#include<iostream>
#include <cstdio>

using namespace std;

void commandError(){
	cout << "Unable to process command line arguments. Make sure they are: [ --help | --h ] filename" << endl;
	return;
}

//Program call should be: ./compiler [-help] filename
int main(int argc, char **argv){
	string filename;
	if( (argc == 0) || (argc > 2) ){
		commandError();
		return 0;
	}
	else if( ( argv[0] == "--help") || ( argv[0] == "--h") ){
		cout << "This is a compiler written for the University of Cincinnati class: EECE6083 Compiler Theory" << endl;
		cout << "The compiler acts as an LL(1) recursive descent compiler and utilizes the C++ programming language and LLVM" << endl;
		cout << "To use this compiler, compile and then run from the command line using the arguments: [ --help | --h ] filename" << endl;
		cout << "The compiler will scan and parse your file and generate code if parsing is successful. Otherwise appropriate errors and warnings will be shown" << endl;
	}
	else if( argc == 1 ){
		//test to see if the file can be read
		ifstream inputfile(filename);
		if( inputfile.is_open() ) inputfile.close();
		else{
			commandError();
			return 0;
		}
	}
	else{
		commandError();
		return 0;
	}
	
	scopeTracker* Scopes = new scopeTracker;
	Scanner scanner(filename);
	Parser parser(scanner.pass_ptr, Scopes);
	return 0;
}
