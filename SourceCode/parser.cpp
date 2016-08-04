#include "parser.h"
#include "scopeTracker.h"
#include "scanner.h"
#include "macro.h"
#include "scopeValue.h"
#include "token_type.h"
#include "codeGenerator.h"
#include <string>
#include <iostream>
#include <queue>

using namespace std;

/* Constructor for the parser
 * Grabs the linked list of tokens from the scanner
 * Starts parsing using the Program() grammar
 * Each function has a comment showing the BNF grammar

 * curly braces '{' and '}' denote optional grammar elements
 * 	these optional elements may be singular { }
 * 	zero or more { }*
 * 	or one or more { }+
 */
Parser::Parser(token_type* tokenPtr, Scanner* scannerPtr, scopeTracker* scopes, codeGenerator* genPtr){
	//Set default values needed to begin parsing and attach other compiler classes to their pointers
	token = tokenPtr;
	Scopes = scopes;
	scanner = scannerPtr;
	generator = genPtr;
	textLine = "";
	currentLine = 0;
	warning = false;
	error = false;
	lineError = false;
	outArg = false;
	
	//Start program parsing
	Program();
	
	// Ensure the end of the file is reached
	if(token->type != T_EOF) ReportWarning("Tokens remaining. Parsing stopped at 'end program' and will not include any tokens after that.");
	
	// Display all errors / warnings
	DisplayErrorQueue();
	
	if(error) cout << "\nParser completed with some errors.\n   Code cannot be generated.\n" << endl;
	else if(warning) cout << "\nParser completed with some warnings.\n\tCode can still be generated.\n" << endl;
	else cout << "\nParser completed with no errors or warnings.\n\tCode has been generated.\n" << endl;

}

// Destructor
Parser::~Parser(){
	// set all pointers to nullptr
	token = nullptr;
	scanner = nullptr;
	Scopes = nullptr;
	generator = nullptr;
}

// Report fatal error and terminate parsing.
void Parser::ReportFatalError(string message){
	generator->stopCodeGeneration();
	error_queue.push("Fatal Error: line - " + to_string(currentLine) + "\n\t" + message + "\n\tFound: " + textLine + " " + token->ascii);
	error = true;
	DisplayErrorQueue();
	exit(EXIT_FAILURE);
}

//report error line number and desriptive message. Get tokens until the next line or a ';' is found.
void Parser::ReportLineError(string message, bool skipSemicolon = true){
	generator->stopCodeGeneration();
	error = true;
	lineError = true;
	
	// Use a '^' under the error text line to indicate where the error was encountered
	int error_location = textLine.size();
	string error_callout = "";
	while(error_location > 0){
		error_location--;
		error_callout.append(" ");
	}
	error_callout.append("^");

	// Get the rest of the line of tokens ( looks for newline or a semicolon )
	bool getNext = true;
	while(getNext){
		textLine.append(" " + token->ascii);
		// attempt to resync for structure keywords
		if(skipSemicolon){
			if( token->type == T_SEMICOLON){
				*token = scanner->getToken();
				getNext = false;
			}
		}
		switch(token->type){
			case T_SEMICOLON: case T_BEGIN: case T_END: case T_PROCEDURE: case T_THEN: case T_ELSE: case T_FOR:
				getNext = false;
				break;
			default:
				break;
		}
		if(token->line != currentLine) getNext = false;
		if(getNext) *token = scanner->getToken();
	}
	error_queue.push("Line Error: line - " + to_string(currentLine) + "\n\t" + message +"\n\tFound: " + textLine + "\n\t       " + error_callout);
	textLine = "";
	currentLine = token->line;
	return;
}

//report error line number and descriptive message
void Parser::ReportError(string message){
	generator->stopCodeGeneration();
	error_queue.push("Error: line - " + to_string(currentLine) + "\n\t" + message + "\n\tFound: " + textLine + " " + token->ascii);
	error = true;
	return;
}

//report warning and descriptive message
void Parser::ReportWarning(string message){
	error_queue.push("Warning: line - " + to_string(currentLine) + "\n\t" + message + "\n\tFound: " + textLine + " " + token->ascii);
	warning = true;
	return;
}

// Display all of the stored warnings/errors after parsing is complete or a fatal error occurs
void Parser::DisplayErrorQueue(){
	if(!error_queue.empty())
		cout << "\nWarnings / Errors:\n" << endl;
	else return;

	while(!error_queue.empty()){
		cout << error_queue.front() << "\n" << endl;
		error_queue.pop();
	}
	return;
}

// Check if current token is the correct type, if so get next
bool Parser::CheckToken(int type){
	//skip all comment tokens
	while (token->type == T_COMMENT){
		*token = scanner->getToken();
	}

	if(token->line != currentLine ){
		currentLine = token->line;
		textLine = "";
	}
	//check that current token matches input type, if so move to next token
	if(token->type == type){
		textLine.append(" " + token->ascii);
		*token = scanner->getToken();
		return true;
	}
	else if(token->type == T_UNKNOWN){
			ReportError("Found unknown token " + token->ascii);
			*token = scanner->getToken();
		return CheckToken(type);
	}
	else if(token->type == T_EOF){
		return false;
	}
	else return false;
}

