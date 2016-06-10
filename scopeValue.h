#ifndef SCOPEVALUE_H
#define SCOPEVALUE_H

#include <vector>
using namespace std;

struct scopeValue{
	int type; //symbol type
	int size; //size for arrays (0 for non arrays)
	vector<int> arguments; //vector of input argument types
	int paramType;
};

#endif
