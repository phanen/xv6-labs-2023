
## bigfile

seems `itruct` also need to be modified, but it's not considerde in test

## symbolic links

notes
- if the linked file is also a symbolic link, you must recursively follow it until a non-link file is reached.
- if the links form a cycle, you must return an error code. (approximation: limit search depth)
- choose somewhere to store the target path of a symbolic link, for example, in the inode's data blocks
- the target does not need to exist for the system call to succeed.

difference between hardlink and symlink
- hardlink: two file point to same inodeno as dirent -> same inode (with rc of links as `nlink`)
- symlink: new file has a standalone inode, inode is used to store link to target, link is "symbolic" (e.g. pathname)

> store directly in inode, that's efficient but not elegant, since it need much more refactor than just store pathname as blob in block.
for the later way, we only need to provide one more flag to the `open`
- with `O_NOFOLLOW`, we'll get the fd of the `target`
  - if we adopt the former way, this fd must be handled differently in fd layer... laziness is the virtue
- without `O_NOFOLLOW`, we'll get the fd of `symlink`
- (since `O_NOFOLLOW` cannot be used with `O_CREATE`... it's easy to know where to put our code...
