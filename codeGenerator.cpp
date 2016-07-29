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

	//Declare registers
	writeLine( "void* " + regIdentifier + " = (void*) calloc( REG_SIZE, sizeof(int) );" );
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

void codeGenerator::pushStack( string value, char stackID ){
	cout << "Push: " << stackID << " | " << value << endl;
	switch( stackID ){
		case 'L':
			leftStack.push( value );
			return;
		case 'E':
			exprStack.push( value );
			reg_in_use++;
			return;
		case 'R':
			rightStack.push( value );
			return;
		default:
			cout << "Not a valid expression stack!\n";
			return;
	}
}


string codeGenerator::popStack( char stackID ){
	cout << "Pop: " << stackID << endl;
	string returnStr;
	switch( stackID ){
		case 'L':
			if(leftStack.size() == 0){
				cout << "LEFT STACK EMPTY!\n";
				return "";
			}
			returnStr = leftStack.top();
			leftStack.pop();
			break;
		case 'E':
			if(exprStack.size() == 0){
				cout << "EXPR STACK EMPTY!\n";
				return "";
			}
			returnStr = exprStack.top();
			exprStack.pop();
			reg_in_use--;
			break;
		case 'R':
			if(rightStack.size() == 0){
				cout << "RIGHT STACK EMPTY!\n";
				return "";
			}
			returnStr = rightStack.top();
			rightStack.pop();
			break;
	}
	return returnStr;
}

string codeGenerator::evalExpr( string op, int size_left, int size_right, int type_left, int type_right ){
	string lhs, rhs, result;
	int i, type_result;
	if( size_left == 0 ) size_left++;
	if( size_right == 0 ) size_right++;
	comment("Evaluate expression: " + op);
	//Select int registers or float registers
	if( type_left != type_right ){
		if( ( type_left == TYPE_FLOAT ) || ( type_right == TYPE_FLOAT ) ) type_result = TYPE_FLOAT;
		else if( ( type_left == TYPE_BOOL ) || ( type_right == TYPE_BOOL ) ) type_result = TYPE_BOOL;
		else type_result = TYPE_INTEGER;
	}
	else type_result = type_left;

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
			rhs = popStack();
			pushStack(rhs, 'R');
		}
		for( i = 0; i < size_left; i++ ){
			lhs = popStack();
			pushStack(lhs, 'L');
		}
		for( i = 0; i < size_left; i++ ){
			rhs = popStack('R');
			lhs = popStack('L');
			result = newRegister();
			pushStack( result );
			writeLine( result + " = " + " " + lhs + " " + op + " " + rhs + ";");
		}
	}
	else if( size_left > 1 ){
		rhs = popStack();
		pushStack(rhs, 'R');
		for( i = 0; i < size_left; i++ ){
			lhs = popStack();
			pushStack(lhs, 'L');
		}
		rhs = popStack('R');
		for( i = 0; i < size_left; i++ ){
			lhs = popStack('L');
			result = newRegister();
			pushStack( result );
			writeLine( result + " = " + " " + lhs + " " + op + " " + rhs + ";");
		}
	}
	else if( size_right > 1 ){
		for( i = 0; i < size_right; i++ ){
			rhs = popStack();
			rightStack.push( rhs );
		}
		lhs = popStack();
		for( i = 0; i < size_right; i++ ){
			rhs = rightStack.top();
			rightStack.pop();
			result = newRegister();
			pushStack( result );
			writeLine( result + " = " + " " + lhs + " " + op + " " + rhs + ";");
		}
	}
	else{
		rhs = popStack();
		lhs = popStack();
		result = newRegister();
		pushStack( result );
		writeLine( result + " = " + " " + lhs + " " + op + " " + rhs + ";");
	}

	// Return string with the result. Only needed really if ExprRoot = true or for debugging purposes.
	return result;
}

string codeGenerator::newRegister( ){
	return getRegister( reg_in_use );
}

string codeGenerator::getRegister( int id ){
	return ( regIdentifier + "[" + to_string( id ) + "]");
}

