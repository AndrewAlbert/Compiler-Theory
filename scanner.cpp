#include<string>
#include<map>
#include<fstream>
#include<iostream>
#include<cstdio>
#include<stdio.h>
#include "scanner.h"

using namespace std;

//single ASCII character tokens
#define T_SEMICOLON ";"
#define T_LPAREN "("
#define T_RPAREN ")"
#define T_ASSIGN "="
#define T_DIVIDE "/"
#define T_MULTIPLY "*"
#define T_ADD "+"
#define T_SUBTRACT "-"
#define T_COMMA ","
#define T_LBRACKET "["
#define T_RBRACKET "]"

//reserved keywords
#define T_PROGRAM 257
#define T_IS 258
#define T_BEGIN 259
#define T_END 260
#define T_GLOBAL 261
#define T_PROCEDURE 262
#define T_IN 263
#define T_OUT 264
#define T_INOUT 265
#define T_INTEGER 266
#define T_FLOAT 267
#define T_BOOL 268
#define T_STRING 269
#define T_CHAR 270
#define T_NOT 271
#define T_IF 272
#define T_THEN 273
#define T_ELSE 274
#define T_FOR 275
#define T_RETURN 276
#define T_TRUE 277
#define T_FALSE 278

//Identifiers
#define TYPE_INTEGER 279
#define TYPE_FLOAT 280
#define TYPE_STRING 281
#define TYPE_CHAR 282
#define TYPE_BOOL 283

//other
#define T_EOF 349		//EOF
#define T_UNKNOWN 350	//unknown token

Scanner::Scanner(string filename){
	headPtr = nullptr;
	tailPtr = nullptr;
	fPtr = fopen(filename,"r");
	if (fPtr == NULL){
		cout << "No file exists!" << endl;
		~Scanner();
	}
	else{
		headPtr = new token_type;
		tailPtr = headPtr;
		while(ScanOneToken(fPtr, tailPtr) == 1){
			tailPtr->next = new token_type;
			tailPtr = tailPtr->next;
		}
		PrintTokens();
	}
}

Scanner::~Scanner(){
	tailPtr = nullptr;
	while(headPtr != nullptr){
		tailPtr = headPtr->next;
		headPtr->next = nullptr;
		headPtr = tailPtr;		
	}
	tailPtr = nullptr;
	headPtr = nullptr;	
	fclose(fPtr);
}

void Scanner::PrintTokens(){
	token_type *tmpPtr = headPtr;
	while(tmpPtr != nullptr){
		cout << tmpPtr->ascii << endl;
		tmpPtr = tmpPtr->next;
	}
}

bool Scanner::isNum(char character){
	int ascii = (int)character;
	if ((ascii >= 48) && (ascii <= 57))
		return true;
	else
		return false;
}

bool Scanner::isLetter(char character){
	int ascii = (int)character;
	if (((ascii >= 65) && (ascii <= 90)) || ((ascii >= 97) && (ascii <= 122)))
		return true;
	else
		return false;
}

bool Scanner::isString(char character){
	int ascii = (int)character;
	if (ascii == 34)
		return true;
	else
		return false;
}

bool Scanner::isChar(char character){
	int ascii = (int)character;
	if (ascii == 39)
		return true;
	else
		return false;
}

bool Scanner::isSingleToken(char character){
	switch(character){
		case ';': case '(': case ')': case '=': case ',': case '+': case '-': case '[': case ']':
			return true;
		default:
			return false;
	}
}

bool Scanner::isSpace(char character){
	int ascii = (int)character;
	if (ascii <= 32)
		return true;
	else
		return false;
}

int Scanner::ScanOneToken(FILE *fPtr, struct token_type &token){
	char ch, nextch;
	int i;
	string str = "";
	do{
		ch = getc(fPtr);		
	} while(isSpace(ch));
	
	while(!isSpace(ch) && ch != EOF){
		str.append(ch);
		ch = getc(fPtr);
	}
	token->ascii = str;
	if(ch == EOF)
		return 0;
	else
		return 1;
}

/*
//Map of reserved identifiers
map<string,int> reserved_table;
*/


/*
void InitScanner(){	
	//SINGLE ASCII CHARACTERS
	reserved_table[";"] = T_SEMICOLON;
	reserved_table["("] = T_LPAREN;
	reserved_table[")"] = T_RPAREN;
	reserved_table["="] = T_ASSIGN;
	reserved_table["/"] = T_DIVIDE;
	reserved_table["*"] = T_MULTIPLY;
	reserved_table["+"] = T_ADD;
	reserved_table["-"] = T_SUBTRACT;
	reserved_table[","] = T_COMMA;
	reserved_table["["] = T_LBRACKET;
	reserved_table["]"] = T_RBRACKET;
	
	//RESERVED KEYWORDS
	reserved_table["PROGRAM"] = T_PROGRAM;
	reserved_table["IS"] = T_IS;
	reserved_table["BEGIN"] = T_BEGIN;
	reserved_table["END"] = T_END;
	reserved_table["GLOBAL"] = T_GLOBAL;
	reserved_table["PROCEDURE"] = T_PROCEDURE;
	reserved_table["IN"] = T_IN;
	reserved_table["OUT"] = T_OUT;
	reserved_table["INOUT"] = T_INOUT;
	reserved_table["INTEGER"] = T_INTEGER;
	reserved_table["FLOAT"] = T_FLOAT;
	reserved_table["BOOL"] = T_BOOL;
	reserved_table["STRING"] = T_STRING;
	reserved_table["CHAR"] = T_CHAR;
	reserved_table["NOT"] = T_NOT;
	reserved_table["IF"] = T_IF;
	reserved_table["THEN"] = T_THEN;
	reserved_table["ELSE"] = T_ELSE;
	reserved_table["FOR"] = T_FOR;
	reserved_table["RETURN"] = T_RETURN;
	reserved_table["TRUE"] = T_TRUE;
	reserved_table["FALSE"] = T_FALSE;
	
	//reserved_table[""] = T_END;
	//reserved_table[""] = T_UNKNOWN;
}
*/
