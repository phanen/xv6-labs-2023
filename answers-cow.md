> Any systems problem can be solved with a level of indirection.

copy-on-write
- COW fork() marks all the user PTEs in both parent and child as read-only.
- When either process tries to write one of these COW pages, the CPU will force a page fault.
- The kernel page-fault handler detects this case, allocates a page of physical memory for the faulting process, copies the original page into the new page, and modifies the relevant PTE in the faulting process to refer to the new page, this time with the PTE marked writeable


## hack & tbd

page fault
```c
*(uint64*)0 = 1;
// *(uint64*)2000 = 1;
usertrap(): unexpected scause 0x000000000000000f pid=3
            sepc=0x000000000000048c stval=0x0000000000000000
```

legacy idea (1st commit)
> I believe it can be worked out, after fixing lock issues, maybe give it a try later...
- `cow_uvmcopy` copy pte without kalloc
  - non-leaf will be recreated
- update refcount in `uvmunmap`, `mappages`
- `cowpage`: kalloc a new pa, `memmove`, tinker flags
  - forget `memmove` here -> access a dirty page -> `umalloc` will `scause=0xd`
  - `kalloc` a dirty page will help you debug...
  - add handler in `usertrap`, `copyout`


and why child proc cow, mem barrier?
```c
forktest2(){
  if (fork()) exit(0);
  wait(0);
    // child will trigger cow if not comment the follwing code
  asm volatile("addi	sp,sp,-8");
  asm volatile("sd	ra, 0(sp)");
  asm volatile("addi	sp,sp,8");
}
```

note
> follow the hint can be really helpful, and never keep stubborn...
- a good start: use `kalloc`/`kfree` to inc/dec `rc`
  - do nothing, but it helps you to save tons of refactors...
  - take it as: pa will be always mapped at least once, and only `fork` will dup the `rc`...
- lock to `rc` is really important...
- Counter-intuitive in `usertests`
  - `copyout` -> `read`, `copyin` -> `write`

useful
```sh
riscv64-linux-gnu-objdump -S user/_usertests &| less
```
