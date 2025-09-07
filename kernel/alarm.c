#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64 sys_sigalarm(void)
{
  uint64 addr;
  int ticks;

  argint(0, &ticks);
  argaddr(1, &addr);

  myproc()->ticks = ticks;
  myproc()->handler = addr;

  return 0;
}

uint64 sys_sigreturn(void)
{
  struct proc *p = myproc();
  memmove(p->trapframe, p->alarm_tf, PGSIZE);

  kfree(p->alarm_tf);
  p->alarm_tf = 0;
  p->alarm_on = 0;
  p->cur_ticks = 0;
  return 0;
}