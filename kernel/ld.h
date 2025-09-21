#ifndef __LD_H__
#define __LD_H__

#include "types.h"

void free_imports();

struct symnode {
  char name[32];
  uint64 addr;
  struct symnode *next;
};

struct symlist {
  struct symnode *head;
  struct symnode *tail;
  int valid;
  int ref;
};

struct shdr {
  uint32 name;
  uint32 type;
  uint64 flags;
  uint64 addr;
  uint64 offset;
  uint64 size;
  uint32 link;
  uint32 info;
  uint64 addralign;
  uint64 entsize;
};

struct sym {
  uint32 name;
  uchar info;
  uchar other;
  uint16 shndx;
  uint64 value;
  uint64 size;
};

#endif