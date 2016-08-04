#include "scope.h"
#include "scopeValue.h"
#include <string>
#include <map>
#include <vector>
#include <cstdio>
#include <iostream>

using namespace std;

/* Set initial scope 'name' will only be valid for the program.
 * Procedures will set their name later. 
 */
scope::scope(bool programScope){
	name = "";
	// Procedures allocate the first two bytes of their frame to pointers for stored FP and return address
	if( programScope ) totalBytes = 0;
	else totalBytes = 2;
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
		if(value.type != TYPE_PROCEDURE){
			value.FPoffset = totalBytes;
			if( value.size > 0 ) totalBytes += value.size;
			else totalBytes += 1;
		}
		
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

void scope::printScope(){
	int i;
	cout << "\n" << endl;
	for(i = 0; i < 20; i++)
		cout << "|-";
	cout << "|" << endl;

	// Show the local symbol table entries
	cout << "\nSCOPE: " << name << "\n\nLocal Symbol Table:" << endl;
	map<string, scopeValue>::iterator it;
	for(it = localTable.begin(); it != localTable.end(); it++){
		cout << "id: " << it->first;
		
		// Display the symbol's type identifier
		cout << "\ttype: ";
		switch(it->second.type){
			case TYPE_INTEGER:
				cout << "Integer";
				break;
			case TYPE_FLOAT:
				cout << "Float";
				break;
			case TYPE_CHAR:
				cout << "Char";
				break;
			case TYPE_STRING:
				cout << "String";
				break;
			case TYPE_BOOL:
				cout << "Bool";
				break;
			case TYPE_PROCEDURE:
				cout << "Procedure";
				break;
			default:
				cout << "Unknown";
				break;
		}
		
		// Display frame pointer offset for variables in the scope
		if (it->second.type != TYPE_PROCEDURE) cout << "\n\tFP offset: " << it->second.FPoffset;
		
		// Display all parameter types for procedure entries ex: Integer[5] In/Out
		if(it->second.type == TYPE_PROCEDURE){
			cout << "\n\tparameters:\n\t";
			vector<scopeValue> Vec = it->second.arguments;		
			vector<scopeValue>::size_type vec_it;
			for(vec_it = 0; vec_it != Vec.size(); vec_it++){
				if(vec_it != 0) cout << ", ";
				//cout << Vec[vec_it].type << "\t";
				switch(Vec[vec_it].type){
					case TYPE_INTEGER:
						cout << "Integer";
						break;
					case TYPE_FLOAT:
						cout << "Float";
						break;
					case TYPE_CHAR:
						cout << "Char";
						break;
					case TYPE_STRING:
						cout << "String";
						break;
					case TYPE_BOOL:
						cout << "Bool";
						break;
					case TYPE_PROCEDURE:
						cout << "Procedure";
						break;
					default:
						cout << "Unknown";
						break;
				}
				if(Vec[vec_it].size > 0) cout << "[" << Vec[vec_it].size << "]";
				switch(Vec[vec_it].paramType){
					case TYPE_PARAM_IN:
						cout << " In";
						break;
					case TYPE_PARAM_OUT:
						cout << " Out";
						break;
					case TYPE_PARAM_INOUT:
						cout << " In/Out";
						break;
					default:
						cout << " ?";
						break;
				}
			}
		cout << "\n" << endl;
		}
		else cout << "\n\tsize: " << it->second.size << "\n" << endl;
	}

	for(i = 0; i < 20; i++)
		cout << "|-";
	cout << "|" << endl;
}
