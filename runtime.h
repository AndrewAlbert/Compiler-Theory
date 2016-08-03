#ifndef RUNTIME_H
#define RUNTIME_H

extern void handleBool( int val );

extern int getBool();
extern int getInteger();
extern float getFloat();
extern int getString();
extern char getChar();

extern void putBool( int val );
extern void putInteger( int val );
extern void putFloat( float val );
extern void putString( char* str_ptr );
extern void putChar( char val );

#endif
