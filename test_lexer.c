// Test file for Lexical Analyzer (LA) - Valid C constructs

int global_int_arr[10]; // Declaration for the array used below

// Keywords used in declarations and statements
int global_int_for_lexer;
char global_char_for_lexer;
double global_double_for_lexer;
struct MyLexerStruct {
    int member;
};
void lexer_test_func_void_return() {
    if (1) {
        // if keyword
    } else {
        // else keyword
    }
    while (0) {
        // while keyword
    }
    for (global_int_for_lexer = 0; global_int_for_lexer < 1; global_int_for_lexer = global_int_for_lexer + 1) {
        // for keyword
        break; // break keyword
    }
    return; // return keyword
}

// Identifiers
int myVar_lx, another_ID_lx, _id123_lx, id_With_Caps_lx;

// Constants
int int_consts_lx() {
    int a;
    a = 0;
    int b;
    b = 123;
    int c;
    c = 0x1A; // Hexadecimal
    int d;
    d = 0XFf; // Hexadecimal
    int e;
    e = 077;  // Octal
    int f;
    f= 01;
    return a + b + c + d + e + f;
}

double real_consts_lx() {
    double r1;
    r1 = 0.0;
    double r2;
    r2 = 1.23;
    double r3;
    r3 = 0.5;
    return r1 + r2 + r3;
}

char char_consts_lx() {
    char c1;
    c1 = 'a';
    char c2;
    c2 = '\n';
    char c3;
    c3 = '\t';
    char c4;
    c4 = '\'';
    char c5;
    c5 = '\"';
    char c6;
    c6 = '\\';
    char c7;
    c7 = ' '; // Space character
    return c1;
}


// Operators and Delimiters in use
int operators_delimiters_lx(int p1, int p2) {
    int result;
    result = (p1 + p2) * p1 - p2 / 1; 
    if (p1 == p2 && p1 != 0 || p1 > p2 && p1 >= p2 || p1 < p2 && p1 <= p2) { 
        result = !p1; // NOT
    }
    global_int_arr[0] = result; 
                            
    struct MyLexerStruct mls_instance;
    mls_instance.member = 5; // DOT
    
    return result;
}

int main() { // main function using various tokens
    global_char_for_lexer = '\n';
    global_double_for_lexer = 3.14159;
    if (global_double_for_lexer > 0.0) {
        // do something
    }
    lexer_test_func_void_return();
    int_consts_lx();
    real_consts_lx();
    char_consts_lx();
    operators_delimiters_lx(5, 3);
    return 0;
}