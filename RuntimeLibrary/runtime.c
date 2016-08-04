#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void handleBoolOp( int val1, int val2 ){
	if( (val1 != 0 && val1 != 1) || (val2 != 0 && val2 != 1) ){
		printf("Runtime Error: numbers %d and %d are not compatible with boolean operations.\n", val1, val2);
		exit(EXIT_FAILURE);
	}
	else return;
}

int getBool(){
	int val;
	scanf("%d", &val);
	if( val!= 0 && val != 1 ){
		printf("Runtime Error: %d is not a boolean value\n", val);
		exit(EXIT_FAILURE);
	}
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
	return ( strlen(buffer)/sizeof(int) + 1);
}

char getChar(){
	char val;
	scanf("%c", &val);
	return val;
}

void putBool( int val ){
	if( val!= 0 && val != 1 ){
		printf("Runtime Error: %d is not a boolean value\n", val);
		exit(EXIT_FAILURE);
	}
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

void putString( char* val ){
	printf("%s", val);
	return; 
}

void putChar( char val ){
	printf("%c", val);
	return;
}
