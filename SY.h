#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include "LA.h"

void tkerr(const Token *tk, const char *fmt, ...);

void parse(Token *tokens);