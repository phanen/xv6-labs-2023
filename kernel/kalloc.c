// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock[CPUS];
  struct run *freelist[CPUS];
  uint64 len[CPUS];
} kmem;


void
kinit()
{
  static char lknames[CPUS][8];
  for (int i = 0; i < CPUS; ++i) {
    snprintf(lknames[i], 8, "kmem%d", i);
    initlock(&kmem.lock[i], lknames[i]);
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;
  // push_off();
  int i = cpuid();
  acquire(&kmem.lock[i]);
  r->next = kmem.freelist[i];
  kmem.freelist[i] = r;
  ++kmem.len[i];
  release(&kmem.lock[i]);
  // pop_off();
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  push_off();
  int d = cpuid();
  acquire(&kmem.lock[d]);
  r = kmem.freelist[d];
  if (r) {
    kmem.freelist[d] = r->next;
    --kmem.len[d];
  }
  else {
    // should release?
    int s = (d + 1) % CPUS;
    for (; s != d; s = (s + 1) % CPUS) {
      // TODO: maybe we can steal from richest one...
      // but that will always need CPUS-time acquire...
      acquire(&kmem.lock[s]);
      r = kmem.freelist[s];
      if (r) {
        // pick one page for alloc
        kmem.freelist[s] = r->next;
        --kmem.len[s];

        // steal half of freelist
        int slen = kmem.len[s] / 2;
        for (int i = 0; i < slen; ++i) {
          struct run *sr = kmem.freelist[s];
          kmem.freelist[s] = kmem.freelist[s]->next;
          sr->next = kmem.freelist[d];
          kmem.freelist[d] = sr;
        }
        kmem.len[s] -= slen;
        kmem.len[d] += slen;

        release(&kmem.lock[s]);
        break;
      }
      release(&kmem.lock[s]);
    }
  }
  release(&kmem.lock[d]);
  pop_off();

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
