#ifndef SCOPE_H
#define SCOPE_H

#include <string>
#include <map>
#include <vector>
#include "macro.h"
#include "token.h"
#include "scopeValue.h"
#include <cstdio>

using namespace std;

class scope
{
	private:
		map<string, scopeValue > globalTable;
		map<string, scopeValue > localTable;
	public:
		//scope* nextScope;
		scope* prevScope;
		scope();
		~scope();
		void printScope();
		bool addSymbol(string identifier, bool global, scopeValue value);
		bool checkSymbol(string identifier, bool global);
		scopeValue getSymbol(string identifier);
};

#endif
