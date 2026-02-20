
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include "LA.h"
#include "DA.h"

Token *consumedTk;
Token *crtTk;

Type createErrorType();
int isErrorType(Type t);

void tkerr(const Token *tk, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "error in line %d:", crtTk -> line);
    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);
    va_end(va);
    exit(-1);
}

int isScalarType(Type t) {

    return (t.typeBase == TB_INT || t.typeBase == TB_DOUBLE || t.typeBase == TB_CHAR) && t.nElements == 0;
}

int checkAssignmentTypes(Type dest, Type src, Token* opTk) {
    if (dest.nElements != 0 && dest.nElements != -2) {
        tkerr(opTk, "Assignment to an array type is not allowed.");
        return 0;
    }
    if (src.typeBase == TB_VOID && src.nElements != -1) {
        tkerr(opTk, "Cannot assign from void type.");
        return 0;
    }

    if (dest.typeBase != src.typeBase && !(isScalarType(dest) && isScalarType(src))) { 
        tkerr(opTk, "Type mismatch in assignment. Cannot assign type %d to type %d", src.typeBase, dest.typeBase);
        return 0;
    }

    return 1;
}

int checkArgumentType(Type paramType, Type argType, int paramIndex, Token* funcTk) {
    if (paramType.typeBase != argType.typeBase && 
        !((isScalarType(paramType) && isScalarType(argType)) || 
          (paramType.typeBase == TB_STRUCT && argType.typeBase == TB_STRUCT && paramType.s == argType.s) )) {
    }
    if (paramType.nElements != argType.nElements && !(paramType.nElements == -1 && argType.nElements >=0) && !(paramType.nElements == -1 && argType.nElements == -2) ) { 

        if (!(paramType.nElements == -1 && paramType.typeBase == TB_CHAR && argType.nElements == -2 && argType.typeBase == TB_CHAR)) {
             tkerr(funcTk, "Type mismatch for argument %d. Array dimensions or type differ.", paramIndex + 1);
             return 0;
        }
    }

    if (!checkAssignmentTypes(paramType, argType, funcTk)) {
         tkerr(funcTk, "Type mismatch for argument %d.", paramIndex + 1);
         return 0;
    }
    return 1; 
}

int isLValue(Type t, Token* tk) {
    if (t.nElements == -2) {
        tkerr(tk, "String literal cannot be assigned to.");
        return 0;
    }
    return 1;
}


int consume(int code)
{
    if(crtTk->code==code){
        consumedTk=crtTk;
        crtTk=crtTk->next;
        return 1;
    }
    return 0;
}

int unit();
int declStruct();
int declVar();
int typeBase(Type *t);
int arrayDecl(Type *t);
int typeName(Type *t);
int declFunc();
int funcArg();
int stm();
int stmCompound();
Type expr();
Type exprAssign();
Type exprOr();
Type exprOrPrim(Type leftType);
Type exprAnd();
Type exprAndPrim(Type leftType);
Type exprEq();
Type exprEqPrim(Type leftType);
Type exprRel();
Type exprRelPrim(Type leftType);
Type exprAdd();
Type exprAddPrim(Type leftType);
Type exprMul();
Type exprMulPrim(Type leftType);
Type exprCast();
Type exprUnary();
Type exprPostfix();
Type exprPostfixPrim(Type leftType);
Type exprPrimary();

int unit()
{
    while(1)
    {
        if(declStruct()) {}
        else if(declFunc()) {}
        else if(declVar()) {}
        else break;
    }
    if(consume(END))
    {
        return 1;
    }
    else tkerr(crtTk, "Error: Missing end of the program");

    return 0;
}

