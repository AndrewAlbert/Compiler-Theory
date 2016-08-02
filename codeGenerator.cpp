#include "codeGenerator.h"
#include "macro.h"
#include "scopeValue.h"
#include <stack>
#include <queue>
#include <string>
#include <cstdio>
#include <iostream>
#include <map>

using namespace std;

codeGenerator::codeGenerator(){
	ireg_in_use = 0;
	freg_in_use = 0;
	label_count = 0;
	tabs = 0;
	HeapSize = 0;
	ContinueToGenerate = true;
	HeapSize = MM_SIZE - 1;
	outputName = "";
	call_count = 0;
}

codeGenerator::~codeGenerator(){
	if( testOutFile() ){
		buildBranchTable();
		buildHeap();
		fprintf( oFile, "\nPROGRAM_END:\n" );
		footer();
		fclose( oFile );
	}
	else{
		fclose( oFile );
	//Remove the generated file if there was an error
		if( ContinueToGenerate == false && outputName.compare("") != 0 ) remove(outputName.c_str());
	}
}

// Stops code generation from occuring if an error occurs in parsing
void codeGenerator::stopCodeGeneration(){
	ContinueToGenerate = false;
	return;
}

bool codeGenerator::ShouldGenerate(){
	if( ContinueToGenerate ) return true;
	else return false;
}

bool codeGenerator::attachOutputFile(string filename){
	outputName = filename;
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
	
	comment("Include runtime functions\n");
	
	writeLine("extern void handleBool( int val );");
	writeLine("extern int getBool();");
	writeLine("extern int getInteger();");
	writeLine("extern float getFloat();");
	writeLine("extern int getString();");
	writeLine("extern char getChar();\n");
	writeLine("extern void putBool( int val );");
	writeLine("extern void putInteger( int val );");
	writeLine("extern void putFloat( float val );");
	writeLine("extern void putString( char* str_ptr );");
	writeLine("extern void putChar( char val );\n");

	//Create registers and stack
	writeLine("int MM[MM_SIZE];");
	writeLine("int iReg[REG_SIZE];");
	writeLine("float fReg[REG_SIZE];");
	writeLine("int FP_reg;");
	writeLine("int SP_reg;");
	writeLine("int TP_reg;");
	writeLine("int HP_reg;");
	writeLine("char buffer[256];\n");

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
	writeLine( "goto BUILD_HEAP;\n" );
	
	// Add the runtime function inline code
	WriteRuntime();
}

// Close the main function with return 0
void codeGenerator::footer(){
	writeLine( "return 0;");
	tabDec();
	writeLine("}");
}

// Increment tab (3 spaces) for writing lines of code
void codeGenerator::tabInc(){
	tabs++;
	return;
}

// Decrement tab (3 spaces) for writing lines of code
void codeGenerator::tabDec(){
	if( tabs > 0 )tabs--;
	return;
}

// Convert the given integer type from macro.h into the corresponding string for that type declaration
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
		}
		fprintf( oFile, "%s\n", line.c_str() );
		return;
	}
}

/* Push register identifier string to the given stack.
 * L/E/R is used to identify the given stack
 * only E will increment the register count
 * L/R are only used for evaluating expressions in push/pop order (arrays) */
void codeGenerator::pushStack( string value, char stackID, int type ){
	if( !ShouldGenerate() ) return;
	switch( stackID ){
		case 'L':
			leftStack.push( value );
			break;
		case 'E':
			exprStack.push( value );
			if( type == TYPE_FLOAT ) freg_in_use++;
			else ireg_in_use++;
			break;
		case 'R':
			rightStack.push( value );
			break;
		default:
			cout << "Not a valid expression stack!\n";
			break;
	}
	return;
}

/* Ppop register identifier string from the given stack.
 * L/E/R is used to identify the given stack
 * only E will decrement the register count
 * L/R are only used for evaluating expressions in push/pop order (arrays) */
