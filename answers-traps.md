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

> Note that the return address lives at a fixed offset (-8) from the frame pointer of a stackframe, and that the saved frame pointer lives at fixed offset (-16) from the frame pointer.
- maybe not... for leaf, `ra` didn't need to be maintained -> `fp = sp-8`