int declStruct()
{
    Token *startTk = crtTk;
    Token *tkIdOfStruct = NULL;

    if(!consume(STRUCT)) {
        return 0;
    }
    
    if(!consume(ID)) {
        crtTk = startTk; 
        return 0;
    }
    tkIdOfStruct = consumedTk;

    if(crtTk->code != LACC) {

        crtTk = startTk; 
        return 0;
    }

    consume(LACC);


    Symbol *existingSymbol = findSymbol(&symbols, tkIdOfStruct->text);
    if(existingSymbol && existingSymbol->depth == crtDepth) {
         tkerr(tkIdOfStruct, "struct redefinition: %s", tkIdOfStruct->text);

         return 0;
    }
    
    Symbol *s = addSymbol(&symbols, tkIdOfStruct->text, CLS_STRUCT);
    if (!s) {
        tkerr(tkIdOfStruct, "Failed to add struct symbol: %s (internal error)", tkIdOfStruct->text);
        return 0;
    }
    s->type.typeBase = TB_STRUCT;
    s->type.s = s;
    initSymbols(&s->u.members);
    crtStruct = s;
    int originalDepth = crtDepth;
    crtDepth++;

    while(declVar()){}

    
    if(!consume(RACC)) {
        crtDepth = originalDepth; 
        crtStruct = NULL;
        tkerr(crtTk, "Missing '}' at the end of 'struct %s' definition", tkIdOfStruct->text);
        return 0; 
    }

    if(!consume(SEMICOLON)) {
        crtDepth = originalDepth; 
        crtStruct = NULL;
        tkerr(crtTk, "Missing ';' after 'struct %s' definition", tkIdOfStruct->text);
        return 0; 
    }


    crtDepth = originalDepth; 
    crtStruct = NULL;
    return 1;
}

int declVar()
{
    Token* startTk = crtTk;
    Type baseType;
    baseType.nElements = 0;
    baseType.s = NULL;

    if(typeBase(&baseType))
    {
        Token *tkName = NULL;
        if(consume(ID))
        {
            tkName = consumedTk;
            Type currentVarType = baseType;
            if(arrayDecl(&currentVarType)) {}
            addVar(tkName, &currentVarType);

            while(consume(COMMA))
            {
                if(consume(ID))
                {
                    tkName = consumedTk;
                    Type nextVarType = baseType;
                    if(arrayDecl(&nextVarType)) {}
                    addVar(tkName, &nextVarType);
                } else {
                    tkerr(crtTk, "declVar: Missing variable name after ','");
                    crtTk = startTk; return 0;
                }
            }
            if(consume(SEMICOLON))
            {
                return 1;
            }
            tkerr(crtTk, "declVar: Missing ; at the end of statement");
            crtTk = startTk; return 0;
        } else {
            tkerr(crtTk, "declVar: Missing variable name after type specifier");
            crtTk = startTk; return 0;
        }
    }
    crtTk = startTk;

    return 0;
}

int typeBase(Type *t)
{
    Token *startTk = crtTk;
    t->s = NULL;
    t->nElements = 0;

    if(consume(INT)) {
        t->typeBase = TB_INT;
        return 1;
    }
    if(consume(DOUBLE)) {
        t->typeBase = TB_DOUBLE;
        return 1;
    }
    if(consume(CHAR)) {
        t->typeBase = TB_CHAR;
        return 1;
    }

    if(consume(STRUCT))
    {
        if(consume(ID))
        {
            Token *tkName = consumedTk;
            Symbol *s = findSymbol(&symbols, tkName->text);
            if(s == NULL){
                tkerr(tkName, "Undefined structure: %s", tkName->text);
            }
            if(s->cls != CLS_STRUCT){
                 tkerr(tkName, "%s is not a structure", tkName->text);
            }
            t->typeBase = TB_STRUCT;
            t->s = s;
            return 1;
        }
        else tkerr(crtTk, "typeBase: Expected identifier after 'struct'");
             
    }

    crtTk = startTk;
    return 0;
}

int arrayDecl(Type *t)
{

    
    if(consume(LBBRACKET))
    {
        if(crtTk->code == CT_INT) { 
            Token *tkSize = crtTk;
            consume(CT_INT);
            if (tkSize->i < 0) {
                tkerr(tkSize, "arrayDecl: Array size cannot be negative.");
            } else {
                t->nElements = tkSize->i;
            }
        } else if(crtTk->code == RBRACKET) { 
            consume(RBRACKET);
            t->nElements = -1; 
            return 1;
        } else {
            tkerr(crtTk, "arrayDecl: Array size must be an integer constant or empty '[]'.");

        }

        if(consume(RBRACKET))
        {
            return 1;
        }
        else {
            tkerr(crtTk,"arrayDecl: Missing ']' in array declaration");
        }
    }

    return 0;
}