void codeGenerator::NotOnRegister( int type, int size ){
	// Perform logical not ( boolean ) or bitwise not ( integer ) on the most recent register
	string reg, line;
	comment("Not Operation on Register");
	if( size == 0 ) size = 1;
	for( int i = 0; i < size; i++ ){
		reg = popStack();
		pushStack( reg, 'R' );
	}
	for( int i = 0; i < size; i++ ){
		reg = popStack('R');
		pushStack(reg);
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
string codeGenerator::VALtoREG( string val, int type ){
	string destination, typeMark;
	comment("Constant Value to Register");
	destination = newRegister();
	
	if( type == TYPE_CHAR ) 
		val = "\'" + val + "\'";
	else if( type == TYPE_STRING )
		val = "\"" + val + "\"";

	pushStack( destination );
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

string codeGenerator::ArrayMMtoREGIndirect( int type, int FPoffset, string index_reg, int previousFrames){
	int entrySize = typeSize( type );
	string destination, source;
	comment("Array Element Memory to Register Indirect");
	source = popStack();
	index_reg = popStack();
	destination = newRegister();

	writeLine( index_reg + " = " + index_reg + " * " + to_string(entrySize) + ";" );
	writeLine( index_reg + " = " + index_reg + " + " + to_string(FPoffset) + ";" );
	
	destination = "*(" + typeString(type) + "*)( SP + " + index_reg + " )";

	writeLine( destination + " = " + source + ";");
	pushStack( destination );
	return destination;
}

string codeGenerator::ArrayMMtoREG( int type, int FPoffset, int index, int size, int previousFrames){
	// Get size of each array entry in memory
	int entrySize = typeSize( type );
	string source, destination;
	if(index < 0){
		// Move each array element onto the stack
		comment("Full Array Memory to Register1");
		int i;
		if( size == 0 ) size++;
		for(i = 0; i < size; i++){
			destination = newRegister();
			source = "*(" + typeString( type ) + "*)( SP + " + to_string(FPoffset + entrySize * i ) +" )";
			writeLine( destination + " = " + source + ";" );
			pushStack( destination);
		}
		return destination;
	}
	else{
		comment("Array Element Memory to Register2");
		destination = newRegister();
		source = "*(" + typeString( type ) + "*)( SP + " + to_string(FPoffset + entrySize * index ) +" )";
		writeLine( destination + " = " + source + ";");
		pushStack( destination);
		return destination;
	}
}

void codeGenerator::NegateTopRegister( int type, int size ){
	string reg, cast;
	int i;

	cast = "*(" + typeString(type) + "*)";
	for( i = 0; i < size; i++ ){
		reg = popStack();
		pushStack( reg, 'L' );
		writeLine( cast + reg + " = - " + cast + reg + ";");
	}
	for( i = 0; i < size; i++ ){
		reg = popStack('L');
		writeLine( cast + reg + " = - " + cast + reg + ";");
	}
	return;
}

string codeGenerator::reg2mm(int regType, int memType, int regSize, int memSize, int FPoffset, bool isGlobal ){
	int i;
	string reg, mem, source, destination;
	comment("regSize: " + to_string(regSize) + " memSize: " + to_string(memSize));
	if( isGlobal ){
		mem = "MM";
		comment("Global Variable: Reg to MM\n");
	}
	else{
		mem = "FP";
		comment("Variable: Reg to MM");
	}

	string memCast = "*(" + typeString(memType) + "*)";
	string regCast = "*(" + typeString(regType) + "*)";
	int memBytes = typeSize( memType );
	int regBytes = typeSize( regType );
	if( regSize == 0 ) regSize++;
	if( memSize == 0 ) memSize++;
	
	for( i = 0; i < regSize; i++ ){
		reg = popStack();
		pushStack( reg, 'R' );
	}

	if( regSize > 1 && regSize == memSize ){
		cout << "regsize: " << regSize << " memSize: " << memSize << endl;
		for( i = 0; i < memSize; i++ ){
			reg = popStack('R');
			source = regCast + reg;
			destination = memCast + "( " + mem + to_string(FPoffset + i * memBytes ) + " )";
		  writeLine( destination + " = " + source + ";");
		}
	}
	else if ( regSize == 1 ){
		cout << "regsize: " << regSize << " memSize: " << memSize << endl;
		source = popStack('R');
		source = regCast + reg;
		for( i = 0; i < memSize; i++ ){
		  destination = memCast + "( " + mem + to_string(FPoffset + i * memBytes ) + " )";
		  writeLine( destination + " = " + source + ";");
		}
	}
	return "";
}

// Returns the string representing the destination register for stack operations.
string codeGenerator::MMtoREG( int type, int FPoffset, int prevFrames){
	if( prevFrames != 0 ) comment("global variable");
	string source, destination;
	comment("Memory to Register");
	destination = newRegister();
	source = "*(" + typeString( type ) + "*)( SP + " + to_string(FPoffset) +" )";
	writeLine( destination + " = " + source + ";");
	pushStack( destination);
	return destination;
}

// Get a guaranteed unique label string
string codeGenerator::newLabel( string prefix ){
	return ("Label_" + to_string( label_count++ ) + "_" + prefix);
}

// Put label in file
void codeGenerator::placeLabel( string label ){
	tabDec();
	writeLine( "\n" + label + ":" );
	tabInc();
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
	
void codeGenerator::callProcedure( string retLabel, scopeValue procValue ){
	comment("Call procedure");
	writeLine( "goto *(void*)( FP + " + to_string(sizeof(void*)) + ");" );
}

void codeGenerator::pushParameter( ){
	
	return;
}

void codeGenerator::popParameter( ){

	return;
}

// Set address of label to return to after procedure call
void codeGenerator::setReturnAddress( int FPoffset, string label ){
	string source, destination;
	comment("set return address for procedure call");
	source = "*(void*)( SP + " + to_string(FPoffset) + " )";
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

void codeGenerator::condBranch( string labelTrue, string labelFalse ){
	string cond;
	cond = popStack();
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
