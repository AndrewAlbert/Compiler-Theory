#include "macro.h"
#include "scope.h"
#include "scopeTracker.h"
#include <cstdio>

scopeTracker::scopeTracker(){
	tmpPtr = nullptr;
	curPtr = nullptr;
}

scopeTracker::~scopeTracker(){

}

void scopeTracker::newScope(string scopeID){
	if(curPtr != nullptr){
		tmpPtr = curPtr;
		curPtr = new scope(scopeID);
		curPtr->prevScope = tmpPtr;
	}
	else{
		curPtr = new scope(scopeID);
		curPtr->prevScope = tmpPtr;
	}
}

void scopeTracker::exitScope(){
	if(curPtr != nullptr){
		//curPtr->printScope();
		tmpPtr = curPtr;
		curPtr = curPtr->prevScope;
		delete tmpPtr;
	}
	else cout << "not in a scope!" << endl;
}

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

void scopeTracker::ChangeScopeName(string name){
	curPtr->setName(name);
	return;
}
