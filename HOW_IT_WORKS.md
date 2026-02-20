# How This Compiler Works — A Beginner's Guide

## What Is a Compiler?

A compiler is a program that **reads source code** (written by a human) and **translates it** into something a computer can understand. Think of it like a translator between two languages — you write in C, and the compiler converts it into instructions the machine can execute.

A full compiler typically has these stages:

```
Your Code (.c file)
      │
      ▼
  ┌──────────────────────┐
  │  1. LEXICAL ANALYSIS │  "What words are in this text?"
  │     (Tokenizer)      │  Breaks code into tokens (words)
  └──────────┬───────────┘
             │
             ▼
  ┌──────────────────────┐
  │  2. SYNTAX ANALYSIS  │  "Is this a valid sentence?"
  │     (Parser)         │  Checks grammar rules
  └──────────┬───────────┘
             │
             ▼
  ┌──────────────────────┐
  │  3. SEMANTIC ANALYSIS│  "Does this sentence make sense?"
  │  (Domain + Type)     │  Checks types, scopes, declarations
  └──────────┬───────────┘
             │
             ▼
  ┌──────────────────────┐
  │  4. CODE GENERATION  │  "Translate to machine language"
  │     (Backend)        │  ⚠️ NOT implemented in this project
  └──────────────────────┘
```

**This project implements stages 1–3** (the entire front-end). It can tell you if your code is correct, but it doesn't generate an executable program.

---

## Stage 1: Lexical Analysis — "Splitting Text into Words"

**File:** `LA.c` / `LA.h` (LA = Lexical Analyzer)

Imagine reading the sentence: `int x = 5 + 3;`

A human sees this as individual words and symbols. The lexer does the same thing for code. It scans character by character and groups them into **tokens**:

```
"int"  →  keyword INT
"x"    →  identifier ID (name = "x")
"="    →  operator ASSIGN
"5"    →  constant CT_INT (value = 5)
"+"    →  operator ADD
"3"    →  constant CT_INT (value = 3)
";"    →  delimiter SEMICOLON
```

### What the lexer recognizes:

| Category | Examples | What it produces |
|----------|----------|-----------------|
| **Keywords** | `int`, `if`, `while`, `return`, `struct` | A specific token code (e.g. `IF`, `WHILE`) |
| **Identifiers** | `myVar`, `count`, `main` | `ID` token with the name stored |
| **Integer numbers** | `42`, `0xFF` (hex), `077` (octal) | `CT_INT` token with numeric value |
| **Real numbers** | `3.14`, `1.5e-3` | `CT_REAL` token with the value |
| **Characters** | `'a'`, `'\n'` | `CT_CHAR` token |
| **Strings** | `"hello world"` | `CT_STRING` token |
| **Operators** | `+`, `-`, `==`, `&&`, `!` | `ADD`, `SUB`, `EQUAL`, `AND`, `NOT` |
| **Delimiters** | `(`, `)`, `{`, `}`, `;`, `,` | `LPAR`, `RPAR`, `LACC`, `RACC`, etc. |
| **Comments** | `// line` or `/* block */` | Skipped entirely (not tokens) |

The output is a **linked list of tokens**, each with a code, a value, and the line number where it appeared.

---

## Stage 2: Syntax Analysis — "Checking Grammar"

**File:** `SY.c` / `SY.h` (SY = Syntax)

Once we have tokens, the parser checks if they form a **valid program** according to the grammar rules. This is like checking if an English sentence follows grammar — "I eat cake" is valid, but "eat I cake" is not.

The parser uses a technique called **recursive descent parsing**. Each grammar rule is a function that tries to match tokens:

```c
// Simplified example of how it works inside:

int declVar() {
    if (typeBase())          // expects: int, double, char, struct
        if (consume(ID))     // expects: a variable name
            if (consume(SEMICOLON))  // expects: ;
                return 1;    // ✅ Valid variable declaration!
    return 0;                // ❌ Not a variable declaration
}
```

### What the parser validates:

| Rule | Example | What it checks |
|------|---------|---------------|
| **Variable declaration** | `int x;` | Type followed by name followed by `;` |
| **Array declaration** | `int arr[10];` | Variable with `[size]` |
| **Struct definition** | `struct Point { int x; int y; };` | `struct` keyword, members inside `{}` |
| **Function definition** | `int add(int a, int b) { return a+b; }` | Return type, name, params, body |
| **If/Else** | `if (x > 0) { ... } else { ... }` | Condition in `()`, statement after |
| **While loop** | `while (i < 10) { ... }` | Condition in `()`, body |
| **For loop** | `for (i=0; i<10; i=i+1) { ... }` | Three expressions separated by `;` |
| **Expressions** | `a + b * c` | Operator precedence (`*` before `+`) |

If anything is wrong — like a missing `;` or mismatched `{}` — the parser prints an error with the line number.

---

