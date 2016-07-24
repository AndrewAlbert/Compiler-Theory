#include "codeGenerator.h"
#include "macro.h"
#include "scopeValue.h"
#include <stack>
#include <queue>
#include <string>
#include <cstdio>
#include <iostream>

using namespace std;

codeGenerator::codeGenerator(){
	reg_in_use = 0;
	label_count = 0;
}

codeGenerator::~codeGenerator(){
	if( testOutFile() ) footer();
	fclose( oFile );
}

bool codeGenerator::attachOutputFile(string filename){
	oFile = fopen( filename.c_str(), "w" );
	if( testOutFile() ){
		header();
		return true;
	}
	else{
		perror("Failed to open output file for writing.");
		fclose( oFile );
		return false;
	}
}

bool codeGenerator::testOutFile(){
	if( ferror(oFile) ) return false;
	else return true;
}

void codeGenerator::comment(string str){
	fprintf( oFile, "/*\n%s\n*/", str.c_str() );
	return;
}

void codeGenerator::header(){
	//Attach necessary header files for c program
	fprintf( oFile, "#include <stdio.h>\n" );
	fprintf( oFile, "#include <stdlib.h>\n" );
	fprintf( oFile, "#include <stdbool.h>\n" );
	fprintf( oFile, "#include <string.h>\n\n" );

	//Call to main for function
	fprintf( oFile, "int main( void ){\n" );

	//Declare the memory space attributes
	fprintf( oFile, "int MM_SIZE = %d;\n", MM_SIZE );
	fprintf( oFile, "int REG_SIZE = %d;\n\n", REG_SIZE );

	//Declare stack
	fprintf( oFile, "void* %s = malloc( MM_SIZE );\n", memoryIdentifier.c_str() );
	fprintf( oFile, "void* FP = %s;\n", memoryIdentifier.c_str() );
	fprintf( oFile, "void* SP = %s;\n\n", memoryIdentifier.c_str() );

	//Declare int / float registers
	fprintf( oFile, "int* %s = (int*) calloc( REG_SIZE, sizeof(int) );\n", regIdentifier.c_str() );
	fprintf( oFile, "float* f%s = (float*) calloc( REG_SIZE, sizeof(float) );\n\n", regIdentifier.c_str() );
	
}

void codeGenerator::footer(){
	fprintf( oFile, "return 0;\n}\n");
}

string codeGenerator::typeString( int type ){
	switch( type ){
		case TYPE_CHAR: 
			return "char";
		case TYPE_INTEGER:
			return "int";
		case TYPE_FLOAT:
			return"float";
		case TYPE_BOOL:
			return "int";
		case TYPE_STRING: 
			return "char*";
		default: 
			return "ERROR:";
	}
}

void codeGenerator::printStack(){

}

void codeGenerator::pushStack( string value ){
	reg_in_use++;
	exprStack.push( value );
	cout << "Push Stack: " << exprStack.top() << endl;
}

string codeGenerator::popStack(){
	string ret = exprStack.top();
	cout << "Pop Stack: " << ret << endl;
	exprStack.pop();
	return ret;
}

string codeGenerator::evalExpr( string op, bool ExprRoot ){
	cout << "Evaluate Expression " << endl;
	string lhs, rhs, result;
	rhs = popStack();
	lhs = popStack();
	reg_in_use -= 2;
	cout << "Get result register" << endl;
	result = newRegister();
	
	//If there is more to the expression, keep the register on the stack. Otherwise allow it to be overwritten
	if ( !ExprRoot ){
		pushStack( result );
		reg_in_use++;
	}
	fprintf( oFile, "%s = %s %s %s;\n", result.c_str(), lhs.c_str(), op.c_str(), rhs.c_str());
	cout << "return result" << endl;
	// Return string with the result. Only needed really if ExprRoot = true or for debugging purposes.
	return result;
}

string codeGenerator::newRegister( ){
	cout << "New Register" << endl;
	return getRegister( reg_in_use );
}

string codeGenerator::getRegister( int id ){
	cout << "Get Register" << endl;
	return ( regIdentifier + "[" + to_string( id ) + "]");
}

//
string codeGenerator::VALtoREG( string val, int type, int reg, bool push ){
	cout << "Value " << val << " to Register" << endl;
	string destination, typeMark;
	if( reg < 0 ) destination = newRegister();
	else destination = getRegister( reg );
	
	if( type == TYPE_CHAR ) 
		val = "\'" + val + "\'";
	else if( type == TYPE_STRING )
		val = "\"" + val + "\"";

	if ( push ) pushStack( destination);
	fprintf( oFile, "%s = %s;\n", destination.c_str(), val.c_str() );
	return destination;
}

