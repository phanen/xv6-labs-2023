Looking at the backtrace output, which function called syscall?
- usertrap

What is the value of p->trapframe->a7 and what does that value represent? (Hint: look user/initcode.S, the first user program xv6 starts.)
- `#define SYS_exec 7`

What was the previous mode that the CPU was in?
- 0x200000000 -> 0x200000022 (`p /x $sstatus`)
- SIE SPIE

make kernel panic
```sh
# num = * (int *) 0;
scause 0x000000000000000d
sepc=0x0000000080002054 stval=0x0000000000000000
panic: kerneltrap
```

Write down the assembly instruction the kernel is panicing at. Which register corresponds to the variable num?
```
// a3 = 0x505050505050505
0x80002054 <syscall+20> lw      a3,0(zero) # 0x0

// panic
0x80005bce <panic+72>   j       0x80005bce <panic+72>
```


What is the name of the binary that was running when the kernel paniced? What is its process id (pid)?
- p->name = "initcode"
- p->pid = 1
