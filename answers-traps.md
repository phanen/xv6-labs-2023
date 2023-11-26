Which registers contain arguments to functions? For example, which register holds 13 in main's call to printf?
- a0 a1 ...
For example, which register holds 13 in main's call to printf?
- a2

Where is the call to function f in the assembly code for main? Where is the call to g? (Hint: the compiler may inline functions.)
- inlined...

At what address is the function printf located?
- 0x630

What value is in the register ra just after the jalr to printf in main?
- 0x38

```cc
unsigned int i = 0x00646c72;
printf("H%x Wo%s", 57616, &i);
// he110 World
```

The output depends on that fact that the RISC-V is little-endian. If the RISC-V were instead big-endian what would you set i to in order to yield the same output?
```
0x00646c72 -> 0x726c6400
```
Would you need to change 57616 to a different value?
- no, 57616 is a union, not byte-by-byte

In the following code, what is going to be printed after 'y='? (note: the answer is not a specific value.) Why does this happen?
```cc
printf("x=%d y=%d", 3);
```
- peek stack? depends on va_arg impl..

## backtrace

for leaf, `ra` will not be saved, `fp = sp-8` rather than `fp = sp-16` ???
- (who care, we don't use bt in leaf in this lab...
```asm
  // non-leaf prologue (main)
  addi	sp,sp,-16
  sd	ra,8(sp)
  sd	s0,0(sp)
  addi	s0,sp,16
  // leaf prologue (strcpy)
  addi	sp,sp,-16
  sd	s0,8(sp)
  addi	s0,sp,16
```

but why `addi sp,sp,-16`
> In the standard RISC-V calling convention, the stack grows downward and the stack pointer is always kept 16-byte aligned.

## alarm

note
- bug: `periodic = 0` in user virtual address, `sigalarm(0, 0)` is ambiguous
  - use `sigalarm(0, x)` instead
- avoid reentrant: no lock needed, and flag `p->alarm_running` is enough
    - since at most one handler can running
- resume `a0`: rewrite it by `syscall[num]()`, so `return p->trapframe->a0`
