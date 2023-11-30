
## hack

since we know xv6 use a naive way (turn off the interrupt) to avoid interrupt acquire deadlock

run `make CPUS=1 qemu`
- `#test-and-set` is 0, since it can never be reached (lock-area is not interruptable...)
> that's the reason of: "For this lab, you must use a dedicated unloaded machine with multiple cores."

```
start test1
test1 results:
--- lock kmem/bcache stats
lock: kmem: #test-and-set 0 #acquire() 433034
lock: bcache: #test-and-set 0 #acquire() 362
--- top 5 contended locks:
lock: cons: #test-and-set 0 #acquire() 23
lock: cons: #test-and-set 0 #acquire() 23
lock: cons: #test-and-set 0 #acquire() 23
lock: cons: #test-and-set 0 #acquire() 23
lock: cons: #test-and-set 0 #acquire() 23
tot= 0
test1 OK
start test2
total free number of pages: 32497 (out of 32768)
.....
test2 OK
start test3
child done 1
child done 100000
test3 OK
```

for multicore case...
- `#test-and-set` is roughly the spin times
- `#acquire` is still just `acquire` calling times
- so in nature, it's race of core but not process

implement per-CPU freelists, and stealing when a CPU's free list is empty

> so i know why lock need a unique name, that's only used for logging...

## why `cpuid` need to turn intr off...

cpuid read `tp`, which is initilzied as `mhartid`

`tp` will be switch between user/trap
```asm
# uservec
sd tp, 64(a0)
# make tp hold the current hartid, from p->trapframe->kernel_hartid
ld tp, 32(a0)
```
- but when bootstrap a process, its `trapframe->tp` is filled with `kalloc` junk
- so `tp` is nonsense for user, and only meaningful in trap...


what if you don't turn off intr, let's have a review on scheduling...
- you call `cpuid()`, load `tp` to variable, then timer interrupt
- current cpu go away -> `sched`(`swtch` to cpu) -> `scheduler`(`swtch` to another process)
- similarly, another cpu schedule back here, pick up this process(in trap)
  - now `tp` has changed

so why does it hurt?
- when `kfree`, we don't care about which cpu alloc mem from which freelist...
- but this way is very random, may cause much more contentions...
  - e.g. cpuB acquire mem of cpuA, if falled due to cpuA's mem run out, cpuC then steal it from cpuD...
- we can avoid it by
  - `push_off` before get cpuid
  - `pop_off` after this mem is allocated
  - then cpuB will acquire it's own mem first, then steal from cpuD

> annoying, we `push_off`, anyway

## steal

before steal
```
start test1
test1 results:
--- lock kmem/bcache stats
lock: kmem: #test-and-set 45571 #acquire() 433272
lock: bcache: #test-and-set 0 #acquire() 2260
--- top 5 contended locks:
lock: proc: #test-and-set 395034 #acquire() 306709
lock: virtio_disk: #test-and-set 127469 #acquire() 156
lock: kmem: #test-and-set 45571 #acquire() 433272
lock: proc: #test-and-set 39065 #acquire() 706683
lock: proc: #test-and-set 35906 #acquire() 706682
tot= 45571
test1 FAIL
start test2
total free number of pages: 32497 (out of 32768)
.....
test2 OK
start test3
child done 1
child done 100000
test3 OK
```

after steal
```
start test1
test1 results:
--- lock kmem/bcache stats
lock: kmem0: #test-and-set 0 #acquire() 32968
lock: kmem1: #test-and-set 0 #acquire() 201117
lock: kmem2: #test-and-set 0 #acquire() 199099
lock: bcache: #test-and-set 0 #acquire() 2994
--- top 5 contended locks:
lock: proc: #test-and-set 142344 #acquire() 253298
lock: virtio_disk: #test-and-set 100092 #acquire() 156
lock: proc: #test-and-set 35561 #acquire() 652991
lock: proc: #test-and-set 29632 #acquire() 652998
lock: proc: #test-and-set 16750 #acquire() 252844
tot= 0
test1 OK
start test2
total free number of pages: 32497 (out of 32768)
.....
test2 OK
start test3
child done 1
child done 100000
test3 OK
```

how `statistics` work
- before test: write "statistics" file, then `m = ntas(0)`
- after test: upd "statistics" file, then `n = ntas(0)`
- `n - m` should be the diff to `tot` -> `#test-and-set` after init


## bcache lock

> like kalloctest's `fork`+`sbrk` to gen contentions on `kmem.lock`
  bcachetest do `fork`+`read` to gen contentions on `bcache.lock`


> For kalloc, one could eliminate most contention by giving each CPU its own allocator; that won't work for the block cache.
- kalloc randomly pick mem from pool
- "bcache-alloc" match target from pool first... but i don't figure out there're large difference...
