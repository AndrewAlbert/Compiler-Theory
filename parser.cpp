#include "parser.h"
#include "scope.h"
#include "scopeTracker.h"
#include "macro.h"
#include <string>
#include <stdlib.h>
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
Parser::Parser(token_type* headptr, scopeTracker* scopes){
	//Set values needed to begin parsing
	token = headptr;
	prev_token = nullptr;
	Scopes = scopes;
	warning = false;
	error = false;
	textLine = "";
	currentLine = 0;
	
	//Start program parsing
	Program();
	
	if(token == nullptr) cout << "Program parsing completed" << endl;
	else ReportError("Tokens remaining. Parsing failed.");
	
	if(error) DisplayWarningQueue();
}

Parser::~Parser(){

}

//Report error and terminate parsing.
void Parser::ReportFatalError(string message){
	warning_queue.push("Error: line - " + to_string(currentLine) + "\n\t" + message + "\n\tFound: " + textLine + " " + token->ascii);
	error = true;
	DisplayWarningQueue();
	exit(EXIT_FAILURE);
}

//report error line number and desriptive message. Get tokens until the next line or a ';' is found.
void Parser::ReportLineError(string message){
	warning_queue.push("Error: line - " + to_string(currentLine) + "\n\t" + message + "\n\tFound: " + textLine + " " + token->ascii);
	error = true;
	bool getNext = true;
	while(getNext){
		token = token->next;
		if(token->line != currentLine) getNext = false;
		else if( CheckToken(T_SEMICOLON) ) getNext = false;
	}
	currentLine = token->line;
	return;
}

//report error line number and descriptive message
void Parser::ReportError(string message){
	warning_queue.push("Error: line - " + to_string(currentLine) + "\n\t" + message + "\n\tFound: " + textLine + " " + token->ascii);
	error = true;
	return;
}

//report warning and descriptive message
void Parser::ReportWarning(string message){
	warning_queue.push("Warning: line - " + to_string(currentLine) + "\n\t" + message + "\n\tFound: " + textLine + " " + token->ascii);
	warning = true;
	return;
}

//display all of the stored warnings after parsing is complete or a fatal error occurs
void Parser::DisplayWarningQueue(){
	while(!warning_queue.empty()){
		cout << warning_queue.front() << endl;
		warning_queue.pop();
	}
	return;
}

//check if current token is the correct type, if so get next
bool Parser::CheckToken(int type){
	//skip all comment tokens
	while (token->type == T_COMMENT){
		prev_token = token;
		token = token->next;
	}

	if(token->line != currentLine ){
		currentLine = token->line;
		textLine = "";
	}
	//check that current token matches input type, if so move to next token
	if(token->type == type){
		prev_token = token;
		token = token->next;
		return true;
	}
	else{
		if(token->type == T_UNKNOWN){
			ReportError("Found unknown token");
			token = token->next;
		}
		return false;
	}
}

void Parser::declareRunTime(){
	//Procedure to be added to the symbol tables
	scopeValue procVal;
	procVal.size = 0;
	procVal.type = TYPE_PROCEDURE;
	procVal.paramType = TYPE_PARAM_NULL;
	
	//input / output parameter of the procedure
	scopeValue inputVal;
	inputVal.arguments.clear();
	
	string IDs[10] = {"GETBOOL", "GETINTEGER", "GETFLOAT", "GETSTRING", "GETCHAR", "PUTBOOL", "PUTINTEGER", "PUTFLOAT", "PUTSTRING", "PUTCHAR"};
	int ParamTypes[10] = {TYPE_PARAM_OUT, TYPE_PARAM_OUT, TYPE_PARAM_OUT, TYPE_PARAM_OUT, TYPE_PARAM_OUT, TYPE_PARAM_IN, TYPE_PARAM_IN, TYPE_PARAM_IN, TYPE_PARAM_IN, TYPE_PARAM_IN};
	int Types[10] = {TYPE_BOOL, TYPE_INTEGER, TYPE_FLOAT, TYPE_STRING, TYPE_CHAR, TYPE_BOOL, TYPE_INTEGER, TYPE_FLOAT, TYPE_STRING, TYPE_CHAR};

	for(int i = 0; i < 10; i++){
		//clear parameter list
		procVal.arguments.clear();
		
		if(Types[i] == TYPE_STRING) inputVal.size = STRING_SIZE;
		else inputVal.size = 0;

		//get new parameter values
		inputVal.type = Types[i];
		inputVal.paramType = ParamTypes[i];
		
		//start new parameter list
		procVal.arguments.push_back(inputVal);
		
		//add procedure as a global symbol to the outermost scope
		string symbolID = IDs[i];
		Scopes->addSymbol(symbolID, procVal, true);
	}
	return;	
}

