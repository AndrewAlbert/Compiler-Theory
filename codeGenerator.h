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
		int MM_SIZE = 1024*1024*32;
		int REG_SIZE = 1024*1024;
		string regIdentifier = "Reg";
		string fregIdentifier = "fReg";
		string memoryIdentifier = "MM";
		int HeapSize;

		struct data{
			string value;
			int type;
			int size;
			int index;
		};

		//Stack for evaluating expressions
		int reg_in_use;
		int freg_in_use;
		int label_count;
		stack<string> exprStack;
		stack<string> rightStack;
		stack<string> leftStack;
		
		//Output file for generated C code
		FILE* oFile;
		int tabs;
		void writeLine( string line );
		bool ContinueToGenerate;
	public:
		//Miscellaneous
		codeGenerator();
		~codeGenerator();
		void stopCodeGeneration();
		bool ShouldGenerate();
		void tabInc();
		void tabDec();
		void comment(string str, int line = 1);
		bool attachOutputFile(string filename);
		bool testOutFile();
		void header();
		void footer();

		// Convert integer type identifier to string for output file
		string typeString( int type );
		int typeSize( int type );

		// Goto handling functions
		string newLabel( string prefix = "" );
		void placeLabel( string label );
		void setReturnAddress( int MMoffset, string label );

		// 
		void createProcedure();
		void pushParameter();
		void popParameter();

		// Expression evaluations
		void pushStack( string value, int stackID = 0 );
		string popStack( int stackID = 0);
		string evalExpr( string op, int size_left = 0, int size_right = 0, int type_left = TYPE_INTEGER, int type_right = TYPE_INTEGER );

		// MM and Reg operations. Reg < 0 results in grabbing a free register
		string MMtoREG( int type, int MMoffset, int prevFrames = 0, int reg = -1, bool push = true );
		string REGtoMM( int type, int MMoffset, int size = 0, int prevFrames = 0, int reg = -1, bool push = true );
		string VALtoREG( string val, int type = TYPE_INTEGER, int prevFrames = 0, int reg = -1, bool push = true );
		string ArrayMMtoREG( int type, int MMoffset, int index = -1, int size = -1, int reg = -1, bool push = true);
		string ArrayMMtoREGIndirect( int type, int MMoffset, string index_reg = "", int previousFrames = 0, int reg = -1, bool push = true);
		string REGtoMMIndirect(int type, int MMoffset, string index_reg = "", int previousFrames = 0, int reg = -1, bool push = true);
		string newRegister( int type = TYPE_INTEGER);
		string getRegister( int id, int type = TYPE_INTEGER );
		void callProcedure( string retLabel, scopeValue procValue );
		void createProcedureFooter(string procedureName);
		void createProcedureHeader(string procedureName);
		void ProcedureReturn();
		void printStack();
		void condBranch( string labelTrue, string labelFalse = "");
		void branch( string label );
		char* AddStringHeap( string str );
		void NotOnRegister( int type, int size = 0 );	
};

#endif
