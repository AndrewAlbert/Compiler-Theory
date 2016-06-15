#include "macro.h"
#include "scope.h"
#include "scopeTracker.h"
#include <cstdio>

scopeTracker::scopeTracker(){
	//pFile = fopen("debug.txt","w");
}

scopeTracker::~scopeTracker(){
	//fclose(pFile);
}
/*
void scopeTracker::newFile(char* filename){
	fclose(pFile);
	pFile = fopen(filename,"w");
}*/

void scopeTracker::newScope(){
	if(curPtr != nullptr){
		cout << "scope added" << endl;
		tmpPtr = curPtr;
		curPtr = new scope();
		curPtr->prevScope = tmpPtr;
		tmpPtr = nullptr;
	}
	else{
		cout << "first scope" << endl;
		curPtr = new scope();
		curPtr->prevScope = nullptr;
	}
}

void scopeTracker::exitScope(){
	if(curPtr != nullptr){
		curPtr->printScope();
		tmpPtr = curPtr;
		curPtr = curPtr->prevScope;
		delete tmpPtr;
	}
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
bool scopeTracker::checkSymbol(string identifier, scopeValue &value, bool procedure){
	tmpPtr = curPtr;
	//check local symbols of current scope
	found = tmpPtr->checkSymbol(string identifier, false);
	if(found){
		value = tmpPtr->getSymbol(string identifier)
		return true;
	}
	else tmpPtr = tmpPtr->prevScope;
	//check global symbols of all upper scopes
	while(tmpPtr != nullptr){
		found = tmpPtr->checkSymbol(string identifier, true);
		if(found){
			value = tmpPtr->getSymbol(string identifier)
			return true;
		}
		else tmpPtr = tmpPtr->prevScope;
	}
	return false;
}

void scopeTracker::reportError(string message){

}
