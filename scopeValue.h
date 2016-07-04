#ifndef SCOPEVALUE_H
#define SCOPEVALUE_H

#include <vector>

using namespace std;

/* Struct to hold information about a symbol in scope table.
 *    type - symbol type (procedure, string, integer, etc.)
 *    size - size of arrays (0 for non-arrays)
 *    arguments - vector of input arguments for procedures.
 *    paramType - parameter type IN | OUT | INOUT | NULL
 */
struct scopeValue{
	int type;
	int size;
	vector<scopeValue> arguments; 
	int paramType;
};

#endif
