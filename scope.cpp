#include "scope.h"
#include <string>
#include <map>
#include <vector>
#include <cstdio>

using namespace std;

/* Set initial scope 'name' will only be valid for the program.
 * Procedures will set their name later. */
scope::scope(string id){
	name = id;
}

scope::~scope(){

}

//Change the string 'name' member of this scope
void scope::setName(string id){
	name = id;
	return;
}

//Add procedure or variable symbol to this scope's local and/or global table along with scopeValue attributes.
bool scope::addSymbol(string identifier, bool global, scopeValue value){
	map<string, scopeValue>::iterator it;
	it = localTable.find(identifier);
	if(it != localTable.end()) return false;
	else{
		if(global) globalTable[identifier] = value;
		localTable[identifier] = value;
	}
}

//check to see if the given symbol exists in this scope's local or global symbol table.
bool scope::checkSymbol(string identifier, bool global){
	map<string, scopeValue>::iterator it;
	if(global){
		it = globalTable.find(identifier);
		if( it != globalTable.end() ) return true;
		else return false;
	}
	else{
		it = localTable.find(identifier);
		if( it != localTable.end() ) return true;
		else return false;
	}
}

//Get symbol identifier's scopeValue from this scope's local table if one exists.
scopeValue scope::getSymbol(string identifier){
	map<string, scopeValue>::iterator it;
	it = localTable.find(identifier);
	if(it != localTable.end())
		return it->second;
	else{
		scopeValue nullVal;
		nullVal.type = T_UNKNOWN;
		vector<scopeValue> nullVector;
		nullVal.arguments = nullVector;
		return nullVal;
	}
}

//print the local table entries
void scope::printScope(){
	cout << "SCOPE: " << name << "\n\nLocal Table:\n";
	map<string, scopeValue>::iterator it;
	for(it = localTable.begin(); it != localTable.end(); it++){
		cout << "id: " << it->first;
		cout << "\ttype: " << it->second.type << "\tsize: " << it->second.size << endl;
		cout << "\tparameters:\t";
		vector<scopeValue> Vec = it->second.arguments;		
		vector<scopeValue>::size_type vec_it;
		for(vec_it = 0; vec_it != Vec.size(); vec_it++){
			cout << Vec[vec_it].type << "\t";
		}
		cout << endl;
	}
}
