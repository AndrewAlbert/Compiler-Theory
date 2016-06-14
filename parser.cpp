#include "parser.h"
#include "scope.h"
#include "scopeTracker.h"
#include "macro.h"
#include<string>
#include<stdlib.h>
#include<iostream>

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
	token = headptr;
	prev_token = nullptr;
	Scopes = scopes;
	warning = false;
	error = false;
	Program();
}

Parser::~Parser(){

}

//report error line number and descriptive message
void Parser::ReportError(string message){
	cout << "Error: " << message << " at line: " << token->line << endl;
	cout << "Found: " << token->ascii << endl;
	error = true;
	DisplayWarningQueue();
	exit(EXIT_FAILURE);
}

//report error line number and descriptive message
void Parser::ReportWarning(string message){
	string msg = "Warning: " + message + " at line: " + token->line + "\n\tFound: " << token->ascii;
	warning_queue.push(msg);
	warning = true;
}

//display all of the stored warnings after parsing is complete or a fatal error occurs
void Parser::DisplayWarningQueue(){
	while(!warning_queue.empty()){
		cout << warning_queue.front() << endl;
		warning_queue.pop();
	}
}

//check if current token is the correct type, if so get next
bool Parser::CheckToken(int type){
	//skip all comment tokens
	while (token->type == T_COMMENT){
		prev_token = token;
		token = token->next;
	}
	//check that current token matches input type, if so move to next token
	if(token->type == type){
		prev_token = token;
		token = token->next;
		return true;
	}
	else return false;
}

//clear the variables used when adding/checking to/from the symbol tables for variables
void Parser::clearScopeVals(){
	ScopeValue.type = T_UNKNOWN;
	ScopeValue.size = 0;
	ScopeValue.arguments.erase(ScopeValue.arguments.begin(), ScopeValue.arguments.end());
	ScopeGlobal = false;
	ScopeIdentifier = "";
}

//clear the variables used when adding/checking to/from the symbol tables for procedures -- helps deal with parameter declarations
void Parser::clearProcVals(){
	ProcValue.type = T_UNKNOWN;
	ProcValue.size = 0;
	ProcValue.arguments.erase(ProcValue.arguments.begin(), ProcValue.arguments.end());
	ProcGlobal = false;
	ProcIdentifier = "";
}

//<program> ::= <program_header><program_body>
void Parser::Program(){
	Scopes->newScope(); //Create new scope for the program
	ProgramHeader();
	ProgramBody();
	if(!CheckToken(T_PERIOD)) ReportWarning("expected '.' at end of program");
	
	if(CheckToken(T_EOF)) Scopes->exitScope(); //exit program scope once program ends
	else ReportError("expected end of program");
}

//<program_header> ::= program <identifier> is
void Parser::ProgramHeader(){
	if(CheckToken(T_PROGRAM)){
		clearScopeVals(); //reset ScopeValue used for adding variables to scope tables
		if(CheckToken(TYPE_IDENTIFIER)){
			//get identifier, type, and global for variable to add to program
			//ScopeIdentifier = prev_token->ascii;
			//ScopeValue.type = TYPE_PROGRAM;
			//ScopeGlobal = true;
			if(CheckToken(T_IS)){
				//Scopes->addSymbol(ScopeIdentifier, ScopeValue, ScopeGlobal); 
				return;
			}
			else ReportError("expected 'is'");
		}
		else ReportError("expected program identifier");
	}
	else ReportError("expected 'program'");
}

/* <program_body> ::=
 *	{ <declaration> ;}*
 *	begin
 *	{ <statement> ;}*
 *	end program
 */
void Parser::ProgramBody(){
	//get declarations
	Declaration();
	//get statements
	if(CheckToken(T_BEGIN)){
		Statement();
		if(CheckToken(T_END)){
			if(CheckToken(T_PROGRAM)) return;
			else ReportError("expected 'program'");
		}
		else ReportError("expected 'end'");
	}
	else ReportError("expected 'begin'");
}

/*	<declaration> ::=
 *		 {global} <procedure_declaration>
 *		|{global} <variable_declaration>
 */
