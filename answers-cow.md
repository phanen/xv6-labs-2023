> Any systems problem can be solved with a level of indirection.

copy-on-write
- COW fork() marks all the user PTEs in both parent and child as read-only.
- When either process tries to write one of these COW pages, the CPU will force a page fault.
- The kernel page-fault handler detects this case, allocates a page of physical memory for the faulting process, copies the original page into the new page, and modifies the relevant PTE in the faulting process to refer to the new page, this time with the PTE marked writeable


## hack

page fault
```c
*(uint64*)0 = 1;
// *(uint64*)2000 = 1;
usertrap(): unexpected scause 0x000000000000000f pid=3
            sepc=0x000000000000048c stval=0x0000000000000000
```


note
- add refcount tbl in `kalloc.c`
- transform `uvmunmap`, `mappage` into refcount ver
- `cowpage`: kalloc a new pa, `memmove`, tinker flags
  - forget `memmove` here -> access a dirty page -> `umalloc` will `scause=0xd`
  - `kalloc` a dirty page will help you debug...
- add handler in `usertrap`, `copyout`
