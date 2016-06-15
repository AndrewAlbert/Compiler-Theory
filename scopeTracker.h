#ifndef SCOPETRACKER_H
#define SCOPETRACKER_H

#include "scope.h"
#include "macro.h"
#include <cstdio>

/*
 * Interface for managing nested scope tables. Uses the 'scope' class to implement all functionality.
 * Will add and check symbols to the nested scope tables.
*/
class scopeTracker
{
	private:
		scope* curPtr;
		scope* tmpPtr;
		void reportError(string message);
	public:
		scopeTracker();
		~scopeTracker();
		void newScope();
		void exitScope();
		bool addSymbol(string identifier, scopeValue value, bool global);
		//identical to addSymbol, but for one scope level up. Used to add procedure declaration to its parent scope and own scope
		bool prevAddSymbol(string identifier, scopeValue value, bool global);
		bool checkSymbol(string identifier, scopeValue &value);
};

#endif
