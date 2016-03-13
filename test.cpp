#include "scanner.h"
#include "parser.h"
#include<string>
#include<iostream>
using namespace std;

int main(){
	string filename;
	cout << "enter filename: ";	
	getline(cin, filename);
	cout << endl;
	//filename = "test.txt";
	Scanner scanner(filename);
	Parser parser(scanner.pass_ptr);
	return 0;
}
