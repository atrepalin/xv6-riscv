#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"
#include "mappings.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"

uint64 
sys_mmap(void) {
  uint64 addr;
  size_t length, offset;
  int prot, flags, fd;

  argaddr(0, &addr);
  arglong(1, &length);
  argint(2, &prot);
  argint(3, &flags);
  argint(4, &fd);
  arglong(5, &offset);

  struct proc *p = myproc();
  
  uint64 start_addr = PGROUNDUP(p->sz);

  struct vm_area *vm = 0;

  for(int i = 0; i < VMA_SIZE; i++) {
    if(!p->vma[i].valid) {
      vm = &p->vma[i];
      break;
    }
  }

  if(vm) {
    vm->valid = 1;
    vm->start_ad = start_addr;
    vm->end_ad = start_addr + length;
    vm->len = length;
    vm->prot = prot;
    vm->flags = flags;
    vm->fd = fd;
    vm->file = p->ofile[fd];
    vm->file->ref++;
  } else {
    return (uint64)MAP_FAILED;
  }

  if(flags == MAP_SHARED) {
    if(!vm->file->writable && (prot & PROT_WRITE)) {
      return (uint64)MAP_FAILED;
    }
  }

  p->sz = PGROUNDUP(start_addr + length) + 1;
  return start_addr;
}

void mmap_read(struct file *f, uint64 va, int off, int size) {
  ilock(f->ip);
  readi(f->ip, 1, va, off, size);
  iunlock(f->ip);
} 

void free_vma(pagetable_t pagetable, uint64 start, uint64 end) {
  pte_t *pte;
  for(int i = start; i <= end; i += PGSIZE){
    if((pte = walk(pagetable, i, 0)) == 0) {
      if(*pte & PTE_V) {
        uvmunmap(pagetable, i, PGSIZE, 0);
      }
    }
  }
}

void free_all_vma(struct proc *p) {
  struct vm_area *vm = 0;
  for(int i = 0; i < VMA_SIZE; i++) {
    if(p->vma[i].valid == 0) {
      continue;
    }
    vm = &p->vma[i];
    vm->valid = 0;

    free_vma(p->pagetable, vm->start_ad, vm->end_ad);

    if(vm->file) {
      vm->file->ref--;
    }
  }
}

void copy_vma(struct vm_area *dst, struct vm_area *src) {
  dst->valid = 1;
  dst->start_ad = src->start_ad;
  dst->end_ad = src->end_ad;
  dst->len = src->len;
  dst->prot = src->prot;
  dst->flags = src->flags;
  dst->fd = src->fd;
  dst->file = src->file;
  dst->file->ref++;
}

void copy_all_vma(struct proc *p, struct proc *np) {
  for(int i = 0; i < VMA_SIZE; i++) {
    if(p->vma[i].valid) {
      struct vm_area *src = 0, *dst = 0;
      src = &p->vma[i];
      
      for(int j = 0; j < VMA_SIZE; j++) {
        if(np->vma[j].valid == 0) {
          dst = &np->vma[j];
          break;
        }
      }

      if(dst) {
        copy_vma(dst, src);
        dst->file = np->ofile[dst->fd];
      }
    }
  }
}

uint64
sys_munmap(void) {
  uint64 addr;
  size_t length;

  argaddr(0, &addr);
  arglong(1, &length);

  uint64 start_base = PGROUNDDOWN(addr);
  uint64 end_base = PGROUNDUP(addr + length);

  struct proc *p = myproc();
  struct vm_area *vm = 0;
  for(int i = 0; i < VMA_SIZE; i++) {
    if(p->vma[i].valid && 
        p->vma[i].start_ad <= start_base &&
        end_base <= p->vma[i].end_ad) {
      vm = &p->vma[i];
      break;
    }
  }

  if(!vm) {
    return -1;
  }

  if(vm->start_ad == start_base && end_base < vm->end_ad) {
    vm->start_ad = end_base;
    vm->len -= length;
  } else if(vm->start_ad < start_base && end_base == vm->end_ad) {
    vm->end_ad = start_base - 1;
    vm->len = length;
  } else if(vm->start_ad == start_base && vm->end_ad == end_base) {
    vm->file->ref--;
    vm->valid = 0;
    vm->len = 0;
  } 
  
  if(vm->flags & MAP_SHARED) {
    struct file *f = vm->file;

    f->writable = 1;
    filewrite(f, addr, length);
  }

  free_vma(p->pagetable, start_base, end_base);

  return 0;
}