int typeName(Type *t)
{
    Token* startTk = crtTk;
    t->nElements = 0;
    t->s = NULL;

    if(typeBase(t))
    {
        arrayDecl(t);
        return 1;
    }
    crtTk = startTk;

    return 0;
}

int declFunc()
{
    Token *startTk = crtTk;
    Type retType;
    retType.nElements = 0;
    retType.s = NULL;
    Symbol *s = NULL;


    if (consume(VOID)) {
        retType.typeBase = TB_VOID;
    } else if (typeBase(&retType)) {

    } else {

        return 0;
    }


    Token *actualFuncNameTokenHolder = crtTk;
    if (actualFuncNameTokenHolder->code != ID) {
        crtTk = startTk; 
        return 0;
    }
    consume(ID);
    Token *actualFuncNameToken = consumedTk; 


    if (crtTk->code != LPAR) {
        crtTk = startTk; 
        return 0; 
    }
    consume(LPAR); 




    Symbol* existingFuncCheck = findSymbol(&symbols, actualFuncNameToken->text);
    if (existingFuncCheck && existingFuncCheck->depth == crtDepth && existingFuncCheck->cls == CLS_FUNC) {
        tkerr(actualFuncNameToken, "function redefinition: %s", actualFuncNameToken->text);

    }
    
    s = addSymbol(&symbols, actualFuncNameToken->text, CLS_FUNC);
    if (!s) { 
        tkerr(actualFuncNameToken, "Failed to add function symbol: %s (internal error)", actualFuncNameToken->text);
    }
    s->type = retType;
    initSymbols(&s->u.args);
    crtFunc = s; 


    crtDepth++; 
    Symbol *markerBeforeArgsAndLocals = getLastSymbol(&symbols); 

    if (crtTk->code != RPAR) {
        if (!funcArg()) { 
            tkerr(crtTk, "Invalid first argument in function '%s'", s->name);
        }
        while (consume(COMMA)) {
            if (!funcArg()) {
                tkerr(crtTk, "Invalid argument after comma in function '%s'", s->name);
            }
        }
    }


    if (!consume(RPAR)) {
        tkerr(crtTk, "Missing ')' after parameters in function '%s'", s->name);
    }


    if (!stmCompound()) {
        tkerr(crtTk, "Missing function body for '%s'", s->name);
    }


    deleteSymbolsAfter(&symbols, markerBeforeArgsAndLocals); 
    crtDepth--;
    crtFunc = NULL;
    return 1;
}

int funcArg()
{
    Token* startTk = crtTk;
    Type t_arg;
    t_arg.nElements = 0; t_arg.s = NULL;
    Token *tkName_arg = NULL;

    if(!typeBase(&t_arg)) {
        crtTk = startTk;
        return 0;
    }

    if(!consume(ID)) {
        tkerr(crtTk, "funcArg: Missing identifier for argument after type");
        crtTk = startTk;
        return 0;
    }
    tkName_arg = consumedTk;

    if(arrayDecl(&t_arg)) {

    }
            
    if(!crtFunc) {
         tkerr(tkName_arg, "INTERNAL ERROR: funcArg called when crtFunc is NULL.");
         crtTk = startTk; return 0; 
    }

    if(findSymbol(&crtFunc->u.args, tkName_arg->text)){
        tkerr(tkName_arg, "argument redefinition in function '%s': %s", crtFunc->name, tkName_arg->text);
    } else {
        Symbol *sArgMember = addSymbol(&crtFunc->u.args, tkName_arg->text, CLS_VAR);
        if(sArgMember) {
            sArgMember->type = t_arg;
            sArgMember->mem = MEM_ARG;
        }
    }

    Symbol* sExistingInScope = findSymbol(&symbols, tkName_arg->text);
    if(sExistingInScope && sExistingInScope->depth == crtDepth){
        tkerr(tkName_arg, "symbol redefinition (argument '%s') in current scope of function '%s'", tkName_arg->text, crtFunc->name);
    } else {
        Symbol *sVisible = addSymbol(&symbols, tkName_arg->text, CLS_VAR); 
        if(sVisible) {
            sVisible->type = t_arg;
            sVisible->mem = MEM_ARG;
        }
    }
    return 1;
}

