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
	else if( oFile != nullptr ){ 
			fclose( oFile );
	//Remove the generated file if there was an error
	}
	if( ContinueToGenerate == false && outputName.compare("") != 0 ) remove(outputName.c_str());
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
	if( oFile == nullptr ) return false;
	if( ferror(oFile) ) return false;
	else return true;
}

void codeGenerator::comment(string str, bool multi_line){
	if( multi_line ){
		fprintf( oFile, "\n" );
		fprintf( oFile, "// %s\n", str.c_str() );
		fprintf( oFile, "\n" );
	}
	else fprintf( oFile, "// %s\n", str.c_str() );
	return;
}

void codeGenerator::header(){
	//Attach necessary header files for c program
	fprintf( oFile, "#include <stdio.h>\n" );
	fprintf( oFile, "#include <stdlib.h>\n" );
	fprintf( oFile, "#include <string.h>\n\n" );

	//Define the memory space attributes
	fprintf( oFile, "#define MM_SIZE %d\n", MM_SIZE );
	fprintf( oFile, "#define REG_SIZE %d\n", REG_SIZE );
	
	comment("Include runtime functions\n");
	
	fprintf( oFile, "extern void handleBoolOp( int val1, int val2 );\n" );
	fprintf( oFile, "extern int getBool();\n" );
	fprintf( oFile, "extern int getInteger();\n" );
	fprintf( oFile, "extern float getFloat();\n" );
	fprintf( oFile, "extern int getString();\n" );
	fprintf( oFile, "extern char getChar();\n\n" );
	fprintf( oFile, "extern void putBool( int val );\n" );
	fprintf( oFile, "extern void putInteger( int val );\n" );
	fprintf( oFile, "extern void putFloat( float val );\n" );
	fprintf( oFile, "extern void putString( char* str_ptr );\n" );
	fprintf( oFile, "extern void putChar( char val );\n\n" );

	//Create registers and stack
	fprintf( oFile, "int MM[MM_SIZE];\n" );
	fprintf( oFile, "int iReg[REG_SIZE];\n" );
	fprintf( oFile, "float fReg[REG_SIZE];\n" );
	fprintf( oFile, "int FP_reg;\n" );
	fprintf( oFile, "int SP_reg;\n" );
	fprintf( oFile, "int TP_reg;\n" );
	fprintf( oFile, "int HP_reg;\n" );
	fprintf( oFile, "char buffer[256];\n\n" );

	comment( "Begin program execution" );
	//Call to main for function
	fprintf( oFile, "int main( void ){\n" );
	
	//Declare registers
	comment("Allocated registers for stack operations");
	fprintf( oFile, "   FP_reg = 0;\n" );
	fprintf( oFile, "   SP_reg = 0;\n" );
	fprintf( oFile, "   TP_reg = 0;\n" );
	fprintf( oFile, "   HP_reg = MM_SIZE - 1;\n" );
	fprintf( oFile, "   goto BUILD_HEAP;\n\n" );
	
	// Add the runtime function inline code
	WriteRuntime();
}

// Close the main function with return 0
void codeGenerator::footer(){
	fprintf( oFile, "   return 0;\n");
	fprintf( oFile, "}\n");
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
	bool checkBool = false;
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
			if( (type_left == TYPE_INTEGER ) || (type_right == TYPE_INTEGER) ) checkBool = true;
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
			if(checkBool) fprintf( oFile, "   handleBoolOp( %s, %s );\n", lhs.c_str(), rhs.c_str() );
			fprintf( oFile, "   %s = %s %s %s;\n", result.c_str(), lhs.c_str(), op.c_str(), rhs.c_str() );
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
			if(checkBool) fprintf( oFile, "   handleBoolOp( %s, %s );\n", lhs.c_str(), rhs.c_str() );
			fprintf( oFile, "   %s = %s %s %s;\n", result.c_str(), lhs.c_str(), op.c_str(), rhs.c_str() );
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
			if(checkBool) fprintf( oFile, "   handleBoolOp( %s, %s );\n", lhs.c_str(), rhs.c_str() );
			fprintf( oFile, "   %s = %s %s %s;\n", result.c_str(), lhs.c_str(), op.c_str(), rhs.c_str() );
		}
	}
	else{
		// If LHS and RHS are both single values
		rhs = popStack('E', type_right);
		lhs = popStack('E', type_left);
		result = newRegister(type_result);
		pushStack( result, 'E', type_result );
		if(checkBool) fprintf( oFile, "   handleBoolOp( %s, %s );\n", lhs.c_str(), rhs.c_str() );
		fprintf( oFile, "   %s = %s %s %s;\n", result.c_str(), lhs.c_str(), op.c_str(), rhs.c_str() );
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
			fprintf( oFile, "   %s = !( %s );\n", reg.c_str(), reg.c_str() );
		}
		else if( type == TYPE_INTEGER ){
			// bitwise not
			fprintf( oFile, "   %s = ~( %s );\n", reg.c_str(), reg.c_str() );
		}
	}
	return;
}

