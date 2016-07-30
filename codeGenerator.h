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
			int index;
			int offset;
			bool isGlobal;
		};
		stack<argument> argListStack;

		//Output file for generated C code
		FILE* oFile;
		int tabs;
		void writeLine( string line );
	public:
		//Miscellaneous
		codeGenerator();
		~codeGenerator();

		void pushStack( string value, char stackID = 'E' );
		string popStack( char stackID = 'E');

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

		string mm2reg(int memType, int memSize, int FPoffset, bool isGlobal, int index = -1);
		string reg2mm(int regType, int memType, int regSize, int memSize, int FPoffset, bool isGlobal );

		// MM and Reg operations. Reg < 0 results in grabbing a free register
		string MMtoREG( int type, int FPoffset, int prevFrames = 0);
		string VALtoREG( string val, int type );
		string ArrayMMtoREG( int type, int FPoffset, int index = -1, int size = -1, int previousFrames = 0);
		string ArrayMMtoREGIndirect( int type, int FPoffset, string index_reg = "", int previousFrames = 0);

		// Register handling
		string newRegister();
		string getRegister( int id );

		// Procedures
		void createProcedure();
		void createProcedureHeader(string procedureName);
		void createProcedureFooter(string procedureName);
		void callProcedure( string retLabel, scopeValue procValue );
		void setReturnAddress( int SPoffset, string label );
		void ProcedureReturn();

		// Branching
		void condBranch( string labelTrue, string labelFalse = "");
		void branch( string label );

		// Strings
		char* AddStringHeap( string str );
/*
		void pushParameter(int paramOffset, int varOffset, bool isGlobal, int type, int paramSize, int varSize, int varIndex);
		void popParameter(int paramOffset, int varOffset, bool isGlobal, int type, int paramSize, int varSize, int varIndex);*/
		void procedureCall( scopeValue calledProcedure, int frameSize, string returnLabel );
};

#endif