void Parser::declareRunTime(){
	//Procedure to be added to the symbol tables
	scopeValue procVal;
	procVal.size = 0;
	procVal.type = TYPE_PROCEDURE;
	procVal.paramType = TYPE_PARAM_NULL;
	
	// input / output parameter of the runtime procedure
	scopeValue inputVal;
	inputVal.arguments.clear();
	
	string IDs[10] = {"GETBOOL", "GETINTEGER", "GETFLOAT", "GETSTRING", "GETCHAR", "PUTBOOL", "PUTINTEGER", "PUTFLOAT", "PUTSTRING", "PUTCHAR"};
	int ParamTypes[10] = {TYPE_PARAM_OUT, TYPE_PARAM_OUT, TYPE_PARAM_OUT, TYPE_PARAM_OUT, TYPE_PARAM_OUT, TYPE_PARAM_IN, TYPE_PARAM_IN, TYPE_PARAM_IN, TYPE_PARAM_IN, TYPE_PARAM_IN};
	int Types[10] = {TYPE_BOOL, TYPE_INTEGER, TYPE_FLOAT, TYPE_STRING, TYPE_CHAR, TYPE_BOOL, TYPE_INTEGER, TYPE_FLOAT, TYPE_STRING, TYPE_CHAR};

	for(int i = 0; i < 10; i++){
		//clear parameter list
		procVal.arguments.clear();
		
		inputVal.size = 0;

		//get new parameter values
		inputVal.type = Types[i];
		inputVal.paramType = ParamTypes[i];
		
		//start new parameter list
		procVal.arguments.push_back(inputVal);
		
		//add procedure as a global symbol to the outermost scope
		string symbolID = IDs[i];
		procVal.CallLabel = IDs[i];
		Scopes->addSymbol(symbolID, procVal, true);
	}
	return;	
}

//<program> ::= <program_header> <program_body>
void Parser::Program(){
	Scopes->newScope(); //Create new scope for the program
	declareRunTime(); //set up runtime functions as global in the outermost scope
	if( !ProgramHeader() ) ReportError("Expected program header");
	if( !ProgramBody() ) ReportError("Expected program body");
	if( !CheckToken(T_PERIOD) ) ReportWarning("expected '.' at end of program");
	if( CheckToken(T_EOF) ) Scopes->exitScope(); //exit program scope once program ends
		
	else ReportError("Found some tokens remaining in file when end of program was expected");
	return;
}

//<program_header> ::= program <identifier> is
bool Parser::ProgramHeader(){
	if(CheckToken(T_PROGRAM)){
		string id;
		if( Identifier(id) ){
			Scopes->ChangeScopeName("Program " + id);	
			if(CheckToken(T_IS)) return true;
			else{
				ReportError("expected 'is' after program identifier");
				return false;
			}
		}
		else{
			ReportError("expected program identifier after 'program'");
			return false;
		}
	}
	else return false;
}

/* <program_body> ::=
 *	{ <declaration> ;}*
 *	begin
 *	{ <statement> ;}*
 *	end program
 */
bool Parser::ProgramBody(){
	bool resyncEnabled = true;
	bool procDec = false;
	string labelBegin = generator->newLabel("Begin_Program");
	//Get Procedure and Variable Declarations
	while(true){
		while( Declaration(procDec) ){
			if(procDec){
				if( !CheckToken(T_SEMICOLON) ) ReportWarning("expected ';' after procedure declaration in procedure");
			}
			else if( !CheckToken(T_SEMICOLON) ) ReportLineError("expected ';' after variable declaration in procedure", true);
		}
		if(CheckToken(T_BEGIN)){
			// GEN: Place label to procedure entry and set the Stack Pointer
			generator->placeLabel(labelBegin);
			generator->setSPfromFP( Scopes->getFrameSize() );
			//reset resync for statements
			resyncEnabled = false;
			while(true){
				//get all valid statements
				while( Statement() ){
					if( !CheckToken(T_SEMICOLON) ) ReportLineError("Expected ';' at end of statement in program body.", true);
				}
				//get program body's end
				if(CheckToken(T_END)){
					if( CheckToken(T_PROGRAM) ) return true;
					else ReportError("Expected 'end program' to close program execution.");
				}
				//use up resync attempt if parser can't find a statement or 'end'
				else if(resyncEnabled){
					resyncEnabled = false;
					ReportLineError("Bad line. Expected Statement or 'END' reserved keyword in program body.");
				}
				//if resync failed, report a fatal error
				else ReportFatalError("Parser resync failed. Could not find another valid statement or end of program.");
			}
		}
		//use up resync attempt if parser can't find a declaration or 'begin'
		else if(resyncEnabled){
			resyncEnabled = false;
			ReportLineError("Bad line. Expected Declaration or 'BEGIN' reserved keyword in program body.");
		}
		//if resync failed, report a fatal error
		else{
			ReportFatalError("Parser resync failed. Could not find another valid declaration or start of program execution.");
		}
	}
}

/*	<declaration> ::=
 *		 {global} <procedure_declaration>
 *		|{global} <variable_declaration>
 */
bool Parser::Declaration(bool &procDec){
	bool global;
	string id;
	scopeValue newSymbol;
	
	/* Ensure that the arguments member and paramType are both cleared before starting. 
	 * None of these cases will set paramType, that will be set later
	 * ProcedureDeclaration will set the arguments based on the procedure's parameters. */
	newSymbol.arguments.clear();
	newSymbol.paramType = TYPE_PARAM_NULL;
	
	//determine if symbol declaration is global in scope
	if( CheckToken(T_GLOBAL) ) global = true;
	else global = false;

	//Determine if a procedure or variable declaration exists
	if( ProcedureDeclaration(id, newSymbol, global) ){
		Scopes->exitScope();
		Scopes->addSymbol(id, newSymbol, global);
		procDec = true;
		return true;
	}
	else if( VariableDeclaration(id, newSymbol) ){
		//Add symbol to current scope. VariableDeclaration will pass the symbol's type and size members.
		Scopes->addSymbol(id, newSymbol, global);
		return true;
	}
	else if( global ){
		ReportLineError("Bad line. Expected either a valid procedure or variable declaration after 'global' keyword");
		return false;
	}
	else return false;
}

