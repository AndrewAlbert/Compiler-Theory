#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void handleBool( int val ){
	if( val != 0 && val != 1 ){
		printf("Runtime Error: integer %d is not compatible with boolean values.\n", val);
		exit(EXIT_FAILURE);
	}
	else return;
}

int getBool(){
	int val;
	scanf("%d", &val);
	handleBool( val );
	return val;
}

int getInteger(){
	int val;
	scanf("%d", &val);
	return val;
}

float getFloat(){
	float val;
	scanf("%f", &val);
	return val;
}

int getString( char* buffer ){
	fgets(buffer, 256, stdin);
	//float val = (float)sizeof(char)/sizeof(int);
	//return ( val*(float)strlen(buffer) + 0.5 );
	return ( strlen(buffer) + 1);
}

char getChar(){
	char val;
	scanf("%c", &val);
	return val;
}

void putBool( int val ){
	handleBool( val );
	printf("%d", val);
	return;
}

void putInteger( int val ){
	printf("%d", val);
	return;	
}

void putFloat( float val ){
	printf("%f", val);
	return;
}

void putString( char* str_ptr ){
	printf("%s", str_ptr);
	return; 
}

void putChar( char val ){
	printf("%c", val);
	return;
}
