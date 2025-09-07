#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define FREE        0x0
#define RUNNING     0x1
#define RUNNABLE    0x2

#define STACK_SIZE  8192
#define MAX_THREAD  4

struct context {
  uint64 ra;
  uint64 sp;

  // callee-saved
  uint64 s0;
  uint64 s1;
  uint64 s2;
  uint64 s3;
  uint64 s4;
  uint64 s5;
  uint64 s6;
  uint64 s7;
  uint64 s8;
  uint64 s9;
  uint64 s10;
  uint64 s11;
};

struct thread {
  char              stack[STACK_SIZE];
  int               state;
  struct context    context;
};
struct thread all_thread[MAX_THREAD];
struct thread *current_thread;

extern void thread_switch(struct context*, struct context*);
              
void thread_init()
{
    for (int i = 0; i < MAX_THREAD; i++) {
        all_thread[i].state = FREE;
        memset(&all_thread[i].context, 0, sizeof(all_thread[i].context));
    }

    current_thread = &all_thread[0];
    current_thread->state = RUNNING;
}

void thread_schedule()
{
    struct thread *t, *next_thread;

    next_thread = 0;
    t = current_thread + 1;
    for(int i = 0; i < MAX_THREAD; i++){
        if(t >= all_thread + MAX_THREAD)
            t = all_thread;

        if(t->state == RUNNABLE) {
            next_thread = t;
            break;
        }
        t++;
    }


    if (next_thread == 0) {
        printf("thread_schedule: no runnable threads\n");
        exit(-1);
    }

    if (current_thread != next_thread) {
        next_thread->state = RUNNING;
        t = current_thread;
        current_thread = next_thread;
        thread_switch(&t->context, &current_thread->context);
    }
}

void thread_create(void (*func)())
{
    struct thread *t;

    for (t = all_thread; t < all_thread + MAX_THREAD; t++) {
        if (t->state == FREE) break;
    }

    if (t == all_thread + MAX_THREAD) {
        printf("thread_create: no free thread slot\n");
        exit(-1);
    }

    t->state = RUNNABLE;

    memset(&t->context, 0, sizeof t->context);
    t->context.ra = (uint64)func;
    t->context.sp = (uint64)(t->stack + STACK_SIZE);
}

void thread_yield()
{
    current_thread->state = RUNNABLE;
    thread_schedule();
}

void thread_exit()
{
    current_thread->state = FREE;
    thread_schedule();
}