#include "scanner.h"
#include "parser.h"
#include "macro.h"
#include "token_type.h"
#include "scopeTracker.h"
#include <string>
#include <iostream>

using namespace std;

void commandError(){
	cout << "Unable to process command line arguments. Make sure they are: [ --help | --h ] filename" << endl;
	return;
}

//Program call should be: ./compiler [-help] filename
int main(int argc, char **argv){
	bool debug;
	string filename;
	if( (argc == 2) || (argc == 3) ){
		if( (string(argv[1]) == "--help") || (string(argv[1]) == "--h") ){
			cout << "\nThis is a compiler written for the University of Cincinnati class: EECE6083 Compiler Theory" << endl;
			cout << "\nThe compiler is an LL(1) recursive descent compiler and utilizes the C++ programming language to scan, parse, and type check the program and LLVM to generate the compiler backend." << endl;
			cout << "\nTo use this compiler, compile and then run from the command line using the arguments: [ --help | --h | --debug | --d ] filename." << endl;
			cout << "\nThe compiler will scan and parse your file and generate code if parsing is successful. Otherwise relevant errors and warnings will be shown." << endl;
			cout << "\n--debug or --d argument will print out each token as it is scanned and print out each scope's symbol table after the scope is exited." << endl;
			return 0;
		}
		else if( (string(argv[1]) == "--debug") || (string(argv[1]) == "--d") ){
			debug = true;
			filename = string(argv[2]);
		}
		else if(argc == 2){
			debug = false;
			filename = string(argv[1]);
		}	
	}
	else{
		commandError();
		return 0;
	}

	//Initialize scanner and symbol tables.
	Scanner *scanner = new Scanner;
	scopeTracker *scopes = new scopeTracker(debug);
	codeGenerator *gen = new codeGenerator;

	//Contains token currently being scanned/parsed
	token_type *token = new token_type;

	scanner->token = token;
	//Initialize scanner, then begin parsing if there are no errors
	if(scanner->InitScanner(filename, debug)){
		*token = scanner->getToken();
		gen->attachOutputFile( (filename) + ".c" );
		Parser parser(token, scanner, scopes, gen);
	}
	delete scanner;
	delete scopes;
	delete gen;

	return 0;
}