void Parser::Declaration(){
	//reset ScopeValue used for adding variables to scope tables
	clearScopeVals();
	
	//determine if symbol declaration is global in scope
	if(CheckToken(T_GLOBAL)) ScopeGlobal = true;
	else ScopeGlobal = false;

	//determine type of declaration, if one exists. ReportErrors if declaration is bad. Otherwise look for next declaration
	if(ProcedureDeclaration()){
		if(CheckToken(T_SEMICOLON)) Declaration();
		else ReportError("expected ';'");
	}
	else if(VariableDeclaration()){
		if(CheckToken(T_SEMICOLON)) Declaration();
		else ReportError("expected ';'");
	}
	else{
		//if 'global' reserved word was found, then a declaration should exist
		if(ScopeGlobal) ReportError("expected either procedure or variable declaration after 'global'");
		else return;
	}
}

/*	<statement> ::=
 *		 <assignment_statement>
 *		|<if_statement>
 *		|<loop_statement>
 *		|<return_statement>
 *		|<procedure_call>
 */
bool Parser::Statement(){
	//determine type of statement, if one exists
	if( IfStatement() );
	else if( LoopStatement() );
	else if( ReturnStatement() );
	else if( Assignment() ); //will check for identifier for procedure call as well
	else if( ProcedureCall() );
	else return false;

	//check for another valid statement after semicolon
	if( CheckToken(T_SEMICOLON) ){
		Statement();
		return true;
	}
	else ReportError("expected semicolon");
}

//<procedure_declaration> ::= <procedure_header><procedure_body>
bool Parser::ProcedureDeclaration(){
	//get procedure header
	if(ProcedureHeader()){
		//add procedure symbol to both its own scope (to allow recursion) and its parent scope
		Scopes->addSymbol(ProcIdentifier, ProcValue, ProcGlobal);
		Scopes->prevAddSymbol(ProcIdentifier, ProcValue, ProcGlobal);
		if(ProcedureBody()){
			Scopes->exitScope();
			return true;
		}
		else ReportError("expected procedure body");
	}
	else return false;
}

