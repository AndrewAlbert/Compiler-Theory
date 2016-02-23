#include "parser.h"
using namespace std;

Parser::Parser(token_type* headptr){
	token = headptr;
}

Parser::~Parser(){

}

void Parser::Program(){
	ProgramHeader();
	ProgramBody();
}

void Parser::ProgramHeader(){
	if (token->type == T_PROGRAM){
		token = token->next;
		if(token->type == T_IDENTIFIER){
			token = token->next;
		}
	}
	else{
		cout << "Error: expected Program Header declaration" << endl;
		exit(EXIT_FAILURE);
	}
}

void Parser::ProgramBody(){

}

void Parser::Declaration(){

}

void Parser::Statement(){

}

void Parser::ProcedureDeclaration(){

}

void Parser::ProcedureHeader(){

}

void Parser::ProcedureBody(){

}

void Parser::ProcedureCall(){

}

void Parser::ArgumentList(){

}

void Parser::VariableDeclaration(){

}

void Parser::TypeMark(){

}

void Parser::Parameter(){

}

void Parser::ParameterList(){

}

void Parser::Assignment(){

}

void Parser::Destination(){

}

void Parser::IfStatement(){

}

void Parser::LoopStatement(){

}

void Parser::ReturnStatement(){

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