//<variable_declarartion> ::= <type_mark><identifier>{ [<array_size>] }
bool Parser::VariableDeclaration(string &id, scopeValue &varEntry){
	//get variable type, otherwise no variable should be declared
	if( !TypeMark(varEntry.type) ) return false;
	else{
		//get variable identifier
		if( Identifier(id) ){
			// Get size for array variable declarations
			if(CheckToken(T_LBRACKET)){
				if(CheckToken(TYPE_INTEGER)){
					varEntry.size = token->val.intValue;
					if( !CheckToken(T_RBRACKET) ) ReportError("expected ']' at end of array variable declaration.");
					return true;
				}
				else{
					ReportLineError("Expected integer for array size in variable declaration.");
					return true;
				}
			}
			else{
				varEntry.size = 0;
				return true;
			}
		}
		else{
			ReportLineError("Bad line. Expected variable identifier for declaration after type mark.");
			return true;
		}
	}
}

/*	<type_mark> ::=
 *		 integer
 *		|float
 *		|bool
 *		|string
 *		|char
 */
bool Parser::TypeMark(int &type){
	if( CheckToken(T_INTEGER) ) type = TYPE_INTEGER;
	else if( CheckToken(T_FLOAT) ) type = TYPE_FLOAT;
	else if( CheckToken(T_BOOL) ) type = TYPE_BOOL;
	else if( CheckToken(T_STRING) ) type = TYPE_STRING;
	else if( CheckToken(T_CHAR) ) type = TYPE_CHAR;
	else return false;
	return true;
}

//<procedure_declaration> ::= <procedure_header><procedure_body>
bool Parser::ProcedureDeclaration(string &id, scopeValue &procDeclaration, bool global){
	//Get Procedure Header
	if( ProcedureHeader(id, procDeclaration, global) ){
		if( ProcedureBody() ) return true;
		else{
			ReportFatalError("Expected procedure body after procedure header.");
			return true;
		}
	}
	else return false;
}

//<procedure_header> ::= procedure <identifier> ( { <parameter_list> } )
bool Parser::ProcedureHeader(string &id, scopeValue &procDeclaration, bool global){
	if( CheckToken(T_PROCEDURE) ){
		//Create new scope in nested symbol tables for the procedure
		Scopes->newScope();
		
		//Set the symbol table entry's type and size to the correct values for a procedure
		procDeclaration.type = TYPE_PROCEDURE;
		procDeclaration.size = 0;
		
		//get procedure identifier and set value to be added to the symbol table
		if( Identifier(id) ){
			Scopes->ChangeScopeName(id);
			
			// Get unique label from code generator and place at the start of the procedure
			procDeclaration.CallLabel = generator->newLabel( "Procedure_" + id );
			generator->placeLabel( procDeclaration.CallLabel );
		
			//get parameter list for the procedure, if it has parameters
			if( CheckToken(T_LPAREN) ){
				ParameterList(procDeclaration);
				if( !CheckToken(T_RPAREN) ) ReportLineError("Bad Line. Expected ')' after parameter list in procedure header");
				Scopes->addSymbol(id, procDeclaration, global);
				return true;
				
			}
			else{
				ReportFatalError("Expected '(' in procedure header before parameter list.");
				return true;
			}
		}
		else{
			ReportFatalError("Expected procedure identifier in procedure header.");
			return true;
		}
	}
	else return false;
}

/*	<procedure_body> ::=
 *		{ <declaration> ; }*
 *		begin
 *		{ <statement> ; }*
 *		end procedure
 */
bool Parser::ProcedureBody(){
	bool resyncEnabled = true;
	bool procDec = false;
	//get symbol declarations for next procedure
	while(true){
		while( Declaration(procDec) ){
			if(procDec){
				if( !CheckToken(T_SEMICOLON) ) ReportWarning("expected ';' after procedure declaration in procedure");
			}
			else if( !CheckToken(T_SEMICOLON) ) ReportLineError("expected ';' after variable declaration in procedure", true);
		}
		
		// GEN: set the procedure's FP and SP in the program body after all declarations have been made
		// Each procedure declaration added its bytes to the frame size so this is the max size
		int frameSize = Scopes->getFrameSize();
		generator->setProcedurePointers(frameSize);
		
		//get statements for procedure body
		if( CheckToken(T_BEGIN) ){
			resyncEnabled = true;
			while(true){
				while( Statement() ){
					if( !CheckToken(T_SEMICOLON) ) ReportLineError("expected ';' after statement in procedure",true);
				}
				if( CheckToken(T_END) ){

					// Add label for procedure end and return
					string labelEnd;
					labelEnd = generator->newLabel("Procedure_End");
					generator->placeLabel( labelEnd );
					generator->ProcedureReturn();

					if( CheckToken(T_PROCEDURE) ) return true;
					else{
						ReportError("expected 'end procedure' at end of procedure declaration");
						return true;
					}
				}
				else if(resyncEnabled){
					resyncEnabled = false;
					ReportLineError("Bad line. Expected Statement or 'end' reserved keyword in procedure body.");
				}
				else{
					ReportFatalError("expected 'end procedure' at end of procedure declaration");
					return false;
				}
			}
		}
		else if(resyncEnabled){
			resyncEnabled = false;
			ReportLineError("Bad line. Expected Declaration or 'begin' reserved keyword in procedure body.");
		}
		else{
			ReportFatalError("Parser resync failed. Couldn't find a valid declaration or the 'begin reserved keyword in procedure body.");
			return false;
		}
		
	}

}

/* <procedure_call> ::= <identifier>( { <argument_list> } )
 * Note: the identifier is found in the previously called assignment statement and will have its value passed to the ProcedureCall. */
