struct MyPoint { int x; double y; };
struct AnotherStruct { int data; };

int global_int_arr[10];
char global_char_val;


struct MyPoint process_point(struct MyPoint p_param, int scale) {
    struct MyPoint result;
    result.x = p_param.x * scale;
    result.y = p_param.y * (double)scale; 
    return result;
}

void type_tests_in_func(int int_param, double double_param, char char_array_param[]) {
    int local_int1, local_int2;
    double local_double;
    char local_char;
    struct MyPoint pt1, pt2;
    struct AnotherStruct as1;


    local_int1 = int_param;
    local_double = double_param;
    local_char = 'a';
    local_int2 = local_char; 
    local_double = local_int1; 
    pt1 = process_point(pt2, local_int1); 
    global_char_val = char_array_param[0]; 

    local_int1 = local_int2 + int_param - 5;
    local_double = local_double * double_param / 2.0;
    local_int1 = local_int1 + local_char; 

    // Relational and Equality Operations - Valid
    if (local_int1 > local_int2) { /* ... */ }
    if (local_double == (double)int_param) { /* ... */ }
    if (local_char != 'b') { /* ... */ }
    if (pt1 == pt2) {} // Valid: Comparing two instances of the same struct type (checks for type equality, not deep value)
 


    if (local_int1 && (local_double > 0.0)) {  }
    if (!local_int2) {  }

    local_int1 = global_int_arr[0];
    global_int_arr[1] = local_int2;
    local_char = char_array_param[int_param]; // Index can be an expression

    // Array Indexing - Invalid
               // ERROR: Subscripting a non-array (struct)

    // Struct Member Access - Valid
    pt1.x = 10;
    local_double = pt2.y;

    // Struct Member Access - Invalid
    // pt1.z = 100;                     // ERROR: Member 'z' not found in struct MyPoint
    // local_int1.x = 5;                // ERROR: 'local_int1' is not a struct
    // as1.x = 10;                      // ERROR: Member 'x' not found in struct AnotherStruct

    // Function Calls - Valid
    pt1 = process_point(pt2, 5);

    // Function Calls - Invalid
    // pt1 = process_point(local_int1, pt2); // ERROR: Argument type mismatch
    // pt1 = process_point(pt2);             // ERROR: Too few arguments
    // pt1 = process_point(pt2, 5, 10);    // ERROR: Too many arguments
    // local_int1 = process_point(pt1, 1); // ERROR: Return type mismatch (struct MyPoint to int)

    // Conditional expressions (if, while, for) - Valid (scalar)
    if (local_int1) { /* ... */ }
    while (local_char) { /* ... */ }
    for (local_int1 = 0; local_int1 < 10; local_int1 = local_int1 + 1) { if (local_int1) break; }

    // Conditional expressions - Invalid
    // if (pt1) { /* ... */ }              // ERROR: Struct used as condition
    // while (global_int_arr) { /* ... */ } // ERROR: Array used as condition (pointer semantics differ)

    // Return statements
    return; // Valid for void function
}

int main() {
    struct MyPoint p_main1, p_main2;
    int int_val;
    int_val = 10;
    char char_arr[20];

    p_main1.x = 1;
    p_main1.y = 1.0;

    p_main2 = process_point(p_main1, 2);

    // Return statement tests (for main, which is int)
    // type_tests_in_func(1, 2.0, char_arr); // ERROR: if type_tests_in_func was not void and returned wrong type for main
    // return p_main1; // ERROR: Returning struct MyPoint from int function main

    return 0; // Valid
}

void test_void_return() {
    // return 5; // ERROR: Returning value from void function
    return; // Valid
}

int test_no_return_non_void() {
    int x;
    x = 5;
    // Implicit return without value from non-void function would typically be an error or warning.
    return x; // Added explicit return to make it valid.
} 