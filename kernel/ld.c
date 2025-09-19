#include "ld.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "elf.h"

extern int loadseg(pde_t *, uint64, struct inode *, uint, uint);
extern int flags2perm(int flags);

#define SHT_SYMTAB  2

#define ELF_ST_BIND(i)   ((i) >> 4)
#define ELF_ST_TYPE(i)   ((i) & 0xf)

#define STB_GLOBAL  1
#define STT_FUNC    2

int
sys_load(void) {
  char path[MAXPATH];

  argstr(0, path, MAXPATH);

  int i, off;
  struct elfhdr elf;
  struct inode *ip;
  struct proghdr ph;
  struct proc *p = myproc();

  begin_op();

  if((ip = namei(path)) == 0){
    end_op();
    return -1;
  }
  ilock(ip);

  if(readi(ip, 0, (uint64)&elf, 0, sizeof(elf)) != sizeof(elf))
    goto bad;

  if(elf.magic != ELF_MAGIC)
    goto bad;

  uint64 base = PGROUNDUP(p->sz);

  for(i = 0, off = elf.phoff; i < elf.phnum; i++, off += sizeof(ph)){
    if(readi(ip, 0, (uint64)&ph, off, sizeof(ph)) != sizeof(ph))
      goto bad;
    if(ph.type != ELF_PROG_LOAD)
      continue;
    if(ph.memsz < ph.filesz)
      goto bad;
    if(ph.vaddr + ph.memsz < ph.vaddr)
      goto bad;
    if(ph.vaddr % PGSIZE != 0)
      goto bad;

    uint64 va = base + ph.vaddr;

    uint64 new_sz;
    if((new_sz = uvmalloc(p->pagetable, p->sz, va + ph.memsz, flags2perm(ph.flags))) == 0)
      goto bad;

    p->sz = new_sz;

    if(loadseg(p->pagetable, va, ip, ph.off, ph.filesz) < 0)
      goto bad;
  }

  struct symlist *symlist = 0;

  int handle;

  for(handle = 0; handle < SYM_SIZE; handle++){
    if(!p->symlist[handle].valid){
      symlist = &p->symlist[handle];
      symlist->valid = 1;
      symlist->ref = 1;
      break;
    }
  }

  if(!symlist) goto bad;

  struct shdr sh;

  for(i = 0; i < elf.shnum; i++){
    uint64 shoff = elf.shoff + i * elf.shentsize;
    if(readi(ip, 0, (uint64)&sh, shoff, sizeof(sh)) != sizeof(sh))
      goto bad;

    if(sh.type == SHT_SYMTAB){
      struct shdr strsh;
      uint64 stroff = elf.shoff + sh.link * elf.shentsize;
      if(readi(ip, 0, (uint64)&strsh, stroff, sizeof(strsh)) != sizeof(strsh))
        goto bad;

      char *strtab = kalloc();
      if(!strtab) goto bad;
      if(readi(ip, 0, (uint64)strtab, strsh.offset, strsh.size) != strsh.size){
        kfree(strtab);
        goto bad;
      }

      int nsyms = sh.size / sh.entsize;

      for(int j = 0; j < nsyms; j++){
        struct sym sym;
        uint64 symoff = sh.offset + j * sh.entsize;

        if(readi(ip, 0, (uint64)&sym, symoff, sizeof(sym)) != sizeof(sym))
          break;

        if(sym.name < strsh.size 
          && ELF_ST_BIND(sym.info) == STB_GLOBAL
          && ELF_ST_TYPE(sym.info) == STT_FUNC){
          char *sname = strtab + sym.name;
          
          struct symnode *node = kalloc();
          if(node == 0) goto bad;
          safestrcpy(node->name, sname, sizeof(node->name));
          node->addr = base + sym.value;
          node->next = 0;

          if(symlist->head == 0)
            symlist->head = node;
          else
            symlist->tail->next = node;
          symlist->tail = node;
        }
      }

      kfree(strtab);
    }
  }

  iunlockput(ip);
  end_op();
  ip = 0;
  return handle;
  
bad:
  if(ip){
    iunlockput(ip);
    end_op();
  }
  return -1;
}

int
strcmp(const char *p, const char *q)
{
  while(*p && *p == *q)
    p++, q++;
  return (uchar)*p - (uchar)*q;
}

uint64
sys_symbol(void) {
  char symbol[32];
  int handle;
  
  argstr(0, symbol, 32);
  argint(1, &handle);

  struct proc *p = myproc();
  struct symlist *symlist = &p->symlist[handle];

  if(!symlist->valid) return -1;

  struct symnode *node = symlist->head;

  while(node){
    if(strcmp(symbol, node->name) == 0){
      return node->addr;
    }
    node = node->next;
  }

  return -1;
}