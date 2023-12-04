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
// this name may be not appropriate, though...
struct vma*
valloc()
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

void
vfree(struct vma* v)
{
  v->start = 0;
  v->end = 0;
  v->perms = 0;
  v->flags = 0;
  fileclose(v->file);
  v->file = 0;
  v->offset = 0;
  v->next = 0;
  acquire(&vtable.lock);
  v->used = 0;
  release(&vtable.lock);
}

// lazy-mmap va page, also load va to buf
int
mmap(struct vma* v, pagetable_t pagetable, uint64 va)
{
  // vma start from page align
  // va in vma -> va0 in vma
  uint64 va0 = PGROUNDDOWN(va);

  char *mem;
  if((mem = kalloc()) == 0) {
    printf("mmap: kalloc\n");
    return -1;
  }
  memset(mem, 0, PGSIZE);

  struct file *f = v->file;
  int off = v->offset + (va0 - v->start);
  int rem = v->end - va0;
  if (rem > PGSIZE)
    rem = PGSIZE;

  ilock(f->ip);
  // since maybe mmap-ed size != inode size
  if (off <= f->ip->size)
    readi(f->ip, 0, (uint64)mem, off, rem);
  iunlock(f->ip);

  if(mappages(pagetable, va0, PGSIZE, (uint64)mem, v->perms) != 0){
    printf("mmap: mappages\n");
    kfree(mem);
    return -1;
  }
  return 0;
}

// writeback mmap-ed page from user's va
int
vwrite(struct vma* v, uint64 addr, int n)
{
  // no need to write
  if ((v->perms & PTE_W) == 0 || (v->flags & MAP_SHARED) == 0)
    return 0;

  struct file *f = v->file;
  if (f->writable == 0) {
    // should have been forbidden from sys_mmap
    printf("vwrite: not writable\n");
    return -1;
  }
  int max = ((MAXOPBLOCKS-1-1-2) / 2) * BSIZE;
  int i = 0;
  int off = v->offset + (addr - v->start);
  int r;

  // write out of file
  if (off > f->ip->size) {
    printf("vwrite: out of inode\n");
    return -1;
  }

  while(i < n) {
    int n1 = n - i;
    if(n1 > max)
      n1 = max;

    begin_op();
    ilock(f->ip);
    if ((r = writei(f->ip, 1, addr + i, off, n1)) > 0)
      off += r;
    iunlock(f->ip);
    end_op();

    if(r != n1) // error from write
      return -1;
    i += r;
  }
  return 0;
}

// write back if needed and delete unneeded pte
int
munmap(struct vma *v, pagetable_t pagetable, uint64 addr, uint64 length)
{
  uint64 end = addr + length;
  // NOTE: if addr is not page-aligned
  // then we cannot delelte first pte
  uint64 va = PGROUNDUP(addr);
  if (va != addr) {
    size_t n = va - addr;
    if (length < n)
      n = length;
    vwrite(v, addr, n);
  }
  // then write back and del pte by-page
  for (; va < end; va += PGSIZE) {
    // ignore non-mmaped page
    pte_t *pte = walk(pagetable, va, 0);
    if (pte == 0 || (*pte & PTE_V) == 0)
      continue;
    if (vwrite(v, va, PGSIZE) == -1) {
      printf("munmap: vwrite\n");
      return -1;
    }
    uvmunmap(pagetable, va, 1, 1);
  }
  return 0;
}
