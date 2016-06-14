#include "scanner.h"
#include "parser.h"
#include "macro.h"
#include "scope.h"
#include "scopeTracker.h"
#include "scopeValue.h"
#include<string>
#include<iostream>
#include <cstdio>

using namespace std;

int main(){
	string filename;
	cout << "enter filename: ";	
	getline(cin, filename);
	cout << endl;
	scopeTracker* Scopes = new scopeTracker;
	//Scopes->newFile(recordFile);
	Scanner scanner(filename);
	Parser parser(scanner.pass_ptr, Scopes);
	return 0;
}
