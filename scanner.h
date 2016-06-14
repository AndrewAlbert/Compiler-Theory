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
		token_type *headPtr;
		int line_number;
		FILE * fPtr;
		token_type *tailPtr;
		
		int ScanOneToken(FILE * fPtr, token_type *token);
		bool isNum(char character);
		bool isLetter(char character);
		bool isString(char character);
		bool isChar(char character);
		bool isSingleToken(char character);
		bool isSpace(char character);

	public:
		Scanner(string filename);
		virtual ~Scanner();
		int InitScanner();	
		map<string,int> reserved_table;
		void PrintTokens();	
		token_type* pass_ptr;
};

#endif
