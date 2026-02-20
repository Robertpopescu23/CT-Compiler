#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include "LA.h"


void err(const char* fmt, ...);

int line = 1;

Token *lastToken;
Token *tokens;
char *pCrtCh;

Token *addTk(int code)
{
	Token *tk;
	SAFEALLOC(tk, Token);
	tk -> code = code;
	tk -> line = line;
	tk -> next = NULL;
	if(lastToken)
	{
		lastToken -> next = tk;
	}
	else
	{
		tokens = tk;
	}
	lastToken = tk;
	return tk;
}

void err(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	fprintf(stderr, "error...");
	vfprintf(stderr, fmt, va);
	fputc('\n', stderr);
	va_end(va);
	exit(-1);
}

void Tkerr(const Token *tk, const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	fprintf(stderr, "error in line %d:", tk -> line);
	vfprintf(stderr, fmt, va);
	fputc('\n', stderr);
	va_end(va);
	exit(-1);
}

char *createString(const char *start, const char *end)
{
	int length = end - start;
	char *str = (char *) malloc(length + 1);
	if(!str)
	{
		err("Not enough memory to allocate string.");
	}
	memcpy(str, start, length);
	str[length] = '\0';
	return str;
}

void freeTokens(Token *tokens)
{
	while(tokens)
	{
		Token *next = tokens -> next;
		if(tokens -> code == ID || tokens -> code == CT_STRING)
		{
			free(tokens -> text);
		}
		free(tokens);
		tokens = next;
	}
}

