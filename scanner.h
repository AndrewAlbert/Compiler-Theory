#ifndef SCANNER_H
#define SCANNER_H

class Scanner
{
	public:
		Scanner(string filename);
		virtual ~Scanner();
		bool isNum(char character);
		bool isLetter(char character);
		bool isString(char character);
		bool isChar(char character);
		bool isSingleToken(char character);
		bool isSpace(char character);
		void PrintTokens();
	private:
		struct token_type{
			int type;
			union {
				//string stringValue; 
				int intValue;			
				double doubleValue;	
			} val;
			string ascii;
			token_type* next;
		};
		FILE * fPtr;
		token_type *headPtr;
		token_type *tailPtr;
		int ScanOneToken(FILE * fPtr, struct token_type &token);		
}
#endif