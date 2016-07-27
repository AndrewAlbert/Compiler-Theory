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
	ContinueToGenerate = true;
}

codeGenerator::~codeGenerator(){
	if( testOutFile() ) footer();
	fclose( oFile );
}

// Stops code generation from occuring if an error occurs in parsing
void codeGenerator::stopCodeGeneration(){
	ContinueToGenerate = false;
	return;
}

bool codeGenerator::ShouldGenerate(){
	if( ContinueToGenerate) return true;
	else return false;
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

void codeGenerator::comment(string str, int line){
	if( line == 1 )	writeLine( "// " + str );
	else writeLine( "/*\n" + str + "\n*/" );
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
	writeLine( "goto *(&&Label_0_Begin_Program);" );
	
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
	if( !ShouldGenerate() ) return;
	else{
		for( int i = tabs; i > 0; i-- ){
			fprintf( oFile, "   " );	
			cout << "   ";
		}
		cout << line << endl;
		fprintf( oFile, "%s\n", line.c_str() );
		return;
	}
}

void codeGenerator::pushStack( string value, int stackID ){
	size_t found;
	found = value.find(fregIdentifier);
	switch( stackID ){
		case -1:
			leftStack.push( value );
			break;
		case 0:
			exprStack.push( value );
			if( found != string::npos ) freg_in_use++;
			else reg_in_use++;
			break;
		case 1:
			rightStack.push( value );
			break;
	}
	return;
}


string codeGenerator::popStack( int stackID ){
	size_t found;
	string ret;
	
	switch( stackID ){
		case -1:
			if(leftStack.size() == 0){
				cout << "LEFT STACK EMPTY!\n";
				//break;
			}
			ret = leftStack.top();
			leftStack.pop();
			break;
		case 0:
			if(exprStack.size() == 0){
				cout << "EXPR STACK EMPTY!\n";
				//break;
			}
			ret = exprStack.top();
			exprStack.pop();
			found = ret.find(fregIdentifier);
			if( found != string::npos ) freg_in_use--;
			else reg_in_use--;
			break;
		case 1:
			if(rightStack.size() == 0){
				cout << "RIGHT STACK EMPTY!\n";
				//break;
			}
			ret = rightStack.top();
			rightStack.pop();
			break;
	}
	return ret;
}

void codeGenerator::condBranch( string labelTrue, string labelFalse ){
	string cond = popStack();
	comment("Conditional branch");
	// goto true condition
	writeLine("if( " + cond + " ) goto *( &&" + labelTrue + " );");
	// goto else condition
	if( labelFalse.compare("") != 0 ) writeLine( "goto *( &&" + labelFalse + " );");
	return;
}

void codeGenerator::branch( string label ){
	comment("Unconditional branch");
	writeLine( "goto *( &&"+ label + " );");
	return;
}

string codeGenerator::evalExpr( string op, int size_left, int size_right, int type_left, int type_right ){
	string lhs, rhs, result;
	int i, type_result;
	if( size_left == 0 ) size_left++;
	if( size_right == 0 ) size_right++;
	comment("Evaluate expression: " + op);
	//Select int registers or float registers
	if( type_left != type_right ) type_result = TYPE_FLOAT;
	else type_result = TYPE_INTEGER;

	if(op.compare("&") == 0){ 
		if( (type_left == TYPE_BOOL) || (type_right == TYPE_BOOL) ) op = "&&";
		else op = "&";
	}
	else if(op.compare("|") == 0){ 
		if( (type_left == TYPE_BOOL) || (type_right == TYPE_BOOL) ) op = "||";
		else op = "|";
	}
	else if(op.compare("NOT") == 0){
		if( (type_left == TYPE_BOOL) || (type_right == TYPE_BOOL) ) op = "!";
		else op = "~";
	}
	if( ( size_left > 1 ) && ( size_right > 1) ){
		for( i = 0; i < size_right; i++ ){
			rhs = popStack(0);
			pushStack(rhs, 1);
		}
		for( i = 0; i < size_left; i++ ){
			lhs = popStack(0);
			pushStack(lhs, -1);
		}
		for( i = 0; i < size_left; i++ ){
			rhs = popStack(1);
			lhs = popStack(-1);
			result = newRegister(type_result);
			pushStack( result );
			writeLine( result + " = " + " " + lhs + " " + op + " " + rhs + ";");
		}
	}
	else if( size_left > 1 ){
		rhs = popStack(0);
		pushStack(rhs, 1);
		for( i = 0; i < size_left; i++ ){
			lhs = popStack(0);
			pushStack(lhs, -1);
		}
		for( i = 0; i < size_left; i++ ){
			rhs = rightStack.top();
			lhs = popStack(-1);
			result = newRegister(type_result);
			pushStack( result );
			writeLine( result + " = " + " " + lhs + " " + op + " " + rhs + ";");
		}
		rightStack.pop();
	}
	else if( size_right > 1 ){
		for( i = 0; i < size_right; i++ ){
			rhs = popStack(0);
			rightStack.push( rhs );
		}
		lhs = popStack(0);
		leftStack.push( lhs );
		for( i = 0; i < size_right; i++ ){
			rhs = rightStack.top();
			rightStack.pop();
			lhs = leftStack.top();
			result = newRegister(type_result);
			pushStack( result );
			writeLine( result + " = " + " " + lhs + " " + op + " " + rhs + ";");
		}
		leftStack.pop();
	}
	else{
		rhs = popStack(0);
		lhs = popStack(0);
		result = newRegister(type_result);
		pushStack( result );
		writeLine( result + " = " + " " + lhs + " " + op + " " + rhs + ";");
	}

	// Return string with the result. Only needed really if ExprRoot = true or for debugging purposes.
	return result;
}

string codeGenerator::newRegister( int type ){
	if( type == TYPE_FLOAT) return getRegister( freg_in_use, type );
	return getRegister( reg_in_use, type );
}

string codeGenerator::getRegister( int id, int type ){
	if( type == TYPE_FLOAT )
		return ( fregIdentifier + "[" + to_string( id ) + "]");
	else
		return ( regIdentifier + "[" + to_string( id ) + "]");
}

void codeGenerator::NotOnRegister( int type, int size ){
	// Perform logical not ( boolean ) or bitwise not ( integer ) on the most recent register
	string reg, line;
	comment("Not Operation on Register");
	if( size == 0 ) size = 1;
	for( int i = 0; i < size; i++ ){
		reg = exprStack.top();
		rightStack.push( reg );
		exprStack.pop();
	}
	for( int i = 0; i < size; i++ ){
		reg = rightStack.top();
		exprStack.push( reg );
		rightStack.pop();
		line = "";
		if( type == TYPE_BOOL ){
			line = reg + " = !( " + reg + " );";
		}
		else if( type == TYPE_INTEGER ){
			line = reg + " = ~( " + reg + " );";
		}
		writeLine( line );
	}
	return;
}

//
string codeGenerator::VALtoREG( string val, int type, int prevFrames, int reg, bool push ){
	string destination, typeMark;
	if( prevFrames != 0 ) comment("global variable");
	comment("Constant Value to Register");
	if( reg < 0 ) destination = newRegister(type);
	else destination = getRegister( reg, type );
	
	if( type == TYPE_CHAR ) 
		val = "\'" + val + "\'";
	else if( type == TYPE_STRING )
		val = "\"" + val + "\"";

	if ( push ) pushStack( destination);
	writeLine( destination + " = " + val + ";");
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

int codeGenerator::typeSize( int type ){
	switch(type){
		case TYPE_CHAR:
			return sizeof(char);
		case TYPE_BOOL:
		case TYPE_INTEGER:
			return sizeof(int);
		case TYPE_STRING:
			return sizeof(char*);
		case TYPE_FLOAT:
			return sizeof(float);
		default:
			return sizeof(void*);
	}
}

string codeGenerator::ArrayMMtoREGIndirect( int type, int MMoffset, string index_reg, int previousFrames, int reg, bool push){
	int entrySize = typeSize( type );
	string destination, source;
	comment("Array Element Memory to Register Indirect");
	index_reg = popStack(0);
	if( reg < 0 ) destination = newRegister(type);
	else destination = getRegister( reg, type );

	writeLine( index_reg + " = " + index_reg + " * " + to_string(entrySize) + ";" );
	writeLine( index_reg + " = " + index_reg + " + " + to_string(MMoffset) + ";" );
	
	destination = "*(" + typeString(type) + "*)( SP + " + index_reg + " )";

	writeLine( destination + " = " + source + ";");

	/*source = "*(" + typeString( type ) + "*)( SP + " + to_string(MMoffset) + " + " + to_string(entrySize) + " * " + index_reg + " ) " + " )";
	writeLine( destination + " = " + source + ";");*/
	if ( push ) pushStack( destination);
	return destination;
}

string codeGenerator::ArrayMMtoREG( int type, int MMoffset, int index, int size, int reg , bool push){
	// Get size of each array entry in memory
	int entrySize = typeSize( type );
	string source, destination;
	if(index < 0){
		// Move each array element onto the stack
		comment("Full Array Memory to Register");
		int i;
		if( size == 0 ) size++;
		for(i = 0; i < size; i++){
			if( reg < 0 ) destination = newRegister(type);
			else destination = getRegister( reg + i, type );
			source = "*(" + typeString( type ) + "*)( SP + " + to_string(MMoffset + entrySize * i ) +" )";
			writeLine( destination + " = " + source + ";" );
			if ( push ) pushStack( destination);
		}
		return destination;
	}
	else{
		comment("Array Element Memory to Register");
		if( reg < 0 ) destination = newRegister(type);
		else destination = getRegister( reg, type );
		source = "*(" + typeString( type ) + "*)( SP + " + to_string(MMoffset + entrySize * index ) +" )";
		writeLine( destination + " = " + source + ";");
		if ( push ) pushStack( destination);
		return destination;
	}
}

// Returns the string representing the destination register for stack operations.
string codeGenerator::MMtoREG( int type, int MMoffset, int prevFrames, int reg , bool push){
	if( prevFrames != 0 ) comment("global variable");
	string source, destination;
	comment("Memory to Register");
	if( reg < 0 ) destination = newRegister(type);
	else destination = getRegister( reg, type );
	source = "*(" + typeString( type ) + "*)( SP + " + to_string(MMoffset) +" )";
	writeLine( destination + " = " + source + ";");
	if ( push ) pushStack( destination);
	return destination;
}

// Returns the string representing the source register for stack operations.
string codeGenerator::REGtoMM( int type, int MMoffset, int size, int prevFrames, int reg, bool push ){
	if( prevFrames != 0 ) comment("global variable");
	string source, destination;
	int i, entrySize;
	entrySize = typeSize( type );
	comment("Register to Memory");
	if( size == 0 ) size++;
	for( i = 0; i < size; i++ ){
		if(reg < 0 ) source = popStack();
		else source = getRegister( reg, type );
		rightStack.push( source );
	}
	for( i = 0; i < size; i++ ){
		source = rightStack.top();
		rightStack.pop();
		destination = "*(" + typeString(type) + "*)( SP + " + to_string(MMoffset + i * entrySize ) + " )";
		writeLine( destination + " = " + source + ";");
	}
	return source;
}

string codeGenerator::REGtoMMIndirect(int type, int MMoffset, string index_reg, int previousFrames, int reg, bool push){
	if( previousFrames != 0 ) comment("global variable");
	string source, destination;
	int i, entrySize;
	entrySize = typeSize( type );
	comment("Register to Memory Indirect");
	source = popStack(0);
	index_reg = popStack(0);

	writeLine( index_reg + " = " + index_reg + " * " + to_string(entrySize) + ";" );
	writeLine( index_reg + " = " + index_reg + " + " + to_string(MMoffset) + ";" );
	
	destination = "*(" + typeString(type) + "*)( SP + " + index_reg + " )";

	writeLine( destination + " = " + source + ";");
		
	return source;
}

// Get a guaranteed unique label string
string codeGenerator::newLabel( string prefix ){
	return ("Label_" + to_string( label_count++ ) + "_" + prefix);
}

// Put label in file
void codeGenerator::placeLabel( string label ){
	writeLine( "\n" + label + ":" );
	return;
}

// Set address of label to return to after procedure call
void codeGenerator::setReturnAddress( int MMoffset, string label ){
	string source, destination;
	comment("set return address for procedure call");
	source = "*(void*)( SP + " + to_string(MMoffset) + " )";
	destination = "(void*)&&" + label;
	writeLine( source + " = " + destination );
	return;
}

//Return address of instruction is always after the void* to previous frame which is placed at the frame pointer
void codeGenerator::ProcedureReturn(){
	comment("procedure return");
	//fprintf( oFile, MMtoREG( TYPE_INTEGER, );
	writeLine( "(void*)FP = *(void*)(FP);" );
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
	comment("Call procedure");
	writeLine( "goto *(void*)( FP + " + to_string(sizeof(void*)) + ");" );
}
