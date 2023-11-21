#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  uint64 va;
  argaddr(0, &va);
  int pg_n;
  argint(1, &pg_n);
  uint64 umask_ptr;
  argaddr(2, &umask_ptr);

  pagetable_t pagetable = myproc()->pagetable;

  // HACK: multiplex it...
#ifdef VMPRINT_STUB
  if (pg_n == -1) {
    struct proc *p = myproc();
    if(!memcmp(p->name, "pgtbltest", strlen(p->name))) vmprint(p->pagetable);
    return 0;
  }
#endif

  // poor test limit, use 32 bit
  uint kb = 0;
  uint64 va_c = va;
  pg_n = pg_n < 32 ? pg_n : 32;
  for (int i = 0; i < pg_n; ++i) {
    pte_t* pte = walk(pagetable, va, 0);
    if (*pte & PTE_A) kb |= (1 << i);
    va += PGSIZE;
    // *pte = (*pte & PTE_A) ^ *pte;
  }
  copyout(pagetable, umask_ptr, (char*)&kb, 4);

  for (int i = 0; i < pg_n; ++i) {
    pte_t* pte = walk(pagetable, va_c, 0);
    *pte = (*pte & PTE_A) ^ *pte;
    va_c += PGSIZE;
  }
  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
