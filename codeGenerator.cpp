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
	ireg_in_use = 0;
	freg_in_use = 0;
	label_count = 0;
	tabs = 0;
	HeapSize = 0;
	ContinueToGenerate = true;
	HeapSize = MM_SIZE - 1;
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
		ContinueToGenerate = false;
		return false;
	}
}

bool codeGenerator::testOutFile(){
	if( ferror(oFile) ) return false;
	else return true;
}

void codeGenerator::comment(string str, bool multi_line){
	if( multi_line ) writeLine( "/*\n" + str + "\n*/" );
	else writeLine( "// " + str );
	return;
}

void codeGenerator::header(){
	//Attach necessary header files for c program
	writeLine( "#include <stdio.h>" );
	writeLine( "#include <stdlib.h>" );
	writeLine( "#include <string.h>\n" );

	//Define the memory space attributes
	writeLine( "#define MM_SIZE " + to_string(MM_SIZE));
	writeLine( "#define REG_SIZE " + to_string(REG_SIZE));
	
	//Create registers and stack
	writeLine("\nint MM[MM_SIZE];");
	writeLine("int iReg[REG_SIZE];");
	writeLine("float fReg[REG_SIZE];");
	writeLine("int FP_reg;");
	writeLine("int SP_reg;");
	writeLine("int TP_reg;");
	writeLine("int HP_reg;");

	comment( "Begin program execution" );
	//Call to main for function
	writeLine( "int main( void ){" );

	tabInc();
	
	//Declare registers
	comment("Allocated registers for stack operations");
	writeLine( FP_REG + " = 0;" );
	writeLine( SP_REG + " = 0;" );
	writeLine( TP_REG + " = 0;" );
	writeLine( HP_REG + " = MM_SIZE - 1;" );
	writeLine( "goto Label_0_Begin_Program;" );
	
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

void codeGenerator::procedureCall( scopeValue calledProcedure, int frameSize, string returnLabel ){
	string source, destination;
	int retAddr = calledProcedure.retAddressOffset;
	int prevFP = calledProcedure.prevFrameOffset;
	
	destination = "( " + memoryIdentifier + " + " + FP_REG + " )";
	// Set previous FP onto the stack, return address should already be placed by procedure call
	
	writeLine( FP_REG + " = " + TP_REG + ";" );	
	// set new edge of frame as SP
	writeLine( SP_REG + " = " + FP_REG + " + " + to_string(frameSize) + ";" );
	
}

void codeGenerator::writeLine( string line ){
	if( !ShouldGenerate() ) return;
	else{
		for( int i = tabs; i > 0; i-- ){
			fprintf( oFile, "   " );	
		}
		fprintf( oFile, "%s\n", line.c_str() );
		return;
	}
}

void codeGenerator::pushStack( string value, char stackID, int type ){
	switch( stackID ){
		case 'L':
			leftStack.push( value );
			return;
		case 'E':
			exprStack.push( value );
			if( type == TYPE_FLOAT ) freg_in_use++;
			else ireg_in_use++;
			return;
		case 'R':
			rightStack.push( value );
			return;
		default:
			cout << "Not a valid expression stack!\n";
			return;
	}
}

string codeGenerator::popStack( char stackID, int type ){
	string returnStr;
	switch( stackID ){
		case 'L':
			if(leftStack.size() == 0){
				cout << "LEFT STACK EMPTY!\n";
				return "";
			}
			returnStr = leftStack.top();
			leftStack.pop();
			return returnStr;
		case 'E':
			if(exprStack.size() == 0){
				cout << "EXPR STACK EMPTY!\n";
				return "";
			}
			returnStr = exprStack.top();
			exprStack.pop();
			if( type == TYPE_FLOAT) freg_in_use--;
			else ireg_in_use--;
			return returnStr;
		case 'R':
			if(rightStack.size() == 0){
				cout << "RIGHT STACK EMPTY!\n";
				return "";
			}
			returnStr = rightStack.top();
			rightStack.pop();
			return returnStr;
		default:
			cout << "INVALID STACK ID!\n";
			return "invalid stack id";
	}
}

string codeGenerator::evalExpr( string op, int size_left, int size_right, int type_left, int type_right ){
	string lhs, rhs, result;
	int i, type_result;
	if( size_left == 0 ) size_left++;
	if( size_right == 0 ) size_right++;
	
	// Invalidate OUT/INOUT arguments since an expresion such as <Name> * 2 has occurred and there is no return value now
	if( checkArguments() ) invalidateArgument();
	
	comment("Evaluate expression: " + op);
	
	//Select int registers or float registers
	if( type_left != type_right ){
		if( ( type_left == TYPE_FLOAT ) || ( type_right == TYPE_FLOAT ) ) 
			type_result = TYPE_FLOAT;
		else if( ( type_left == TYPE_BOOL ) || ( type_right == TYPE_BOOL ) ) 
			type_result = TYPE_BOOL;
		else type_result = TYPE_INTEGER;
	}
	else type_result = type_left;

	if(op.compare("&") == 0){ 
		if( (type_left == TYPE_BOOL) || (type_right == TYPE_BOOL) ) 
			op = "&&";
		else op = "&";
	}
	else if(op.compare("|") == 0){ 
		if( (type_left == TYPE_BOOL) || (type_right == TYPE_BOOL) ) 
			op = "||";
		else op = "|";
	}
	else if(op.compare("NOT") == 0){
		if( (type_left == TYPE_BOOL) || (type_right == TYPE_BOOL) ) 
			op = "!";
		else op = "~";
	}

	if( ( size_left > 1 ) && ( size_right > 1) ){
		for( i = 0; i < size_right; i++ ){
			rhs = popStack('E', type_right);
			pushStack(rhs, 'R', type_right);
		}
		for( i = 0; i < size_left; i++ ){
			lhs = popStack('E', type_left);
			pushStack(lhs, 'L', type_left);
		}
		for( i = 0; i < size_left; i++ ){
			rhs = popStack('R', type_right);
			lhs = popStack('L', type_left);
			result = newRegister(type_result);
			pushStack( result, 'E', type_result );
			writeLine( result + " = " + " " + lhs + " " + op + " " + rhs + ";");
			writeLine("printf(\"result: %d\\n\", " + result + ");");
		}
	}
	else if( size_left > 1 ){
		rhs = popStack('E', type_right);
		pushStack(rhs, 'R', type_right);
		for( i = 0; i < size_left; i++ ){
			lhs = popStack('E', type_left);
			pushStack(lhs, 'L', type_left);
		}
		rhs = popStack('R', type_right);
		for( i = 0; i < size_left; i++ ){
			lhs = popStack('L', type_left);
			result = newRegister(type_result);
			pushStack( result, 'E', type_result );
			writeLine( result + " = " + " " + lhs + " " + op + " " + rhs + ";");
			writeLine("printf(\"result: %d\\n\", " + result + ");");
		}
	}
	else if( size_right > 1 ){
		for( i = 0; i < size_right; i++ ){
			rhs = popStack('R', type_right);
			pushStack(rhs, 'R', type_right);
		}
		lhs = popStack('E', type_left);
		for( i = 0; i < size_right; i++ ){
			rhs = popStack('R', type_right);
			result = newRegister(type_result);
			pushStack( result, 'E', type_result );
			writeLine( result + " = " + " " + lhs + " " + op + " " + rhs + ";");
			writeLine("printf(\"result: %d\\n\", " + result + ");");
		}
	}
	else{
		rhs = popStack('E', type_right);
		lhs = popStack('E', type_left);
		result = newRegister(type_result);
		pushStack( result, 'E', type_result );
		writeLine( result + " = " + " " + lhs + " " + op + " " + rhs + ";");
		writeLine("printf(\"result: %d\\n\", " + result + ");");
	}

	return result;
}

string codeGenerator::newRegister( int type ){
	if( type == TYPE_FLOAT ) return getRegister( type, freg_in_use);
	return getRegister( type, ireg_in_use );
}

string codeGenerator::getRegister( int type, int id ){
	if (type == TYPE_FLOAT) return ( fRegId + "[" + to_string(id) + "]");
	return ( iRegId + "[" + to_string( id ) + "]");
}

void codeGenerator::NotOnRegister( int type, int size ){
	// Perform logical not ( boolean ) or bitwise not ( integer ) on the most recent register
	string reg, line;
	comment("Not Operation on Register");
	if( size == 0 ) size = 1;
	for( int i = 0; i < size; i++ ){
		reg = popStack('E', type);
		pushStack( reg, 'R', type );
	}
	for( int i = 0; i < size; i++ ){
		reg = popStack('R', type);
		pushStack(reg, 'E', type);
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

string codeGenerator::VALtoREG( string val, int type ){
	string destination;
	comment("Constant Value to Register");
	destination = newRegister( type );
	
	if( type == TYPE_CHAR ) 
		val = "\'" + val + "\'";
	else if( type == TYPE_STRING )
		val = "\"" + val + "\"";

	pushStack( destination, 'E', type );
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

void codeGenerator::NegateTopRegister( int type, int size ){
	string reg;
	int i;

	for( i = 0; i < size; i++ ){
		reg = popStack('E', type);
		pushStack( reg, 'L', type );
		writeLine( reg + " = -" + reg + ";");
	}
	for( i = 0; i < size; i++ ){
		reg = popStack('L', type);
		writeLine( reg + " = -" + reg + ";");
	}
	return;
}

string codeGenerator::mm2reg(int memType, int memSize, int FPoffset, bool isGlobal, int index, bool indirect, int indirect_type){
	string reg, mem;
	if( isGlobal ) comment("Global Variable: MM to Reg");
	else comment("Variable: MM to Reg");

	// Indirect off of register (top)
	if( indirect ) string indirect_reg = popStack('E', indirect_type); 

	if( memSize == 0 ) memSize++;
	
	if( index < 0){
		for( index = 0; index < memSize; index++ ){
			reg = newRegister(memType);
			pushStack(reg, 'E', memType);
			
			if( isGlobal ) mem = "MM[" + to_string(FPoffset + index) + "]";
			else mem = "MM[FP_reg + " + to_string(FPoffset + index) + "]";
			
			if( memType == TYPE_FLOAT) writeLine("memcpy( &" + reg + " , &" + mem + ", sizeof(int) );");
			else writeLine(reg + " = " + mem + ";");
			writeLine("printf(\"result: %d\\n\", " + reg + ");");
		}
	}
	else{
		reg = newRegister(memType);
		pushStack(reg, 'E', memType);
		
		if( isGlobal ) mem = "MM[" + to_string(FPoffset + index) + "]";
		else mem = "MM[FP_reg + " + to_string(FPoffset + index) + "]";
		
		if( memType == TYPE_FLOAT) writeLine("memcpy( &" + reg + " , &" + mem + ", sizeof(int) );");
		else writeLine(reg + " = " + mem + ";");
		writeLine("printf(\"result: %d\\n\", " + reg + ");");
	}
	
	return "";
}

string codeGenerator::reg2mm(int regType, int memType, int regSize, int memSize, int FPoffset, bool isGlobal, bool indirect, int indirect_type ){
	int index;
	string reg, mem, indirect_reg;
	
	if( regSize == 0 ) regSize++;
	if( memSize == 0 ) memSize++;
	
	for( index = 0; index < regSize; index++ ){
		reg = popStack('E', regType);
		pushStack( reg, 'R', regType );
	}

	if( indirect ) indirect_reg = popStack('E', regType);

	if( regSize > 1 && regSize == memSize ){
		for( index = 0; index < memSize; index++ ){
			reg = popStack('R', regType);
			if( isGlobal ){
				mem = "MM[" + to_string(FPoffset + index) + "]";
				comment("Global Variable: Reg to MM");
			}
			else{
				mem = "MM[FP_reg + " + to_string(FPoffset + index) + "]";
				comment("Variable: Reg to MM");
			}
			if( memType == TYPE_FLOAT) writeLine("memcpy( &" + mem + " , &" + reg + ", sizeof(int) );");
			else writeLine(mem + " = " + reg + ";");
		}
	}
	else if ( regSize == 1 ){
		reg = popStack('R', regType);
		for( index = 0; index < memSize; index++ ){
			if( isGlobal ){
				mem = "MM[" + to_string(FPoffset) + "]";
				comment("Global Variable: Reg to MM");
			}
			else{
				mem = "MM[FP_reg + " + to_string(FPoffset + index) + "]";
				comment("Variable: Reg to MM");
			}
			if( memType == TYPE_FLOAT) writeLine("memcpy( &" + mem + " , &" + reg + ", sizeof(int) );");
			else writeLine(mem + " = " + reg + ";");
		}
	}
	return "";
}


// Get a guaranteed unique label string
string codeGenerator::newLabel( string prefix ){
	return ("Label_" + to_string( label_count++ ) + "_" + prefix);
}

// Put label in file
void codeGenerator::placeLabel( string label ){
	tabDec();
	writeLine( "\n" + label + ":" );
	writeLine("printf(\"" + label + "\\n\");");
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

void codeGenerator::setProcedurePointers(int frameSize){
	comment("Set FP and SP using the caller's FP/SP values and the new frame size");
	writeLine("FP_reg = SP_reg;");
	writeLine("SP_reg = FP_reg + " + to_string(frameSize) + ";" );
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
	writeLine("MM[SP_reg] = FP_reg;");
	setReturnAddress(1, retLabel);
	writeLine( "goto " + procValue.CallLabel + ";" );
	placeLabel( retLabel );
}

// Set address of label to return to after procedure call
void codeGenerator::setReturnAddress( int SPoffset, string label ){
	string source, destination;
	comment("Set return address for procedure call");
	writeLine( "MM[SP_reg + " + to_string(SPoffset) + "] = (int)&&" + label + ";" );
	return;
}

//Return address of instruction is always after the void* to previous frame which is placed at the frame pointer
void codeGenerator::ProcedureReturn(int SPoffset){
	comment("Procedure return");
	//fprintf( oFile, MMtoREG( TYPE_INTEGER, );
	writeLine("TP_reg = FP_reg + 1;");
	writeLine( "FP_reg = MM[FP_reg + " +to_string(SPoffset) + "];" );
	writeLine( "goto *(void*)MM[TP_reg];" );
	return;
}

void codeGenerator::condBranch( string labelTrue, string labelFalse ){
	string cond;
	cond = popStack('E', TYPE_BOOL);
	comment("Conditional branch");
	// goto true condition
	writeLine("if( " + cond + " ) goto " + labelTrue + ";");
	// goto else condition if one exists
	if( labelFalse.compare("") != 0 ) writeLine( "goto " + labelFalse + ";");
	return;
}

void codeGenerator::branch( string label ){
	comment("Unconditional branch");
	writeLine( "goto "+ label + ";");
	return;
}

// Check for EvalExpr() to see if it should invalidate savedArgument for OUT and INOUT
bool codeGenerator::checkArguments(){
	if( savedArgument.set && !savedArgument.invalid ) return true;
	else return false;
}

void codeGenerator::resetArgument(){
	savedArgument.paramType = TYPE_PARAM_NULL;
	savedArgument.type = 0;
	savedArgument.size = 0;
	savedArgument.index = -1;
	savedArgument.offset = -1;
	savedArgument.isGlobal = false;
	savedArgument.set = false;
	savedArgument.invalid = false;
	return;
}

// Set all savedArgument parameters for Name() call in parser.
// Does not set the invalid bool parameter to prevent accidental reset
void codeGenerator::setOutputArgument( scopeValue value, bool isGlobal, bool index ){
	savedArgument.paramType = value.paramType;
	savedArgument.type = value.type;
	savedArgument.size = value.size;
	savedArgument.index = index;
	savedArgument.offset = value.FPoffset;
	savedArgument.isGlobal = isGlobal;
	savedArgument.set = true;
}

void codeGenerator::invalidateArgument(){
	savedArgument.invalid = true;
}

void codeGenerator::pushArgument( int &SPoffset, int paramType){
	savedArgument.paramType = paramType;
	if( savedArgument.set ) argListStack.push( savedArgument );
	else{
		cout << "Pushed unsaved Argument! paramtype:" << paramType << " SPoffset: " << SPoffset << endl;
		argListStack.push( savedArgument );
	}
	if(savedArgument.paramType == TYPE_PARAM_IN || savedArgument.paramType == TYPE_PARAM_INOUT){
		comment("Push argument.");
		reg2mm(savedArgument.type, savedArgument.type, savedArgument.size, savedArgument.size, SPoffset, savedArgument.isGlobal );
	}
	cout << "PARAM_TYPE = " << savedArgument.paramType << " SPoffset: " << SPoffset << endl;

	if( savedArgument.size > 1 ) SPoffset += savedArgument.size;
	else SPoffset += 1;
	resetArgument();
	return;
}

void codeGenerator::popArguments( int SPoffset ){
	while( argListStack.size() != 0 ){
		savedArgument = argListStack.top();
		argListStack.pop();
		if( savedArgument.paramType == TYPE_PARAM_OUT || savedArgument.paramType == TYPE_PARAM_INOUT ){
			if( savedArgument.invalid ) cout << "invalid output argument, code generation will contine" << endl;
			else{
				comment("Pop argument.");
				mm2reg(savedArgument.type, savedArgument.size, SPoffset, false, savedArgument.index );
				reg2mm(savedArgument.type, savedArgument.type, savedArgument.size, savedArgument.size, savedArgument.offset, savedArgument.isGlobal);
			}
		}
		if( savedArgument.index < 0 && savedArgument.size > 1 ){
			SPoffset -= savedArgument.size;
		}
		else{
			SPoffset -= 1;
		}
	}
	
	return;
}