int stm()
{
    if(stmCompound())
    {
        return 1;
    }
    else if(consume(IF))
    {
        if(consume(LPAR))
        {
            Type condType = expr();
            if(!isErrorType(condType))
            {
                if (!isScalarType(condType)) {
                    tkerr(consumedTk, "IF condition must be a scalar type.");
                }
                if(consume(RPAR))
                {
                    if(stm())
                    {
                        if(consume(ELSE))
                        {
                            if(stm())
                            {
                                return 1;
                            }else tkerr(crtTk, "stm: Missing statement after else");
                        } 
                        return 1;
                    }else tkerr(crtTk, "stm: Missing statement after ')'");
                }else tkerr(crtTk, "stm: Missing ')' after expression");
            }
        }
    }
    else if(consume(WHILE))
    {
        if(consume(LPAR))
        {
            Type condType = expr();
            if(!isErrorType(condType))
            {
                if (!isScalarType(condType)) {
                    tkerr(consumedTk, "WHILE condition must be a scalar type.");
                }
                if(consume(RPAR))
                {
                    if(stm())
                    {
                        return 1;
                    }else tkerr(crtTk, "stm: Missing statement after ')'");
                }else tkerr(crtTk, "stm: Missing ')' after expression");
            }else tkerr(crtTk, "stm: Missing expression after '(' in stm while");
        }else tkerr(crtTk, "stm: Missing '(' after while");
    }
    else if(consume(FOR))
    {
        if(consume(LPAR))
        {

            if (crtTk->code != SEMICOLON) expr(); 


            if(consume(SEMICOLON))
            {
                Type expr2Type = createErrorType(); // Condition expression
                if (crtTk->code != SEMICOLON) expr2Type = expr();
                
                if (!isErrorType(expr2Type) && crtTk->code != SEMICOLON) { 
                     if (!isScalarType(expr2Type) && !(expr2Type.typeBase == TB_VOID && expr2Type.nElements == 0) ) { 

                         if (expr2Type.typeBase != TB_VOID) {
                            tkerr(consumedTk, "FOR condition must be a scalar type.");
                         }
                    }
                } else if (isErrorType(expr2Type)) {
                    // Error already reported by expr()
                }

                if(consume(SEMICOLON))
                {

                    if (crtTk->code != RPAR) expr();


                    if(consume(RPAR))
                    {
                        if(stm())
                        {
                             return 1;
                        }    
                    } else tkerr(crtTk, "FOR: Missing ')' after expressions");
                } else tkerr(crtTk, "FOR: Missing ';' after second expression");
            } else tkerr(crtTk, "FOR: Missing ';' after first expression");
        } else tkerr(crtTk, "FOR: Missing '(' after for");
    }
    else if(consume(BREAK))
    {
        if(consume(SEMICOLON))
        {
            return 1;
        }
    }
    else if(consume(RETURN))
    {
        Type retExprType = createErrorType();
        int hasExpr = 0;
        if (crtTk->code != SEMICOLON) {
            retExprType = expr();
            hasExpr = 1;
        }

        if (!crtFunc) {
            tkerr(consumedTk, "RETURN statement outside of a function.");
        } else {
            if (crtFunc->type.typeBase == TB_VOID) {
                if (hasExpr && !isErrorType(retExprType)) {
                    tkerr(consumedTk, "RETURN with value in a void function.");
                }
            } else {
                if (!hasExpr) {
                    tkerr(consumedTk, "RETURN without value in a non-void function.");
                } else if (!isErrorType(retExprType)) {
                    if (!checkAssignmentTypes(crtFunc->type, retExprType, consumedTk)) {

                        tkerr(consumedTk, "Invalid return type. Expected base type %d, got base type %d.", crtFunc->type.typeBase, retExprType.typeBase);
                    }
                }
            }
        }
        
        if(consume(SEMICOLON))
        {
            return 1;
        }
        else tkerr(crtTk, "stm: Missing ';' after 'return'");
    }
    else 
    {
        Type exprStatementType = expr(); 
        if(!isErrorType(exprStatementType))
        {
            if(consume(SEMICOLON))
            {
                return 1;
            }
        }
    }
    if(consume(SEMICOLON)) return 1;
    

    return 0;
}

