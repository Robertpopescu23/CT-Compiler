
int global_var;

struct MyStruct {
    int member_var;
    double member_arr[5];
};

void func_global_access() {
    global_var = 20; 
}


int func_params_locals(int param1, char param2) {
    int local_var1;

    param1 = global_var; 
    local_var1 = param1 + 1;
    {

        int inner_local;
        inner_local = 5;
        local_var1 = inner_local; 
        global_var = inner_local; 
        param2 = 'a'; 
    }

    return local_var1;
}


int shadow_global;
void func_shadowing(int shadow_global_param) {
    int local_shadow;
    local_shadow = 200;
    shadow_global_param = 5; 
    global_var = shadow_global_param; 

    local_shadow = local_shadow + shadow_global_param;
}

struct MyStruct s_instance;
void struct_access() {
    s_instance.member_var = 1;
    s_instance.member_arr[0] = 1.0;

    int member_var_local;
    member_var_local = 5; 
    member_var_local = member_var_local + 1;
}



struct TestStructRedef_NoError {
    int a;

    int b;
};


void func_redefinition_test() {
    int x_redef;
    x_redef = 1;

    x_redef = x_redef + 1; 
    {
        char y_redef;
        y_redef = 'c';

        y_redef = y_redef + 1;
    }
}



void use_undeclared_test {

}


void func_declared_later_test_callee() {

} 

void call_later_defined_test() {
    func_declared_later_test_callee(); 
}


int main() {
    func_global_access();
    func_params_locals(1, 'c');
    func_shadowing(50);
    struct_access();
    use_undeclared_test(); 
    func_redefinition_test(); 
    call_later_defined_test();
    return 0;
} 