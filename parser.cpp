#include "parser.h"
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
Parser::Parser(token_type* headptr){
	token = headptr;
	Program();
}

Parser::~Parser(){

}

//report error line number and descriptive message
void Parser::ReportError(string message){
	cout << "Error: " << message << " at line: " << token->line << endl;
	exit(EXIT_FAILURE);
}

//check if current token is the correct type, if so get next
bool Parser::CheckToken(int type){
	while (token->type == T_COMMENT){
		token = token->next;
	}
	if(token->type == type){
		cout << token->ascii << " current type " << token->type << " compare against " << type << endl;
		token = token->next;
		return true;
	}
	else return false;
}

//<program> ::= <program_header><program_body>
void Parser::Program(){
	ProgramHeader();
	ProgramBody();
}

//<program_header> ::= program <identifier> is
void Parser::ProgramHeader(){
	if(CheckToken(T_PROGRAM)){
		if(CheckToken(TYPE_IDENTIFIER)){
			if(CheckToken(T_IS)){
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
	Declaration();
	if(CheckToken(T_BEGIN)){
		Statement();
		if(CheckToken(T_END)){
			if(CheckToken(T_PROGRAM)){
				return;
			}
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
	bool global;
	if(CheckToken(T_GLOBAL)) global = true;
	else global = false;

	if(ProcedureDeclaration()){
		if(CheckToken(T_SEMICOLON)) Declaration();
		else ReportError("expected ';'");
	}
	else if(VariableDeclaration()){
		if(CheckToken(T_SEMICOLON)) Declaration();
		else ReportError("expected ';'");
	}
	else{
		if(global) ReportError("expected either procedure or variable declaration after 'global'");
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
	if(Assignment());
	else if(IfStatement());
	else if(LoopStatement());
	else if(ReturnStatement());
	else if(ProcedureCall());
	else return false;

	if(CheckToken(T_SEMICOLON)){
		Statement();
		return true;
	}
	else ReportError("expected semicolon");
}

//<procedure_declaration> ::= <procedure_header><procedure_body>
bool Parser::ProcedureDeclaration(){
	if(ProcedureHeader()){
		if(ProcedureBody()) return true;
		else ReportError("expected procedure body");
	}
	else return false;
}

//<procedure_header> ::= procedure <identifier> ( { <parameter_list> } )
bool Parser::ProcedureHeader(){
	if(CheckToken(T_PROCEDURE)){
		if(CheckToken(TYPE_IDENTIFIER)){
			if(CheckToken(T_LPAREN)){
				ParameterList();
				if(CheckToken(T_RPAREN)) return true;
				else ReportError("expected ')'");
			}
			else return true;
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
	Declaration();
	if(CheckToken(T_BEGIN)){
		Statement();
		if(CheckToken(T_END)){
			if(CheckToken(T_PROCEDURE))	return true;
			else ReportError("expected 'procedure'");
		}
		else ReportError("expected 'end'");
	}
	else ReportError("expected 'begin'");
}

//<procedure_call> ::= <identifier>( { <argument_list> } )
bool Parser::ProcedureCall(){
	Identifier();
	if(CheckToken(T_LPAREN)){
		ArgumentList();
		if(CheckToken(T_RPAREN)) return true;
		else ReportError("expected ')'");
	}
	else ReportError("expected '('");
}

/*	<argument_list> ::=
 *	 <expression> , <argument_list>
 *	|<expression>
 */
void Parser::ArgumentList(){
	Expression();
	if(CheckToken(T_COMMA)) ArgumentList();
	else return;
}

//<variable_declarartion> ::= <type_mark><identifier>{ [<array_size>] }
bool Parser::VariableDeclaration(){
	if(!TypeMark()) return false;
	if(CheckToken(TYPE_IDENTIFIER)){
		if(CheckToken(T_LBRACKET)){
			if(CheckToken(TYPE_INTEGER)){
				if(CheckToken(T_RBRACKET)) return true;
				else ReportError("expected ']'");
			}
			else ReportError("expected integer array size");
		}
		else return true;
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
bool Parser::TypeMark(){
	if(CheckToken(T_INTEGER));
	else if(CheckToken(T_FLOAT));
	else if(CheckToken(T_BOOL));
	else if(CheckToken(T_STRING));
	else if(CheckToken(T_CHAR));
	else return false;
	return true;
}

//<paramter> ::= <variable_declarartion>(in | out | inout)
bool Parser::Parameter(){
	if(VariableDeclaration()){
		if(CheckToken(T_IN));
		else if(CheckToken(T_OUT));
		else if(CheckToken(T_INOUT));
		else ReportError("expected 'in', 'out', or 'inout'");
		return true;
	}
	else return false;
}

/*	<parameter_list> ::=
 *		 <parameter> , <parameter_list>
 *		|<parameter>
 */
void Parser::ParameterList(){
	if(Parameter()) Parameter();
	else return;
}

// <assignment_statement> ::= <destination> := <expression>
bool Parser::Assignment(){
	Destination();
	if(CheckToken(T_ASSIGNMENT)){
		Expression();
	}
	else ReportError("expected '='");	
}

//<destination> ::= <identifier> { [<expression] }
void Parser::Destination(){
	if(CheckToken(TYPE_IDENTIFIER)){
		if(CheckToken(T_LPAREN)){
			if(Expression()){
				if(CheckToken(T_RPAREN)) return;
				else ReportError("expected ')'");
			}
			else ReportError("expected expression");
		}
		else return;
	}
	else ReportError("expected destination identifier");
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
					if(CheckToken(T_END)){
						if(CheckToken(T_IF)) return true;
						else ReportError("expected 'if'");
					}
					else ReportError("expected 'end'");
				}
				else ReportError("expected 'then'");
			}
			else ReportError("expected '('");
		}
		else ReportError("expected '('");
	}
	else return false;
}

/*	<loop_statement> ::=
 *		for ( <assignment_statement> ; <expression> )
 *		{ <statement> ; }*
 *		end for
 */
bool Parser::LoopStatement(){
	if(CheckToken(T_IF)){
		if(CheckToken(T_LPAREN)){
			Expression();
			if(CheckToken(T_RPAREN)){
				if(CheckToken(T_THEN)){
					if(Statement()){
						if(CheckToken(T_ELSE)){
							if(Statement()){
								if(CheckToken(T_END)){
									if(CheckToken(T_IF)) return true;
									else ReportError("expected 'if'");
								}
								else ReportError("expected 'end'");
							}
							else ReportError("expected at least 1 statement after else condition");
						}
						else{
							if(CheckToken(T_END)){
								if(CheckToken(T_IF)) return true;
								else ReportError("expected 'if'");
							}
							else ReportError("expected 'end'");
						}
					}
					else ReportError("expected at least 1 statement after if condition");
				}
				else ReportError("expected 'then'");
			}
			else ReportError("expected ')'");
		}
		else ReportError("expeceted '('");
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
		return true;
	}
	else if(CheckToken(T_NOT)){
		if(ArithOp()) return true;
		else ReportError("expected arithmetic operation");
	}
	else if(Expression()){
		if(CheckToken(T_LOGICAL)){
			ArithOp();
		}
		else ReportError("expected arithmetic operation");
	}
	else ReportError("expected valid expression");
	
}

/*	<arithOp> ::=
 *		<arithOp> + <relation>
 *		<arithOp> - <relation>
 *		<relation>
 */
bool Parser::ArithOp(){
	if(Relation()){
		return true;
	}
	else if(ArithOp()){
		if(CheckToken(T_ADD) || CheckToken(T_SUBTRACT)){
			if(Relation()) return true;
			else ReportError("expected arithmetic operation");
		}
		else ReportError("expected '+' or '-'");
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
		if(CheckToken(T_COMPARE)){
			if(Term()) return true;
			else ReportError("expected term");
		}
		else return true;
	}
	else ReportError("expected term"); 
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

			if(CheckToken(T_MULTIPLY)) next = true;
			else if(CheckToken(T_DIVIDE)) next = true;
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
	if(Expression()) return true;
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
				else ReportError("expected ']'");
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
