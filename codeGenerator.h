#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include <stack>
#include <queue>
#include <string>
#include <cstdio>
#include "macro.h"
#include "scopeValue.h"

using namespace std;

class codeGenerator
{
	private:
		//Memory and Stack space
		int MM_SIZE = 32000000;
		int REG_SIZE = 1024;
		string regIdentifier = "Reg";
		string memoryIdentifier = "MM";

		//Stack for evaluating expressions
		int reg_in_use;
		int label_count;
		stack<string> exprStack;
		
		//Output file for generated C code
		FILE* oFile;
		
	public:
		//Miscellaneous
		codeGenerator();
		~codeGenerator();
		void comment(string str);
		bool attachOutputFile(string filename);
		bool testOutFile();
		void header();
		void footer();

		// Convert integer type identifier to string for output file
		string typeString( int type );

		// Goto handling functions
		string newLabel( string prefix = "" );
		void placeLabel( string label );
		void setReturnAddress( int MMoffset, string label );

		// 
		void createProcedure();
		
		// Expression evaluations
		void pushStack( string value );
		string popStack();
		string evalExpr( string op, bool ExprRoot = false );

		// MM and Reg operations. Reg < 0 results in grabbing a free register
		string MMtoREG( int type, int MMoffset, int reg = -1, bool push = true );
		string REGtoMM( int type, int MMoffset, int reg = -1, bool push = true );
		string VALtoREG( string val, int type = TYPE_INTEGER, int reg = -1, bool push = true );
		string ArrayMMtoREG( int type, int MMoffset, int index = -1, int size = -1, int reg = -1, bool push = true);
		string newRegister();
		string getRegister( int id );
		void callProcedure( string retLabel, scopeValue procValue );
		void createProcedureFooter(string procedureName);
		void createProcedureHeader(string procedureName);
		void ProcedureReturn();
		void printStack();
};
#endif
