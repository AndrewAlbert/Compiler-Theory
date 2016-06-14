#include "scope.h"
#include <string>
#include <map>
#include <vector>
#include <cstdio>

using namespace std;

scope::scope(){
	cout << "in scope()" << endl;
}

scope::~scope(){

}

bool scope::addSymbol(string identifier, bool global, scopeValue value){
	map<string, scopeValue>::iterator it;
	it = localTable.find(identifier);
	if(it != localTable.end()) return false;
	else{
		if(global) globalTable[identifier] = value;
		localTable[identifier] = value;
	}
}

bool scope::checkSymbol(string identifier, bool global){
	map<string, scopeValue>::iterator it;
	if(global){
		//cout << "global true" << endl;
		it = globalTable.find(identifier);
		if(it != globalTable.end()) return true;
		else return false;
	}
	else{
		//cout << "global false" << endl;
		it = localTable.find(identifier);
		if(it != localTable.end()) return true;
		else return false;
	}
}

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
	cout << "SCOPE:\n\nLocal Table:\n";
	map<string, scopeValue>::iterator it;
	for(it = localTable.begin(); it != localTable.end(); it++){
		cout << "id: " << it->first;
		cout << "\ttype: " << it->second.type << endl;
		cout << "\tparameters:\t";
		vector<int> Vec = it->second.arguments;		
		vector<int>::size_type vec_it;
		for(vec_it = 0; vec_it != Vec.size(); vec_it++){
			cout << Vec[vec_it] << "\t";
		}
		cout << endl;
	}
}
