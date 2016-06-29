#include "scanner.h"
#include "parser.h"
#include "macro.h"
#include "scope.h"
#include "scopeTracker.h"
#include <string>
#include <iostream>
#include <cstdio>

using namespace std;

void commandError(){
	cout << "Unable to process command line arguments. Make sure they are: [ --help | --h ] filename" << endl;
	return;
}

//Program call should be: ./compiler [-help] filename
int main(int argc, char **argv){
	string filename;
	if( (argc == 2) || (argc == 3) ){
		if( (string(argv[1]) == "--help") || (string(argv[1]) == "--h") ){
		cout << "This is a compiler written for the University of Cincinnati class: EECE6083 Compiler Theory" << endl;
		cout << "The compiler acts as an LL(1) recursive descent compiler and utilizes the C++ programming language and LLVM" << endl;
		cout << "To use this compiler, compile and then run from the command line using the arguments: [ --help | --h ] filename" << endl;
		cout << "The compiler will scan and parse your file and generate code if parsing is successful. Otherwise appropriate errors and warnings will be shown" << endl;
		return 0;
		}
		else{
			filename = string(argv[1]);
			//test to see if the file can be read
			ifstream inputfile(filename);
			if( inputfile.is_open() ) inputfile.close();
			else{
				commandError();
				return 0;
			}
		}	
	}
	else{
		commandError();
		return 0;
	}
	
	//Initialize scanner and symbol tables.
	Scanner *scanner = new Scanner;
	scopeTracker *scopes = new scopeTracker;

	//Contains token currently being scanned/parsed
	token_type *token = new token_type;
	/* Contains previous token. 
	 * Is not checked, just allows the compiler to grab the token string and values after CheckToken has been called 	
	 * (since a new token will be scanned if the check is succesful) */
	token_type *prev_token = new token_type;

	scanner->prev_token = token;
	scanner->token = prev_token;
	//Initialize scanner, then begin parsing if there are no errors
	if(scanner->InitScanner(filename)){
		*token = scanner->getToken();
		Parser parser(token, prev_token, scanner, scopes);
	}
	else return 0;
}
