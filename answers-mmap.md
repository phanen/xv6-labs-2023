> If `MAP_SHARED` is specified, write references shall change the underlying object.
  If `MAP_PRIVATE` is specified, modifications to the mapped data by
  the calling process shall be visible only to the calling process
  and shall not change the underlying object.

notes
- keep track of mapped regions for each proc
  - use a static vma array with 16 slots
  - each proc maintain its vma linklist in proc
  - vma alloc: "assume that addr will always be zero"
  - vma contains ptr to a file, inc rc of file(`fileup`) to avoid `fileclose`
- lazy: `mmap` don't directly write page table, so fallback it to page-fault handler
- `munmap`: i assumes both addr and length to be page-aligned
  - otherwise, it will "overwrite" area back to file, that's wired...
- `exit`: need to auto "munmap", though we cannot use syscall here
- `fork`: child has the same mapped regions as the parent
  - when page fault, child can share page with parent
  - but we reload file, for simplicity's sake

> The reason to be lazy is to ensure that mmap of a large file is fast,
  and that mmap of a file larger than physical memory is possible.

> an munmap call might cover only a portion of an mmap-ed region,
  but you can assume that it will either unmap at the start,
  or at the end, or the whole region
- but it's not hard to support "punch a hole in the middle of using linklist

why bother `mmap`, when there has been impl `read`/`write`
- `mmap` can copy once instead of twice by pin mmap-ed file in bcache
- useful for inter-process/user-kernel communication?

inode size != mmap-ed file size
- if mmap-ed size > inode size
- write out of inode, then munmap -> ambiguous when write back
  - throw a error or fill previous off with zero first?
- maybe it's a undefined behavior?

ref
- <https://www.cnblogs.com/huxiao-tee/p/4660352.html>
