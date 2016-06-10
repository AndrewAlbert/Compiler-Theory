#include "scope.h"
#include "scopeValue.h"
#include "scopeTracker.h"
#include "token.h"
#include<string>
#include<iostream>
#include <map>
using namespace std;

int main(){
	vector<int> first;
	first.push_back(1);
	vector<int> second(first);
	second.push_back(2);
	vector<int> third(second);
	third.push_back(3);
	vector<int> fourth(third);
	fourth.push_back(4);
	vector<int> fifth(fourth);
	fifth.push_back(5);
	vector<int> sixth(fifth);
	sixth.push_back(6);

	scopeValue a,b,c,d,e,f;
	a.type = TYPE_INTEGER;
	a.arguments = first;

	b.type = TYPE_FLOAT;
	b.arguments = second;

	c.type = TYPE_STRING;
	c.arguments = third;

	d.type = TYPE_INTEGER;
	d.arguments = fourth;

	e.type = TYPE_FLOAT;
	e.arguments = fifth;

	f.type = TYPE_STRING;
	f.arguments = sixth;

	scopeTracker SCOPE;
	bool result;
	SCOPE.newScope();
	cout << "new scope added in scopetest.cpp" << endl;
	result = SCOPE.addSymbol("first", a, true);
	cout << result << endl;
	result = SCOPE.addSymbol("second", b, false);
	cout << result << endl;
	result = SCOPE.addSymbol("third", c, true);
	cout << result << endl;
	SCOPE.newScope();
	result = SCOPE.addSymbol("first", a, true);
	cout << result << endl;
	result = SCOPE.addSymbol("fourth", d, true);
	cout << result << endl;
	result = SCOPE.addSymbol("fifth", e, true);
	cout << result << endl;
	SCOPE.newScope();
	result = SCOPE.addSymbol("sixth", f, true);
	cout << result << endl;
	result = SCOPE.addSymbol("sixth", f, true);
	cout << result << endl;

	SCOPE.exitScope();
	SCOPE.exitScope();
	SCOPE.exitScope();
}
