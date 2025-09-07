#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_sandbox(void)
{
	argint(0, &myproc()->sandboxmask);

	return 0;
}