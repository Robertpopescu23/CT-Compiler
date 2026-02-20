#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include "LA.h"
#include "SY.h"
#include "DA.h"

extern Token *tokens;
extern char *pCrtCh;

int main(int argc, char **argv)
{

    if(argc != 2)
    {
        fprintf(stderr, "Usage: %s file_name\n", argv[0]);
        return EXIT_FAILURE;
    }

	pCrtCh = loadFile(argv[1]);

	while(1)
	{
		int code = getNextToken();
		if(code == END)
		{
			break;
		}
	}


	crtDepth = 0;
	crtFunc = NULL;
	crtStruct = NULL;
	initSymbols(&symbols);
	//showTokens(tokens);
	parse(tokens);
	freeTokens(tokens);


}