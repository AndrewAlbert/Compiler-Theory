#ifndef SCANNER_H
#define SCANNER_H

#include <string>
#include <map>
#include <cstdio>
#include "macro.h"
#include "token_type.h"

using namespace std;
class Scanner
{
	private:
		int line_number;
		token_type return_token;
		FILE * fPtr;
		bool debug;	
		map<string,int> reserved_table;
		int ScanOneToken(FILE * fPtr, token_type *token);
		bool isNum(char character);
		bool isLetter(char character);
		bool isString(char character);
		bool isChar(char character);
		bool isSingleToken(char character);
		bool isSpace(char character);	
	public:
		Scanner();
		~Scanner();		
		bool InitScanner(string filename, bool debug_input);
		token_type getToken();	
		void PrintToken();			
		token_type *token;
};

#endif
