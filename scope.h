#ifndef SCOPE_H
#define SCOPE_H

#include <string>
#include <map>
#include "macro.h"
#include "scopeValue.h"

using namespace std;

// The 'scope' class implement symbol tables to add and check variable/procedure declarations and calls
class scope
{
	private:
		/* maps of the key value pairs <identifier, identifier variables>
		 * localTable is used for checking symbols in the current scope, globalTable can be used by scopes further down
		 * globalTable contains all of the global declarations
		 * localTable contains all declarations (includes a duplicate of everything in the globalTable)
		 */
		map<string, scopeValue > globalTable;
		map<string, scopeValue > localTable;
		string name;
		
	public:
		scope(bool programScope = false);
		~scope();
		int totalBytes;
		
		//pointer to parent scope one level up
		scope* prevScope;

		//print, purely for debugging purposes
		void printScope();
		
		//used as a label for the scope table. Will be useful for code generation.
		void setName(string id);

		//symbol table management
		bool addSymbol(string identifier, bool global, scopeValue value);
		bool checkSymbol(string identifier, bool global);
		scopeValue getSymbol(string identifier);
};

#endif
