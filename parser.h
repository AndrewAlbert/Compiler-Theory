#ifndef PARSER_H
#define PARSER_H

#include <string>
#include "macro.h"
#include "scopeTracker.h"
#include <cstdio>
#include <queue>
using namespace std;

class Parser
{
	private:
		//methods and queue used for warning/error reporting in parser
		queue<string> warning_queue;
		bool warning;
		bool error;
		void ReportError(string message);
		void ReportWarning(string message);
		void DisplayWarningQueue();
		
		//methods and variable for using the tokens passed from the scanner
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
		bool VariableDeclaration(bool parameter);
		int TypeMark();
		bool Parameter();
		int ParameterList();
		bool Assignment();
		bool LoopAssignment();
		bool Destination();
		bool IfStatement();
		bool LoopStatement();
		bool ReturnStatement();
		
		bool Expression(int &type, int &size);
		bool ArithOp(int &type, int &size);
		bool Relation(int &type, int &size);
		bool Term(int &type, int &size);
		bool Factor(int &type, int &size);
		bool Name(int &type, int &size);
		
		bool Number();
		bool Integer();
		bool Float();
		bool String();
		bool Char();
		bool Identifier();
	public:
		Parser(token_type* headPtr, scopeTracker* scopes);
		~Parser();
};

#endif
