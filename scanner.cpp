#include "scanner.h"
#include "macro.h"
#include <iostream>

using namespace std;

//Constructor
Scanner::Scanner(){
	
}

//Destructor - close input file
Scanner::~Scanner(){
	if(fPtr != nullptr) fclose(fPtr);
}

bool Scanner::InitScanner(string filename, bool debug_input){
	debug = debug_input;
	line_number = 1;
	fPtr = fopen(filename.c_str(),"r");
	if (fPtr == nullptr){
		cout << "\nThe file: " << filename << "\ndoes not exist, or cannot be opened.\n" << endl;
		return false;
	}

	//Populate the reserved keyword table
	reserved_table[";"] = T_SEMICOLON;
	reserved_table["("] = T_LPAREN;
	reserved_table[")"] = T_RPAREN;
	reserved_table[":="] = T_ASSIGNMENT;
	reserved_table[">="] = T_COMPARE;
	reserved_table[">"] = T_COMPARE;
	reserved_table["<="] = T_COMPARE;
	reserved_table["<"] = T_COMPARE;
	reserved_table["/"] = T_DIVIDE;
	reserved_table["*"] = T_MULTIPLY;
	reserved_table["+"] = T_ADD;
	reserved_table["-"] = T_SUBTRACT;
	reserved_table[","] = T_COMMA;
	reserved_table["["] = T_LBRACKET;
	reserved_table["]"] = T_RBRACKET;
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
	return true;
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
	if (character == '\"')
		return true;
	else
		return false;
}

bool Scanner::isChar(char character){
	if (character == '\'')
		return true;
	else
		return false;
}

bool Scanner::isSingleToken(char character){
	switch(character){
		case '.': 
		case ':': 
		case ';': 
		case '(': 
		case ')': 
		case '=': 
		case ',': 
		case '+': 
		case '-': 
		case '*': 
		case '[': 
		case ']': 
		case '>': 
		case '<': 
		case '!': 
		case '&': 
		case '|':
			return true;
		default:
			return false;
	}
}

bool Scanner::isSpace(char character){
	int ascii = (int)character;
	if(ascii == '\n') line_number++;
	if (ascii <= 32 && character != EOF){
		return true;
	}
	else
		return false;
}

token_type Scanner::getToken(){
	return_token.type = ScanOneToken(fPtr, &return_token);
	return_token.line = line_number;	
	if(debug && return_token.type != T_EOF){
		cout << return_token.ascii << " ";
	}
	return return_token;
}

int Scanner::ScanOneToken(FILE *fPtr, token_type *token){
	char ch, nextch;
	string str = "";
	do{
		ch = getc(fPtr);
		if(debug && (ch == '\n') ) cout << endl;		
	} while(isSpace(ch));
	
	//handle comments or divisor token
	if(ch == '/'){
		str += ch;
		nextch = getc(fPtr);
		if (nextch == '/'){
			while(nextch != '\n'){
				str += nextch;
				nextch = getc(fPtr);
			}
			token->ascii = str;
			line_number++;
			return T_COMMENT;
		}
		else if (nextch == '*'){
			str += nextch;
			int i = 1;
			while (i > 0){
				nextch = getc(fPtr);
				str += nextch;
				if (nextch == '*'){
					nextch = getc(fPtr);
					str += nextch;
					if(nextch == '/') i -= 1;
				}
				else if (nextch == '/'){
					nextch = getc(fPtr);
					str += nextch;
					if(nextch == '*') i += 1;
				}
				else if (nextch == '\n') line_number++;
			}
			token->ascii = str;
			return T_COMMENT;
		}
		else{
			ungetc(nextch, fPtr);
			token->ascii = str;
			return T_DIVIDE;
		}
	}
	//handle integer and float tokens
	else if(isNum(ch)){
		str += ch;
		nextch = getc(fPtr);
		while(isNum(nextch)){
			str += nextch;
			nextch = getc(fPtr);
		}
		if (nextch == '.'){
			str+= nextch;
			nextch = getc(fPtr);
			while(isNum(nextch)){
				str += nextch;
				nextch = getc(fPtr);
			}
			token->val.doubleValue = stod(str);
			token->ascii = str;
			ungetc(nextch, fPtr);
			return TYPE_FLOAT;
		}
		else {
			token->val.intValue = stoi(str);
			token->ascii = str;
			ungetc(nextch, fPtr);
			return TYPE_INTEGER;
		}
	}
	//handle string tokens
	else if (isString(ch)){
		str += ch;
		nextch = getc(fPtr);
		int i = 0;
		while(!isString(nextch)){
			token->val.stringValue[i++] = nextch;
			str += nextch;
			nextch = getc(fPtr);
		}
		str += nextch;
		token->ascii = str;
		return TYPE_STRING;
	}
	//handle char tokens
	else if (isChar(ch)){
		str += ch;
		nextch = getc(fPtr);
		int i = 0;
		while(!isChar(nextch)){
			token->val.stringValue[i++] = nextch;
			str += nextch;
			nextch = getc(fPtr);
		}
		str += nextch;
		token->ascii = str;
		return TYPE_CHAR;
	}
	//handle identifier tokens
	else if (isLetter(ch)){
		int i = 0;
		token->val.stringValue[i++] = toupper(nextch);
		str += toupper(ch);
		nextch = getc(fPtr);
		while(isLetter(nextch) || isNum(nextch) || nextch == '_')
		{
			token->val.stringValue[i++] = toupper(nextch);
			str += toupper(nextch);
			nextch = getc(fPtr);
		}
		ungetc(nextch, fPtr);
		token->ascii = str;
		
		map<string,int>::iterator it;
		it = reserved_table.find(str);
		if (it != reserved_table.end()) return reserved_table.find(str)->second;
		else return TYPE_IDENTIFIER;
	}
	else if (isSingleToken(ch)){
		str += ch;
		token->ascii = str;
		switch(ch){
			case '.': return T_PERIOD;
			case ';': return T_SEMICOLON;
			case '(': return T_LPAREN;
			case ')': return T_RPAREN;
			case '/': return T_DIVIDE;
			case '*': return T_MULTIPLY;
			case '+': return T_ADD;
			case '-': return T_SUBTRACT;
			case ',': return T_COMMA;
			case '[': return T_LBRACKET;
			case ']': return T_RBRACKET;
			case ':':
				ch = getc(fPtr);
				if (ch == '='){
					str += ch;
					token->ascii = str;
					return T_ASSIGNMENT;
				}
				else{
					ungetc(ch, fPtr);
					token->ascii = str;
					return T_UNKNOWN;
				}
			case '>': case '<': case '=':
				ch = getc(fPtr);
				if (ch == '='){
					str+=ch;
					token->ascii = str;
					return T_LOGICAL;
				}
				else{
					ungetc(ch, fPtr);
					if (str == "=") return T_UNKNOWN;
					else return T_LOGICAL;
				}
			case '!':
				ch = getc(fPtr);
				if (ch == '='){
					str += ch;
					token->ascii = str;
					return T_LOGICAL;
				}
				else{
					ungetc(ch,fPtr);
					return T_UNKNOWN;
				}
			case '&': case '|':
				return T_BITWISE;			
			default: return T_UNKNOWN;
		}
	}
	else if (ch == EOF) return T_EOF;
	else{
		str += ch;
		token->ascii = str;
		return T_UNKNOWN;
	}
}
