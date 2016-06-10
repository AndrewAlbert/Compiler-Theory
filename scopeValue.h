#ifndef SCOPEVALUE_H
#define SCOPEVALUE_H

#include <vector>
#include "scopeValue.h"

using namespace std;

struct scopeValue{
	int type; //symbol type
	int size; //size for arrays (0 for non arrays)
	vector<scopeValue> arguments; //vector of input arguments
	int paramType; //parameter type IN | OUT | INOUT | not a parameter
};

#endif
