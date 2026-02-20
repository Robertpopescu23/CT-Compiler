#ifndef LA_H
#define LA_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>

#define SAFEALLOC(var, Type) if((var = (Type*)malloc(sizeof(Type))) == NULL) err("not enough memory");

enum {ID, BREAK, CHAR, DOUBLE, ELSE, FOR, IF, INT, RETURN, STRUCT, 
	  VOID, WHILE, CT_INT, CT_REAL, CT_CHAR, CT_STRING, COMMA, SEMICOLON, LPAR, RPAR, 
	  LBBRACKET, RBRACKET, LACC, RACC, ADD, SUB, MUL, DIV, INC, DEC, DOT, AND, END, 
	  OR, NOT, ASSIGN, EQUAL, NOTEQ, LESS, LESSEQ, GREATER, GREATERQ, SPACE, LINECOMMENT, COMMENT};
      
typedef struct _Token
{
	int code; 	
	union				 
	{
		char *text;
		long int i;
		double r;
	};
	int line;

	struct _Token *next;
}Token;      

int getNextToken();

void err(const char *fmt, ...);

void showTokens(const Token* tokens);

void freeTokens(Token* tokens);

char *loadFile(const char *fileName);

#endif
