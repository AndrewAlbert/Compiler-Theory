#ifndef PARSER_H
#define PARSER_H
#include<string>
using namespace std;

class Parser
{
	private:
		//token type from scanner
		struct token_type{
			int type;
			union {
				char stringValue[256]; 
				int intValue;			
				double doubleValue;	
			} val;
			string ascii;
			token_type* next;
		};
		void ReportError(string message);
		bool CheckToken();
		token_type* token;
		void Program();
		void ProgramHeader();
		void ProgramBody();
		void Declaration();
		bool Statement();
		bool ProcedureDeclaration();
		void ProcedureHeader();
		void ProcedureBody();
		bool ProcedureCall();
		void ArgumentList();
		bool VariableDeclaration();
		void TypeMark();
		bool Parameter();
		void ParameterList();
		bool Assignment();
		void Destination();
		bool IfStatement();
		bool LoopStatement();
		bool ReturnStatement();
		void Expression();
		void ArithOp();
		void Relation();
		void Term();
		void Factor();
		void Name();
		void Number();
		void String();
		void Char();
	public:
		Parser(token_type* headPtr);
		~Parser();

};

//single ASCII character tokens
#define T_SEMICOLON 300
#define T_LPAREN 301
#define T_RPAREN 302
#define T_ASSIGN 303
#define T_DIVIDE 304
#define T_MULTIPLY 305
#define T_ADD 306
#define T_SUBTRACT 307
#define T_COMMA 308
#define T_LBRACKET 309
#define T_RBRACKET 310
#define T_LCARET 311
#define T_RCARET 312
#define T_LOGICAL 313

//reserved keywords
#define T_PROGRAM 257
#define T_IS 258
#define T_BEGIN 259
#define T_END 260
#define T_GLOBAL 261
#define T_PROCEDURE 262
#define T_IN 263
#define T_OUT 264
#define T_INOUT 265
#define T_INTEGER 266
#define T_FLOAT 267
#define T_BOOL 268
#define T_STRING 269
#define T_CHAR 270
#define T_NOT 271
#define T_IF 272
#define T_THEN 273
#define T_ELSE 274
#define T_FOR 275
#define T_RETURN 276
#define T_TRUE 277
#define T_FALSE 278

//Identifiers
#define TYPE_INTEGER 279
#define TYPE_FLOAT 280
#define TYPE_STRING 281
#define TYPE_CHAR 282
#define TYPE_BOOL 283
#define TYPE_IDENTIFIER 284

//other
#define T_COMMENT 348
#define T_EOF 349		//EOF
#define T_UNKNOWN 350	//unknown token
