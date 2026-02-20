// Test file for Parser (SY)

// Struct declarations
struct Point {
    int x;
    double y;
};

struct EmptyStruct { }; // Empty struct

struct Vector {
    struct Point start;
    struct Point end;
    char name[20];
};

// Global variable declarations
int globalVar1;
double globalVar2, globalVar3[10];
struct Point globalPoint;
char globalString[];

// Function declarations
void no_args_no_return() {
    // empty body
}

int simple_return(int a) {
    return a + 1;
}

char process_chars(char c1, char c2[]) {
    c1 = 'a';
    c2[0] = 'b';
    return c1;
}

struct Point createPoint(int x_val, double y_val) {
    struct Point p;
    p.x = x_val;
    p.y = y_val;
    return p;
}

// Main function with various statements
int main() { // Parameter as array of arrays (if supported by your syntax)
    int i, j;
    double d;
    d = 0.5;
    struct Vector v1;

    // Variable declarations within a block
    {
        int local_block_var;
        local_block_var = 10;
        i = local_block_var;
    }

    // If-Else statement
    if (i > 0) {
        j = i * 2;
    } else {
        j = 1;
    }

    if (j == 2) i = 0; // Single line if without else

    // While loop
    while (i < 10) {
        i = i + 1;
        if (i == 5) {
            break; // Break statement
        }
    }

    // For loop
    for (j = 0; j < 5; j = j + 1) {
        d = d + 0.1;
    }

    for (;;) { // Infinite loop, or loop with conditions elsewhere
        break;
    }
    
    for (i = 0; i > 2 ; i++) { // Missing condition
        // do stuff
    }

    for ( ; i < 10 ; ) { // Missing init and increment
        i = i + 1;
    }

    // Expression statements
    v1.start.x = 10;
    v1.end = createPoint(100, 200.5);
    simple_return(v1.start.x);

    // Return statement
    return 0;
}

void another_function() {
    int x_single_dim[10]; // Multi-dimensional array (if parser supports its declaration)
    x_single_dim[0] = 100;
    return; // Return from void function
} 