int stmCompound()
{

    Symbol *markerBeforeLocals = NULL; 

    if(consume(LACC))
    {
        crtDepth++; 
        markerBeforeLocals = getLastSymbol(&symbols);

        while(1) {
            if(declVar()) {}
            else if(stm()) {}
            else break;
        }

        if(consume(RACC)) {
            deleteSymbolsAfter(&symbols, markerBeforeLocals);
            crtDepth--;
            return 1;
        } else {
            tkerr(crtTk, "stmCompound: Missing '}' at end of compound statement");
            deleteSymbolsAfter(&symbols, markerBeforeLocals);
            crtDepth--; 
            return 0;
        }
    }

    return 0; 
}

Type expr()
{
    return exprAssign();
}

Type exprAssign()
{
    Token* startTk = crtTk;
    Type leftType, rightType;


    leftType = exprUnary(); 
    if(!isErrorType(leftType)) {
        Token* opTk = crtTk; 
        if(consume(ASSIGN)) {
            if (!isLValue(leftType, consumedTk)) {
                 tkerr( consumedTk /* or opTk */, "Left hand side of assignment must be an l-value.");

            } else {
                rightType = exprAssign(); 
                if(!isErrorType(rightType)) {
                    if (!checkAssignmentTypes(leftType, rightType, opTk)) {
                        // Error message handled by checkAssignmentTypes
                    }
                    return leftType; 
                }
            }

            return createErrorType();
        }
    }
    
    crtTk = startTk;
    return exprOr();
}

Type exprOr()
{
    Type leftType = exprAnd();
    if(isErrorType(leftType)) return leftType;
    
    return exprOrPrim(leftType);
}

Type exprOrPrim(Type leftType)
{
    Token* opTk = crtTk;
    if(consume(OR)) {
        if (!isScalarType(leftType)) {
            tkerr(opTk, "Left operand of OR must be a scalar type.");
            return createErrorType();
        }
        Type rightType = exprAnd();
        if(isErrorType(rightType)) return rightType;
        if (!isScalarType(rightType)) {
            tkerr(opTk, "Right operand of OR must be a scalar type.");
            return createErrorType();
        }
        
        Type resultType;
        resultType.typeBase = TB_INT;
        resultType.s = NULL;
        resultType.nElements = 0;
        
        return exprOrPrim(resultType); 
    }
    return leftType;
}

Type exprAnd()
{
    Type leftType = exprEq();
    if(isErrorType(leftType)) return leftType;
    
    return exprAndPrim(leftType);
}

Type exprAndPrim(Type leftType)
{
    Token* opTk = crtTk;
    if(consume(AND)) {
        if (!isScalarType(leftType)) {
            tkerr(opTk, "Left operand of AND must be a scalar type.");
            return createErrorType();
        }
        Type rightType = exprEq();
        if(isErrorType(rightType)) return rightType;
        if (!isScalarType(rightType)) {
            tkerr(opTk, "Right operand of AND must be a scalar type.");
            return createErrorType();
        }
        
        Type resultType;
        resultType.typeBase = TB_INT; 
        resultType.s = NULL;
        resultType.nElements = 0;
        
        return exprAndPrim(resultType); 
    }
    return leftType;
}

Type exprEq()
{
    Type leftType = exprRel();
    if(isErrorType(leftType)) return leftType;
    
    return exprEqPrim(leftType);
}

