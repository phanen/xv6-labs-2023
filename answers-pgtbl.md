Which other xv6 system call(s) could be made faster using this shared page? Explain how.
- syscalls which use proc()->something, we can just put it in usyscall page.


For every leaf page in the vmprint output, explain what it logically contains and what its permission bits are.
```
page table 0x0000000087f6b000
 ..0: pte 0x0000000021fd9c01 pa 0x0000000087f67000 -------V
 .. ..0: pte 0x0000000021fd9801 pa 0x0000000087f66000 -------V
 .. .. ..0: pte 0x0000000021fda01b pa 0x0000000087f68000 ---US-RV
 .. .. ..1: pte 0x0000000021fd9417 pa 0x0000000087f65000 ---U-WRV
 .. .. ..2: pte 0x0000000021fd9007 pa 0x0000000087f64000 -----WRV
 .. .. ..3: pte 0x0000000021fd8c17 pa 0x0000000087f63000 ---U-WRV
 ..255: pte 0x0000000021fda801 pa 0x0000000087f6a000 -------V
 .. ..511: pte 0x0000000021fda401 pa 0x0000000087f69000 -------V
 .. .. ..509: pte 0x0000000021fdcc13 pa 0x0000000087f73000 ---U--RV
 .. .. ..510: pte 0x0000000021fdd007 pa 0x0000000087f74000 -----WRV
 .. .. ..511: pte 0x0000000020001c0b pa 0x0000000080007000 ----S-RV
```
![img:perms](https://i.imgur.com/mvEXhAZ.png)
- e.g. 0x0000000021fd9c01 -> perms = 00_0000_0001(V)
- e.g. 0x0000000021fda01b -> perms = 00_0001_1011(VRXU)
- e.g. 0x0000000021fd9417 -> perms = 00_0001_0111(VRWU)
- e.g. 0x0000000021fd9007 -> perms = 00_0000_0111(VRW)


Be sure to clear PTE_A after checking if it is set. Otherwise, it won't be possible to determine if the page was accessed since the last time pgaccess() was called (i.e., the bit will be set forever).
- TODO: so where will pgtest access...