string codeGenerator::popStack( char stackID, int type ){
	if( !ShouldGenerate() ) return "";
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
	if( !ShouldGenerate() ) return "";
	string lhs, rhs, result;
	int i, type_result;
	
	// Ensure sizes are at least 1 so that loops will execute for scalars
	if( size_left == 0 ) size_left++;
	if( size_right == 0 ) size_right++;
	
	/* Invalidate OUT/INOUT arguments since an expresion such as <Name> * 2 has occurred
	 * there is no return value now and so there is no location to put an OUT argument */
	if( checkArguments() ) invalidateArgument();
	
	//Select int registers or float registers to put the result in
	if( type_left != type_right ){
		if( ( type_left == TYPE_FLOAT ) || ( type_right == TYPE_FLOAT ) ){ 
			type_result = TYPE_FLOAT;
		}
		else if( ( type_left == TYPE_BOOL ) || ( type_right == TYPE_BOOL ) ){
			type_result = TYPE_BOOL;
		}
		else type_result = TYPE_INTEGER;
	}
	else type_result = type_left;

	// Convert operations for bitwise or boolean comparison to the correct C operator
	if(op.compare("&") == 0){ 
		if( (type_left == TYPE_BOOL) || (type_right == TYPE_BOOL) ){
			op = "&&";
			// add runtime check for integer/bool
		}
		else op = "&";
	}
	else if(op.compare("|") == 0){ 
		if( (type_left == TYPE_BOOL) || (type_right == TYPE_BOOL) ){
			op = "||";
			// add runtime check for integer/bool
		}
		else op = "|";
	}
	else if(op.compare("NOT") == 0){
		if( (type_left == TYPE_BOOL) || (type_right == TYPE_BOOL) ){
			op = "!";
			// add runtime check for integer/bool
		}
		else op = "~";
	}

	// Push the LHS and RHS operands to their respective stacks and then result to exprStack
	if( ( size_left > 1 ) && ( size_right > 1) ){
		// If LHS and RHS are both arrays ( same size checked by parser )
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
			writeLine( result + " = " + lhs + " " + op + " " + rhs + ";");
		}
	}
	else if( size_left > 1 ){
		// If LHS is an array and RHS is a single value
		rhs = popStack('E', type_right);
		for( i = 0; i < size_left; i++ ){
			lhs = popStack('E', type_left);
			pushStack(lhs, 'L', type_left);
		}
		for( i = 0; i < size_left; i++ ){
			lhs = popStack('L', type_left);
			result = newRegister(type_result);
			pushStack( result, 'E', type_result );
			writeLine( result + " = " + lhs + " " + op + " " + rhs + ";");
		}
	}
	else if( size_right > 1 ){
		// If LHS is a single value and RHS is an array
		for( i = 0; i < size_right; i++ ){
			rhs = popStack('R', type_right);
			pushStack(rhs, 'R', type_right);
		}
		lhs = popStack('E', type_left);
		for( i = 0; i < size_right; i++ ){
			rhs = popStack('R', type_right);
			result = newRegister(type_result);
			pushStack( result, 'E', type_result );
			writeLine( result + " = " + lhs + " " + op + " " + rhs + ";");
		}
	}
	else{
		// If LHS and RHS are both single values
		rhs = popStack('E', type_right);
		lhs = popStack('E', type_left);
		result = newRegister(type_result);
		pushStack( result, 'E', type_result );
		writeLine( result + " = " + lhs + " " + op + " " + rhs + ";");
	}
	return result;
}

// Get new floating point / integer register
string codeGenerator::newRegister( int type ){
	if( !ShouldGenerate() ) return "";
	if( type == TYPE_FLOAT ) return getRegister( type, freg_in_use);
	return getRegister( type, ireg_in_use );
}

// Get the given register id and type
string codeGenerator::getRegister( int type, int id ){
	if( !ShouldGenerate() ) return "";
	if (type == TYPE_FLOAT) return ( fRegId + "[" + to_string(id) + "]");
	return ( iRegId + "[" + to_string( id ) + "]");
}

// Perform logical not ( boolean ) or bitwise not ( integer ) on the top N register on exprStack
void codeGenerator::NotOnRegister( int type, int size ){
	if( !ShouldGenerate() ) return;
	string reg;
	comment("Not Operation on Register");
	
	//Ensure size is at least 1 for the loops
	if( size == 0 ) size++;
	
	for( int i = 0; i < size; i++ ){
		reg = popStack('E', type);
		pushStack( reg, 'R', type );
	}
	for( int i = 0; i < size; i++ ){
		reg = popStack('R', type);
		pushStack(reg, 'E', type);
		if( type == TYPE_BOOL ){
			// boolean not
			writeLine( reg + " = !( " + reg + " );" );
		}
		else if( type == TYPE_INTEGER ){
			// bitwise not
			writeLine( reg + " = ~( " + reg + " );" );
		}
	}
	return;
}

