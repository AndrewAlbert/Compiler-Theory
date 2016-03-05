#ifndef TOKENTYPE_H
#define TOKENTYPE_H
#include<string>
#include<iostream>
using namespace std;
struct token_type{
	int type;
	int line;
	union {
		char stringValue[256]; 
		int intValue;			
		double doubleValue;	
	} val;
	string ascii;
	token_type* next;
};
#endif
