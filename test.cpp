#include "scanner.h"
#include "parser.h"
#include<string>
#include<iostream>
using namespace std;

int main(){
	string filename = "test.txt";
	Scanner scanner(filename);
	Parser parser(scanner.pass_ptr);
	return 0;
}