Type exprEqPrim(Type leftType)
{
    Token* opTk = crtTk;
    if(consume(EQUAL) || consume(NOTEQ)) {

        if (!isScalarType(leftType) && leftType.typeBase != TB_STRUCT) { 
            tkerr(opTk, "Left operand of equality operator must be a scalar or compatible type.");
            return createErrorType();
        }
        Type rightType = exprRel();
        if(isErrorType(rightType)) return rightType;
        if (!isScalarType(rightType) && rightType.typeBase != TB_STRUCT) {
            tkerr(opTk, "Right operand of equality operator must be a scalar or compatible type.");
            return createErrorType();
        }

        if (leftType.typeBase == TB_STRUCT || rightType.typeBase == TB_STRUCT) {
            if (leftType.typeBase != rightType.typeBase || leftType.s != rightType.s) {
                 tkerr(opTk, "Operands of equality operator are not compatible (structs must be same type).");
                 return createErrorType();
            }
        } else if (!isScalarType(leftType) || !isScalarType(rightType)) {
             tkerr(opTk, "Operands of equality operator must be scalar types or compatible struct types.");
             return createErrorType();
        }


        Type resultType;
        resultType.typeBase = TB_INT;
        resultType.s = NULL;
        resultType.nElements = 0;
        
        return exprEqPrim(resultType);
    }
    return leftType;
}

Type exprRel()
{
    Type leftType = exprAdd();
    if(isErrorType(leftType)) return leftType;
    
    return exprRelPrim(leftType);
}

Type exprRelPrim(Type leftType)
{
    Token* opTk = crtTk;
    if(consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATERQ)) {
        if (!isScalarType(leftType)) {
            tkerr(opTk, "Left operand of relational operator must be a scalar type.");
            return createErrorType();
        }
        Type rightType = exprAdd();
        if(isErrorType(rightType)) return rightType;
        if (!isScalarType(rightType)) {
            tkerr(opTk, "Right operand of relational operator must be a scalar type.");
            return createErrorType();
        }


        Type resultType;
        resultType.typeBase = TB_INT;
        resultType.s = NULL;
        resultType.nElements = 0;
        
        return exprRelPrim(resultType);
    }
    return leftType;
}

Type exprAdd()
{
    Type leftType = exprMul();
    if(isErrorType(leftType)) return leftType;
    
    return exprAddPrim(leftType);
}

Type exprAddPrim(Type leftType)
{
    Token* opTk = crtTk;
    if(consume(ADD) || consume(SUB)) {
        if (!isScalarType(leftType)) {
            tkerr(opTk, "Left operand of ADD/SUB must be an arithmetic type.");
            return createErrorType();
        }
        Type rightType = exprMul();
        if(isErrorType(rightType)) return rightType;
        if (!isScalarType(rightType)) {
            tkerr(opTk, "Right operand of ADD/SUB must be an arithmetic type.");
            return createErrorType();
        }


        Type resultType;
        resultType.s = NULL;
        resultType.nElements = 0;
        if (leftType.typeBase == TB_DOUBLE || rightType.typeBase == TB_DOUBLE) {
            resultType.typeBase = TB_DOUBLE;
        } else if (leftType.typeBase == TB_INT || rightType.typeBase == TB_INT) {
            resultType.typeBase = TB_INT;
        } else {
            resultType.typeBase = TB_CHAR;
        }

        
        return exprAddPrim(resultType);
    }
    return leftType;
}

Type exprMul()
{
    Type leftType = exprCast();
    if(isErrorType(leftType)) return leftType;
    
    return exprMulPrim(leftType);
}