//<procedure_header> ::= procedure <identifier> ( { <parameter_list> } )
bool Parser::ProcedureHeader(){
	if(CheckToken(T_PROCEDURE)){
		//create new scope for the procedure
		Scopes->newScope();
		
		//set ProcValue type for procedure's identifier to be added to the symbol tables
		ProcValue.type = TYPE_PROCEDURE;
		
		//get procedure identifier and set value to be added to the symbol table
		if(CheckToken(TYPE_IDENTIFIER)){
			ProcIdentifier = prev_token->ascii;
			
			//get parameter list for the procedure if it has parameters
			if(CheckToken(T_LPAREN)){
				ScopeValue.size = ParameterList();
				if(CheckToken(T_RPAREN)) return true;
				else ReportError("expected ')' in procedure header");
			}
			else ReportError("expected '(' in procedure header");
		}
		else ReportError("expected procedure identifier");
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
	//get symbol declarations for next procedure
	Declaration();
	
	//get statements for procedure body
	if(CheckToken(T_BEGIN)){
		Statement();
		if(CheckToken(T_END)){
			if(CheckToken(T_PROCEDURE)) {
				//semicolon at end of procedure body is optional
				if( !CheckToken(T_SEMICOLON) ) ReportWarning("expected ';' at end of procedure body");
				return true;
			}
			else ReportError("expected 'procedure'");
		}
		else ReportError("expected 'end'");
	}
	else ReportError("expected 'begin'");
}

//<procedure_call> ::= <identifier>( { <argument_list> } )
bool Parser::ProcedureCall(){
	//does not check for the identifier since procedure call is only called right before the assignment check which will determine if there is an identifier
	if(CheckToken(T_LPAREN)){
		ArgumentList();
		if(CheckToken(T_RPAREN)) return true;
		else ReportError("expected ')' in procedure call");
	}
	else return false;
}

/*	<argument_list> ::=
 *	 <expression> , <argument_list>
 *	|<expression>
 */
bool Parser::ArgumentList(){
	if( Expression() ){
		if( CheckToken(T_COMMA) ){
			if( !ArgumentList() ) ReportError("expected another argument after ','");
		}
		return true;
	}
	else return false;
}

//<variable_declarartion> ::= <type_mark><identifier>{ [<array_size>] }
bool Parser::VariableDeclaration(){
	//get variable type, otherwise no variable should be declared
	int type_mark = TypeMark();
	if(type_mark != T_UNKNOWN) ScopeValue.type = type_mark;
	else return false;
	
	//get variable identifier
	if(CheckToken(TYPE_IDENTIFIER)){
		//set identifier to be added to the symbol table
		ScopeIdentifier = prev_token->ascii;
		
		if(CheckToken(T_LBRACKET)){
			if(CheckToken(TYPE_INTEGER)){
				//if variable is an array, get size of the array
				ScopeValue.size = prev_token->val.intValue;
				if(CheckToken(T_RBRACKET)){
					//add array variable to the symbol table
					Scopes->addSymbol(ScopeIdentifier, ScopeValue, ScopeGlobal);
					return true;
				}
				else ReportError("expected ']' variable declaration");
			}
			else ReportError("expected integer array size");
		}
		else{
			//add variable to the symbol table
			Scopes->addSymbol(ScopeIdentifier, ScopeValue, ScopeGlobal);
			return true;
		}
	}
	else ReportError("expected variable identifier");
}

/*	<type_mark> ::=
 *		 integer
 *		|float
 *		|bool
 *		|string
 *		|char
 */
int Parser::TypeMark(){
	if(CheckToken(T_INTEGER)) return TYPE_INTEGER;
	else if(CheckToken(T_FLOAT)) return TYPE_FLOAT;
	else if(CheckToken(T_BOOL)) return TYPE_BOOL;
	else if(CheckToken(T_STRING)) return TYPE_STRING;
	else if(CheckToken(T_CHAR)) return TYPE_CHAR;
	else return T_UNKNOWN;
}

/*	<parameter_list> ::=
 *		 <parameter> , <parameter_list>
 *		|<parameter>
 */
int Parser::ParameterList(){
	int count = 0;
	if(Parameter()){
		count++;
		while(CheckToken(T_COMMA)){
			count++;
			if(!Parameter()) ReportError("expected parameter after comma");
		}
	}
	return count;
}

//<parameter> ::= <variable_declaration>(in | out | inout)
bool Parser::Parameter(){
	if(VariableDeclaration()){
		if(CheckToken(T_IN)) ScopeValue.paramType = TYPE_PARAM_IN;
		else if(CheckToken(T_OUT)) ScopeValue.paramType = TYPE_PARAM_OUT;
		else if(CheckToken(T_INOUT)) ScopeValue.paramType = TYPE_PARAM_INOUT;
		else ReportError("expected 'in', 'out', or 'inout' in parameter declaration");
		
		//add to current scope's parameter list
		ProcValue.arguments.push_back(ScopeValue);
		return true;
	}
	else return false;
}

// <assignment_statement> ::= <destination> := <expression>
bool Parser::Assignment(){
	if(!Destination()) return false;
	if(CheckToken(T_ASSIGNMENT)){
		int type, size;
		Expression(type, size);
		return true;
	}
	else return false;	
}

bool Parser::LoopAssignment(){
	if(!Destination()) return false;
	if(CheckToken(T_ASSIGNMENT)){
		int type, size;
		Expression(type, size);
		return true;
	}
	else return false;	
}

//<destination> ::= <identifier> { [<expression] }
bool Parser::Destination(){
	if(CheckToken(TYPE_IDENTIFIER)){
		if(CheckToken(T_LBRACKET)){
			int type, size;
			if(Expression(type, size)){
				if(CheckToken(T_RBRACKET)) return true;
				else ReportError("expected ']' destination");
			}
			else ReportError("expected expression");
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
	if(CheckToken(T_IF)){
		if(CheckToken(T_LPAREN)){
			int type, size;
			Expression(type, size);
			if(CheckToken(T_RPAREN)){
				if(CheckToken(T_THEN)){
					Statement();
					if(CheckToken(T_ELSE)){
						if(!Statement()) ReportError("expected at least 1 statement after else");
					}
					if(CheckToken(T_END)){
						if(CheckToken(T_IF)) return true;
						else ReportError("expected 'if'");
					}
					else ReportError("expected 'end'");
				}
				else ReportError("expected 'then'");
			}
			else ReportError("expected ')' in if statement");
		}
		else ReportError("expected '(' in if declaration");
	}
	else return false;
}

/*	<loop_statement> ::=
 *		for ( <assignment_statement> ; <expression> )
 *		{ <statement> ; }*
 *		end for
 */
bool Parser::LoopStatement(){
	if(CheckToken(T_FOR)){
		if(CheckToken(T_LPAREN)){
			if(LoopAssignment()){
				if(CheckToken(T_SEMICOLON)){
					int type, size;
					if(Expression(type, size)){
						if(!CheckToken(T_RPAREN)) ReportError("expected ')' in loop assignment");
						while(Statement());
						if(CheckToken(T_END)){
							if(CheckToken(T_FOR)) return true;
							else ReportError("expected 'FOR' at end of loop statement");
						}
						else ReportError("expected 'end for' at end of loop statement");
					}
					else ReportError("expected expression in loop assignment");
				}
				else ReportError("expected ';' after loop assignment statement");
			}
			else ReportError("expected loop assignment statement");
		}
		else ReportError("expected '(' in for declaration");
	}
	else return false;
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
	int type1, type2, size1, size2;
	if( ArithOp(type1, size1) ){
		while( CheckToken(T_LOGICAL) ){
			if( !ArithOp(type2, size2) ) ReportError("expected ArithOp");
			if( type1 != type2 || type2 != TYPE_INTEGER) ReportError("only integer values allowed for bitwise operators");
			
			if( size1 != 0 && size2 != 0 && size1 != size2) ReportError("incompatible array sizes");
			else if(size2 != 0) size1 = size2;
		}
		type = type1;
		size = size1;
		return true;
	}
	else if( CheckToken(T_NOT) ){
		if( ArithOp(type1, size1) ){
			while( CheckToken(T_LOGICAL) ){
				if( !ArithOp(type2, size2) ) ReportError("expected ArithOp");
				
				if( type1 != type2 || type2 != TYPE_INTEGER) ReportError("only integer values allowed for bitwise operators");
				
				if( size1 != 0 && size2 != 0 && size1 != size2) ReportError("incompatible array sizes");
				else if(size2 != 0) size1 = size2;
			}
			type = type1;
			size = size1;
			return true;
		}
		else ReportError("expected ArithOp");
	}
	else return false;
}

/*	<arithOp> ::=
 *		<arithOp> + <relation>
 *		<arithOp> - <relation>
 *		<relation>
 */
bool Parser::ArithOp(int &type, int &size){
	int type1, type2, size1, size2;
	if( Relation(type1, size1) ){
		bool next = false;
		if(CheckToken(T_ADD)){
			if( !(type1 == TYPE_INTEGER || type1 == TYPE_FLOAT) ) ReportError("only float and integer values are allowed for arithmetic operations")
			next = true;
		}
		else if(CheckToken(T_SUBTRACT)){
			if( !(type1 == TYPE_INTEGER || type1 == TYPE_FLOAT) ) ReportError("only float and integer values are allowed for arithmetic operations")
			next = true;
		}
		while(next){
			if( !Relation(type2, size2) ) ReportError("expected Relation as part of ArithOp");
			if( !(type2 == TYPE_INTEGER || type2 == TYPE_FLOAT) ) ReportError("only float and integer values are allowed for arithmetic operations")
			else{
				if(size1 != 0 && size2 !=0 && size1 != size2) ReportError("incompatible array sizes");
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
		if( CheckToken(T_COMPARE) ){
			if(type1 == TYPE_INTEGER || type1 == TYPE_BOOLEAN){
				type1 = TYPE_BOOLEAN;
				next = true;
			}
			else ReportError("can only use integers '0' and '1' or booleans for relations");
		}
		while(next){
			//get term 2 for the relation
			if(!Term(type2, size2)) ReportError("expected term");
			
			//check that term 2 is an integer or boolean value
			if(!(type2 == TYPE_INTEGER || type2 == TYPE_BOOLEAN)) ReportError("can only use integers '0' and '1' or booleans for relations");
			
			//if term 1 is an array, check that term 2 is a scalar or an identically sized array
			if(size1 != 0){
				if(size2 != 0 && size2 != size1) ReportError("incompatible array size");
			}
			//if term 2 is an array, now we are dealing with a problem involving array sizes
			else if(size2 != 0){
				size1 = size2;
			}
			if(!CheckToken(T_COMPARE)) next = false;
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
		if(CheckToken(T_MULTIPLY)) next = true;
		else if(CheckToken(T_DIVIDE)) next = true;

		if(next){
			//only allow arithmetic operations on integers and floats (conversion is allowed between the two)
			if(!(type1 == TYPE_FLOAT || type1 == TYPE_INTEGER)) ReportError("only integer and float factors allowed preceeding arithmetic operator");
			while(next){
				if(!Factor(type2, size2)) ReportError("expected factor");
				
				if(!(type2 == TYPE_FLOAT || type2 == TYPE_INTEGER)) ReportError("expected integer or float factor in term after arithmetic operator");
				else if(size1 == 0 && size2 != 0) size1 = size2;
				else if(size1 != 0 && size2 != 0 && size1 != size2) ReportError("incompatiable array sizes");
				
				if(type1 != TYPE_FLOAT && type2 == TYPE_FLOAT) type1 = type2;
				
				if(CheckToken(T_MULTIPLY)) continue;
				else if(CheckToken(T_DIVIDE)) continue;
				else next = false;
			}
		}
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
	if( CheckToken(T_LPAREN) ){
		if( Expression(type, size) ){
			if( !CheckToken(T_RPAREN) ) ReportError("expected ')' in factor");
		}
		else ReportError("expected expression");
	}
	else if( CheckToken(T_SUBTRACT) ){
		if( Name(type, size) );
		else if( Integer() ){
			type = TYPE_INTEGER;
			size = 0;
		}
		else if( Float() ){
			type = TYPE_FLOAT;
			size = 0;
		}
		else return false;
	}
	else if( Name(type, size) );
	else if( Integer() ){
		type = TYPE_INTEGER;
		size = 0;
	}
	else if( Float() ){
		type = TYPE_FLOAT;
		size = 0;
	}
	else if( String() ){
		type = TYPE_STRING;
		size = 0;
	}
	else if( Char() ){
		type = TYPE_CHAR;
		size = 0;
	}
	else if( CheckToken(T_FALSE) ){
		type = TYPE_BOOL;
		size = 0;
	}
	else if( CheckToken(T_TRUE) ){
		type = TYPE_BOOL;
		size = 0;
	}
	else return false;
	return true;
}

// <name> ::= <identifier> { [ <epression> ] }
bool Parser::Name(int &type, int &size){
	if( Identifier() ){
		type = prev_token->type;
		size = prev_token->size;
		if( CheckToken(T_LBRACKET) ){
			if(size < 2) ReportError("not an array");
			if( Expression() ){
				size = 1;
				if( CheckToken(T_RBRACKET) ) return true;
				else ReportError("expected ']' after expression in name");
			}
			else ReportError("expected expression between brackets");
		}
		else return true;
	}
	else return false;
}

// <number> ::= [0-9][0-9]*[.[0-9]*]
bool Parser::Number(){
	if(CheckToken(TYPE_INTEGER)) return true;
	else if(CheckToken(TYPE_FLOAT)) return true;
	else return false;
}

// <integer> ::= [0-9][0-9]*
bool Parser::Integer(){
	if(CheckToken(TYPE_INTEGER)) return true;
	else return false;
}

// <float> ::= [0-9][0-9]*[.[0-9]+]
bool Parser::Float(){
	if(CheckToken(TYPE_FLOAT)) return true;
	else return false;
}

// <string> ::= "[a-zA-Z0-9_,;:.']*"
bool Parser::String(){
	if(CheckToken(TYPE_STRING)) return true;
	else return false;
}

// <char> ::= '[a-zA-Z0-9_,;:."]'
bool Parser::Char(){
	if(CheckToken(TYPE_CHAR)) return true;
	else return false;
}

// <identifier> ::= [a-zA-Z][a-zA-Z0-9_]*
bool Parser::Identifier(){
	if(CheckToken(TYPE_IDENTIFIER)) return true;
	else return false;
}
