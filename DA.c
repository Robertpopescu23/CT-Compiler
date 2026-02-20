#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include "LA.h"
#include "SY.h"
#include "DA.h"

Symbols symbols;         
int crtDepth = 0;       
Symbol *crtFunc = NULL;  
Symbol *crtStruct = NULL; 

void initSymbols(Symbols *symbols_arg)
{
    symbols_arg -> begin = NULL;
    symbols_arg -> end = NULL;
    symbols_arg -> after = NULL;
}

Symbol *addSymbol(Symbols *symbols_arg, const char *name, int cls)
{
    Symbol *s;
    if(symbols_arg -> end == symbols_arg -> after)
    {
        int count = symbols_arg -> end - symbols_arg -> begin;
        int n = count * 2; 
        if(n == 0) n = 1; 
        symbols_arg -> begin = (Symbol **)realloc(symbols_arg -> begin, n * sizeof(Symbol *));
        if(symbols_arg -> begin == NULL) err("not enough memory");
        symbols_arg -> end = symbols_arg -> begin + count;
        symbols_arg -> after = symbols_arg -> begin + n;
    }
    SAFEALLOC(s, Symbol);
    *symbols_arg -> end ++= s;
    s -> name = name;
    s -> cls = cls;
    s -> depth = crtDepth;
    return s;
}

Symbol *findSymbol(Symbols *symbols_arg, const char *name)
{
    for(int i = crtDepth; i >= 0; i--)
    {
        for(Symbol **s_ptr = symbols_arg->begin; s_ptr < symbols_arg->end; s_ptr++)
        {
            if((*s_ptr) -> depth == i && strcmp((*s_ptr) -> name, name) == 0)
            {
                return *s_ptr;
            }
        }
    }
    return NULL;
}

Symbol *addVar(Token *tkName, Type *t) {
    Symbol *s = NULL; 

    if (crtStruct) { 
        if (findSymbol(&crtStruct->u.members, tkName->text)) {
            tkerr(tkName, "member redefinition in struct '%s': %s", crtStruct->name, tkName->text);
            return NULL; 
        }
        s = addSymbol(&crtStruct->u.members, tkName->text, CLS_VAR);
        s->mem = MEM_LOCAL; 
    }
    else if (crtFunc) { 
        Symbol *existingSymbol = findSymbol(&symbols, tkName->text);
        if (existingSymbol && existingSymbol->depth == crtDepth) {
            tkerr(tkName, "symbol redefinition in function '%s': %s", crtFunc->name, tkName->text);
            return NULL; 
        }
        s = addSymbol(&symbols, tkName->text, CLS_VAR);
        s->mem = MEM_LOCAL; 
    }
    else {
        Symbol *existingSymbol = findSymbol(&symbols, tkName->text);
        if (existingSymbol && existingSymbol->depth == 0) { 
            tkerr(tkName, "global symbol redefinition: %s", tkName->text);
            return NULL;
        }
        s = addSymbol(&symbols, tkName->text, CLS_VAR);
        s->mem = MEM_GLOBAL;
    }

    if (s) {
        s->type = *t;

    }
    return s;
}

void deleteSymbolsAfter(Symbols *symbols_arg, Symbol *start) {
    Symbol **pos = NULL;
    for (Symbol **p = symbols_arg->begin; p < symbols_arg->end; p++) {
        if (*p == start) {
            pos = p;
            break;
        }
    }
    if (!pos) {
        return;
    }

    for (Symbol **p = pos + 1; p < symbols_arg->end; p++) {
        if (*p) {
            free(*p);
            *p = NULL;
        }
    }

    symbols_arg->end = pos + 1;
}

Symbol *getLastSymbol(Symbols *symbols_arg)
{
    if(symbols_arg -> end == symbols_arg -> begin)
        return NULL;

    return *(symbols_arg -> end - 1);   
}


