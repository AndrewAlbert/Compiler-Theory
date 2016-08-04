#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include <stack>
#include <queue>
#include <string>
#include <cstdio>
#include "macro.h"
#include "scopeValue.h"
#include <map>

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

		// For creating the return branch table off of stored integer return addresses in memory space
		int call_count;
		map<int,string> branchTable;
		string newReturnLabel( string prefix );
		bool buildBranchTable();
		void createEntry( int lPos, int uPos );
		string branchTableLabelTrue(int id, bool cond);
		
		// Get a free register or specific register id, type will determine either floating point register or integer register
		string newRegister( int type );
		string getRegister( int type, int id );
		int ireg_in_use;
		int freg_in_use;
		
		// Stackw for evaluating expressions and keeping count of labels
		int label_count;
		void pushStack( string value, char stackID, int type );
		string popStack( char stackID, int type);
		stack<string> exprStack;
		stack<string> rightStack;
		stack<string> leftStack;
		
		// Stack for writing constant strings to heap at compile time
		int HeapSize;
		int AddStringHeap( string str );
		void buildHeap();
		struct entry{
			int MMlocation;
			string contents;
		} stringEntry;
		queue<entry> string_heap;

		// Stack for evaluating IN/OUT/INOUT arguments in procedure calls
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
		bool checkArguments();
		void resetArgument();
		void invalidateArgument();
		
		void WriteRuntime();
		
		// Convert integer type identifier to string for output file
		string typeString( int type );
		int typeSize( int type );
		
	public:
		// Miscellaneous
		codeGenerator();
		~codeGenerator();
		
		// Output file for generated C code
		FILE* oFile;
		string outputName;
		bool attachOutputFile(string filename);
		bool testOutFile();
		void header();
		void footer();

		void comment(string str, bool multi_line = false);
		
		// To control 
		bool ContinueToGenerate;
		bool ShouldGenerate();
		void stopCodeGeneration();

		// Label creation and placement
		string newLabel( string prefix = "" );
		void placeLabel( string label );
		
		// Branching
		void condBranch( string labelTrue, string labelFalse = "");
		void branch( string label );
		
		// Expressions
		string evalExpr( string op, int size_left = 0, int size_right = 0, int type_left = TYPE_INTEGER, int type_right = TYPE_INTEGER );
		void NotOnRegister( int type, int size = 0 );
		void NegateTopRegisters( int type, int size = -1 );

		/* Moving data to/from Reg/MM
		 *   memType  : value type of memory location
		 *   memSize  : size of memory locations ( 0 for non arrays )
		 *   regType  : value type of register
		 *   regSize  : size of register ( number of registers on exprStack to use, 0 indicates single register for scalars)
		 *   isGlobal : indicate if the memory location used is a Global value
		 *   index    : index in arrays ( < 0 indicates entire array )
		 *   FPoffset : offset from the current frame pointer ( absolute location for global memory values )
		 *   indirect : true if memory location should be based using indirection off the top register on the exprStack
		 *   indirectType: register type of register indirection is done off of
		 *   useSp    : true if memory location should be based off SP rather than FP ( for arguments to procedures )
		 */
		string val2reg( string val, int type );
		string mm2reg(int memType, int memSize, int FPoffset, bool isGlobal, int index = -1, bool indirect = false, int indirect_type = TYPE_INTEGER, bool useSP = false );
		string reg2mm(int regType, int memType, int regSize, int memSize, int FPoffset, bool isGlobal, bool indirect = false, int indirect_type = TYPE_INTEGER, bool useSP = false );
		
		// Procedures
		void createProcedure();
		void callProcedure( scopeValue procValue, string id );
		void ProcedureReturn();
		void setProcedurePointers(int frameSize);
		void setSPfromFP( int offset );
		void setArgument( scopeValue value, bool isGlobal, int index = -1, bool indirect = false );
		void pushArgument( int &SPoffset, int paramType = TYPE_PARAM_NULL );
		void popArguments( int SPoffset );
};

#endif
