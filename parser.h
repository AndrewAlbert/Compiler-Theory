#ifndef PARSER_H
#define PARSER_H
#include <string>
#include "token.h"
#include "macro.h"
#include "scopeTracker.h"
#include <cstdio>
using namespace std;

class Parser
{
	private:
		//token type from scanner
		void ReportError(string message);
		bool CheckToken(int type);
		token_type* token;
		token_type* prev_token;
		//Pointer to scope symbol tables
		scopeTracker* Scopes;
		//variables to hold info for scope table symbols 
		scopeValue ScopeValue;
		scopeValue ProcValue;
		bool ScopeGlobal;
		bool ProcGlobal;
		string ScopeIdentifier;
		string ProcIdentifier;
		//private functions to handle manipulating symbol data
		void clearScopeVals();
		void clearProcVals();
		//bool SetSymbol();
		bool CheckSymbol();
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
		int ParameterList();
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
		Parser(token_type* headPtr, scopeTracker* scopes);
		~Parser();
};

#endif