bool Parser::ProcedureCall(string id){
	//argument list whose type and size values will be compared against those declared in the procedure's parameter list
	vector<scopeValue> argList;
	scopeValue procedureCall;
	bool isGlobal;
	int offset = 2;
	bool found;
	
	//ensure an id was found in the assignment statement check called right before ProcedureCall, otherwise return false
	if( id == "" ) return false;
	
	// Get procedure's declared information from scope table
	found = Scopes->checkSymbol(id, procedureCall, isGlobal);
	
	//get argument list used in the procedure call
	if( CheckToken(T_LPAREN) ){
		ArgumentList(argList, procedureCall, offset);
		if( !CheckToken(T_RPAREN) ) ReportLineError("Expected ')' closing procedure call");
	}
	else ReportError("expected '(' in procedure call");
	
	// Compare called argument list against the declared parameter list
	if( found ){
		bool match = true;
		
		// Iterators to go through the parameter and argument lists
		vector<scopeValue>::iterator it1 = argList.begin();
		vector<scopeValue>::iterator it2 = procedureCall.arguments.begin();
		
		while( (it1 != argList.end()) && (it2 != procedureCall.arguments.end()) ){
			if( (it1->type != it2->type) || (it1->size != it2->size) ) match = false;
			++it1;
			++it2;
		}
		if( (it1 != argList.end()) || (it2 != procedureCall.arguments.end()) || (!match) ){
			ReportError("Procedure call argument list does not match declared parameter list.");
		}
		
		// Set procedure call, then pop arguments after call and reset the Frame and Stack pointers
		generator->callProcedure( procedureCall, id);
		generator->setSPfromFP( Scopes->getFrameSize() );
		generator->popArguments( offset );
		return true;
	}
	else{
		ReportError("Procedure: " + id + " was not declared in this scope.");
		return true;
	}
}

/*	<argument_list> ::=
 *	 <expression> , <argument_list>
 *	|<expression>
 */
bool Parser::ArgumentList(vector<scopeValue> &list, scopeValue procValue, int &offset){
	//create vector<scopeValue> where all argument list information will be stored and later returned by the function
	list.clear();

	/* argEntry variable will contain argument information about expression type and size. 
	 * All arguments will have an empty arguments vector and have their paramType = TYPE_PARAM_NULL
	 * The procedure call, when checking these arguments, will not compare the paramType value against the declared values */
	scopeValue argEntry;
	argEntry.arguments.clear();
	argEntry.paramType = TYPE_PARAM_NULL;

	//For each comma-separated expression, store the type and size values, in the order encountered, in the 'list' vector
	vector<scopeValue>::iterator it = procValue.arguments.begin();
	if( it != procValue.arguments.end() ){
		if( it->paramType == TYPE_PARAM_OUT) outArg = true;
	}
	
	if( Expression(argEntry.type, argEntry.size) ){
		// GEN: add arguments from register to correct frame
		
		offset = 2;
		generator->pushArgument(offset, it->paramType);
		++it;
		
		list.push_back(argEntry);
		while( CheckToken(T_COMMA) ){
			if( it != procValue.arguments.end() ){
				if( it->paramType == TYPE_PARAM_OUT ){
					outArg = true;
				}
				else outArg = false;
			}
			if( Expression(argEntry.type, argEntry.size) ){
				list.push_back(argEntry);
				// Add arguments from register to correct frame
				generator->pushArgument(offset, it->paramType);
				++it;
			}
			else ReportError("expected another argument after ',' in argument list of procedure call");
		}
	}
	outArg = false;
	return true;
}


/*	<parameter_list> ::=
 *		 <parameter> , <parameter_list>
 *		|<parameter>
 */
bool Parser::ParameterList(scopeValue &procEntry){
	if(Parameter(procEntry)){
		while(CheckToken(T_COMMA)){
			if( !Parameter(procEntry) ) ReportError("Expected parameter after ',' in procedure's parameter list");
		}
	}
	return true;
}

//<parameter> ::= <variable_declaration>(in | out | inout)
bool Parser::Parameter(scopeValue &procEntry){
	scopeValue paramEntry;
	string id;
	//get parameter declaration
	if( VariableDeclaration(id, paramEntry) ){
		//determine parameter's input / output type
		if(CheckToken(T_IN)) paramEntry.paramType = TYPE_PARAM_IN;
		else if(CheckToken(T_OUT)) paramEntry.paramType = TYPE_PARAM_OUT;
		else if(CheckToken(T_INOUT)) paramEntry.paramType = TYPE_PARAM_INOUT;
		else ReportError("expected 'in', 'out', or 'inout' in parameter declaration");
		
		//add parameter to current scope after updating the parameter type
		Scopes->addSymbol(id, paramEntry, false);
		
		//add to current procedure declaration's parameter list
		procEntry.arguments.push_back(paramEntry);
		
		return true;
	}
	else return false;
}

/*	<statement> ::=
 *		 <assignment_statement>
 *		|<if_statement>
 *		|<loop_statement>
 *		|<return_statement>
 *		|<procedure_call>
 */
bool Parser::Statement(){
	string id = "";
	if( IfStatement() ) return true;
	else if( LoopStatement() ) return true;
	else if( ReturnStatement() ) return true;
	else if( Assignment(id) ) return true;
	else if( ProcedureCall(id) ) return true;
	else return false;
}

// <assignment_statement> ::= <destination> := <expression>
bool Parser::Assignment(string &id){
	int type, size, dType, dSize;
	bool found;
	scopeValue destinationValue;
	bool isGlobal;
	bool indirect;
	int indirect_type;
	
	//Determine destination if this is a valid assignment statement
	if( !Destination(id, dType, dSize, destinationValue, found, isGlobal, indirect, indirect_type) ) return false;
	
	//get assignment expression
	if( CheckToken(T_ASSIGNMENT) ){
		Expression(type, size);

		if(found){
			if(size != dSize && (size > 1) && (dSize <= 1)) ReportError("Bad assignment, size of expression must match destination's size.");
			if( (type != dType) && ( (!isNumber(dType)) || (!isNumber(type)) ) ) ReportError("Bad assignment, type of expression must match destination");
		}
		// Move expression result from register(s) to MM using one of the various addressing modes.
		generator->reg2mm( type, dType, size, dSize, destinationValue.FPoffset, isGlobal, indirect, indirect_type );
		return true;
	}
	else{
		ReportLineError("Bad line. Expected ':=' after destination in assignment statement", false);
		return true;
	}
}