/* Put constant value into the register.
 * String values place the string memory location in the register
 */
string codeGenerator::VALtoREG( string val, int type ){
	if( !ShouldGenerate() ) return "";
	string reg;
	comment("Constant Value to Register");
	cout << "val: " << val << " to " << reg << endl;
	reg = newRegister( type );
	cout << "Val to reg " << type << endl;
	pushStack( reg, 'E', type );

	if(type == TYPE_STRING){
		val = to_string( AddStringHeap( val ) );
	}
	writeLine( reg + " = " + val + ";");
	cout << "Done val to reg" << endl;
	return reg;
}

// Add string to the heap and return the memory location
int codeGenerator::AddStringHeap( string str ){
	if( !ShouldGenerate() ) return -1;
	HeapSize += ( str.size() + 1);
	cout << "string: " << str << " size: " << str.size() << endl;
	stringEntry.MMlocation = HeapSize;
	stringEntry.contents = str;
	string_heap.push(stringEntry);
	return HeapSize;
}

void codeGenerator::buildHeap(){
	fprintf( oFile, "\nBUILD_HEAP:\n" );
	while( string_heap.size() > 0 ){
		stringEntry = string_heap.front();
		string_heap.pop();
		fprintf( oFile, "strcpy((char*)&MM[%d], %s);\n", stringEntry.MMlocation, stringEntry.contents.c_str() );
		fprintf( oFile, "TP_reg = %d;\n", stringEntry.MMlocation );
	}
	writeLine( "goto Label_0_Begin_Program;\n" );
	return;
}

// Negate the value of the top N registers ( for integers and float values/variables )
void codeGenerator::NegateTopRegister( int type, int size ){
	if( !ShouldGenerate() ) return;
	string reg;
	int i;
	// Ensure size is at least 1 for loops
	if( size == 0 ) size++;
	
	for( i = 0; i < size; i++ ){
		reg = popStack('E', type);
		pushStack( reg, 'L', type );
	}
	for( i = 0; i < size; i++ ){
		reg = popStack('L', type);
		pushStack( reg, 'E', type );
		writeLine( reg + " = -" + reg + ";");
	}
	return;
}

string codeGenerator::mm2reg(int memType, int memSize, int FPoffset, bool isGlobal, int index, bool indirect, int indirect_type, bool useSP){
	if( !ShouldGenerate() ) return "";
	string reg, mem, pointer;
	
	// Set comment to indicate if this is Global/Local MM to Reg
	if( isGlobal ) comment("Global MM to Reg");
	else comment("Local MM to Reg");

	// Get either the FP or SP register based on 
	if(useSP) pointer = "SP_reg";
	else pointer = "FP_reg";

	// Ensure source size is at least 1 for loops
	if( memSize == 0 ) memSize++;

	// Indirect off of the top register
	if( indirect ){
		string indirect_reg = popStack('E', indirect_type);
		comment("indirect off " + indirect_reg);
		writeLine("TP_reg = " + pointer + " + " + to_string(FPoffset) + ";");
		reg = newRegister(memType);
		pushStack(reg, 'E', memType);
		writeLine( reg + " = MM[TP_reg + " + indirect_reg + "];");
	} 
	else{
		if( index < 0){
			for( index = 0; index < memSize; index++ ){
				reg = newRegister(memType);
				pushStack(reg, 'E', memType);
			
				if( isGlobal ) mem = "MM[" + to_string(FPoffset + index) + "]";
				else mem = "MM[" + pointer + " + " + to_string(FPoffset + index) + "]";
			
				if( memType == TYPE_FLOAT) writeLine("memcpy( &" + reg + " , &" + mem + ", sizeof(int) );");
				else writeLine(reg + " = " + mem + ";");
			}
		}
		else{
			reg = newRegister(memType);
			pushStack(reg, 'E', memType);
		
			if( isGlobal ) mem = "MM[" + to_string(FPoffset + index) + "]";
			else mem = "MM[" + pointer + " + " + to_string(FPoffset + index) + "]";
		
			if( memType == TYPE_FLOAT) writeLine("memcpy( &" + reg + " , &" + mem + ", sizeof(int) );");
			else writeLine(reg + " = " + mem + ";");
		}
	}
	return "";
}

