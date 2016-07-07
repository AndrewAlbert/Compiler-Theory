#include "macro.h"
#include "scopeValue.h"
#include "scope.h"
#include "scopeTracker.h"
#include <iostream>

scopeTracker::scopeTracker(bool debug_input){
	debug = debug_input;
	tmpPtr = nullptr;
	curPtr = nullptr;
}

scopeTracker::~scopeTracker(){

}

//Create a new scope with local and global symbol tables.
void scopeTracker::newScope(){
	if(curPtr != nullptr){
		tmpPtr = curPtr;
		curPtr = new scope();
		curPtr->prevScope = tmpPtr;
	}
	else{
		curPtr = new scope();
		curPtr->prevScope = nullptr;
	}
}

//Exit scope after the end of a procedure declaration or at end of program.
void scopeTracker::exitScope(){
	if(curPtr != nullptr){
		if(debug) curPtr->printScope();
		tmpPtr = curPtr;
		curPtr = curPtr->prevScope;
		delete tmpPtr;
	}
	else if(debug) cout << "not in a scope!" << endl;
	return;
}

//Add a symbol to the current scope's local/global table.
bool scopeTracker::addSymbol(string identifier, scopeValue value, bool global){
	if(curPtr != nullptr){
		if(!curPtr->checkSymbol(identifier, false)){
			curPtr->addSymbol(identifier, global, value);
			return true;
		}
		else return false;
	}
	else return false;
}

//Add symbol to the previous scope's table (used by procedure declarations).
bool scopeTracker::prevAddSymbol(string identifier, scopeValue value, bool global){
	scope* prevPtr = curPtr->prevScope;
	if(prevPtr != nullptr){
		if(!prevPtr->checkSymbol(identifier, false)){
			prevPtr->addSymbol(identifier, global, value);
			return true;
		}
		else return false;
	}
	else return false;
}

//returns true if symbol exists and puts its table entry into &value
bool scopeTracker::checkSymbol(string identifier, scopeValue &value){
	if(curPtr == nullptr) return false;
	else tmpPtr = curPtr;
	//check local symbols of current scope
	bool found = tmpPtr->checkSymbol(identifier, false);
	if(found){
		value = tmpPtr->getSymbol(identifier);
		return true;
	}
	else if(tmpPtr->prevScope != nullptr){
		tmpPtr = tmpPtr->prevScope;
	}
	else{
		return false;
	}
	//check global symbols of all upper scopes
	while(tmpPtr != nullptr){
		found = tmpPtr->checkSymbol(identifier, true);
		if(found){
			value = tmpPtr->getSymbol(identifier);
			return true;
		}
		else{
			if(tmpPtr->prevScope == nullptr) return false;
			else tmpPtr = tmpPtr->prevScope;
		}
	}
	return false;
}

//Set the scope name to appear when each local scope table is printed in debug mode.
void scopeTracker::ChangeScopeName(string &name){
	curPtr->setName(name);
	return;
}
