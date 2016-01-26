#include<string>
#include<map>
#include<fstream>
#include<iostream>
#include<cstdio>

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
#define T_END 349		//EOF
#define T_UNKNOWN 350	//unknown token

struct token_type{
	int type;					//token code
	//token can only hold one of the data types specified in union type val
	union {
		string stringValue; 	//string/identifier
		int intValue;			//integer
		double doubleValue;		//double
	} val;
	token_type* next;
};

//Map of reserved identifiers
map<string,int> reserved_table;
	
int main()
{	
	ifstream myfile;
	myfile.open("Test.txt")
	
	char character;
	
	
	while(myfile){
		myfile.get(character)
		cout << ch;
	}

	InitScanner();
	
	token_type* headPtr = new token_type;
	token_type* tailPtr = tailPtr;
	
	while(ScanOneToken(stdin, &token) != T_END){
		
	}

	return 0;
}

int ScanOneToken(FILE *fp, struct token_type &token){
	return 0;
}

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

bool isNum(char character){
	int ascii = (int)character;
	if ((ascii >= 48) && (ascii <= 57))
		return true;
	else
		return false;
}

bool isLetter(char character){
	int ascii = (int)character;
	if (((ascii >= 65) && (ascii <= 90)) || ((ascii >= 97) && (ascii <= 122)))
		return true;
	else
		return false;
}

bool isString(char character){
	int ascii = (int)character;
	if (ascii == 32)
		return true;
	else
		return false;
}

bool isChar(char character){
	int ascii = (int)character;
	if (ascii == 39)
		return true;
	else
		return false;
}

bool isSingleToken(char character){
	switch(character){
		case ';': case '(': case ')': case '=': case ',': case '+': case '-': case '[': case ']':
			return true;
		default:
			return false;
	}
}

bool isSpace(char character){
	int ascii = (int)character;
	if (ascii <= 32)
		return true;
	else
		return false;
}