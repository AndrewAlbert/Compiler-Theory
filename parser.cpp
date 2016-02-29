#include "parser.h"
#include<string>
using namespace std;

Parser::Parser(token_type* headptr){
	token = headptr;
}

Parser::~Parser(){

}

//report error line number and descriptive message
void Parser::ReportError(string message){
	cout << "Error: " << message << " at line: " << token->line << endl;
	exit(EXIT_FAILURE);
}

//get next token
bool Parser::CheckToken(int type){
	if(token->type == type){
		token = token->next;
		return true;
	}
	else return false;
}

//syntax for a program
void Parser::Program(){
	ProgramHeader();
	ProgramBody();
}

//declaration of program header
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

//syntax for the program body
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

void Parser::Declaration(){
	bool global;
	if(CheckToken(T_GLOBAL)) global = true;
	else global = false;

	if(ProcedureDeclaration()) Declaration();
	else if(VariableDeclaration()) Declaration();
	else{
		if(global) ReportError("expected either procedure or variabl declaration after 'global'");
		else return;
	}
}

void Parser::Statement(){
	if(Assignment()) Statement();
	else if(IfStatement()) Statement();
	else if(LoopStatement()) Statement();
	else if(ReturnStatement()) Statement();
	else if(ProcedureCall()) Statement();
	else return;
}

bool Parser::ProcedureDeclaration(){
	ProcedureHeader();
	ProcedureBody();
}

void Parser::ProcedureHeader(){
	if(CheckToken(T_PROCEDURE)){
		if(CheckToken(T_IDENTIFIER)){
			if(CheckToken(T_LPAREN)){
				ParameterList();
				if(CheckToken(T_RPAREN)) return;
				else ReportError("expected ')'");
			}
			else return;
		}
		else ReportError("expected procedure identifier");
	}
	else ReportError("expected 'procedure'");
}

void Parser::ProcedureBody(){
	Declaration();
	if(CheckToken(T_BEGIN)){
		Statement();
		if(CheckToken(T_END)){
			if(CheckToken(T_PROCEDURE))	return;
			else ReportError("expected 'procedure'");
		}
		else ReportError("expected 'end'");
	}
	else ReportError("expected 'begin'");
}

void Parser::ProcedureCall(){
	Identifier();
	if(CheckToken(T_LPAREN)){
		ArgumentList();
		if(CheckToken(T_RPAREN)) return;
		else ReportError("expected ')'");
	}
	else ReportError("expected '('");
}

void Parser::ArgumentList(){
	Expression();
	if(CheckToken(T_COMMA)) ArgumentList();
	else return;
}

void Parser::VariableDeclaration(){
	TypeMark();
	if(CheckToken(T_IDENTIFIER)){
		if(CheckToken(T_LBRACKET)){
			if(CheckToken(TYPE_INTEGER)){
				if(CheckToken(T_RBRACKET)) return;
				else ReportError("expected ']'");
			}
			else ReportError("expected integer array size");
		}
		else return;
	}
	else ReportError("expected variable identifier");
}

void Parser::TypeMark(){
	if(CheckToken(T_INTEGER));
	else if(CheckToken(T_FLOAT));
	else if(CheckToken(T_BOOL));
	else if(CheckToken(T_STRING));
	else if(CheckToken(T_CHAR));
	else ReportError("expected variable type mark");
}

bool Parser::Parameter(){
	if(VariableDeclarartion()){
		if(CheckToken(T_IN));
		else if(CheckToken(T_OUT));
		else if(CheckToken(T_INOUT));
		else ReportError("expected 'in', 'out', or 'inout'");
		return true;
	}
	else return false;
}

void Parser::ParameterList(){
	if(Parameter()) Parameter();
	else return;
}

bool Parser::Assignment(){
	Destination();
	if(CheckToken(T_EQUALS)){
		Expression();
	}
	else ReportError("expected '='");	
}

void Parser::Destination(){
	if(CheckToken(TYPE_IDENTIFIER)){
		if(CheckToken(T_LPAREN){
			if(Expression()){

			}
			else ReportError(
		}
		else return;
	}
	else ReportError("expected destination identifier");
}

bool Parser::IfStatement(){

}

bool Parser::LoopStatement(){

}

bool Parser::ReturnStatement(){

}

void Parser::Expression(){

}

void Parser::ArithOp(){

}

void Parser::Relation(){

}

void Parser::Term(){

}

void Parser::Factor(){

}

void Parser::Name(){

}

void Parser::Number(){

}

void Parser::String(){

}

void Parser::Char(){

}
