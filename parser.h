#ifndef PARSER_H
#define PARSER_H

#include <string>
#include "macro.h"
#include "scopeTracker.h"
#include <cstdio>
//#include <queue>
using namespace std;

class Parser
{
	private:
		/* Methods, flags, and queue used for warning / error reporting in the Parser
		 * ReportError and ReportWarning will both enqueue a message along with line number and ascii value of current token.
		 * ReportError will stop parsing, but ReportWarning will allow continued parsing. Both prevent code generation.
		 * When parsing completes, or an error stops program execution, the messages stored in the warning_queue will be printed */
		//queue<std::string> warning_queue;
		//void DisplayWarningQueue();
		bool warning;
		bool error;
		void ReportError(string message);
		void ReportWarning(string message);

		/* Pointers and method to handle the token stream passed from the Scanner.
		 * CheckToken() determines if the current token type is the input type. The stream moves forward one token.
		 * The two pointers point to the current token in the stream, and the previous token in the stream (useful for getting information) */
		bool CheckToken(int type);
		token_type* token;
		token_type* prev_token;
		
		//Pointer to scope symbol tables
		scopeTracker* Scopes;
		
		void Program();
		bool ProgramHeader();
		bool ProgramBody();
		
		//Declaration()
		bool Declaration();
		
		bool VariableDeclaration(string &id, scopeValue &varEntry);
		bool TypeMark(int &type);
		
		bool ProcedureDeclaration(string &id, scopeValue &procDeclaration, bool global);
		bool ProcedureHeader(string &id, scopeValue &procDeclaration, bool global);
		bool ProcedureBody();
		bool ProcedureCall(string id);
		
		bool ParameterList(scopeValue &procEntry);
		bool Parameter(scopeValue &procEntry);
		
		/* ArgumentList() is used by ProcedureCall() to return a vector of scopeValues to compare against the parameter list.
		 * Each argument return in the vector has a type and size determined from using an Expression() statement */
		vector<scopeValue> ArgumentList();
		
		/* Statement Calls. Statement() checks for one of the following statement types */
		bool Statement();
		bool Assignment(string &id);
		bool Destination(string &id, int &dType, int &dSize);
		bool IfStatement();
		bool LoopStatement();
		bool ReturnStatement();
		
		/* Expression and its associated recursive calls. 
		 * Each passes a type and size to the function that calls it. */
		bool Expression(int &type, int &size);
		bool ArithOp(int &type, int &size);
		bool Relation(int &type, int &size);
		bool Term(int &type, int &size);
		bool Factor(int &type, int &size);
		bool Name(int &type, int &size);
		
		/* Constant value tokens. Simple boolean returns that indicate if the current token is the associated type. 
		 * Number is an integer or float
		 * Identifier is for variables and procedures */
		bool Number();
		bool Integer();
		bool Float();
		bool String();
		bool Char();
		bool Identifier();
	public:
		/* Initializer which attaches the token stream and scopeTracker
		 * Token stream is created from the Scanner reading the input file.
		 * The scopeTracker containes the nested symbols tables holding variable and procedure declarations for each scope */
		Parser(token_type* headPtr, scopeTracker* scopes);
		~Parser();
};

#endif
