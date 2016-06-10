#ifndef SCOPETRACKER_H
#define SCOPETRACKER_H

#include "scopeValue.h"
#include "scope.h"
#include "macro.h"
#include <cstdio>

class scopeTracker
{
	private:
		scope* curPtr;
		scope* tmpPtr;
		void reportError(string message);
		//FILE *pFile;
	public:
		scopeTracker();
		~scopeTracker();
		//void newFile(char* filename);
		void newScope();
		void exitScope();
		bool addSymbol(string identifier, scopeValue value, bool global);
		bool prevAddSymbol(string identifier, scopeValue value, bool global);
		bool checkSymbol(string identifier, scopeValue value);
};

#endif