string codeGenerator::reg2mm(int regType, int memType, int regSize, int memSize, int FPoffset, bool isGlobal, bool indirect, int indirect_type, bool useSP ){
	if( !ShouldGenerate() ) return "";
	int index;
	string reg, mem, indirect_reg, pointer;

	if( useSP ) pointer = "SP_reg";
	else pointer = "FP_reg";
	
	if( regSize == 0 ) regSize++;
	if( memSize == 0 ) memSize++;
	
	for( index = 0; index < regSize; index++ ){
		reg = popStack('E', regType);
		pushStack( reg, 'R', regType );
	}

	if( indirect ){
		reg = popStack('R', regType);
		indirect_reg = popStack('E', indirect_type);
		comment("indirect off " + indirect_reg);
		writeLine("MM[" + indirect_reg + "] = " + reg + ";" );
	}
	else{
		if( isGlobal ) comment("Global Variable: Reg to MM");
		else comment("Variable: Reg to MM");
		
		if( regSize > 1 && regSize == memSize ){
			for( index = 0; index < memSize; index++ ){
				reg = popStack('R', regType);
				if( isGlobal ){
					mem = "MM[" + to_string(FPoffset + index) + "]";
				}
				else{
					mem = "MM[" + pointer + " + " + to_string(FPoffset + index) + "]";
				}
				if( memType == TYPE_FLOAT) writeLine("memcpy( &" + mem + " , &" + reg + ", sizeof(int) );");
				else writeLine(mem + " = " + reg + ";");
			}
		}
		else if ( regSize == 1 ){
			reg = popStack('R', regType);
			for( index = 0; index < memSize; index++ ){
				if( isGlobal ){
					mem = "MM[" + to_string(FPoffset + index) + "]";
				}
				else{
					mem = "MM[" + pointer + " + " + to_string(FPoffset + index) + "]";
				}
				if( memType == TYPE_FLOAT) writeLine("memcpy( &" + mem + " , &" + reg + ", sizeof(int) );");
				else writeLine(mem + " = " + reg + ";");
			}
		}
	}
	return "";
}


// Get a guaranteed unique label string
string codeGenerator::newLabel( string prefix ){
	if( !ShouldGenerate() ) return "";
	return ("Label_" + to_string( label_count++ ) + "_" + prefix);
}

// Place label in the generated file
void codeGenerator::placeLabel( string label ){
	if( !ShouldGenerate() ) return;
	tabDec();
	writeLine( "\n" + label + ":" );
	//writeLine("printf(\"" + label + "\\n\");");
	tabInc();
	return;
}

// Set the values of SP and FP for a called procedure after the old values have been saved in memory 
void codeGenerator::setProcedurePointers( int frameSize ){
	if( !ShouldGenerate() ) return;
	comment("Set FP and SP using the caller's FP/SP values and the new frame size");
	writeLine("FP_reg = SP_reg;");
	writeLine("SP_reg = FP_reg + " + to_string(frameSize) + ";" );
}

// Call the procedure and store the return address of retLabel
void codeGenerator::callProcedure( scopeValue procValue, string id ){
	if( !ShouldGenerate() ) return;
	
	
	comment( "Call procedure" );
	writeLine( "MM[SP_reg] = FP_reg;" );
	writeLine( "MM[SP_reg + 1] = " + to_string(call_count) + ";" );
	writeLine( "goto " + procValue.CallLabel + ";" );
	string retLabel = newReturnLabel(id);
	placeLabel( retLabel );
}

// Return address of instruction is always after the void* to previous frame which is placed at the frame pointer
void codeGenerator::ProcedureReturn(){
	if( !ShouldGenerate() ) return;
	comment( "Procedure return" );
	writeLine( "TP_reg = MM[FP_reg + 1];" );
	writeLine( "FP_reg = MM[FP_reg];" );
	writeLine( "goto BRANCH_TABLE;" );
	return;
}

//
void codeGenerator::setSPfromFP( int offset ){
	if( !ShouldGenerate() ) return;
	writeLine("SP_reg = FP_reg + " + to_string(offset) + ";");
	return;
}

