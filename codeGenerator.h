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
		int MM_SIZE = 1024*32;
		int REG_SIZE = 1024;
		string iRegId = "iReg";
		string fRegId = "fReg";
		string memoryIdentifier = "MM";
		string FP_REG = "FP_reg";
		string SP_REG = "SP_reg";
		string HP_REG = "HP_reg";
		string TP_REG = "TP_reg";

		int HeapSize;
		bool ContinueToGenerate;
		bool ShouldGenerate();
		
		//Stack for evaluating expressions
		int ireg_in_use;
		int freg_in_use;
		int label_count;
		stack<string> exprStack;
		stack<string> rightStack;
		stack<string> leftStack;
		
		struct argument{
			int paramType;
			int type;
			int size;
			bool index;
			int offset;
			bool isGlobal;
			bool set;
			bool invalid;
		} savedArgument;
		stack<argument> argListStack;

		//Output file for generated C code
		FILE* oFile;
		int tabs;
		void writeLine( string line );
	
	public:
		//Miscellaneous
		codeGenerator();
		~codeGenerator();

		bool checkArguments();
		void resetArgument();
		void setOutputArgument( scopeValue value, bool isGlobal, bool index = false );
		void invalidateArgument();
		void confirmArgument();
		void pushArgument( int &SPoffset, int paramType = TYPE_PARAM_NULL );
		void popArguments( int SPoffset );
		void procedureCall( scopeValue calledProcedure, int frameSize, string returnLabel );
		void setProcedurePointers(int frameSize);
		
		void pushStack( string value, char stackID, int type );
		string popStack( char stackID, int type);

		void stopCodeGeneration();
		void tabInc();
		void tabDec();
		void comment(string str, bool multi_line = false);
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
		
		// Expressions
		string evalExpr( string op, int size_left = 0, int size_right = 0, int type_left = TYPE_INTEGER, int type_right = TYPE_INTEGER );
		void NotOnRegister( int type, int size = 0 );
		void NegateTopRegister( int type, int size = -1 );

		string VALtoREG( string val, int type );
		string mm2reg(int memType, int memSize, int FPoffset, bool isGlobal, int index = -1, bool indirect = false, int indirect_type = TYPE_INTEGER );
		string reg2mm(int regType, int memType, int regSize, int memSize, int FPoffset, bool isGlobal, bool indirect = false, int indirect_type = TYPE_INTEGER );

		// MM and Reg operations. Reg < 0 results in grabbing a free register
		
		// Register handling
		string newRegister( int type );
		string getRegister( int type, int id );

		// Procedures
		void createProcedure();
		void createProcedureHeader(string procedureName);
		void createProcedureFooter(string procedureName);
		void callProcedure( string retLabel, scopeValue procValue );
		void setReturnAddress( int SPoffset, string label );
		void ProcedureReturn( int SPoffset = 0);

		// Branching
		void condBranch( string labelTrue, string labelFalse = "");
		void branch( string label );

		// Strings
		char* AddStringHeap( string str );
};

#endif
