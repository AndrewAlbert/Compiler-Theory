#ifndef SCOPEVALUE_H
#define SCOPEVALUE_H

#include <vector>

using namespace std;

/* Struct to hold information about a symbol in scope table.
 *    type - symbol type (procedure, string, integer, etc.)
 *    size - size of arrays (0 for non-arrays)
 *    arguments - vector of input arguments for procedures.
 *    paramType - parameter type IN | OUT | INOUT | NULL
 *    FPoffset - number of bytes offset from Frame Pointer on the stack
 *    prevFrameOffset - location to store pointer to previous stack frame
 *    retAddressOffset - location to store pointer to return address for procedure call in program
 *    bytes - total number of bytes in the procedure's frame
 */
struct scopeValue{
	int type;
	int size;
	
	// Used solely for procedures
	vector<scopeValue> arguments;
	int bytes;
	int prevFrameOffset;
	int retAddressOffset;
	
	// Used solely for variables
	int paramType;
	int FPoffset;
	
};

#endif
