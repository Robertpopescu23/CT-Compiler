#ifndef DA_H
#define DA_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>


struct _Symbol;
struct _Symbols;

typedef struct _Symbol Symbol;
typedef struct _Symbols Symbols;


enum { TB_INT, TB_DOUBLE, TB_CHAR, TB_STRUCT, TB_VOID };
enum { CLS_VAR, CLS_FUNC, CLS_EXTFUNC, CLS_STRUCT };
enum { MEM_GLOBAL, MEM_ARG, MEM_LOCAL };


typedef struct _Type {
    int typeBase;
    Symbol *s; 
    int nElements;
} Type;


struct _Symbols {
    Symbol **begin;
    Symbol **end;
    Symbol **after;
};


struct _Symbol {
    const char *name;
    int cls;
    int mem;
    Type type;     
    int depth;
    union {
        Symbols args;    
        Symbols members; 
    } u; 
};


extern Symbols symbols; 
extern int crtDepth;
extern Symbol *crtFunc;
extern Symbol *crtStruct;


void initSymbols(Symbols *symTable); 
Symbol *addSymbol(Symbols *symTable, const char *name, int cls);
Symbol *findSymbol(Symbols *symTable, const char *name);
void deleteSymbolsAfter(Symbols *symTable, Symbol *startAfter);
Symbol *getLastSymbol(Symbols *symTable);
Symbol *addVar(Token *tkName, Type *t); 

#endif // DA_H





