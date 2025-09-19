#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "syscall.h"
#include "defs.h"

// Fetch the uint64 at addr from the current process.
int
fetchaddr(uint64 addr, uint64 *ip)
{
  struct proc *p = myproc();
  if(addr >= p->sz || addr+sizeof(uint64) > p->sz) // both tests needed, in case of overflow
    return -1;
  if(copyin(p->pagetable, (char *)ip, addr, sizeof(*ip)) != 0)
    return -1;
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Returns length of string, not including nul, or -1 for error.
int
fetchstr(uint64 addr, char *buf, int max)
{
  struct proc *p = myproc();
  if(copyinstr(p->pagetable, buf, addr, max) < 0)
    return -1;
  return strlen(buf);
}

static uint64
argraw(int n)
{
  struct proc *p = myproc();
  switch (n) {
  case 0:
    return p->trapframe->a0;
  case 1:
    return p->trapframe->a1;
  case 2:
    return p->trapframe->a2;
  case 3:
    return p->trapframe->a3;
  case 4:
    return p->trapframe->a4;
  case 5:
    return p->trapframe->a5;
  }
  panic("argraw");
  return -1;
}

// Fetch the nth 32-bit system call argument.
void
argint(int n, int *ip)
{
  *ip = argraw(n);
}

void 
arglong(int n, uint64 *lp) {
  *lp = argraw(n);
}

// Retrieve an argument as a pointer.
// Doesn't check for legality, since
// copyin/copyout will do that.
void
argaddr(int n, uint64 *ip)
{
  *ip = argraw(n);
}

// Fetch the nth word-sized system call argument as a null-terminated string.
// Copies into buf, at most max.
// Returns string length if OK (including nul), -1 if error.
int
argstr(int n, char *buf, int max)
{
  uint64 addr;
  argaddr(n, &addr);
  return fetchstr(addr, buf, max);
}

#define SYSCALL_LIST \
    X(SYS_fork,       fork) \
    X(SYS_exit,       exit) \
    X(SYS_wait,       wait) \
    X(SYS_pipe,       pipe) \
    X(SYS_read,       read) \
    X(SYS_kill,       kill) \
    X(SYS_exec,       exec) \
    X(SYS_fstat,      fstat) \
    X(SYS_chdir,      chdir) \
    X(SYS_dup,        dup) \
    X(SYS_getpid,     getpid) \
    X(SYS_sbrk,       sbrk) \
    X(SYS_pause,      pause) \
    X(SYS_uptime,     uptime) \
    X(SYS_open,       open) \
    X(SYS_write,      write) \
    X(SYS_mknod,      mknod) \
    X(SYS_unlink,     unlink) \
    X(SYS_link,       link) \
    X(SYS_mkdir,      mkdir) \
    X(SYS_close,      close) \
    X(SYS_poweroff,   poweroff) \
    X(SYS_trace,      trace) \
    X(SYS_sigalarm,   sigalarm) \
    X(SYS_sigreturn,  sigreturn) \
    X(SYS_sandbox,    sandbox) \
    X(SYS_log_time,   log_time) \
    X(SYS_mmap,       mmap) \
    X(SYS_munmap,     munmap) \
    X(SYS_load,       load) \
    X(SYS_symbol,     symbol) \

// Syscalls
#define X(num, name) extern uint64 sys_##name(void);
SYSCALL_LIST
#undef X

// An array mapping syscall numbers from syscall.h
// to the function that handles the system call.
static uint64 (*syscalls[])(void) = {
#define X(num, name) [num] = sys_##name,
  SYSCALL_LIST
#undef X
};

// An array mapping syscall numbers to their names.
static char *syscallnames[] = {
#define X(num, name) [num] = #name,
  SYSCALL_LIST
#undef X
};

void
syscall(void)
{
  int num;
  struct proc *p = myproc();

  num = p->trapframe->a7;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    // Use num to lookup the system call function for num, call it,
    // and store its return value in p->trapframe->a0
    if(p->sandboxmask & 1 << num)
      p->trapframe->a0 = -1;
    else
      p->trapframe->a0 = syscalls[num]();
  } else {
    printf("%d %s: unknown sys call %d\n",
            p->pid, p->name, num);
    p->trapframe->a0 = -1;
  }

  if (p->tracemask & 1 << num) {
    printf("%d: syscall %s -> %ld\n", 
        p->pid, syscallnames[num], p->trapframe->a0);
  }
}