/* <destination> ::= <identifier> { [<expression] }
 * Returns the destination's identifier (will be used in procedure call if assignment fails).
 * Returns destination's type and size for comparison with what is being assigned to the destination. 
 */
bool Parser::Destination(string &id, int &dType, int &dSize, scopeValue &destinationValue, bool &found, bool &isGlobal, bool &indirect, int &indirect_type){
	//Variable to hold destination symbol's information from the nested scope tables
	int type, size;

	if( Identifier(id) ){
		found = Scopes->checkSymbol(id, destinationValue, isGlobal);

		/* if a procedure is found, return false. 
		 *This can't be a destination and the found id will be passed to a procedure call */
		if( (found) && (destinationValue.type == TYPE_PROCEDURE) ) return false;
		else if(!found){
			ReportError("Destination: " + id + " was not declared in this scope");
			dType = T_UNKNOWN;
			dSize = 0;
		}
		else{
			dType = destinationValue.type;
			dSize = destinationValue.size;
		}
		
		if(CheckToken(T_LBRACKET)){
			indirect = true;
			if( Expression(type, size) ){
				//ensure array index is a single numeric value
				indirect_type = type;
				if( size != 0 || ( (type != TYPE_FLOAT) && ( type != TYPE_INTEGER) && ( type != TYPE_BOOL) ) ) {
					ReportError("Destination array's index must be a scalar numeric value");
				}
				else dSize = 0;
				
				if( CheckToken(T_RBRACKET) ){
					return true;
				}
				else{
					ReportLineError("expected ']' after destination array's index");
					return true;
				}
			}
			else{
				ReportLineError("Bad Line. Expected scalar numeric expression in array index.");
				return true;
			}
		}
		else{
			indirect = false;
			return true;
		}
	}
	else return false;
}

/*	<if_statement> ::=
 *		if ( <expression> ) then { <statement> ; }+
 *		{ else { <statement> ; }+ }
 *		end if
 */
bool Parser::IfStatement(){
	/* flag to determine if at least one statement occured after 'then' and 'else' 
	 * type and size variables are unused but needed to perfrom the call to Expression(type, size) */
	bool flag;
	int type, size;
	string labelTrue, labelFalse, labelEnd;
	bool resyncEnabled = true;
	
	//determine if this is the start of an if statement
	if( !CheckToken(T_IF) ) return false;
	
	//get expression for conditional statement: '( <expression> )'
	if( !CheckToken(T_LPAREN) ) ReportLineError("expected '(' before condition in if statement");
	else if( !Expression(type, size) ) ReportLineError("Expected condition for if statement");
	else if (type != TYPE_BOOL) ReportLineError("Conditional expression in if statement must evaluate to type bool.");
	else if( !CheckToken(T_RPAREN) ) ReportLineError("expected ')' after condition in if statement");
	
	// GEN: Check result of expression
	labelTrue = generator->newLabel( "IfTrue" );
	labelFalse = generator->newLabel( "IfFalse" );
	labelEnd = generator->newLabel( "IfEnd" );
	generator->condBranch(labelTrue, labelFalse);;
	
	/* Get statements to be evaluated if the statement's expression evaluates to true. 
	 * There must be at least one statement following 'then'. */
	if( CheckToken(T_THEN) ){
		generator->placeLabel( labelTrue );
		flag = false;
		while(true){
			while( Statement() ){
				flag = true;
				if( !CheckToken(T_SEMICOLON) ) ReportLineError("expected ';' after statement in conditional statement's 'if' condition",true);
			}
			if( !flag ) ReportError("expected at least one statement after 'then' in conditional statement");
			
			// goto end if after true statement
			generator->branch(labelEnd);

			/* Get statements to be evaluated if the statement's expression evaluates to false. 
		 	 * There must be at least one statement if there is an 'else' case. */
			generator->placeLabel(labelFalse);
			if(CheckToken(T_ELSE)){
				flag = false;
				resyncEnabled = true;
				while(true){
					while( Statement() ){
						flag = true;
						if( !CheckToken(T_SEMICOLON) ) ReportLineError("Expected ';' after statement in conditional statement's 'else' condition.",true);
					}
					/* check for correct closure of statement: 'end if' */
					
					// goto end if after else statement
					generator->branch(labelEnd);

					if( CheckToken(T_END) ){
						generator->placeLabel(labelEnd);
						if( !flag ) ReportError("expected at least one statement after 'else' in conditional statement.");
						if( !CheckToken(T_IF) ) ReportFatalError("missing 'if' in the 'end if' closure of conditional statement");
						return true;
					}
					else if(resyncEnabled){
						resyncEnabled = false;
						ReportLineError("Bad Line. Unable to find valid statement or 'else' or 'end' reserved keywords.");
					}
					else{
						ReportFatalError("Parser resync failed. Unable to find valid statement, 'else' or 'end if' reserved keywords.");
						return true;
					}
				}
			}
			/* check for correct closure of statement: 'end if' */
			else if( CheckToken(T_END) ){
				generator->placeLabel(labelEnd);
				if( !CheckToken(T_IF) ) ReportFatalError("missing 'if' in the 'end if' closure of the if statement");
				return true;
			}
			else if(resyncEnabled){
				resyncEnabled = false;
				ReportLineError("Bad Line. Unable to find valid statement or 'else' or 'end' reserved keywords.");
			}
			else ReportFatalError("Parser resync failed. Unable to find valid statement, 'else' or 'end if' reserved keywords.");
		}
	}
	else ReportFatalError("Expected 'then' after condition in if statement.");
}

/*	<loop_statement> ::=
 *		for ( <assignment_statement> ; <expression> )
 *		{ <statement> ; }*
 *		end for
 */