int getNextToken()
{
	int state = 0, nCh;
	char ch;
	const char *pStartCh;
	Token *tk;

	while(1)
	{
		ch = *pCrtCh;
		switch(state)
		{
		case 0:
			if(isalpha(ch) || ch == '_')
			{
				pStartCh = pCrtCh;
				pCrtCh++;
				state = 1;
			}
			else if(ch == '=')
			{
				pCrtCh++;
				state = 3;
			}
			else if(ch == ' ' || ch == '\r' || ch == '\t')
			{
				pCrtCh++;
			}
			else if(ch == '\n')
			{
				line++;
				pCrtCh++;
			}

			else if(ch == '0')
			{
				pStartCh = pCrtCh++;
				state = 6;
			}

			else if(isalpha(ch))
			{
				Tkerr(addTk(END), "invalid constant: missing 'x' for hexadecimal or invalid octal format");
			}
			
			else if('1' <= ch && ch <= '9')
			{
				pStartCh = pCrtCh++;
				state = 12;
			}

			else if(ch == ',')
			{
				pCrtCh++;
				addTk(COMMA);
				return COMMA;
			}
			
			else if(ch == ';')
			{
				pCrtCh++;
				addTk(SEMICOLON);
				return SEMICOLON;
			}
			
			else if(ch == '(')	//LPAR
			{
				pCrtCh++;
				addTk(LPAR);
				return LPAR;
			}

			else if(ch == ')') //RPAR
			{
				pCrtCh++;
				addTk(RPAR);
				return RPAR;
			}
			
			else if(ch == '[') //LBBRACKET
			{
				pCrtCh++;
				addTk(LBBRACKET);
				return LBBRACKET;
			}
			
			else if(ch == ']') //RBRACKET
			{
				pCrtCh++;
				addTk(RBRACKET);
				return RBRACKET;
			}

			else if(ch == '{') //LACC
			{
				pCrtCh++;
				addTk(LACC);
				return LACC;
			}
			
			else if(ch == '}') //RACC
			{
				pCrtCh++;
				addTk(RACC);
				return RACC;
			}

			else if(ch == '+')
			{
				pCrtCh++;
				addTk(ADD);
				return ADD;
			}

			else if(ch == '-')
			{
				pCrtCh++;
				addTk(SUB);
				return SUB;
			}

			else if(ch == '*')
			{
				pCrtCh++;
				addTk(MUL);
				return MUL;
			}

			else if(ch == '/')
			{
				if(*(pCrtCh + 1) == '/')
				{
					pCrtCh += 2;
					while(*pCrtCh && *pCrtCh != '\n' && *pCrtCh != '\r') pCrtCh++;
					//after line commnent restart scanning
				}
				else if(*(pCrtCh + 1) == '*')
				{
					pCrtCh += 2;
					while(1)
					{
						if(*pCrtCh == 0)
						{
							Tkerr(addTk(END), "unterminated comment");
						}
						else if(*pCrtCh == '*' && *(pCrtCh + 1) == '/')
						{
                    		pCrtCh += 2;
                    		break;							
						}
						else
						{
							if(*pCrtCh == '\n') line++;
							pCrtCh++;	
						}
					}
					//after block comment just restart scanning
				}
				else
				{
					pCrtCh++;
					addTk(DIV);
					return DIV;
				}
			}

			else if(ch == '.')
			{
				pCrtCh++;
				addTk(DOT);
				return DOT;
			}
			
			else if(ch == '&')
			{
				if(*(pCrtCh + 1) == '&')
				{
					pCrtCh += 2;
					addTk(AND);
					return AND;
				}
				else
				{
					Tkerr(addTk(END), "invalid character &");
				}
			}

			else if(ch == '|')
			{
				if(*(pCrtCh + 1) == '|')
				{
					pCrtCh += 2;
					addTk(OR);
					return OR;
				}
				else
				{
					Tkerr(addTk(END), "invalid character |");
				}
			}

			else if(ch == '!')
			{
				pCrtCh++;
				state = 15; //keep state 15 for the ST_CHAR
			}

			else if(ch == '<')
			{
				pCrtCh++;
				state = 18;
			}

			else if(ch == '>')
			{
				pCrtCh++;
				state = 21;
			}

			else if(ch == '"')
			{
				pStartCh = pCrtCh;
				pCrtCh++;
				while(*pCrtCh && *pCrtCh != '"')
				{
					if(*pCrtCh == '\\')
					{
						pCrtCh++;
						if(*pCrtCh == 0)
							Tkerr(addTk(END), "unterminated string literal after escape");
					}
					pCrtCh++;
				}
				if(*pCrtCh != '"')
				{
					Tkerr(addTk(END), "unterminated string literal");
				}
				pCrtCh++;
				tk = addTk(CT_STRING);
				tk -> text = createString(pStartCh + 1, pCrtCh - 1);
				return CT_STRING;
			}

			else if(ch == '\'')
			{
				pStartCh = ++pCrtCh;
				if(*pCrtCh == '\\')
				{
					pCrtCh++;
					switch(*pCrtCh)
					{
						case 'n': case 'r': case 't': case '\\': case '\'': case '\"': break;
						default:
						Tkerr(addTk(END), "invalid escape sequence in character constant");
					}
					pCrtCh++;
				}
				else
				{
					if(!*pCrtCh || *pCrtCh == '\'' || *pCrtCh == '\n')
						Tkerr(addTk(END), "invalid character constant");
					pCrtCh++;	
				}
				if(*pCrtCh != '\'')
					Tkerr(addTk(END), "missing closing ' in character constant");
				pCrtCh++;
				tk = addTk(CT_CHAR);	
				if(pStartCh[0] == '\\')
				{
					switch(pStartCh[1])
					{
						case 'n': tk -> i = '\n'; break;
						case 'r': tk -> i = '\r'; break;
						case 't': tk -> i = '\t'; break;
						case '\\': tk -> i = '\\'; break;
						case '\'': tk -> i = '\''; break;
						case '\"': tk -> i = '\"'; break; 
					}
				}
				else
				{
					tk -> i = pStartCh[0];
				}
				return CT_CHAR;
			}

			else if(ch == 0)
			{
				addTk(END);
				return END;
			}
			else
			{
				Tkerr(addTk(END), "Invalid character");
			}
			break;

		case 1:
			if(isalnum(ch) || ch == '_') pCrtCh++;
			else state = 2;
			break;

		case 2:
			nCh = pCrtCh - pStartCh;

			if(nCh == 5 && !memcmp(pStartCh, "break", 5)) tk = addTk(BREAK);
			else if(nCh == 4 && !memcmp(pStartCh, "char", 4)) tk = addTk(CHAR);
			else if(nCh == 6 && !memcmp(pStartCh, "double", 6)) tk = addTk(DOUBLE);
			else if(nCh == 4 && !memcmp(pStartCh, "else", 4)) tk = addTk(ELSE);
			else if(nCh == 3 && !memcmp(pStartCh, "for", 3)) tk = addTk(FOR);
			else if(nCh == 2 && !memcmp(pStartCh, "if", 2)) tk = addTk(IF);
			else if(nCh == 3 && !memcmp(pStartCh, "int", 3)) tk = addTk(INT);
			else if(nCh == 6 && !memcmp(pStartCh, "return", 6)) tk = addTk(RETURN);
			else if(nCh == 6 && !memcmp(pStartCh, "struct", 6)) tk = addTk(STRUCT);
			else if(nCh == 4 && !memcmp(pStartCh, "void", 4)) tk = addTk(VOID);
			else if(nCh == 5 && !memcmp(pStartCh, "while", 5)) tk = addTk(WHILE);
			else
			{
				tk = addTk(ID);
				tk -> text = createString(pStartCh, pCrtCh);
			}
			return tk -> code;

		case 3:
			if(ch == '=')
			{
				pCrtCh++;
				state = 4;
			}
			else state = 5;
			break;

		case 4:
			addTk(EQUAL);
			return EQUAL;

		case 5:
			addTk(ASSIGN);
			return ASSIGN;

		case 6:
			if(ch == 'x' || ch == 'X') //hex
			{
				pCrtCh++;
				state =7;
			}	
			else if('0' < ch && ch <= '7') //otal
			{
				pCrtCh++;
				state = 8;
			}
			else if((ch >= '8' && ch <= '9') || isalpha(ch))
			{
				Tkerr(addTk(END), "invalid octal digit");
			}
			else if(ch == '.')
			{
				pCrtCh++;
				state = 9;
			}
			else
			{
				tk = addTk(CT_INT);
				tk -> i = 0;
				return CT_INT;
			}
			break;

		case 7:
			if(isxdigit(ch))
			{
				pCrtCh++;
			}	
			else if(pCrtCh > pStartCh + 2)
			{
				tk = addTk(CT_INT);
				char *str = createString(pStartCh, pCrtCh);
				tk -> i = strtol(str, NULL, 16);
				free(str);
				return CT_INT;
			}
			else
			{
				Tkerr(addTk(END), "invalid hex constant");
			}
			break;

			case 8:
			if('0' < ch && ch <= '7')
			{
				pCrtCh++;
			}
			else
			{
				tk = addTk(CT_INT);
				char *str = createString(pStartCh, pCrtCh);
				tk -> i = strtol(str, NULL, 8);
				free(str);
				return CT_INT;
			}
			break;

		case 9:
			if('0' <= ch && ch <= '9')
			{
				pCrtCh++;
				state = 10;
			}	
			else
			{
				Tkerr(addTk(END), "invalid real constant after 0.");
			}
			break;

		case 10:
			if('0' <= ch && ch <= '9')
			{
				pCrtCh++;
			}	
			else if(ch == 'e' || ch == 'E')
			{
				pCrtCh++;
				state = 11;
			}
			else
			{
				tk = addTk(CT_REAL);
				char *str = createString(pStartCh, pCrtCh);
				tk -> r = strtod(str, NULL);
				free(str);
				return CT_REAL;
			}
			break;
			
		case 11:
			if(ch == '+' || ch == '-')
			{
				pCrtCh++;
				state = 13;
			}	
			else if('0' <= ch && ch <= '9')
			{
				pCrtCh++;
				state = 14;
			}
			else
			{
				Tkerr(addTk(END), "invalid exponent");
			}
			break;

		case 12:
			if('0' <= ch && ch <= '9')
			{
				pCrtCh++;
			}	
			else if(ch == '.')
			{
				pCrtCh++;
				state = 9;
			}
			else if(ch == 'e' || ch == 'E')
			{
				pCrtCh++;
				state = 11;
			}
			else
			{
                    tk = addTk(CT_INT);
					char *str = createString(pStartCh, pCrtCh);
					tk -> i = strtol(str, NULL, 10);
					free(str);
                    return CT_INT;				
			}
			break;

		case 13:		//after the sign +/- sign in exponent
			if('0' <= ch && ch <= '9')
			{
				pCrtCh++;
				state = 14;
			}	
			else
			{
				Tkerr(addTk(END), "invalid exponent sign");
			}
			break;

		case 14:	//exponent digits
			if('0' <= ch && ch <= '9')
			{
				pCrtCh++;
			}	
			else
			{
                    tk = addTk(CT_REAL);
					char *str = createString(pStartCh, pCrtCh);
					tk -> r = strtod(str, NULL);
					free(str);
                    return CT_REAL;				
			}
			break;					

			case 15:
				if(ch == '=')
				{
					pCrtCh++;
					state = 16;
				}
				else
				{
					state = 17;
				}				
			break;

			case 16:
			addTk(NOTEQ);
			return NOTEQ;

			case 17:
			addTk(NOT);
			return NOT;

			case 18:
				if(ch == '=')
				{
					pCrtCh++;
					state = 19;
				}
				else
				{
					state = 20;
				}
				break;

			case 19:
				addTk(LESSEQ);
				return LESSEQ;

			case 20:
				addTk(LESS);
				return LESS;

			case 21:		
				if(ch == '=')
				{
					pCrtCh++;
					state = 22;
				}
				else
				{
					state = 23;
				}
				break;

			case 22:
				addTk(GREATERQ);
				return GREATERQ;

			case 23:
				addTk(GREATER);
				return GREATER;		
		}
	}
}