Type exprMulPrim(Type leftType)
{
    Token* opTk = crtTk;
    if(consume(MUL) || consume(DIV)) {
        if (!isScalarType(leftType)) {
            tkerr(opTk, "Left operand of MUL/DIV must be an arithmetic type.");
            return createErrorType();
        }
        Type rightType = exprCast();
        if(isErrorType(rightType)) return rightType;
        if (!isScalarType(rightType)) {
            tkerr(opTk, "Right operand of MUL/DIV must be an arithmetic type.");
            return createErrorType();
        }



        Type resultType;
        resultType.s = NULL;
        resultType.nElements = 0;
        if (leftType.typeBase == TB_DOUBLE || rightType.typeBase == TB_DOUBLE) {
            resultType.typeBase = TB_DOUBLE;
        } else if (leftType.typeBase == TB_INT || rightType.typeBase == TB_INT) {
            resultType.typeBase = TB_INT;
        } else {
            resultType.typeBase = TB_CHAR; 
        }

        
        return exprMulPrim(resultType); 
    }
    return leftType;
}

Type exprCast()
{

    Type castType;
    castType.nElements = 0;
    castType.s = NULL;

    Token *beforeLparTk = crtTk;
    if(consume(LPAR)) {
        if(typeName(&castType)) {
            if(consume(RPAR)) {
                Type exprToCastType = exprCast();
                if(!isErrorType(exprToCastType)) {
                    return castType;
                }
                return createErrorType();
            } else {
                tkerr(crtTk, "exprCast: Missing ')' after type name in cast");

            }
        } else {
            crtTk = beforeLparTk; 
        }
    }


    return exprUnary();
}

Type exprUnary()
{
    Token* opTk = crtTk;
    if(consume(SUB)) {
        Type operandType = exprUnary();
        if(isErrorType(operandType)) return operandType;
        if (!isScalarType(operandType)) {
            tkerr(opTk, "Operand of unary minus must be an arithmetic type.");
            return createErrorType();
        }

        return operandType; 
    } else if (consume(NOT)) {
        Type operandType = exprUnary();
        if(isErrorType(operandType)) return operandType;
        if (!isScalarType(operandType)) {
            tkerr(opTk, "Operand of logical NOT must be a scalar type.");
            return createErrorType();
        }
        
        Type resultType;
        resultType.typeBase = TB_INT;
        resultType.s = NULL;
        resultType.nElements = 0;
        return resultType;
    }
    
    return exprPostfix();
}

Type exprPostfix()
{
    Type baseType = exprPrimary();

    
    return exprPostfixPrim(baseType);
}

Type exprPostfixPrim(Type leftType)
{
    if (isErrorType(leftType)) return leftType;


    if (crtTk->code == LBBRACKET) {
        Token *tkLBracket = crtTk;
        consume(LBBRACKET); 

        Type indexType = expr();
        if (isErrorType(indexType)) return indexType; 
        
        if (!(indexType.typeBase == TB_INT && indexType.nElements == 0)) {
            tkerr(tkLBracket, "Array index must be an integer type."); 
            return createErrorType();
        }
        
        if (!consume(RBRACKET)) {
            tkerr(crtTk, "exprPostfixPrim: Missing ']' after expression in array access");
            return createErrorType();
        }
        

        if (!(leftType.nElements != 0 || leftType.nElements == -1 || leftType.nElements == -2)) {
             tkerr(tkLBracket, "Subscripted value is not an array, pointer, or string.");
             return createErrorType();
        }
        Type elementType = leftType;
        elementType.nElements = 0; 

        
        return exprPostfixPrim(elementType); 
    }
    

    if (crtTk->code == DOT) {
        Token *tkDot = crtTk;
        consume(DOT);

        if (!consume(ID)) {
            tkerr(tkDot, "exprPostfixPrim: Missing identifier after '.'");
            return createErrorType();
        }
        Token *memberName = consumedTk;
        
        if (leftType.typeBase != TB_STRUCT || !leftType.s) {
            tkerr(tkDot, "Member access '.' on non-struct type.");
            return createErrorType();
        }
        
        Symbol *member = findSymbol(&leftType.s->u.members, memberName->text);
        if (!member) {
            tkerr(memberName, "Undefined member '%s' in struct '%s'", memberName->text, leftType.s->name);
            return createErrorType();
        }
        
        return exprPostfixPrim(member->type);
    }
    

    if (crtTk->code == INC || crtTk->code == DEC) {
        Token *opToken = crtTk; 
        consume(opToken->code);

        if (!isScalarType(leftType)) {
            tkerr(opToken, "Operand of postfix ++/-- must be a scalar type (or pointer).");
            return createErrorType();
        }
        
        return exprPostfixPrim(leftType);
    }
    
    return leftType;
}