bool Parser::LoopStatement(){
	int type, size;
	string id;
	bool resyncEnabled = true;
	
	//Determine if a loop statement is going to be declared
	if( !CheckToken(T_FOR) ) return false;
	string labelCond, labelTrue, labelEnd;
	labelCond = generator->newLabel("Loop_Check");
	labelTrue = generator->newLabel("Loop_True");
	labelEnd = generator->newLabel("Loop_End");
	
	generator->placeLabel(labelCond);

	/* get assignment statement and expression for loop: '( <assignment_statement> ; <expression> )'
	 * Throws errors if '(' or ')' is missing. Throws warnings for other missing components */
	if( !CheckToken(T_LPAREN) ) ReportFatalError("expected '(' before assignment and expression in for loop statement");

	if( !Assignment(id) ) ReportError("expected an assignment at start of for loop statement");

	if( !CheckToken(T_SEMICOLON) ) ReportError("expected ';' separating assignment statement and expression in for loop statement");

	if( !Expression(type, size) ) ReportError("expected a valid expression following assignment in for loop statement");
	else generator->condBranch( labelTrue, labelEnd );

	if( !CheckToken(T_RPAREN) ) ReportError("expected ')' after assignment and expression in for loop statement");
	generator->placeLabel(labelTrue);
	while(true){
		while( Statement() ){
			if( !CheckToken(T_SEMICOLON) ) ReportLineError("expected ';' after statement in for loop",true);
		}
		
		/* check for correct closure of loop: 'end loop' */
		generator->branch(labelCond);

		if( CheckToken(T_END) ){
			generator->placeLabel(labelEnd);
			if( !CheckToken(T_FOR) ) ReportError("missing 'for' in the 'end for' closure of the for loop statement");
			return true;
		}
		else if(resyncEnabled){
			resyncEnabled = false;
			ReportLineError("Bad line. Could not find a valid statement or 'end' reserved keyword in loop statement.");
		}
		else{
			ReportFatalError("expected 'end for' at end of for loop statement");
			return false;	
		}
	}
}

// <return_statement> ::= return
bool Parser::ReturnStatement(){
	if(CheckToken(T_RETURN)){
		generator->ProcedureReturn();
		return true;
	}
	else return false;
}

/*	<expression> ::=
 *		{ not } <arithOp> <expression'>
 */
bool Parser::Expression(int &type, int &size){
	//flag used to determine if an expression is required following a 'NOT' token
	bool notOperation;
	if( CheckToken(T_NOT) ) notOperation = true;
	else notOperation = false;
	
	//get type and size of first arithOp
	if( ArithOp(type, size) ){
		if( (notOperation) && (type != TYPE_BOOL) && (type != TYPE_INTEGER) )ReportError("'NOT' operator is defined only for type Bool and Integer.");
		else if (notOperation) generator->NotOnRegister(type, size);
		ExpressionPrime(type, size, true, true);
		return true;
	}
	else if (notOperation){
		ReportFatalError("Expected an integer / boolean ArithOp following 'NOT'");
		return true;
	}
	else{
		return false;
	}
}

/*  <expression'> ::=
 *  | 	& <arithOp> <expression'>
 *  | 	| <arithOp> <expression'>
 *  |	null
 */
bool Parser::ExpressionPrime(int &inputType, int &inputSize, bool catchTypeError, bool catchSizeError){
	int arithOpType, arithOpSize;
	
	// Get the operator to pass to the codegenerator evaluation stack
	string op = token->ascii;
	
	if( CheckToken(T_BITWISE) ){
		CheckToken(T_NOT); //'NOT' is always optional and will be good for both integer-bitwise and boolean-boolean expressions.
		if( ArithOp(arithOpType, arithOpSize) ){
			if( catchTypeError ){
				if(inputType == TYPE_INTEGER){
					if(arithOpType != TYPE_INTEGER){
						ReportError("Only integer ArithOps can be used for bitwise operators '&' and '|'.");
						catchTypeError = false;
					}
				}
				else if(inputType == TYPE_BOOL){
					if(arithOpType != TYPE_BOOL){
						ReportError("Only boolean ArithOps can be used for boolean operators '&' and '|'.");
						catchTypeError = false;
					}
				}
				else ReportError("Only integer / boolean ArithOps can be used for bitwise / boolean operators '&' and '|'.");
			}
			if( catchSizeError ){
				//Ensure compatible sizes are used.
				if( (inputSize != arithOpSize) && (inputSize != 0) && (arithOpSize != 0) ){
					ReportError("Expected ArithOp of size " + to_string(inputSize) + ", but found one of size " + to_string(arithOpSize) + ".");
					catchSizeError = false;
				}
				//Set new inputSize if the next ArithOp's size was non-zero and inputSize was zero.
				else if(arithOpSize != 0) inputSize = arithOpSize;
			}
		}
		else{
			ReportError("Expected ArithOp after '&' or '|' operator.");
			catchTypeError = false;
			catchSizeError = false;
		}
		ExpressionPrime(inputType, inputSize, catchTypeError, catchSizeError);
		
		// GEN: Evaluate expression and push register onto the stack to pass to upper expression
		// Do not need resulting string since expression result is pushed back onto the stack
		generator->evalExpr( op, inputSize, arithOpSize, inputType, arithOpType );
		
		return true;
	}
	else return false;
}

/*	<arithOp> ::=
 *		<relation> <arithOp'>
 */
bool Parser::ArithOp(int &type, int &size){
	if( Relation(type, size) ){
		ArithOpPrime(type, size, true, true);
		return true;
	}
	else return false;
}

/*  <ArithOp'> ::=
 *		|	+ <relation> <arithOp'>
 *		|	- <relation> <arithOp'>
 *		|	null
 */
