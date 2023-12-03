#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"
#include "fcntl.h"

struct {
  struct spinlock lock;
  struct vma vma[NVMA];
} vtable;

void
vmainit()
{
  // don't share vma, so no need to use per-vma lock
  initlock(&vtable.lock, "vtable");
}


// return a vma slot with its lock
struct vma*
vget()
{
  struct vma* v = vtable.vma;
  struct vma* ve = vtable.vma + NVMA;
  acquire(&vtable.lock);
  for (; v != ve; ++v) {
    if (v->used == 0) {
      v->used = 1;
      release(&vtable.lock);
      break;
    }
  }
  if (v == ve)
    panic("vget: no vma slots");
  return v;
}

// lazy-mmap only one page (read to buf + create new mapping)
// page fault handler is virtue, mapping only one page is also virtue
int
mmap(struct vma* v, pagetable_t pagetable, uint64 va)
{
  va = PGROUNDDOWN(va);

  char *mem;
  if((mem = kalloc()) == 0) {
    printf("mmap: kalloc\n");
    return -1;
  }
  memset(mem, 0, PGSIZE);

  struct file *f = v->file;
  int off = v->offset + (va - v->start);

  ilock(f->ip);
  int tot = PGSIZE;
  int n;
  while ((n = readi(f->ip, 0, (uint64)mem, off, tot)) != tot) {
    mem += n;
    tot -= n;
    if (tot == 0)
      break;
  }
  iunlock(f->ip);

  if(mappages(pagetable, va, PGSIZE, (uint64)mem, v->perms) != 0){
    printf("mmap: mappages\n");
    kfree(mem);
    return -1;
  }
  return 0;
}
