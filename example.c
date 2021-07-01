#include <stdio.h>

#define Mux2(cond, a, b) ((cond)?(a):(b))

int add(int a,int b)
{
    return (a + b); 
}

int sub(int a, int b)
{
    return (a - b); 
}

int mul(int a, int b)
{
    return (a * b); 
}

int divide(int a, int b)
{
    return (a / b); 
}

int calc(int a, int b, int (*calc_func)(int , int))
{
    return calc_func(a, b); 
}

typedef int (*calc_func)(int, int);
calc_func funcs[] = {add, sub, mul, divide};
char *func_names[] = {"add", "sub", "mul", "divide"};

int main(int argc, const char** argv) {
    
    int num = 100;
    printf("input a number: ");
    scanf("%i", &num);

    int a = 6, b = 7;

    // simple loop
    int loop = 100;
    for (int i = 0; i < loop; i++) {
        // a series of indirect jump
        calc_func func = funcs[i%4];
        int c = calc(a, b, func);
        printf("%p <%s>, value c = %d\n", func, func_names[i%4], c);
    }

    return 0;
}