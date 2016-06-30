#ifndef SCANNER_H
#define SCANNER_H

#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <stdio.h>
#include <cstddef>
#include "macro.h"

using namespace std;
class Scanner
{
	private:
		int line_number;
		FILE * fPtr;
		map<string,int> reserved_table;
		int ScanOneToken(FILE * fPtr, token_type *token);
		bool isNum(char character);
		bool isLetter(char character);
		bool isString(char character);
		bool isChar(char character);
		bool isSingleToken(char character);
		bool isSpace(char character);
		bool debug;
	public:
		Scanner();
		~Scanner();		
		bool InitScanner(string filename, bool debug_input);
		token_type getToken();	
		void PrintToken();	
		token_type return_token;
		token_type *token;
};

#endif
