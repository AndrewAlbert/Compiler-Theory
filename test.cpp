#include "scanner.h"
#include<string>
#include<iostream>
using namespace std;

int main(){
	cout << "started" << endl;
	string filename = "test.txt";
	cout << "named file" << endl;
	Scanner test(filename);
	cout << "/nfinished" << endl;
	return 0;
}
