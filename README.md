# AtomC Compiler

A front-end compiler for **AtomC** — a simplified subset of the C language. Built entirely in C with no external dependencies, it performs lexical analysis, syntax analysis (parsing), domain analysis (symbol table management), and type analysis.

## Table of Contents

- [Language Features](#language-features)
- [Project Structure](#project-structure)
- [Architecture](#architecture)
- [Building](#building)
- [Usage](#usage)
- [Running Tests](#running-tests)
- [Supported Grammar](#supported-grammar)
- [Limitations](#limitations)

## Language Features

AtomC supports a subset of C including:

| Category | Supported |
|----------|-----------|
| **Types** | `int`, `double`, `char`, `void`, `struct` |
| **Variables** | Global, local, function parameters, single-dimension arrays |
| **Functions** | Declarations with typed parameters and return values |
| **Structs** | Definition with members, nested struct types, member access (`.`) |
| **Control Flow** | `if`/`else`, `while`, `for`, `break`, `return` |
| **Operators** | `+`, `-`, `*`, `/`, `=`, `==`, `!=`, `<`, `<=`, `>`, `>=`, `&&`, `\|\|`, `!` |
| **Constants** | Integer (decimal, hex `0xFF`, octal `077`), real (`3.14`, `1.5e-3`), char (`'a'`, `'\n'`), string (`"hello"`) |
| **Comments** | Line (`//`) and block (`/* */`) |
| **Type Casts** | Explicit casting e.g. `(double)x` |

## Project Structure

```
CT/
├── main.c                    # Entry point — loads file, runs lexer & parser
├── LA.c / LA.h               # Lexical Analyzer (tokenizer)
├── SY.c / SY.h               # Syntax Analyzer (recursive-descent parser) + Type Analysis
├── DA.c / DA.h               # Domain Analysis (symbol table management)
├── test_lexer.c              # Test: keywords, operators, constants, identifiers
├── test_parser.c             # Test: grammar constructs (structs, loops, functions)
├── test_domain_analysis.c    # Test: scoping, symbol table, redefinition checks
├── test_type_analysis.c      # Test: type checking, assignments, function calls
├── example1.c                # Example: loops, hex/real constants
├── example2.c                # Example: structs, functions, arrays
├── aa.c                      # Negative test: multi-dimensional arrays (unsupported)
├── run_tests.sh              # Automated test runner script
├── .gitignore                # Excludes compiled artifacts
└── README.md                 # This file
```

## Architecture

The compiler processes source files through four sequential phases:

```
Source File (.c)
      │
      ▼
┌─────────────┐
│   Lexer     │  LA.c — Reads characters, produces a linked list of Tokens
│   (LA)      │  Handles: identifiers, keywords, constants, operators, comments
└─────┬───────┘
      │ Token stream
      ▼
┌─────────────┐
│   Parser    │  SY.c — Recursive-descent parser consuming tokens
│   (SY)      │  Validates syntax against the AtomC grammar
└─────┬───────┘
      │
      ▼
┌─────────────┐
│  Domain     │  DA.c — Symbol table with scoping
│  Analysis   │  Manages: global/local/argument symbols, struct members,
│   (DA)      │  function arguments, duplicate detection, scope cleanup
└─────┬───────┘
      │
      ▼
┌─────────────┐
│   Type      │  Embedded in SY.c
│  Analysis   │  Checks: assignment compatibility, function argument types,
│             │  return types, operator type rules, arithmetic promotion
└─────────────┘
```

## Building

**Prerequisites:** GCC or Clang (any C99-compatible compiler)

```bash
# Compile the compiler
gcc -Wall -Wextra -o atomc main.c LA.c SY.c DA.c
```

## Usage

```bash
# Analyze a C source file
./atomc <source_file.c>
```

The compiler reads the input file, tokenizes it, parses the syntax, and performs domain + type analysis. If the program is valid, it exits silently with code `0`. If there are errors, it prints an error message with the line number and exits with a non-zero code.

**Examples:**

```bash
# Valid program — exits silently
./atomc test_lexer.c

# Program with errors — prints error and line number
./atomc aa.c
# Output: error in line 1:Invalid character
```

## Running Tests

An automated test script is included:

```bash
chmod +x run_tests.sh
./run_tests.sh
```

The script:
1. Compiles the compiler from source
2. Runs it against valid test files (expects exit code `0`)
3. Runs it against files with known errors (expects non-zero exit code)
4. Prints a colored summary

**Expected output:**

```
  Valid tests passed:       2
  Expected errors caught:   5
  Failures:                 0
  Total tests:              7

🎉 All tests behaved as expected!
```

## Supported Grammar

The parser implements a recursive-descent parser for the following grammar (simplified EBNF):

```
unit         = ( declStruct | declFunc | declVar )* END
declStruct   = STRUCT ID LACC declVar* RACC SEMICOLON
declVar      = typeBase ID arrayDecl? ( COMMA ID arrayDecl? )* SEMICOLON
typeBase     = INT | DOUBLE | CHAR | STRUCT ID
arrayDecl    = LBRACKET ( CT_INT )? RBRACKET
declFunc     = ( typeBase | VOID ) ID LPAR funcArg? ( COMMA funcArg )* RPAR stmCompound
funcArg      = typeBase ID arrayDecl?
stmCompound  = LACC ( declVar | stm )* RACC
stm          = stmCompound | IF | WHILE | FOR | BREAK | RETURN | expr SEMICOLON
expr         = exprAssign
exprAssign   = exprUnary ASSIGN exprAssign | exprOr
exprOr       = exprAnd ( OR exprAnd )*
exprAnd      = exprEq ( AND exprEq )*
exprEq       = exprRel ( ( EQUAL | NOTEQ ) exprRel )*
exprRel      = exprAdd ( ( LESS | LESSEQ | GREATER | GREATERQ ) exprAdd )*
exprAdd      = exprMul ( ( ADD | SUB ) exprMul )*
exprMul      = exprCast ( ( MUL | DIV ) exprCast )*
exprCast     = LPAR typeName RPAR exprCast | exprUnary
exprUnary    = ( SUB | NOT ) exprUnary | exprPostfix
exprPostfix  = exprPrimary ( LBRACKET expr RBRACKET | DOT ID )*
exprPrimary  = ID ( LPAR ( expr ( COMMA expr )* )? RPAR )? | CT_INT | CT_REAL | CT_CHAR | CT_STRING | LPAR expr RPAR
```

## Limitations

This is a front-end only compiler (no code generation). The following features are **not supported**:

- **Code generation** — no assembly/bytecode/VM output
- **Preprocessor** — `#include`, `#define`, etc. are not handled
- **Pointers** — `int *p` syntax is not in the grammar
- **Multi-dimensional arrays** — only `int a[10]` is supported, not `int a[2][3]`
- **Increment/Decrement operators** — `++`/`--` tokens are defined but not produced by the lexer
- **Built-in/external functions** — `puti()`, `puts()`, etc. are not pre-registered in the symbol table
- **Additional control flow** — `switch`/`case`, `do...while`, `goto` are not implemented
- **Bitwise operators** — `&`, `|`, `^`, `~`, `<<`, `>>` are not supported (only `&&` and `||` logical operators)
