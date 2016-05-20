#ifndef PARSER_H
#define PARSER_H
#include<string>
#include "token.h"
#include "macro.h"
using namespace std;

class Parser
{
	private:
		//token type from scanner
		void ReportError(string message);
		bool CheckToken(int type);
		token_type* token;
		void Program();
		void ProgramHeader();
		void ProgramBody();
		void Declaration();
		bool Statement();
		bool ProcedureDeclaration();
		bool ProcedureHeader();
		bool ProcedureBody();
		bool ProcedureCall();
		bool ArgumentList();
		bool VariableDeclaration();
		bool TypeMark();
		bool Parameter();
		void ParameterList();
		bool Assignment();
		bool LoopAssignment();
		bool Destination();
		bool IfStatement();
		bool LoopStatement();
		bool ReturnStatement();
		bool Expression();
		bool ArithOp();
		bool Relation();
		bool Term();
		bool Factor();
		bool Name();
		bool Number();
		bool String();
		bool Char();
		bool Identifier();
	public:
		Parser(token_type* headPtr);
		~Parser();
};

#endif