/* Put constant value into the register.
 * String values place the string memory location in the register
 */
string codeGenerator::val2reg( string val, int type ){
	if( !ShouldGenerate() ) return "";
	savedArgument.size = 0;
	savedArgument.type = type;
	savedArgument.indirect = false;
	savedArgument.set = true;
	savedArgument.isGlobal = false;
	
	string reg;
	comment("Constant Value to Register");
	reg = newRegister( type );

	pushStack( reg, 'E', type );

	if(type == TYPE_STRING){
		val = to_string( AddStringHeap( val ) );
	}
	fprintf( oFile, "   %s = %s;\n", reg.c_str(), val.c_str() );
	return reg;
}

// Add string to the heap and return the memory location
int codeGenerator::AddStringHeap( string str ){
	if( !ShouldGenerate() ) return -1;
	HeapSize += ( str.size() + 1);
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
		fprintf( oFile, "   strcpy((char*)&MM[%d], %s);\n", stringEntry.MMlocation, stringEntry.contents.c_str() );
		fprintf( oFile, "   TP_reg = %d;\n", stringEntry.MMlocation );
	}
	fprintf( oFile, "   goto Label_0_Begin_Program;\n" );
	return;
}

// Negate the value of the top N registers ( for integers and float values/variables )
void codeGenerator::NegateTopRegisters( int type, int size ){
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
		fprintf( oFile, "   %s = -%s;\n", reg.c_str(), reg.c_str() );
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
		fprintf( oFile, "   TP_reg = %s + %d;\n", pointer.c_str(), FPoffset );
		
		reg = newRegister(memType);
		pushStack(reg, 'E', memType);
		fprintf( oFile, "   %s = MM[TP_reg + %s];\n", reg.c_str(), indirect_reg.c_str() );
	} 
	else{
		if( index < 0){
			for( index = 0; index < memSize; index++ ){
				reg = newRegister(memType);
				pushStack(reg, 'E', memType);
			
				if( isGlobal ) mem = "MM[" + to_string(FPoffset + index) + "]";
				else mem = "MM[" + pointer + " + " + to_string(FPoffset + index) + "]";
			
				if( memType == TYPE_FLOAT) fprintf( oFile, "   memcpy( &%s, &%s, sizeof(int) );\n", reg.c_str(), mem.c_str() );
				else fprintf( oFile, "   %s = %s;\n", reg.c_str(), mem.c_str() );
			}
		}
		else{
			reg = newRegister(memType);
			pushStack(reg, 'E', memType);
		
			if( isGlobal ) mem = "MM[" + to_string(FPoffset + index) + "]";
			else mem = "MM[" + pointer + " + " + to_string(FPoffset + index) + "]";
		
			if( memType == TYPE_FLOAT) fprintf( oFile, "   memcpy( &%s, &%s, sizeof(int) );\n", reg.c_str(), mem.c_str() );
			else fprintf( oFile, "   %s = %s;\n", reg.c_str(), mem.c_str() );
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
		if(savedArgument.set == false) cout << "not a saved argument" << endl;
		reg = popStack('R', regType);
		indirect_reg = popStack('E', indirect_type);
		comment("indirect off " + indirect_reg);
		fprintf( oFile, "   TP_reg = %s + %d;\n", pointer.c_str(), FPoffset );
		fprintf( oFile, "   MM[TP_reg + %s] = %s;\n", indirect_reg.c_str(), reg.c_str() );
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
				if( memType == TYPE_FLOAT) fprintf( oFile, "   memcpy( &%s, &%s, sizeof(int) );\n", mem.c_str(), reg.c_str() );
				else fprintf( oFile, "   %s = %s;\n", mem.c_str(), reg.c_str() );
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
				if( memType == TYPE_FLOAT) fprintf( oFile, "   memcpy( &%s, &%s, sizeof(int) );\n", mem.c_str(), reg.c_str() );
				else fprintf( oFile, "   %s = %s;\n", mem.c_str(), reg.c_str() );
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
	fprintf( oFile, "\n%s:\n", label.c_str() );
	return;
}

// Set the values of SP and FP for a called procedure after the old values have been saved in memory 
void codeGenerator::setProcedurePointers( int frameSize ){
	if( !ShouldGenerate() ) return;
	comment( "Set FP and SP using the caller's FP/SP values and the new frame size" );
	fprintf( oFile, "   FP_reg = SP_reg;\n" );
	fprintf( oFile, "   SP_reg = FP_reg + %d;\n", frameSize);
	return;
}

// Call the procedure and store the return address of retLabel
void codeGenerator::callProcedure( scopeValue procValue, string id ){
	if( !ShouldGenerate() ) return;
	
	comment( "Call procedure" );
	fprintf( oFile, "   MM[SP_reg] = FP_reg;\n" );
	fprintf( oFile, "   MM[SP_reg + 1] = %d;\n", call_count );
	fprintf( oFile, "   goto %s;\n", procValue.CallLabel.c_str() );
	
	string retLabel = newReturnLabel(id);
	placeLabel( retLabel );
	return;
}

// Return address of instruction is always after the void* to previous frame which is placed at the frame pointer
void codeGenerator::ProcedureReturn(){
	if( !ShouldGenerate() ) return;
	comment( "Procedure return" );
	fprintf( oFile, "   TP_reg = MM[FP_reg + 1];\n" );
	fprintf( oFile, "   FP_reg = MM[FP_reg];\n" );
	fprintf( oFile, "   goto BRANCH_TABLE;\n" );
	return;
}

//
void codeGenerator::setSPfromFP( int offset ){
	if( !ShouldGenerate() ) return;
	fprintf( oFile, "   SP_reg = FP_reg + %d;\n", offset );
	return;
}

// Conditional branch to True/False labels
void codeGenerator::condBranch( string labelTrue, string labelFalse ){
	if( !ShouldGenerate() ) return;
	string cond = popStack('E', TYPE_BOOL);
	comment("Conditional branch");
	// goto true condition
	fprintf( oFile, "   if( %s ) goto %s;\n", cond.c_str(), labelTrue.c_str() );
	// goto else condition if one exists
	if( labelFalse.compare("") != 0 ) fprintf( oFile, "   goto %s;\n", labelFalse.c_str() );
	return;
}

// Unconditional branch to the given label
void codeGenerator::branch( string label ){
	if( !ShouldGenerate() ) return;
	comment("Unconditional branch");
	fprintf( oFile, "   goto %s;\n", label.c_str() );
	return;
}

// Check for EvalExpr() to see if it should invalidate savedArgument for OUT and INOUT arguments in procedure call
bool codeGenerator::checkArguments(){
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
	savedArgument.indirect = false;
	savedArgument.isGlobal = false;
	savedArgument.set = false;
	savedArgument.invalid = false;
	return;
}

/* Set all savedArgument parameters for Name() call in parser.
 * Does not set the invalid bool parameter to prevent accidental reset */
void codeGenerator::setArgument( scopeValue value, bool isGlobal, int index, bool indirect ){
	if( !ShouldGenerate() ) return;
	savedArgument.paramType = value.paramType;
	savedArgument.type = value.type;
	savedArgument.size = value.size;
	savedArgument.index = index;
	savedArgument.indirect = indirect;
	savedArgument.offset = value.FPoffset;
	savedArgument.isGlobal = isGlobal;
	savedArgument.set = true;
	return;
}

// Set the invalid value of savedArgument to indicate for OUT arguments that do not have a valid memory location
// Happens if an output argument is declared as an expression such as : ( A + 5 )
void codeGenerator::invalidateArgument(){
	savedArgument.invalid = true;
	return;
}

void codeGenerator::pushArgument( int &SPoffset, int paramType){
	if( !ShouldGenerate() ) return;
	savedArgument.paramType = paramType;
	if( savedArgument.paramType == TYPE_PARAM_IN || savedArgument.paramType == TYPE_PARAM_INOUT ){
		argListStack.push( savedArgument );
	}
	else if( savedArgument.paramType == TYPE_PARAM_OUT) argListStack.push( savedArgument );

	if(savedArgument.paramType == TYPE_PARAM_IN || savedArgument.paramType == TYPE_PARAM_INOUT){
		comment("Push argument");
		if( savedArgument.index < 0 ){
			reg2mm(savedArgument.type, savedArgument.type, savedArgument.size, savedArgument.size, SPoffset, savedArgument.isGlobal, false, TYPE_INTEGER, true);
		}
		else{
			reg2mm(savedArgument.type, savedArgument.type, 1, 1, SPoffset, savedArgument.isGlobal, false, TYPE_INTEGER, true);
		}
	}

	if( savedArgument.size > 1 ) SPoffset += savedArgument.size;
	else SPoffset += 1;
	
	resetArgument();
	return;
}

// Pop all the arguments from procedure call and place OUT argument values into their correct memory location
void codeGenerator::popArguments(int SPoffset){
	if( !ShouldGenerate() ) return;
	
	while( argListStack.size() != 0 ){
		
		savedArgument = argListStack.top();
		argListStack.pop();
		
		if( savedArgument.index < 0 && savedArgument.size > 1 && !savedArgument.indirect ){
			SPoffset -= savedArgument.size;
		}
		else{
			SPoffset -= 1;
		}

		if( savedArgument.paramType == TYPE_PARAM_OUT || savedArgument.paramType == TYPE_PARAM_INOUT ){
			comment("Pop argument.");
			mm2reg(savedArgument.type, savedArgument.size, SPoffset, false, savedArgument.index, false, TYPE_INTEGER, true );
			reg2mm(savedArgument.type, savedArgument.type, savedArgument.size, savedArgument.size, savedArgument.offset, savedArgument.isGlobal);
		}
	}
	return;
}

// Write the inline code for runtime support that calls the runtime.c functions
void codeGenerator::WriteRuntime(){
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
	return;
}

string codeGenerator::newReturnLabel( string prefix ){
	string label = "Label_Return_" + to_string(call_count) + "_" + prefix;
	branchTable[call_count] = label;
	call_count++;
	return label;
}

bool codeGenerator::buildBranchTable(){
	fprintf( oFile, "\n   goto PROGRAM_END;\n" );
	fprintf( oFile, "\nBRANCH_TABLE:\n" );
	if( !branchTable.empty() ){
		createEntry(0, branchTable.size() - 1);
	}
	return true;
}

void codeGenerator::createEntry( int lPos, int uPos ){
	if( lPos == uPos ){
		fprintf( oFile, "   goto %s;\n", ( branchTable[lPos] ).c_str() );
	}
	else{
		int mPos = ( lPos + uPos )/2;
		int key = mPos;
		string branchTrue = branchTableLabelTrue(mPos, true);
		string branchFalse = branchTableLabelTrue(mPos, false);

		fprintf( oFile, "   iReg[0] = %d;\n", key );
		fprintf( oFile, "   iReg[0] = TP_reg > iReg[0];\n" );
		
		fprintf( oFile, "   if( iReg[0] ) goto %s;\n", branchTrue.c_str() );
		fprintf( oFile, "   goto %s;\n", branchFalse.c_str() );
		
		fprintf( oFile, "\n%s:\n", branchTrue.c_str() );
		createEntry( mPos + 1, uPos );
		
		fprintf( oFile, "\n%s:\n", branchFalse.c_str() );
		createEntry( lPos, mPos );
		return;
	}
}

string codeGenerator::branchTableLabelTrue(int id, bool cond){
	if( cond ) return "Branch_Greater_" + to_string(id);
	else return "Branch_Less_Equal_" + to_string(id);
}