Type exprPrimary()
{
    Token *originalCrtTk = crtTk; 

    if(consume(ID)) {
        Token *tkName = consumedTk;
        Symbol *s = findSymbol(&symbols, tkName->text);
        if(!s) {
            tkerr(tkName, "Undefined identifier: %s", tkName->text);
            return createErrorType(); 
        }
        
        if(consume(LPAR)) { 
            if(s->cls != CLS_FUNC && s->cls != CLS_EXTFUNC) {
                tkerr(tkName, "'%s' is not a function", tkName->text);
                return createErrorType();
            }
            
            // DEBUG PRINT: When calling a function
            // printf("[DEBUG exprPrimary] Calling function '%s', expecting %ld arguments.\n", tkName->text, (long)(s->u.args.end - s->u.args.begin));

            int argCount = 0;
            Symbols* expectedArgs = &(s->u.args);
            Symbol** currentExpectedArg = expectedArgs->begin;

            if (crtTk->code != RPAR) { 
                Type argType = expr();
                if (isErrorType(argType)) return createErrorType();
                
                if (currentExpectedArg == expectedArgs->end) {
                    tkerr(tkName, "Too many arguments for function '%s'", tkName->text);
                    return createErrorType();
                }
                if (!checkArgumentType((*currentExpectedArg)->type, argType, argCount, tkName)) {
                    return createErrorType();
                }
                currentExpectedArg++;
                argCount++;

                while(consume(COMMA)) {
                    argType = expr();
                    if(isErrorType(argType)) return createErrorType();

                    if (currentExpectedArg == expectedArgs->end) {
                        tkerr(tkName, "Too many arguments for function '%s'", tkName->text);
                        return createErrorType();
                    }
                    if (!checkArgumentType((*currentExpectedArg)->type, argType, argCount, tkName)) {
                        return createErrorType(); 
                    }
                    currentExpectedArg++;
                    argCount++;
                }
            }

            int is_different = (currentExpectedArg != expectedArgs->end);

            if (is_different) { 
                tkerr(tkName, "Too few arguments for function '%s'", tkName->text);
                return createErrorType();
            }
            
            if(!consume(RPAR)) {
                tkerr(crtTk, "exprPrimary: Missing ')' after function call arguments for '%s'", tkName->text);
                return createErrorType();
            }
            
            return s->type; 
        } 
        return s->type; 
    }

    if(consume(CT_INT)) {
        Type t;
        t.typeBase = TB_INT;
        t.s = NULL;
        t.nElements = 0;
        return t;
    }
    if(consume(CT_REAL)) {
        Type t;
        t.typeBase = TB_DOUBLE;
        t.s = NULL;
        t.nElements = 0;
        return t;
    }
    if(consume(CT_CHAR)) {
        Type t;
        t.typeBase = TB_CHAR;
        t.s = NULL;
        t.nElements = 0;
        return t;
    }
    if(consume(CT_STRING)) {
        Type t;
        t.typeBase = TB_CHAR;
        t.s = NULL;
        t.nElements = -2; 
        return t;
    }
    
    if(consume(LPAR)) {
        Type exprType = expr();
        if(isErrorType(exprType)) return exprType;
        
        if(!consume(RPAR)) {
            tkerr(crtTk, "exprPrimary: Missing ')' after expression in parentheses");
            return createErrorType();
        }
        return exprType;
    }
    
    crtTk = originalCrtTk; 
    return createErrorType(); 
}

Type createErrorType() {
    Type errType;
    errType.typeBase = TB_VOID; 
    errType.s = NULL;
    errType.nElements = -1;
    return errType;
}

int isErrorType(Type t) {
    return (t.typeBase == TB_VOID && t.nElements == -1);
}

void parse(Token *tokens)
{
    crtTk = tokens;
    if(!unit()) tkerr(crtTk, "Syntax error!");
}