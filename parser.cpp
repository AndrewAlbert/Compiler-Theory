#include "parser.h"
#include "scope.h"
#include "scopeTracker.h"
#include "scopeValue.h"
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
	Program();
}

Parser::~Parser(){

}

//report error line number and descriptive message
void Parser::ReportError(string message){
	cout << "Error: " << message << " at line: " << token->line << endl;
	cout << "Found: " << token->ascii << endl;
	exit(EXIT_FAILURE);
}

//check if current token is the correct type, if so get next
bool Parser::CheckToken(int type){
	while (token->type == T_COMMENT){
		prev_token = token;
		token = token->next;
	}
	//cout << token->ascii << " current type " << token->type << " compare against " << type << " line: " << token->line << endl;
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
	if(CheckToken(T_PERIOD)){
		if(CheckToken(T_EOF)) Scopes->exitScope(); //exit program scope once program ends
	}
	else if(CheckToken(T_EOF)) Scopes->exitScope(); //exit program scope once program ends
	else ReportError("expected '.' at end of program");
}

//<program_header> ::= program <identifier> is
void Parser::ProgramHeader(){
	if(CheckToken(T_PROGRAM)){
		clearScopeVals(); //reset ScopeValue used for adding variables to scope tables
		if(CheckToken(TYPE_IDENTIFIER)){
			//get identifier, type, and global for variable to add to program
			ScopeIdentifier = prev_token->ascii;
			ScopeValue.type = TYPE_PROGRAM;
			ScopeGlobal = true;
			if(CheckToken(T_IS)){
				Scopes->addSymbol(ScopeIdentifier, ScopeValue, ScopeGlobal); 
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

	//determine type of declaration if one exists. ReportErrors if declaration is bad. Otherwise look for next declaration
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
	if(IfStatement());
	else if(LoopStatement());
	else if(ReturnStatement());
	else if(Assignment()); //will check for identifier for procedure call as well
	else if(ProcedureCall());
	else return false;

	//check for next valid statement after semicolon
	if(CheckToken(T_SEMICOLON)){
		Statement();
		return true;
	}
	else ReportError("expected semicolon");
}

//<procedure_declaration> ::= <procedure_header><procedure_body>
bool Parser::ProcedureDeclaration(){
	//get procedure header
	if(ProcedureHeader()){
		Scopes->addSymbol(ProcIdentifier, ProcValue, ProcGlobal);
		Scopes->prevAddSymbol(ProcIdentifier, ProcValue, ProcGlobal);
		if(ProcedureBody()){
			Scopes->exitScope(); //SCOPES
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
			
			//get parameter list for for the procedure if it has parameters, parenthesis must be included though
			if(CheckToken(T_LPAREN)){
				//
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
				CheckToken(T_SEMICOLON); //semicolon at end of procedure body is optional
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
	if(Expression()){
		if(CheckToken(T_COMMA)){
			if(!ArgumentList()) ReportError("expected argument");
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

//<parameter> ::= <variable_declaration>(in | out | inout)
bool Parser::Parameter(){
	if(VariableDeclaration()){
		if(CheckToken(T_IN)) ScopeValue.paramType = TYPE_PARAM_IN;
		else if(CheckToken(T_OUT)) ScopeValue.paramType = TYPE_PARAM_OUT;
		else if(CheckToken(T_INOUT)) ScopeValue.paramType = TYPE_PARAM_INOUT;
		else ReportError("expected 'in', 'out', or 'inout'");
		
		//add to current scope's parameter list
		ProcValue.arguments.push_back(ScopeValue);
		return true;
	}
	else return false;
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

// <assignment_statement> ::= <destination> := <expression>
bool Parser::Assignment(){
	if(!Destination()) return false;
	if(CheckToken(T_ASSIGNMENT)){
		Expression();
		return true;
	}
	else return false;	
}

bool Parser::LoopAssignment(){
	if(!Destination()) return false;
	if(CheckToken(T_ASSIGNMENT)){
		Expression();
		return true;
	}
	else return false;	
}

//<destination> ::= <identifier> { [<expression] }
bool Parser::Destination(){
	if(CheckToken(TYPE_IDENTIFIER)){
		if(CheckToken(T_LBRACKET)){
			if(Expression()){
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
			Expression();
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
					if(Expression()){
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
bool Parser::Expression(){
	if(ArithOp()){
		while(CheckToken(T_LOGICAL)){
			if(ArithOp()) continue;
			else ReportError("expected arithmetic operation");
		}
		return true;
	}
	else if(CheckToken(T_NOT)){
		if(ArithOp()){
			while(CheckToken(T_LOGICAL)){
				if(ArithOp()) continue;
				else ReportError("expected arithmetic operation");
			}
			return true;
		}
		else ReportError("expected arithmetic operation");
	}
	else return false;
}

/*	<arithOp> ::=
 *		<arithOp> + <relation>
 *		<arithOp> - <relation>
 *		<relation>
 */
bool Parser::ArithOp(){
	if(Relation()){
		bool next = false;
		if(CheckToken(T_ADD)){
			next = true;
		}
		else if(CheckToken(T_SUBTRACT)){
			next = true;
		}
		while(next){
			if(!Relation()) ReportError("expected relation");
			else{
				if(CheckToken(T_ADD));
				else if(CheckToken(T_SUBTRACT));
				else next = false;
			}
		}
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
bool Parser::Relation(){
	if(Term()){
		bool next = false;
		if(CheckToken(T_COMPARE)) next = true;
		while(next){
			if(!Term()) ReportError("expected term");
			else if(!CheckToken(T_COMPARE)) next = false;
		}
		return true;
	}
	else return false; 
}

/*	<term> ::=
 *		 <term> * <factor>
 *		|<term> / <factor>
 *		|<factor>
 */
bool Parser::Term(){
	if(Factor()){
		bool next = false;
		if(CheckToken(T_MULTIPLY)) next = true;
		else if(CheckToken(T_DIVIDE)) next = true;

		while(next){
			if(!Factor()) ReportError("expected factor");

			if(CheckToken(T_MULTIPLY));
			else if(CheckToken(T_DIVIDE));
			else next = false;
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
bool Parser::Factor(){
	if(CheckToken(T_LPAREN)){
		if(Expression()){
			if(CheckToken(T_RPAREN)) return true;
			else ReportError("expected ')' in factor");
		}
		else ReportError("expected expression");
	}
	else if(CheckToken(T_SUBTRACT)){
		if(Name()) return true;
		else if(Number()) return true;
		else return false;
	}
	else if(Name()) return true;
	else if(Number()) return true;
	else if(String()) return true;
	else if(Char()) return true;
	else if(CheckToken(T_FALSE)) return true;
	else if(CheckToken(T_TRUE)) return true;
	else return false;
}

// <name> ::= <identifier> { [ <epression> ] }
bool Parser::Name(){
	if(Identifier()){
		if(CheckToken(T_LBRACKET)){
			if(Expression()){
				if(CheckToken(T_RBRACKET)) return true;
				else ReportError("expected ']' name");
			}
			else ReportError("expected expression");
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
