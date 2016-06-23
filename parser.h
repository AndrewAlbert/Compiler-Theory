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
		/* Methods, flags, and queue used for warning / error reporting in the Parser
		 * All report functions will enqueue a message along with line number and found tokens for that line.
		 * ReportFatalError will stop parsing and end the program. 
		 *    -This is used for when recovering is not possible.
		 * ReportLineError will skip to the next line or ';' when an error is found. 
		 *    -This is used when the type of statement has been determined but something went wrong. 
		 *    -For example, an unknown symbol or missing array size might throw this error
		 * ReportError will allow parsing to continue with the next token.
		 *    -This is used for simple errors like using incompatible value types / sizes or incorrect arguments in a procedure call
		 * ReportWarning does not cause an error and will not prevent code generation. The program is valid but there is a suspected bug.
		 *    -This is used for when you use a variable before assigning its value or some other problem that can cause unexpected behavior
		 * When parsing completes, or a fatal error stops program execution, the messages stored in the warning_queue will be printed */
		queue<std::string> warning_queue;
		void DisplayWarningQueue();
		bool warning;
		bool error;
		bool lineError;
		string textLine;
		int currentLine;
		void ReportFatalError(string message);
		void ReportLineError(string message);
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
		void declareRunTime();
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
		bool ArgumentList(vector<scopeValue> &list);
		
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
		bool ExpressionPrime(int &inputType, int &inputSize, bool catchTypeError, bool catchSizeError);
		bool ArithOp(int &type, int &size);
		bool ArithOpPrime(int &inputType, int &inputSize, bool catchTypeError, bool catchSizeError);
		bool Relation(int &type, int &size);
		bool RelationPrime(int &inputType, int &inputSize, bool catchTypeError, bool catchSizeError);
		bool Term(int &type, int &size);
		bool TermPrime(int &inputType, int &inputSize, bool catchTypeError, bool catchSizeError);
		bool Factor(int &type, int &size);
		bool FactorPrime(int &inputType, int &inputSize, bool catchTypeError, bool catchSizeError);
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
		bool isNumber(int &type_value);
	public:
		/* Initializer which attaches the token stream and scopeTracker
		 * Token stream is created from the Scanner reading the input file.
		 * The scopeTracker containes the nested symbols tables holding variable and procedure declarations for each scope */
		Parser(token_type* headPtr, scopeTracker* scopes);
		~Parser();
};

#endif
