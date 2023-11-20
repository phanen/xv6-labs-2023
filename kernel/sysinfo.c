#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"

uint64
sys_sysinfo(void)
{
  struct sysinfo* info;
  argaddr(0, (uint64*)&info);

  // kernel metadata -> userspace
  uint64 freemem = count_freemem();
  uint64 nproc = count_nproc();

  pagetable_t pt = myproc()->pagetable;

  int ok;
  ok = copyout(pt, (uint64)&info->freemem, (char*)&freemem, sizeof(info->freemem));
  if (ok == -1) return -1;
  ok = copyout(pt, (uint64)&info->nproc, (char*)&nproc, sizeof(info->nproc));
  if (ok == -1) return -1;
  return 0;
}
