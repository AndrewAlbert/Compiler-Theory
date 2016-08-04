#ifndef SCOPEVALUE_H
#define SCOPEVALUE_H

#include <vector>
#include <string>

using namespace std;

/* Struct to hold information about a symbol in scope table.
 *    type - symbol type (procedure, string, integer, etc.)
 *    size - size of arrays (0 for non-arrays)
 *    arguments - vector of input arguments for procedures.
 *    paramType - parameter type IN | OUT | INOUT | NULL
 *    FPoffset - number of bytes offset from Frame Pointer on the stack
 *    CallLabel - string of the procedure start label
 */
struct scopeValue{
	int type;
	int size;
	
	// Used solely for procedures
	vector<scopeValue> arguments;
	string CallLabel;
	
	// Used solely for variables
	int paramType;
	int FPoffset;
	
};

#endif