// Conditional branch to True/False labels
void codeGenerator::condBranch( string labelTrue, string labelFalse ){
	if( !ShouldGenerate() ) return;
	string cond;
	cond = popStack('E', TYPE_BOOL);
	comment("Conditional branch");
	// goto true condition
	writeLine("if( " + cond + " ) goto " + labelTrue + ";");
	// goto else condition if one exists
	if( labelFalse.compare("") != 0 ) writeLine( "goto " + labelFalse + ";");
	return;
}

// Unconditional branch to the given label
void codeGenerator::branch( string label ){
	if( !ShouldGenerate() ) return;
	comment("Unconditional branch");
	writeLine( "goto "+ label + ";");
	return;
}

// Check for EvalExpr() to see if it should invalidate savedArgument for OUT and INOUT arguments in procedure call
bool codeGenerator::checkArguments(){
	if( !ShouldGenerate() ) return false;
	if( savedArgument.set && !savedArgument.invalid ) return true;
	else return false;
}

// Reset the savedArgument values 
void codeGenerator::resetArgument(){
	if( !ShouldGenerate() ) return;
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
void codeGenerator::setOutputArgument( scopeValue value, bool isGlobal, int index, bool indirect ){
	if( !ShouldGenerate() ) return;
	savedArgument.paramType = value.paramType;
	savedArgument.type = value.type;
	savedArgument.size = value.size;
	savedArgument.index = index;
	savedArgument.indirect = indirect;
	savedArgument.offset = value.FPoffset;
	savedArgument.isGlobal = isGlobal;
	savedArgument.set = true;
}

// Set the invalid value of savedArgument to indicate for OUT arguments that do not have a valid memory location
// Happens if an output argument is declared as an expression such as : ( A + 5 )
void codeGenerator::invalidateArgument(){
	if( !ShouldGenerate() ) return;
	savedArgument.invalid = true;
}

void codeGenerator::pushArgument( int &SPoffset, int paramType){
	if( !ShouldGenerate() ) return;
	cout << "Push argument:" << endl;
	savedArgument.paramType = paramType;
	if( savedArgument.paramType == TYPE_PARAM_IN || savedArgument.paramType == TYPE_PARAM_INOUT ){
		argListStack.push( savedArgument );
		if( !savedArgument.set ) cout << "Pushed unsaved Argument! paramtype:"  << endl;
	}
	else if( savedArgument.paramType = TYPE_PARAM_OUT) argListStack.push( savedArgument );

	if(savedArgument.paramType == TYPE_PARAM_IN || savedArgument.paramType == TYPE_PARAM_INOUT){
		comment("Push argument");
		cout << "Reg2MM" << endl;
		if( savedArgument.index < 0 ){
			reg2mm(savedArgument.type, savedArgument.type, savedArgument.size, savedArgument.size, SPoffset, savedArgument.isGlobal, savedArgument.indirect, TYPE_INTEGER, true);
		}
		else{
			reg2mm(savedArgument.type, savedArgument.type, 1, savedArgument.size, SPoffset, savedArgument.isGlobal, savedArgument.indirect, TYPE_INTEGER, true);
		}
		cout << "Indirect: " << savedArgument.indirect << endl;
		cout << "Saved size: " << savedArgument.size << endl;
		cout << "Saved type: " << savedArgument.type << endl;
		cout << "SP offset: " << SPoffset << endl;
		cout << "isGlobal: " << savedArgument.isGlobal << endl;
	}
	else{
		//cout << "pop PARAM_OUT" << endl;
		//popStack('E',savedArgument.type);
	}

	if( savedArgument.size > 1 ) SPoffset += savedArgument.size;
	else SPoffset += 1;
	resetArgument();
	return;
}

// Pop all the arguments from procedure call and place OUT argument values into their correct memory location
void codeGenerator::popArguments(int SPoffset){
	if( !ShouldGenerate() ) return;
	// Decrement offset by one since it currently represents the location to place the next argument (if one exists)	
	//SPoffset -= 1;
	while( argListStack.size() != 0 ){
		cout << "\nArg List size: " << argListStack.size() << endl;
		savedArgument = argListStack.top();
		argListStack.pop();
		// TODO: Check this, may be incorrect
		if( savedArgument.index < 0 && savedArgument.size > 1 && !savedArgument.indirect ){
			SPoffset -= savedArgument.size;
		}
		else{
			SPoffset -= 1;
		}
		cout << "SPoffset = " << SPoffset << endl;

		if( savedArgument.paramType == TYPE_PARAM_OUT || savedArgument.paramType == TYPE_PARAM_INOUT ){
			if( savedArgument.invalid ) cout << "invalid output argument, code generation will continue" << endl;
			else{
				comment("Pop argument.");
				cout << "Pop Argument" << endl;
				mm2reg(savedArgument.type, savedArgument.size, SPoffset, false, savedArgument.index, false, TYPE_INTEGER, true );
				reg2mm(savedArgument.type, savedArgument.type, savedArgument.size, savedArgument.size, savedArgument.offset, savedArgument.isGlobal);
			}
		}
		/*if( savedArgument.index < 0 && savedArgument.size > 1 ){
			SPoffset -= savedArgument.size;
		}
		else{
			SPoffset -= 1;
		}
		cout << "SPoffset = " << SPoffset << endl;*/
	}
	
	return;
}

// Write the inline code for runtime support that calls the runtime.c functions
void codeGenerator::WriteRuntime(){
	if( !ShouldGenerate() ) return;
	fprintf( oFile, "\nGETBOOL:\n");
	fprintf( oFile, "   FP_reg = SP_reg;\n");
	fprintf( oFile, "   SP_reg = FP_reg + 3;\n");
	fprintf( oFile, "   iReg[0] = getBool();\n");
	fprintf( oFile, "   MM[FP_reg + 2] = iReg[0];\n");
	fprintf( oFile, "   TP_reg = MM[FP_reg + 1];\n");
	fprintf( oFile, "   FP_reg = MM[FP_reg];\n");
	fprintf( oFile, "   goto BRANCH_TABLE;\n");

	fprintf( oFile, "\nGETINTEGER:\n");
	fprintf( oFile, "   FP_reg = SP_reg;\n");
	fprintf( oFile, "   SP_reg = FP_reg + 3;\n");
	fprintf( oFile, "   iReg[0] = getInteger();\n");
	fprintf( oFile, "   MM[FP_reg + 2] = iReg[0];\n");
	fprintf( oFile, "   TP_reg = MM[FP_reg + 1];\n");
	fprintf( oFile, "   FP_reg = MM[FP_reg];\n");
	fprintf( oFile, "   goto BRANCH_TABLE;\n");

	fprintf( oFile, "\nGETCHAR:\n");
	fprintf( oFile, "   FP_reg = SP_reg;\n");
	fprintf( oFile, "   SP_reg = FP_reg + 3;\n");
	fprintf( oFile, "   iReg[0] = getChar();\n");
	fprintf( oFile, "   MM[FP_reg + 2] = iReg[0];\n");
	fprintf( oFile, "   TP_reg = MM[FP_reg + 1];\n");
	fprintf( oFile, "   FP_reg = MM[FP_reg];\n");
	fprintf( oFile, "   goto BRANCH_TABLE;\n");

	fprintf( oFile, "\nGETFLOAT:\n");
	fprintf( oFile, "   FP_reg = SP_reg;\n");
	fprintf( oFile, "   SP_reg = FP_reg + 3;\n");
	fprintf( oFile, "   fReg[0] = getFloat();\n");
	fprintf( oFile, "   memcpy(&MM[FP_reg + 2], &iReg[0], sizeof(int) );\n");
	fprintf( oFile, "   TP_reg = MM[FP_reg + 1];\n");
	fprintf( oFile, "   FP_reg = MM[FP_reg];\n");
	fprintf( oFile, "   goto BRANCH_TABLE;\n");

	fprintf( oFile, "\nGETSTRING:\n");
	fprintf( oFile, "   FP_reg = SP_reg;\n");
	fprintf( oFile, "   SP_reg = FP_reg + 3;\n");
	fprintf( oFile, "   iReg[0] = getString(buffer);\n");
	fprintf( oFile, "   HP_reg = HP_reg - iReg[0];\n");
	fprintf( oFile, "   MM[FP_reg + 2] = HP_reg;\n");
	fprintf( oFile, "   strcpy((char*)&MM[HP_reg], buffer);\n");
	fprintf( oFile, "   TP_reg = MM[FP_reg + 1];\n");
	fprintf( oFile, "   FP_reg = MM[FP_reg];\n");
	fprintf( oFile, "   goto BRANCH_TABLE;\n");

	fprintf( oFile, "\nPUTBOOL:\n");
	fprintf( oFile, "   FP_reg = SP_reg;\n");
	fprintf( oFile, "   SP_reg = FP_reg + 3;\n");
	fprintf( oFile, "   iReg[0] = MM[FP_reg + 2];\n");
	fprintf( oFile, "   putBool(iReg[0]);\n");
	fprintf( oFile, "   TP_reg = MM[FP_reg + 1];\n");
	fprintf( oFile, "   FP_reg = MM[FP_reg];\n");
	fprintf( oFile, "   goto BRANCH_TABLE;\n");

	fprintf( oFile, "\nPUTINTEGER:\n");
	fprintf( oFile, "   FP_reg = SP_reg;\n");
	fprintf( oFile, "   SP_reg = FP_reg + 3;\n");
	fprintf( oFile, "   iReg[0] = MM[FP_reg + 2];\n");
	fprintf( oFile, "   putInteger(iReg[0]);\n");
	fprintf( oFile, "   TP_reg = MM[FP_reg + 1];\n");
	fprintf( oFile, "   FP_reg = MM[FP_reg];\n");
	fprintf( oFile, "   goto BRANCH_TABLE;\n");

	fprintf( oFile, "\nPUTCHAR:\n");
	fprintf( oFile, "   FP_reg = SP_reg;\n");
	fprintf( oFile, "   SP_reg = FP_reg + 3;\n");
	fprintf( oFile, "   iReg[0] = MM[FP_reg + 2];\n");
	fprintf( oFile, "   putChar(iReg[0]);\n");
	fprintf( oFile, "   TP_reg = MM[FP_reg + 1];\n");
	fprintf( oFile, "   FP_reg = MM[FP_reg];\n");
	fprintf( oFile, "   goto BRANCH_TABLE;\n");

	fprintf( oFile, "\nPUTFLOAT:\n");
	fprintf( oFile, "   FP_reg = SP_reg;\n");
	fprintf( oFile, "   SP_reg = FP_reg + 3;\n");
	fprintf( oFile, "   memcpy(&fReg[0],&MM[FP_reg + 2],sizeof(int));\n");
	fprintf( oFile, "   putFloat(fReg[0]);\n");
	fprintf( oFile, "   TP_reg = MM[FP_reg + 1];\n");
	fprintf( oFile, "   FP_reg = MM[FP_reg];\n");
	fprintf( oFile, "   goto BRANCH_TABLE;\n");

	fprintf( oFile, "\nPUTSTRING:\n");
	fprintf( oFile, "   FP_reg = SP_reg;\n");
	fprintf( oFile, "   SP_reg = FP_reg + 3;\n");
	fprintf( oFile, "   iReg[0] = MM[FP_reg + 2];\n");
	fprintf( oFile, "   putString((char*)&MM[iReg[0]]);\n");
	fprintf( oFile, "   TP_reg = MM[FP_reg + 1];\n");
	fprintf( oFile, "   FP_reg = MM[FP_reg];\n");
	fprintf( oFile, "   goto BRANCH_TABLE;\n");
}

string codeGenerator::newReturnLabel( string prefix ){
	string label = "Label_Return_" + to_string(call_count) + prefix;
	branchTable[call_count] = label;
	call_count++;
	return label;
}

bool codeGenerator::buildBranchTable(){
	fprintf( oFile, "\ngoto PROGRAM_END;\n" );
	//map<int, string>::iterator it = branchTable.cbegin();
	fprintf( oFile, "\nBRANCH_TABLE:\n" );
	if( !branchTable.empty() ){
		createEntry(0, branchTable.size() - 1);
	}
	return true;
}

void codeGenerator::createEntry( int lPos, int uPos ){
	if( lPos == uPos ){
		fprintf( oFile, "goto %s;\n", (branchTable[lPos]).c_str() );
	}
	else{
		int mPos = ( lPos + uPos )/2;
		int key = mPos;
		fprintf( oFile, "iReg[0] = %d;\n", key );
		fprintf( oFile, "iReg[0] = TP_reg > iReg[0];\n" );
		fprintf( oFile, "if( iReg[0] ){\n" );
		createEntry( mPos + 1, uPos );
		fprintf( oFile, "}\n" );
		comment("Else TP <= iReg[0]");
		createEntry( lPos, mPos );
		return;
	}
}

