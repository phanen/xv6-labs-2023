
args
- prot bits: `PROT_READ`, `PROT_WRITE`, `PROT_EXEC`
- `MAP_SHARED`: should be written back to the file
- `MAP_PRIVATE`: should not

notes
- keep track of what `mmap` mapped for each proc
  - `proc->vma`? address, length, permissions, file...
- impl `mmap`
  - add a vma range to proc, via a fixed-size array
    - this doesn't directly write page table, fallback it to trap handler
  - vma contains ptr to a file, inc rc of file(`fileup`) to avoid `fileclose`
- add page-fault handler
  - do `mappages` for mmap-ed range (also set perms)
  - and `readi` to allocated buf
- impl `munmap`
  - `uvmunmap` to clear page table
  - dec rc if file is totally unmaped
  - for `MAP_SHARED` and dirty page, we write it back to file with
- exit: need to auto "munmap", though we cannot use syscall here
- fork: child has the same mapped regions as the parent, since they can share file

why bother `mmap`, when there has been impl `read`/`write`
- `mmap` produce no bcache in kernel, copy once
- useful for inter-process/user-kernel communication?

ref
- <https://www.cnblogs.com/huxiao-tee/p/4660352.html>
