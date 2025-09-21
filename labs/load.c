#include "kernel/types.h"
#include "user/user.h"

int main()
{
    int foo = load("foo");
    int bar = load("bar");

    int (*sum)(int, int), (*sub)(int, int), (*mul)(int, int), (*div)(int, int);
    void (*foo_main)(void), (*bar_main)(void);

    foo_main = symbol("main", foo);
    bar_main = symbol("main", bar);

    foo_main();
    bar_main();

    if(fork() == 0)
    {
        sum = symbol("sum", foo);
        sub = symbol("sub", foo);
        mul = symbol("mul", bar);
        div = symbol("div", bar);

        printf("%d + %d = %d\n", 4, 5, sum(4, 5));
        printf("%d - %d = %d\n", 4, 5, sub(4, 5));
        printf("%d * %d = %d\n", 4, 5, mul(4, 5));
        printf("%d / %d = %d\n", 10, 5, div(10, 5));
        exit(0);
    }

    wait(0);
    exit(0);
}