bool Parser::ArithOpPrime( int &inputType, int &inputSize, bool catchTypeError, bool catchSizeError ){
	//Size and type of next Relation in the ArithOp sequence
	int relationType, relationSize;
	
	//Get operation to pass to the codegenerator
	string op = token->ascii;
	
	//if '+' or '-' can't be found then return false ('null'). Otherwise continue function.
	if( CheckToken(T_ADD) );
	else if( CheckToken(T_SUBTRACT) );
	else return false;
	
	//Get next Relation. Otherwise report Missing Relation error.
	if( Relation(relationType, relationSize)){
		//Only allow number (integer / float) relations in arithmetic operations.
		if(catchTypeError){
			if( !isNumber(relationType) || !isNumber(inputType) ){
				ReportError("Only integer and float values are allowed for arithmetic operations.");
				catchTypeError = false;
			}
		}
		//Ensure compatible sizes are used in ArithOp.
		if(catchSizeError){
			//Error if sizes are not identical and both are non-zero
			if( (inputSize != relationSize) && (inputSize != 0) && (relationSize != 0) ){
				ReportError("Expected Relation of size " + to_string(inputSize) + ", but found one of size " + to_string(relationSize) + ".");
				catchSizeError = false;
			}
			//Assign inputSize as exprSize if inputSize is non-zero.
			else if(relationSize != 0) inputSize = relationSize;
		}
	}
	else{
		ReportError("Expected relation after arithmetic operator.");
		catchTypeError = false;
		catchSizeError = false;
	}
	//Continue to look for other +/- <relation> in case there is a missing arithOp or doubled up arithmetic operators
	ArithOpPrime( inputType, inputSize, catchTypeError, catchSizeError );
	
	//Evaluate arith op and push to the stack
	generator->evalExpr( op, inputSize, relationSize, inputType, relationType );
	
	return true;
}

/*	<relation> ::=
 *		| <term> <relation'>
 */
bool Parser::Relation(int &type, int &size){
	if( Term(type, size) ){
		//Get relational operator if one exists. All relational operators return type bool.
		if( RelationPrime(type, size, true, true) ) type = TYPE_BOOL;
		return true;
	}
	else return false; 
}

/* <relation'> ::=
 *		| <  <term> <relation'> 
 *		| <= <term> <relation'> 
 *		| >  <term> <relation'> 
 *		| >= <term> <relation'> 
 *		| == <term> <relation'> 
 *		| != <term> <relation'> 
 *		| null
 */
bool Parser::RelationPrime(int &inputType, int &inputSize, bool catchTypeError, bool catchSizeError){
	//Type and size of relation being compared to.
	int termType, termSize;

	//get relationsal op to pass to the codegenerator evaluation stack
	string op = token->ascii;
	
	if (CheckToken(T_COMPARE)){
		//Get next term, otherwise report missing term error.
		if( Term(termType, termSize) ){
			if(catchTypeError){
				//Ensure both terms are of type integer or bool. The two types can be compared against each other.
				if( ((inputType != TYPE_BOOL) && (inputType != TYPE_INTEGER)) || ((termType != TYPE_BOOL) && (termType != TYPE_INTEGER)) ){
					ReportError("Relational operators are only valid for terms of type bool or integers '0' and '1'.");
					catchTypeError = false;
				}
			}
			if(catchSizeError){
				//Ensure compatible sizes are used.
				if( (inputSize != termSize) && ( (inputSize != 0) && (termSize != 0) ) ){
					ReportError("Expected term of size " + to_string(inputSize) + ", but found one of size " + to_string(termSize) + ".");
					catchSizeError = false;
				}
				//Assign termSize to inputSize if termSize is non-zero.
				else if(termSize != 0) inputSize = termSize;
			}
		}
		else{
			ReportError("Expected term after relational operator.");
			catchTypeError = false;
			catchSizeError = false;
		}
		//Check for another relational operator and term.
		RelationPrime(inputType, inputSize, catchTypeError, catchSizeError);
		
		//evaluate relation expression and push to the codegenerator evaluation stack
		generator->evalExpr( op, inputSize, termSize, inputType, termType );
		
		return true;
	}
	else return false;
}

/*	<term> ::=
 *		<factor> <term'>
 */
bool Parser::Term(int &type, int &size){
	if(Factor(type, size)){
		TermPrime(type, size, true, true);
		return true;
	}
	else return false;
}

/* <term'> ::=
 *		| * <factor> <term'>
 *		| / <factor> <term'>
 *		| null
 */
bool Parser::TermPrime(int &inputType, int &inputSize, bool catchTypeError, bool catchSizeError){
	//Next factor type and size.
	int factorType, factorSize;
	
	//get term op to pass to the code generator
	string op = token->ascii;
	
	//Check for '*' or '/' token, otherwise return false ('null').
	if( CheckToken(T_MULTIPLY) );
	else if( CheckToken(T_DIVIDE) );
	else return false;
	
	//Get next factor, otherwise report missing factor error.
	if( Factor(factorType, factorSize) ){
		//Ensure both factors are numbers for arithmetic operators.
		if( !isNumber(inputType) || !isNumber(factorType) ){
			ReportError("Only integer and float factors are defined for arithmetic operations in term.");
			catchTypeError = false;
		}
		if( (inputSize != factorSize) && ( (inputSize != 0) && (factorSize != 0) ) ){
			ReportError("Expected factor of size " + to_string(inputSize) + ", but found one of size " + to_string(factorSize) + ".");
			catchSizeError = false;
		}
		else if(factorSize != 0) inputSize = factorSize;
	}
	else{
		ReportError("Expected factor after arithmetic operator in term.");
		catchTypeError = false;
		catchSizeError = false;
	}
	TermPrime(inputType, inputSize, catchTypeError, catchSizeError);
	
	//evaluate term and push to the code generator expression stack
	generator->evalExpr( op, inputSize, factorSize, inputType, factorType );
	
	return true;
}

/*	<factor> ::=
 *		 ( <expression> )
 *		|{-} <name>
 *		|{-} <number>
 *		|<string>
 *		|<char>
 *		|false
 *		|true
 */
