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
	freg_in_use = 0;
	label_count = 0;
	tabs = 0;
	HeapSize = 0;
}

codeGenerator::~codeGenerator(){
	if( testOutFile() ) footer();
	fclose( oFile );
}

bool codeGenerator::attachOutputFile(string filename){
	oFile = fopen( filename.c_str(), "w" );
	if( testOutFile() ){
		cout << "File opened: " << filename << ".c" << endl;
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
	writeLine( "/*\n" + str + "\n*/" );
	return;
}

void codeGenerator::header(){
	//Attach necessary header files for c program
	writeLine( "#include <stdio.h>" );
	writeLine( "#include <stdlib.h>" );
	writeLine( "#include <stdbool.h>" );
	writeLine( "#include <string.h>\n" );

	comment( "Begin program execution" );
	//Call to main for function
	writeLine( "int main( void ){" );

	tabInc();

	//Declare the memory space attributes
	writeLine( "int MM_SIZE = " + to_string(MM_SIZE) + ";" );
	writeLine( "int REG_SIZE = " + to_string(REG_SIZE) + ";\n" );

	//Declare stack
	writeLine( "void* " + memoryIdentifier + " = malloc( MM_SIZE );" );
	writeLine( "void* FP = " + memoryIdentifier + ";" );
	writeLine( "void* SP = " + memoryIdentifier + ";" );

	//Declare int / float registers
	writeLine( "int* " + regIdentifier + " = (int*) calloc( REG_SIZE, sizeof(int) );" );
	writeLine( "float* " + fregIdentifier + " = (float*) calloc( REG_SIZE, sizeof(float) );\n" );
	
}

void codeGenerator::footer(){
	writeLine( "return 0;");
	tabDec();
	writeLine("}");
}

void codeGenerator::tabInc(){
	tabs++;
	return;
}

void codeGenerator::tabDec(){
	if( tabs > 0 )tabs--;
	return;
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
	
	return;
}

void codeGenerator::writeLine( string line ){
	for( int i = tabs; i > 0; i-- ){
		fprintf( oFile, "   " );
	}
	fprintf( oFile, "%s\n", line.c_str() );
}

void codeGenerator::pushStack( string value ){
	reg_in_use++;
	exprStack.push( value );
	//cout << "Push Stack: " << exprStack.top() << endl;
}

string codeGenerator::popStack(){
	reg_in_use--;
	string ret = exprStack.top();
	//cout << "Pop Stack: " << ret << endl;
	exprStack.pop();
	return ret;
}

void codeGenerator::condBranch( string labelTrue, string labelFalse ){
	string cond = popStack();
	// goto true condition
	writeLine("if( " + cond + " ) goto *( &&" + labelTrue + " );");
	// goto else condition
	if( labelFalse.compare("") != 0 ) writeLine( "goto *( &&" + labelFalse + " );");
	cout << "Cond Branch:\n\tif(" << cond << ")goto " << labelTrue << " else goto " << labelFalse << endl;
	return;
}

void codeGenerator::branch( string label ){
	cout << "branch:\n\tgoto " << label << endl;
	writeLine( "goto *( &&"+ label + " );");
	return;
}

string codeGenerator::evalExpr( string op, bool ExprRoot ){
	string lhs, rhs, result;
	rhs = popStack();
	lhs = popStack();
	result = newRegister();
	
	//If there is more to the expression, keep the register on the stack. Otherwise allow it to be overwritten
	if ( !ExprRoot ){
		pushStack( result );
	}
	writeLine( result + " = " + " " + lhs + " " + op + " " + rhs + ";");
	// Return string with the result. Only needed really if ExprRoot = true or for debugging purposes.
	cout << "Evaluate Expression " << op << "\n\t" << result << " = " << lhs << " " << op << " " << rhs << endl;
	return result;
}

string codeGenerator::newRegister( ){
	return getRegister( reg_in_use );
}

string codeGenerator::getRegister( int id ){
	return ( regIdentifier + "[" + to_string( id ) + "]");
}

//
string codeGenerator::VALtoREG( string val, int type, int reg, bool push ){
	string destination, typeMark;
	if( reg < 0 ) destination = newRegister();
	else destination = getRegister( reg );
	
	if( type == TYPE_CHAR ) 
		val = "\'" + val + "\'";
	else if( type == TYPE_STRING )
		val = "\"" + val + "\"";

	if ( push ) pushStack( destination);
	writeLine( destination + " = " + val + ";");
	cout << "Val --> Reg\n\t" << destination << " = " << val << endl;
	return destination;
}

char* codeGenerator::AddStringHeap( string str ){
	HeapSize += ( str.size() + 1);
	int i;
	char ch;
	for( i = 0; i < str.size(); i++ ){
		ch = str[i];
		
	}
	return nullptr;
}

string codeGenerator::ArrayMMtoREG( int type, int MMoffset, int index, int size, int reg , bool push){
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
		case TYPE_STRING:
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
		cout << "Full Array to Reg" << endl;
		int i;
		for(i = 0; i < size; i++){
			if( reg < 0 ) destination = newRegister();
			else destination = getRegister( reg + i );
			source = "*(" + typeString( type ) + "*)( SP + " + to_string(MMoffset + entrySize * i ) +" )";
			writeLine( destination + " = " + source + ";" );
			if ( push ) pushStack( destination);
			cout << "\t" << destination << " = " << source << endl; 
			return destination;
		}
	}
	else{
		cout << "Specific Array element" << endl;
		if( reg < 0 ) destination = newRegister();
		else destination = getRegister( reg );
		source = "*(" + typeString( type ) + "*)( SP + " + to_string(MMoffset + entrySize * index ) +" )";
		writeLine( destination + " = " + source + ";");
		if ( push ) pushStack( destination);
		cout << "\t" << destination << " = " << source << endl; 
		return destination;
	}
}