string codeGenerator::ArrayMMtoREG( int type, int MMoffset, int index, int size, int reg , bool push){
	cout << "MM to Register" << endl;
	// Get size of each array entry in memory
	int entrySize;
	switch(type){
		case TYPE_CHAR:
			entrySize = sizeof(char);
			break;
		case TYPE_BOOL:
		case TYPE_INTEGER:
			entrySize = sizeof(int);
			break;
		case TYPE_CHAR:
			entrySize = sizeof(char*);
			break;
		case TYPE_FLOAT:
			entrySize = sizeof(float);
			break;
		default:
				entrySize = sizeof(void*);
				break;
	}
	string source, destination;
	if(index < 0){
		// Move each array element onto the stack
		cout << "Full Array" << endl;
		int i;
		for(i = 0; i < size; i++){
			if( reg < 0 ) destination = newRegister();
			else destination = getRegister( reg + i );
			source = "*(" + typeString( type ) + "*)( SP + " + to_string(MMoffset + entrySize * i ) +" )";
			fprintf( oFile, "%s = %s;\n", destination.c_str(), source.c_str() );
			if ( push ) pushStack( destination);
			return destination;
		}
	}
	else{
		cout << "Specific Array element" << endl;
		if( reg < 0 ) destination = newRegister();
		else destination = getRegister( reg );
		source = "*(" + typeString( type ) + "*)( SP + " + to_string(MMoffset + entrySize * index ) +" )";
		fprintf( oFile, "%s = %s;\n", destination.c_str(), source.c_str() );
		if ( push ) pushStack( destination);
		return destination;
	}
}

// Returns the string representing the destination register for stack operations.
string codeGenerator::MMtoREG( int type, int MMoffset, int reg , bool push){
	cout << "MM to Register" << endl;
	string source, destination;
	if( reg < 0 ) destination = newRegister();
	else destination = getRegister( reg );
	source = "*(" + typeString( type ) + "*)( SP + " + to_string(MMoffset) +" )";
	fprintf( oFile, "%s = %s;\n", destination.c_str(), source.c_str() );
	if ( push ) pushStack( destination);
	return destination;
}

// Returns the string representing the source register for stack operations.
string codeGenerator::REGtoMM( int type, int MMoffset, int reg, bool push ){
	cout << "Register to MM" << endl;
	string source, destination;
	source = getRegister( reg );
	destination = "*(" + typeString(type) + "*)( SP + " + to_string(MMoffset) + " )";
	fprintf( oFile, "%s = %s;\n", destination.c_str(), source.c_str() );
	if ( push ) pushStack( destination);
	return source;
}

// Get a guaranteed unique label string
string codeGenerator::newLabel( string prefix ){
	cout << "New Label" << endl;
	return ("Label_" + to_string( label_count++ ) + "_" + prefix);
}

// Put label in file
void codeGenerator::placeLabel( string label ){
	cout << "Place Label" << endl;
	fprintf( oFile, "%s:\n", label.c_str() );
	return;
}

// Set address of label to return to after procedure call
void codeGenerator::setReturnAddress( int MMoffset, string label ){
	cout << "Set Return Address" << endl;
	fprintf( oFile, "*(void*)( SP + %d ) = (void*)&&%s;\n", MMoffset, label.c_str() );
	return;
}

//Return address of instruction is always after the void* to previous frame which is placed at the frame pointer
void codeGenerator::ProcedureReturn(){
	cout << "Procedure Return" << endl;
	//fprintf( oFile, MMtoReg( TYPE_INTEGER, );
	fprintf( oFile, "FP = *(void*)(FP);\n");
	fprintf( oFile, "goto *(void*)( FP + %ld );\n", sizeof(void*));
}

//bool checkSymbol(string identifier, scopeValue &value, int &previousFrames);

void codeGenerator::createProcedureHeader(string procedureName){
	cout << "Create Procedure Header" << endl;
	// Check the symbol table and get the procedure name and

	// Get labels for all parts of the program
	string callLabel = newLabel("Procedure_" + procedureName);
	
	fprintf( oFile, "// Procedure %s\n", procedureName.c_str() );
	placeLabel(callLabel);	
}

void codeGenerator::createProcedureFooter(string procedureName){
	cout << "Create Procedure Footer" << endl;
	string endLabel = newLabel("End_Procedure");
	placeLabel(endLabel);
	ProcedureReturn();
}
		
		
void codeGenerator::callProcedure( string retLabel, scopeValue procValue ){
	cout << "Call Procedure" << endl;
	fprintf( oFile, "// Place procedure on stack\n");
}

