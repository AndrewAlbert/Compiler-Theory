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
		/* Memory and Stack space constants and identifiers 
		 * for use in keeping written code consistent */
		int MM_SIZE = 1024*1024*32;
		int REG_SIZE = 1024;
		string iRegId = "iReg";
		string fRegId = "fReg";
		string memoryIdentifier = "MM";
		string FP_REG = "FP_reg";
		string SP_REG = "SP_reg";
		string HP_REG = "HP_reg";
		string TP_REG = "TP_reg";

		bool ContinueToGenerate;
		bool ShouldGenerate();
		
		// Stack for evaluating expressions and 
		int ireg_in_use;
		int freg_in_use;
		int label_count;
		stack<string> exprStack;
		stack<string> rightStack;
		stack<string> leftStack;
		
		// Stack for writing constant strings to heap at compile time
		int HeapSize;
		struct entry{
			int MMlocation;
			string contents;
		} stringEntry;
		queue<entry> string_heap;

		// Stack for evaluating IN/OUT arguments in procedure calls
		struct argument{
			int paramType;
			int type;
			int size;
			int index;
			int offset;
			bool indirect;
			bool isGlobal;
			bool set;
			bool invalid;
		} savedArgument;
		stack<argument> argListStack;

		// Output file for generated C code
		FILE* oFile;
		string outputName;
		int tabs;
		void writeLine( string line );
	
	public:
		// Miscellaneous
		codeGenerator();
		~codeGenerator();
		void stopCodeGeneration();
		void tabInc();
		void tabDec();
		void comment(string str, bool multi_line = false);
		bool attachOutputFile(string filename);
		bool testOutFile();
		
		void setSPfromFP( int offset );
		bool checkArguments();
		void resetArgument();
		void setOutputArgument( scopeValue value, bool isGlobal, int index = -1, bool indirect = false );
		void invalidateArgument();
		void confirmArgument();
		void pushArgument( int &SPoffset, int paramType = TYPE_PARAM_NULL );
		void popArguments( int SPoffset );
		void setProcedurePointers(int frameSize);
		
		void pushStack( string value, char stackID, int type );
		string popStack( char stackID, int type);

		
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

		/* Moving data to/from Reg/MM
		 *   memType  : value type of memory location
		 *   memSize  : size of memory locations ( 0 for non arrays )
		 *   regType  : value type of register
		 *   regSize  : size of register ( number of registers on exprStack to use, 0 indicates single register for scalars)
		 *   isGlobal : indicate if the memory location used is a Global value
		 *   index    : index in arrays ( -1 indicates entire array )
		 *   FPoffset : offset from the current frame pointer ( absolute location for global memory values )
		 *   indirect : true if memory location should be based using indirection off the top register on the exprStack
		 *   indirectType: 
		 *   useSp    : true if memory location should be based off SP rather than FP ( for arguments to procedures )
		 */
		string VALtoREG( string val, int type );
		string mm2reg(int memType, int memSize, int FPoffset, bool isGlobal, int index = -1, bool indirect = false, int indirect_type = TYPE_INTEGER, bool useSP = false );
		string reg2mm(int regType, int memType, int regSize, int memSize, int FPoffset, bool isGlobal, bool indirect = false, int indirect_type = TYPE_INTEGER, bool useSP = false );
		
		// Get a free register or specific register id, type will determine either floating point register or integer register
		string newRegister( int type );
		string getRegister( int type, int id );

		// Procedures
		void createProcedure();
		void callProcedure( scopeValue procValue, string id );
		void ProcedureReturn();

		// Branching
		void condBranch( string labelTrue, string labelFalse = "");
		void branch( string label );

		// Strings
		int AddStringHeap( string str );
		void WriteRuntime();
};

#endif