// Returns the string representing the destination register for stack operations.
string codeGenerator::MMtoREG( int type, int MMoffset, int reg , bool push){
	string source, destination;
	if( reg < 0 ) destination = newRegister();
	else destination = getRegister( reg );
	source = "*(" + typeString( type ) + "*)( SP + " + to_string(MMoffset) +" )";
	writeLine( destination + " = " + source + ";");
	if ( push ) pushStack( destination);
	cout << "MM --> Reg\n\t" << destination << " = " << source << endl;
	return destination;
}

// Returns the string representing the source register for stack operations.
string codeGenerator::REGtoMM( int type, int MMoffset, int reg, bool push ){
	string source, destination;
	if(reg < 0 ) source = popStack();
	else source = getRegister( reg );
	destination = "*(" + typeString(type) + "*)( SP + " + to_string(MMoffset) + " )";
	writeLine( destination + " = " + source + ";");
	//if ( push ) pushStack( destination);
	cout << "Reg --> MM\n\t" << destination << " = " << source << endl;
	return source;
}

// Get a guaranteed unique label string
string codeGenerator::newLabel( string prefix ){
	return ("Label_" + to_string( label_count++ ) + "_" + prefix);
}

// Put label in file
void codeGenerator::placeLabel( string label ){
	cout << "Place Label: " << label << endl;
	writeLine( "\n" + label + ":" );
	return;
}

// Set address of label to return to after procedure call
void codeGenerator::setReturnAddress( int MMoffset, string label ){
	string source, destination;
	source = "*(void*)( SP + " + to_string(MMoffset) + " )";
	destination = "(void*)&&" + label;
	cout << "Set Return Address" << endl;
	writeLine( source + " = " + destination );
	return;
}

//Return address of instruction is always after the void* to previous frame which is placed at the frame pointer
void codeGenerator::ProcedureReturn(){
	cout << "Procedure Return" << endl;
	//fprintf( oFile, MMtoREG( TYPE_INTEGER, );
	writeLine( "FP = *(void*)(FP);" );
	writeLine( "goto *(void*)( FP + " + to_string(sizeof(void*)) + ");" );
	return;
}

//bool checkSymbol(string identifier, scopeValue &value, int &previousFrames);

void codeGenerator::createProcedureHeader(string procedureName){
	// Check the symbol table and get the procedure name and

	// Get labels for all parts of the program
	string callLabel = newLabel("Procedure_" + procedureName);
	
	tabDec();
	placeLabel(callLabel);	
	tabInc();	

	return;
}

void codeGenerator::createProcedureFooter(string procedureName){
	string endLabel = newLabel("End_Procedure");

	tabDec();
	placeLabel(endLabel);
	tabInc();

	ProcedureReturn();
	return;
}
		
void codeGenerator::pushParameter( ){
	
	return;
}

void codeGenerator::popParameter( ){

	return;
}

void codeGenerator::callProcedure( string retLabel, scopeValue procValue ){
	cout << "Call Procedure" << endl;
	writeLine( "goto *(void*)( FP + " + to_string(sizeof(void*)) + ");" );
}
