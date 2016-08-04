#ifndef MACRO_H
#define MACRO_H

//single ASCII character tokens
#define T_PERIOD 299
#define T_SEMICOLON 300
#define T_LPAREN 301
#define T_RPAREN 302
#define T_COMPARE 313
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
#define T_ASSIGNMENT 314
#define T_BITWISE 315

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
#define TYPE_INTEGER 266
#define TYPE_FLOAT 267
#define TYPE_STRING 269
#define TYPE_CHAR 270
#define TYPE_BOOL 268
#define TYPE_IDENTIFIER 284

//used for type checking and scope only
#define TYPE_PROGRAM 285
#define TYPE_PROCEDURE 286
#define TYPE_PARAM_IN 287
#define TYPE_PARAM_OUT 288
#define TYPE_PARAM_INOUT 289
#define TYPE_PARAM_NULL 290

//other
#define TYPE_NUMBER 347
#define T_COMMENT 348
#define T_EOF 349		//EOF
#define T_UNKNOWN 350	//unknown token

#endif