//<program> ::= <program_header> <program_body>
void Parser::Program(){
	Scopes->newScope("program"); //Create new scope for the program
	declareRunTime(); //set up runtime functions as global in the outermost scope
	if( !ProgramHeader() ) ReportError("Expected program header");
	if( !ProgramBody() ) ReportError("Expected program body");
	if( !CheckToken(T_PERIOD) ) ReportWarning("expected '.' at end of program");
	if( CheckToken(T_EOF) ) Scopes->exitScope(); //exit program scope once program ends
	else ReportError("Found some tokens remaining in file when end of program was expected");
}

//<program_header> ::= program <identifier> is
bool Parser::ProgramHeader(){
	if(CheckToken(T_PROGRAM)){
		if( Identifier() ){
			if(CheckToken(T_IS)) return true;
			else{
				ReportError("expected 'is' after program identifier");
				return false;
			}
		}
		else{
			ReportError("expected program identifier");
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
	//Get Procedure and Variable Declarations
	while(true){
		while( Declaration() ){
			if( !CheckToken(T_SEMICOLON) ) ReportError("Expected ';' at end of declaration in program body");
		}
		if(CheckToken(T_BEGIN)){
			//reset resync for statemnets
			resyncEnabled = true;
			while(true){
				//get all valid statements
				while( Statement() ){
					if( !CheckToken(T_SEMICOLON) ) ReportError("Expected ';' at end of statement in program body.");
				}
				//get program body's end
				if(CheckToken(T_END)){
					if( CheckToken(T_PROGRAM) ) return true;
					else ReportError("Expected 'end program' to close program execution.");
				}
				//use up resync attempt if parser can't find a statement or 'end'
				else if(resyncEnabled){
					resyncEnabled = false;
					ReportLineError("Bad line. Expected Statement or 'END' reserved keyword in program body.")
				}
				//if resync failed, report a fatal error
				else ReportFatalError("Parser resync failed. Could not find another valid statement or end of program.");
			}
		}
		//use up resync attempt if parser can't find a declaration or 'begin'
		else if(resyncEnabled){
			resyncEnabled = false;
			ReportLineError("Bad line. Expected Declaration or 'BEGIN' reserved keyword in program body.")
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
bool Parser::Declaration(){
	bool global;
	string id;
	scopeValue newSymbol;
	
	//ensure that the arguments member and paramType are both cleared before starting. None of these cases will set paramType.
	//ProcedureDeclaration will set the arguments based on the procedure's parameters
	newSymbol.arguments.clear();
	newSymbol.paramType = TYPE_PARAM_NULL;
	
	//determine if symbol declaration is global in scope
	if( CheckToken(T_GLOBAL) ) global = true;
	else global = false;

	//Determine if a procedure or variable declaration exists
	if( ProcedureDeclaration(id, newSymbol, global) ){
		Scopes->exitScope();
		Scopes->addSymbol(id, newSymbol, global);
		return true;
	}
	else if( VariableDeclaration(id, newSymbol) ){
		//Add symbol to current scope. VariableDeclaration will assign the symbol's type and size members.
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
		if( Identifier() ){
			//get variable identifier
			id = prev_token->ascii;
			//get size for array variables
			if(CheckToken(T_LBRACKET)){
				if(CheckToken(TYPE_INTEGER)){
					varEntry.size = prev_token->val.intValue;
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
		Scopes->newScope("procedure");
		
		//Set the symbol table entry's type and size to the correct values for a procedure
		procDeclaration.type = TYPE_PROCEDURE;
		procDeclaration.size = 0;
		
		//get procedure identifier and set value to be added to the symbol table
		if( Identifier() ){
			id = prev_token->ascii;
			Scopes->setName(id);
			
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
	//get symbol declarations for next procedure
	while(true){
		while( Declaration() ){
			if( !CheckToken(T_SEMICOLON) ) ReportError("expected ';' after declaration in procedure");
		}
		
		//get statements for procedure body
		if( CheckToken(T_BEGIN) ){
			resyncEnabled = true;
			while(true){
				while( Statement() ){
					if( !CheckToken(T_SEMICOLON) ) ReportError("expected ';' after statement in procedure");
				}
				if( CheckToken(T_END) ){
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
					return true;
				}
			}
		}
		else if(resyncEnabled){
			resyncEnabled = false;
			ReportLineError("Bad line. Expected Declaration or 'begin' reserved keyword in procedure body.");
		}
		else{
			ReportFatalError("Parser resync failed. Couldn't find a valid declaration or the 'begin reserved keyword in procedure body.");
			return true;
		}
		
	}

}

/* <procedure_call> ::= <identifier>( { <argument_list> } )
 * Note: the identifier is found in the previously called assignment statement and will have its value passed to the ProcedureCall. */
bool Parser::ProcedureCall(string id){
	//argument list whose type and size values will be compared against those declared in the procedure's parameter list
	vector<scopeValue> argList;
	
	//variable to store information about the procedure declaration from the symbol table, if one exists
	scopeValue procedureCall;
	
	//ensure an id was found in the assignment statement check called right before ProcedureCall
	if(id == "") return false;
	
	//get argument list used in the procedure call
	if( CheckToken(T_LPAREN) ){
		ArgumentList(argList);
		if( !CheckToken(T_RPAREN) ) ReportLineError("Expected ')' closing procedure call");
	}
	else ReportError("expected '(' before procedure call's argument list"){
		ArgumentList(argList);
		if( !CheckToken(T_RPAREN) ) ReportLineError("Expected '" + id + "( <argument list> )' as procedure call");
	}
	
	//check symbol tables for the correct procedure declaration and compare the argument and parameter lists
	if( Scopes->checkSymbol(id, procedureCall) ){
		bool match = true;
		vector<scopeValue>::iterator it1 = argList.begin();
		vector<scopeValue>::iterator it2 = procedureCall.arguments.begin();
		while( (it1 != argList.end()) && (it2 != procedureCall.arguments.end()) ){
			if( (it1->type != it2->type) || (it1->size != it2->type) ) match = false;
			++it1;
			++it2;
		}
		if( (it1 != argList.end()) || (it2 != procedureCall.arguments.end()) || (!match) ){
			ReportError("Procedure call argument list does not match declared parameter list.");
		}
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
bool Parser::ArgumentList(vector<scopeValue> &list){
	//create vector<scopeValue> where all argument list information will be stored and later returned by the function
	list.clear();

	/* argEntry variable will contain argument information about expression type and size. 
	 * All arguments will have an empty arguments vector and have their paramType = TYPE_PARAM_NULL
	 * The procedure call, when checking these arguments, will not compare the paramType value against the declared values */
	scopeValue argEntry;
	argEntry.arguments.clear();
	argEntry.paramType = TYPE_PARAM_NULL;

	//For each comma-separated expression, store the type and size values, in the order encountered, in the 'list' vector
	if( Expression(argEntry.type, argEntry.size) ){
		list.push_back(argEntry);
		while( CheckToken(T_COMMA) ){
			if( Expression(argEntry.type, argEntry.size) ) list.push_back(argEntry);
			else ReportError("expected another argument after ',' in argument list of procedure call");
		}
	}
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
	
	//Determine destination if this is a valid assignment statement
	if( !Destination(id, dType, dSize) ) return false;
	
	//get assignment expression
	if( CheckToken(T_ASSIGNMENT) ){
		Expression(type, size);
		if(size != dSize){
			ReportError("Bad assignment, size of expression must match destination's size.");
		}
		if( (type != dType) && !( ( (type == TYPE_FLOAT) && (dType == TYPE_INTEGER) ) || ( (type == TYPE_INTEGER) && (dType == TYPE_FLOAT) ) ) ) ReportWarning("Bad assignment, type must match destination");
		return true;
	}
	else{
		ReportLineError("Bad line. Expected ':=' after destination in assignment statement");
		return true;
	}
}

/* <destination> ::= <identifier> { [<expression] }
 * Returns the destination's identifier (will be used in procedure call if assignment fails).
 * Returns destination's type and size for comparison with what is being assigned to the destination. */
bool Parser::Destination(string &id, int &dType, int &dSize){
	//Variable to hold destination symbol's information from the nested scope tables
	scopeValue destinationValue;
	bool found;
	int type, size;
	
	if( Identifier() ){
		id = prev_token->ascii;
		found = Scopes->checkSymbol(id, destinationValue);
		//if a procedure is found, return false. This can't be a destination and the found id will be passed to a procedure call
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
			if( Expression(type, size) ){
				//ensure array index is a single numeric value
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
		else return true;
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
	bool resyncEnabled = true;
	
	//determine if this is the start of an if statement
	if( !CheckToken(T_IF) ) return false;
	
	//get expression for conditional statement: '( <expression> )'
	if( !CheckToken(T_LPAREN) ) ReportError("expected '(' before condition in if statement");
	
	if( !Expression(type, size) ) ReportError("Expected condition for if statement");
	else if (type != TYPE_BOOL) ReportError("Conditional expression in if statement must evaluate to type bool.");
	
	if( !CheckToken(T_RPAREN) ) ReportError("expected ')' after condition in if statement");
	
	
	/* Get statements to be evaluated if the statement's expression evaluates to true. 
	 * There must be at least one statement following 'then'. */
	if( CheckToken(T_THEN) ){
		flag = false;
		while(true){
			while( Statement() ){
				flag = true;
				if( !CheckToken(T_SEMICOLON) ) ReportError("expected ';' after statement in if statement");
			}
			if( !flag ) ReportError("expected at least one statement after 'then' in if statement");
			/* Get statements to be evaluated if the statement's expression evaluates to false. 
		 	 * There must be at least one statement if there is an 'else' case. */
			if(CheckToken(T_ELSE)){
				flag = false;
				resyncEnabled = true;
				while(true){
					while( Statement() ){
						flag = true;
						if( !CheckToken(T_SEMICOLON) ) ReportError("Expected ';' after statement in if statement.");
					}
					/* check for correct closure of statement: 'end if' */
					else if( CheckToken(T_END) ){
						if( !flag ) ReportError("expected at least one statement after 'else' in if statement.");
						if( !CheckToken(T_IF) ) ReportFatalError("missing 'if' in the 'end if' closure of the if statement");
						return true;
					}
					else if(resyncEnabled){
						resyncEnabled = false;
						ReportLineError("Bad Line. Unable to find valid statement or 'else' or 'end' reserved keywords.")
					}
					else ReportFatalError("Parser resync failed. Unable to find valid statement, 'else' or 'end if' reserved keywords.");
				}
			}
			/* check for correct closure of statement: 'end if' */
			else if( CheckToken(T_END) ){
				if( !CheckToken(T_IF) ) ReportFatalError("missing 'if' in the 'end if' closure of the if statement");
				return true;
			}
			else if(resyncEnabled){
				resyncEnabled = false;
				ReportLineError("Bad Line. Unable to find valid statement or 'else' or 'end' reserved keywords.")
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
	
	/* get assignment statement and expression for loop: '( <assignment_statement> ; <expression> )'
	 * Throws errors if '(' or ')' is missing. Throws warnings for other missing components */
	if( !CheckToken(T_LPAREN) ) ReportFatalError("expected '(' before assignment and expression in for loop statement");
	if( !Assignment(id) ) ReportError("expected an assignment at start of for loop statement");
	if( !CheckToken(T_SEMICOLON) ) ReportError("expected ';' separating assignment statement and expression in for loop statement");
	if( !Expression(type, size) ) ReportError("expected a valid expression following assignment in for loop statement");
	if( !CheckToken(T_RPAREN) ) ReportError("expected ')' after assignment and expression in for loop statement");
	
	while(true){
		while( Statement() ){
			if( !CheckToken(T_SEMICOLON) ) ReportWarning("expected ';' after statement in for loop");
		}
		
		/* check for correct closure of loop: 'end loop' */
		if( CheckToken(T_END) ){
			if( !CheckToken(T_FOR) ) ReportError("missing 'for' in the 'end for' closure of the for loop statement");
			return true;
		}
		else if(resyncEnabled){
			resyncEnabled = false
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
	if(CheckToken(T_RETURN)) return true;
	else return false;
}

/*	<expression> ::=
 *		 <expression> & <arithOp>
 *		|<expression> | <arithOp>
 *		|{ not } <arithOp>
 */
bool Parser::Expression(int &type, int &size){
	//temporary type and size variables to evaluate the Expression's resulting type and size
	int type1, type2, size1, size2;
	
	//flag used to determine if an expression is required following a 'NOT' token
	bool notSymbol = false;
	if( CheckToken(T_NOT) ) notSymbol = true;
	
	//get type and size of first arithOp
	if( ArithOp(type1, size1) ){
		type = type1;
		size = size1;
		//if a bitwise logical statement occurs in the expression, all ArithOps must be integer values
		while( CheckToken(T_BITWISE) ){
			if( CheckToken(T_NOT) ) notSymbol = true;
			else notSymbol = false;
			
			//get type and size of each additional arithOps
			if( !ArithOp(type2, size2) ) ReportFatalError("expected ArithOp");
			if( (type1 == TYPE_INTEGER) && (type2 != TYPE_INTEGER) ) ReportError("only integer values are allowed for bitwise expressions");
			else if((type1 == TYPE_BOOL) && (type2 != TYPE_BOOL)) ReportError("only boolean values are allowed for logical '|' and '&'");
			else type = TYPE_BOOL;
			
			
			//ensure ArithOp sizes match for arrays, scalar values (size = 0) will always be compatible
			if( size1 != 0 && size2 != 0 && size1 != size2) ReportError("incompatible array sizes in bitwise expression");
			else if(size2 != 0) size1 = size2;
		}
		//return expression's final type and size
		//type = type1;
		//size = size1;
		return true;
	}
	else if (notSymbol) ReportFatalError("expected an ArithOp following 'NOT'");
	else return false;
}

/*	<arithOp> ::=
 *		<arithOp> + <relation>
 *		<arithOp> - <relation>
 *		<relation>
 */
bool Parser::ArithOp(int &type, int &size){
	int type1, type2, size1, size2;
	bool next;
	
	if( Relation(type1, size1) ){
		next = false;
		if(CheckToken(T_ADD)){	
			if( !(type1 == TYPE_INTEGER || type1 == TYPE_FLOAT) ) ReportError("only float and integer values are allowed for arithmetic operations' term1");
			next = true;
		}
		else if(CheckToken(T_SUBTRACT)){
			if( !(type1 == TYPE_INTEGER || type1 == TYPE_FLOAT) ) ReportError("only float and integer values are allowed for arithmetic operations' term1");
			next = true;
		}
		while(next){
			if( !Relation(type2, size2) ) ReportFatalError("expected Relation (+/-) as part of ArithOp");
			if( !(type2 == TYPE_INTEGER || type2 == TYPE_FLOAT) ) ReportError("only float and integer values are allowed for arithmetic operations' term2");
			else{
				if( (size1 != 0) && (size2 != 0) && (size1 != size2) ) ReportError("incompatible array sizes");
				else if(size2 != 0) size1 = size2;
				
				if(CheckToken(T_ADD));
				else if(CheckToken(T_SUBTRACT));
				else next = false;
			}
		}
		type = type1;
		size = size1;
		return true;
	}
	else return false;
}

/*	<relation> ::=
 *		 <relation> < <term>
 *		|<relation> <= <term>
 *		|<relation> > <term>
 *		|<relation> >= <term>
 *		|<relation> == <term>
 *		|<relation> != <term
 *		|<term>
 */
bool Parser::Relation(int &type, int &size){
	int type1, type2, size1, size2;
	bool next = false;
	if( Term(type1, size1) ){
		type = type1;
		size = size1;
		if( CheckToken(T_COMPARE) ){
			if(type1 == TYPE_INTEGER || type1 == TYPE_BOOL){
				type1 = TYPE_BOOL;
				next = true;
			}
			else ReportError("can only use integers '0' and '1' or booleans for relations");
		}
		while(next){
			//get term 2 for the relation
			if(!Term(type2, size2)) ReportFatalError("expected term");
			
			//check that term 2 is an integer or boolean value
			if(!(type2 == TYPE_INTEGER || type2 == TYPE_BOOL)) ReportError("can only use integers '0' and '1' or booleans for relations");
			
			//if term 1 is an array, check that term 2 is a scalar or an identically sized array
			if(size1 != 0){
				if(size2 != 0 && size2 != size1) ReportError("incompatible array size");
			}
			//if term 2 is an array, now we are dealing with a problem involving array sizes
			else if(size2 != 0){
				size1 = size2;
			}
			if(!CheckToken(T_COMPARE)){
				next = false;
			}
		}
		type = type1;
		size = size1;
		return true;
	}
	else return false; 
}

/*	<term> ::=
 *		 <term> * <factor>
 *		|<term> / <factor>
 *		|<factor>
 */
bool Parser::Term(int &type, int &size){
	int type1, type2, size1, size2;
	if(Factor(type1, size1)){
		bool next = false;
		if( CheckToken(T_MULTIPLY) ) next = true;
		else if( CheckToken(T_DIVIDE) ) next = true;

		if(next){
			//only allow arithmetic operations on integers and floats (conversion is allowed between the two)
			if(!(type1 == TYPE_FLOAT || type1 == TYPE_INTEGER)) ReportError("only integer and float factors allowed preceeding arithmetic operator");
			while(next){
				if(!Factor(type2, size2)) ReportFatalError("expected factor");
				
				if(!(type2 == TYPE_FLOAT || type2 == TYPE_INTEGER)) ReportError("expected integer or float factor in term after arithmetic operator");
				else if(size1 == 0 && size2 != 0) size1 = size2;
				else if(size1 != 0 && size2 != 0 && size1 != size2) ReportError("incompatiable array sizes");
				
				if(type1 != TYPE_FLOAT && type2 == TYPE_FLOAT) type1 = type2;
				
				if(CheckToken(T_MULTIPLY)) continue;
				else if(CheckToken(T_DIVIDE)) continue;
				else next = false;
			}
		}
		type = type1;
		size = size1;
		return true;
	}
	else return false;
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
			if( CheckToken(T_RPAREN) ){
				type = tempType;
				size = tempSize;
				return true;
			}
			else ReportFatalError("expected ')' in factor around the expression");
		}
		else ReportFatalError("expected expression within parenthesis of factor");
	}
	else if( CheckToken(T_SUBTRACT) ){
		if( Integer() ) type = TYPE_INTEGER;
		else if( Float() ) type = TYPE_FLOAT;
		else if( Name(tempType, tempSize) ){
			type = tempType;
			size = tempSize;
			return true;
		}
		else return false;
	}
	else if( Name(tempType, tempSize) ){
		type = tempType;
		size = tempSize;
		return true;
	}
	else if( Integer() ) type = TYPE_INTEGER;
	else if( Float() ) type = TYPE_FLOAT;
	else if( String() ){
		type = TYPE_STRING;
		size = STRING_SIZE;
		return true;
	}
	else if( Char() ) type = TYPE_CHAR;
	else if( CheckToken(T_FALSE) ) type = TYPE_BOOL;
	else if( CheckToken(T_TRUE) ) type = TYPE_BOOL;
	else return false;
	
	//size of 0 for all scalar factors, any potentially non-scalar factors already hit a return true statement before this
	size = 0;
	return true;
}

// <name> ::= <identifier> { [ <epression> ] }
bool Parser::Name(int &type, int &size){
	if( Identifier() ){
		string id = prev_token->ascii;
		scopeValue nameValue;
		bool symbolExists = Scopes->checkSymbol(id, nameValue);
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
			if(nameValue.size == 0 && nameValue.type != TYPE_PROCEDURE) ReportWarning(id + " is not an array");
			int type2, size2;
			if( Expression(type2, size2) ){
				if( (size2 > 1) || ( (type2 != TYPE_INTEGER) && (type2 != TYPE_FLOAT) && (type2 != TYPE_BOOL) ) )
					ReportError("Array index must be a scalar numeric value.");
				size = 0;
				if( CheckToken(T_RBRACKET) ) return true;
				else ReportWarning("Expected ']' after expression in name.");
			}
			else ReportFatalError("Expected expression between brackets.");
		}
		else return true;
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
	return CheckToken(TYPE_INTEGER);
}

// <float> ::= [0-9][0-9]*[.[0-9]+]
bool Parser::Float(){
	return CheckToken(TYPE_FLOAT);
}

// <string> ::= "[a-zA-Z0-9_,;:.']*"
bool Parser::String(){
	return CheckToken(TYPE_STRING);
}

// <char> ::= '[a-zA-Z0-9_,;:."]'
bool Parser::Char(){
	return CheckToken(TYPE_CHAR);
}

// <identifier> ::= [a-zA-Z][a-zA-Z0-9_]*
bool Parser::Identifier(){
	return CheckToken(TYPE_IDENTIFIER);
}