bool Parser::Factor(int &type, int &size){
	int tempType, tempSize;
	if( CheckToken(T_LPAREN) ){
		if( Expression(tempType, tempSize) ){
			type = tempType;
			size = tempSize;
			if( CheckToken(T_RPAREN) ){
				return true;
			}
			else ReportFatalError("expected ')' in factor around the expression");
		}
		else ReportFatalError("expected expression within parenthesis of factor");
	}
	else if( CheckToken(T_SUBTRACT) ){
		if( Integer() ){
			type = TYPE_INTEGER;
			size = 0;
			generator->NegateTopRegisters( type, size );
			return true;
		}
		else if( Float() ){
			type = TYPE_FLOAT;
			size = 0;
			generator->NegateTopRegisters( type, size );
			return true;
		}
		else if( Name(tempType, tempSize) ){
			type = tempType;
			size = tempSize;
			if( !isNumber(type) ) ReportError(" negation '-' before variable name is valid only for integers and floats.");
			generator->NegateTopRegisters( type, size );
			return true;
		}
		else return false;
	}
	else if( Name(tempType, tempSize) ){
		type = tempType;
		size = tempSize;
		return true;
	}
	else if( Integer() ){
		type = TYPE_INTEGER;
		size = 0;
		return true;
	}
	else if( Float() ){
		type = TYPE_FLOAT;
		size = 0;
		return true;
	}
	else if( String() ){
		type = TYPE_STRING;
		size = 0;
		return true;
	}
	else if( Char() ){
		type = TYPE_CHAR;
		size = 0;
		return true;
	}
	else if( Bool() ){
		type = TYPE_BOOL;
		size = 0;
		return true;
	}
	else{
		return false;
	}
}

// <name> ::= <identifier> { [ <epression> ] }
bool Parser::Name(int &type, int &size){
	string id;
	scopeValue nameValue;
	bool isGlobal;
	if( Identifier(id) ){
		bool symbolExists = Scopes->checkSymbol(id, nameValue, isGlobal);
		if(symbolExists){
			if(nameValue.type == TYPE_PROCEDURE){
				ReportError(id + " is a procedure in this scope, not a variable.");
			}
			else{
				size = nameValue.size;
				type = nameValue.type;
			}
		}
		else{
			ReportError(id + " has not been declared in this scope.");
			size = 0;
			type = T_UNKNOWN;
		}

		if( CheckToken(T_LBRACKET) ){
			if(nameValue.size == 0 && nameValue.type != TYPE_PROCEDURE) ReportError(id + " is not an array");
			int type2, size2;
			if( Expression(type2, size2) ){
				if( (size2 > 1) || ( (type2 != TYPE_INTEGER) && (type2 != TYPE_FLOAT) && (type2 != TYPE_BOOL) ) )
					ReportError("Array index must be a scalar numeric value.");
				size = 0;
				if( CheckToken(T_RBRACKET) ){
					// Save the variable <name> values in the generator to be used for arguments
					generator->setArgument( nameValue, isGlobal, 0, true );
					
					// MM to REG for specific array element in IN/INOUT argument
					if( !outArg ) generator->mm2reg(type, size, nameValue.FPoffset, isGlobal, -1, true, type2);
					
					return true;
				}
				else ReportError("Expected ']' after expression in name.");
			}
			else ReportFatalError("Expected expression between brackets.");
		}
		else{
			// Save the variable <name> values in the generator to be used for arguments
			generator->setArgument( nameValue, isGlobal);
			
			// MM to REG for scalars / full arrays that are IN/INOUT arguments
			 if( !outArg ) generator->mm2reg(type, size, nameValue.FPoffset, isGlobal);				
			return true;

		}
	}
	else return false;
}

// <number> ::= [0-9][0-9]*[.[0-9]*]
bool Parser::Number(){
	if( Integer() ) return true;
	else if( Float() ) return true;
	else return false;
}

// <integer> ::= [0-9][0-9]*
bool Parser::Integer(){
	string val = token->ascii;
	if(CheckToken(TYPE_INTEGER)){
		generator->val2reg( val, TYPE_INTEGER );
		return true;
	}
	else return false;
}

// <float> ::= [0-9][0-9]*[.[0-9]+]
bool Parser::Float(){
	string val = token->ascii;
	if(CheckToken(TYPE_FLOAT)){
		generator->val2reg( val, TYPE_FLOAT );
		return true;
	}
	else return false;
}

// <string> ::= "[a-zA-Z0-9_,;:.']*"
bool Parser::String(){
	string val = token->ascii;
	if(CheckToken(TYPE_STRING)){
		generator->val2reg( val, TYPE_STRING );
		return true;
	}
	else return false;
}

// <char> ::= '[a-zA-Z0-9_,;:."]'
bool Parser::Char(){
	string val = token->ascii;
	if(CheckToken(TYPE_CHAR)){
		generator->val2reg( val, TYPE_CHAR);
		return true;
	}
	else return false;
}

bool Parser::Bool(){
	string val = token->ascii;
	if(CheckToken(T_TRUE)){
		generator->val2reg( "1", TYPE_BOOL);
		return true;
	}
	else if(CheckToken(T_FALSE)){
		generator->val2reg( "0", TYPE_BOOL);
		return true;
	}
	else return false;
}

// <identifier> ::= [a-zA-Z][a-zA-Z0-9_]*
bool Parser::Identifier(string &id){
	string tmp = token->ascii;
	bool ret_val = CheckToken(TYPE_IDENTIFIER);
	if(ret_val){
		id = tmp;
		return true;
	}
	else return false;
}

//Check if token is an integer or float.
bool Parser::isNumber(int &type_value){
	if( (type_value == TYPE_INTEGER) || (type_value == TYPE_FLOAT)) return true;
	else return false;
}