## Stage 3a: Domain Analysis — "Who Is Who?"

**File:** `DA.c` / `DA.h` (DA = Domain Analysis)

Domain analysis manages the **symbol table** — a registry of every variable, function, and struct that has been declared. It answers questions like:

- **"Has this name been declared?"** — Prevents using `x` before declaring `int x;`
- **"Is this name already taken?"** — Prevents declaring `int x;` twice in the same scope
- **"Which `x` are we talking about?"** — A local `x` inside a function is different from a global `x`

### How scoping works:

```c
int x = 10;              // global x (depth 0)

void foo() {             // depth 1
    int x = 20;          // local x — "shadows" the global one
    {                    // depth 2
        int x = 30;      // inner local x — shadows the function-level one
        // here, x is 30
    }
    // here, x is 20 again (inner x is gone)
}
// here, x is 10 (global)
```

The symbol table tracks **depth** — each `{` increases depth, each `}` decreases it and removes symbols from that scope.

### What it stores for each symbol:

| Field | Meaning | Example |
|-------|---------|---------|
| `name` | The identifier | `"myVar"` |
| `cls` | Class: variable, function, struct | `CLS_VAR`, `CLS_FUNC`, `CLS_STRUCT` |
| `mem` | Memory: global, local, or argument | `MEM_GLOBAL`, `MEM_LOCAL`, `MEM_ARG` |
| `type` | Type info (base type, array size) | `TB_INT`, `nElements=10` |
| `depth` | Scope nesting level | `0` = global, `1` = function body |

---

## Stage 3b: Type Analysis — "Do the Types Match?"

**Embedded in:** `SY.c`

Type analysis checks that operations **make sense** from a type perspective:

### ✅ Valid operations:
```c
int a = 5;          // int assigned to int — OK
double b = a;       // int assigned to double — OK (implicit conversion)
int c = a + 3;      // int + int = int — OK
double d = a * 1.5; // int * double = double — OK (promotion)
```

### ❌ Invalid operations the compiler catches:
```c
struct Point p;
int x = p;           // ERROR: Can't assign struct to int
p + 5;               // ERROR: Can't add a number to a struct
if (p) { }           // ERROR: Struct can't be used as a condition
foo(1, 2, 3);        // ERROR: Wrong number of arguments
```

### Type rules it enforces:

| Rule | Example |
|------|---------|
| Assignment compatibility | `int = double` ✅, `int = struct` ❌ |
| Arithmetic operands must be scalar | `int + int` ✅, `struct + int` ❌ |
| Conditions must be scalar | `if (int)` ✅, `if (struct)` ❌ |
| Function argument count & types must match | `f(int)` called as `f(1)` ✅, `f("hello")` ❌ |
| Return type must match function declaration | `int f() { return 5; }` ✅, `void f() { return 5; }` ❌ |
| Array index must be integer | `arr[3]` ✅, `arr[1.5]` ❌ |
| Member access only on structs | `point.x` ✅, `integer.x` ❌ |

---

## How to Try It

```bash
# 1. Build the compiler
gcc -Wall -o atomc main.c LA.c SY.c DA.c

# 2. Test with a valid program
./atomc test_lexer.c
# (no output = success!)

# 3. Test with an invalid program
./atomc aa.c
# Output: error in line 1:Invalid character

# 4. Run the full test suite
./run_tests.sh
```

---

## The AtomC Language — Quick Reference

Here's a complete valid AtomC program:

```c
// Struct definition
struct Point {
    int x;
    double y;
};

// Function that creates a point
struct Point makePoint(int x, double y) {
    struct Point p;
    p.x = x;
    p.y = y;
    return p;
}

// Function with arrays
int sum(int arr[], int n) {
    int total;
    total = 0;
    int i;
    for (i = 0; i < n; i = i + 1) {
        total = total + arr[i];
    }
    return total;
}

// Main function
int main() {
    struct Point p;
    p = makePoint(10, 3.14);

    int numbers[5];
    numbers[0] = 1;
    numbers[1] = 2;
    numbers[2] = 3;

    int result;
    result = sum(numbers, 3);

    if (result > 0) {
        // valid!
    } else {
        // also valid!
    }

    return 0;
}
```

### What AtomC does **NOT** support:

| Feature | Standard C | AtomC |
|---------|-----------|-------|
| Pointers | `int *p = &x;` | ❌ Not supported |
| Multi-dim arrays | `int grid[3][3];` | ❌ Only single `[]` |
| `++` / `--` | `i++; --j;` | ❌ Use `i = i + 1;` |
| Preprocessor | `#include <stdio.h>` | ❌ Not handled |
| `switch`/`case` | `switch(x) { case 1: ... }` | ❌ Use `if`/`else` |
| `printf` / `scanf` | `printf("hi");` | ❌ No built-in functions |
| Code execution | Produces `.exe` or runs | ❌ Front-end only (analysis) |
