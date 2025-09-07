#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/riscv.h"
#include "user/user.h"

static int a_started, b_started, c_started;

void thread_a()
{
    int i;
    printf("thread_a started\n");
    a_started = 1;

    while(!b_started || !c_started)
        thread_yield();
    
    for (i = 0; i < 10; i++) {
        printf("thread_a %d\n", i);
        thread_yield();
    }

    printf("thread_a: exit after %d\n", i);

    thread_exit();
}

void thread_b()
{
    int i;
    printf("thread_b started\n");
    b_started = 1;

    while(!a_started || !c_started)
        thread_yield();
    
    for (i = 0; i < 10; i++) {
        printf("thread_b %d\n", i);
        thread_yield();
    }

    printf("thread_b: exit after %d\n", i);

    thread_exit();
}

void thread_c()
{
    int i;
    printf("thread_c started\n");
    c_started = 1;

    while(!a_started || !b_started)
        thread_yield();
    
    for (i = 0; i < 10; i++) {
        printf("thread_c %d\n", i);
        thread_yield();
    }

    printf("thread_c: exit after %d\n", i);

    thread_exit();
}

int main() 
{
    thread_init();
    thread_create(thread_a);
    thread_create(thread_b);
    thread_create(thread_c);
    thread_schedule();
    exit(0);
}
