#ifndef TOKEN_H
#define TOKEN_H

#include <string>

using namespace std;

/* Struct to hold token information
 *    ascii - the actual token string
 *    type - token type (identifier, begin, end, etc.)
 *    line - the line of the inputfile the token is found in
 *    val - stored value of char, integer, double, string symbols
 */
struct token_type{
	string ascii;
	int type;
	int line;
	union {
	    char stringValue[256]; 
		int intValue;			
		double doubleValue;	
	} val;
};

#endif