void showTokens(const Token *tokens)
{
    for (const Token *tk = tokens; tk; tk = tk->next)
    {
        // Print line number and token name/type
        printf("%d:", tk->line);

        switch (tk->code)
        {
            case ID:
                printf("ID:%s ", tk->text);   // Token name (e.g., "speed")
                break;
            case CT_INT:
                printf("CT_INT:%ld ", tk->i);  // Integer constant value (e.g., 70)
                break;
            case CT_REAL:
                printf("CT_REAL:%f ", tk->r);  // Real constant value
                break;
            case CT_CHAR:
                printf("CT_CHAR:'%c' ", (char)tk->i);  // Character constant value
                break;
            case CT_STRING:
                printf("\"%s\" ", tk->text);  // String constant value
                break;

            // Print out the specific token types
            case BREAK: printf("BREAK "); break;
            case CHAR: printf("CHAR "); break;
            case DOUBLE: printf("DOUBLE "); break;
            case ELSE: printf("ELSE "); break;
            case FOR: printf("FOR "); break;
            case IF: printf("IF "); break;
            case INT: printf("INT "); break;
            case RETURN: printf("RETURN "); break;
            case STRUCT: printf("STRUCT "); break;
            case VOID: printf("VOID "); break;
            case WHILE: printf("WHILE "); break;

            case COMMA: printf("COMMA "); break;
            case SEMICOLON: printf("SEMICOLON "); break;
            case LPAR: printf("LPAR "); break;
            case RPAR: printf("RPAR "); break;
            case LBBRACKET: printf("LBRACKET "); break;
            case RBRACKET: printf("RBRACKET "); break;
            case LACC: printf("LACC "); break;
            case RACC: printf("RACC "); break;
            case ADD: printf("ADD "); break;
            case SUB: printf("SUB "); break;
            case MUL: printf("MUL "); break;
            case DIV: printf("DIV "); break;
            case DOT: printf("DOT "); break;
            case ASSIGN: printf("ASSIGN "); break;
            case EQUAL: printf("EQUAL "); break;
            case NOTEQ: printf("NOTEQ "); break;
            case LESS: printf("LESS "); break;
            case LESSEQ: printf("LESSEQ "); break;
            case GREATER: printf("GREATER "); break;
            case GREATERQ: printf("GREATERQ "); break;
            case AND: printf("AND "); break;
            case OR: printf("OR "); break;
            case NOT: printf("NOT "); break;
            case END: printf("END "); break;

            default:
                printf("UNKNOWN TOKEN: %d ", tk->code);
                break;
        }
        printf("\n");
    }
}

void *safeAlloc(size_t nBytes){
	void *p=malloc(nBytes);
	if(!p)err("not enough memory");
	return p;
	}

char *loadFile(const char *fileName){
	FILE *fis=fopen(fileName,"rb");
	if(!fis)err("unable to open %s",fileName);
	fseek(fis,0,SEEK_END);
	size_t n=(size_t)ftell(fis);
	fseek(fis,0,SEEK_SET);
	char *buf = (char*)safeAlloc((size_t)n+1);
	size_t nRead=fread(buf,sizeof(char),(size_t)n,fis);
	fclose(fis);
	if(n!=nRead)err("cannot read all the content of %s",fileName);
	buf[n]='\0';
	return buf;
	}




