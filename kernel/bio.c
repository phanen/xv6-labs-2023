// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBUCKET 13
#define HASH(x) ((x) % NBUCKET)

struct {
  struct spinlock lock[NBUCKET];
  // struct spinlock glock;
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  struct buf head[NBUCKET];
} bcache;


void
binit(void)
{
  struct buf *b;
  #define SZ 20
  static char lknames[NBUCKET][SZ];

  // initlock(&bcache.glock, "bcache");
  for (int i = 0; i < NBUCKET; ++i) {
    snprintf(lknames[i], SZ, "bcache.bucket%d", i);
    initlock(&bcache.lock[i], lknames[i]);
    // Create linked list of buffers
    bcache.head[i].prev = &bcache.head[i];
    bcache.head[i].next = &bcache.head[i];
  }

  // init all free bcache in first linklist
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    b->next = bcache.head[0].next;
    b->prev = &bcache.head[0];
    initsleeplock(&b->lock, "buffer");
    bcache.head[0].next->prev = b;
    bcache.head[0].next = b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  // acquire(&bcache.glock);

  uint i = HASH(blockno);
  acquire(&bcache.lock[i]);

  // Is the block already cached?
  for(b = bcache.head[i].next; b != &bcache.head[i]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock[i]);
      // release(&bcache.glock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  for(b = bcache.head[i].prev; b != &bcache.head[i]; b = b->prev){
    if(b->refcnt == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      release(&bcache.lock[i]);
      // release(&bcache.glock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // no free bcache in current bucket, steal...
  for (int s = (i + 1) % NBUCKET; s != i; s = (s + 1) % NBUCKET) {
    acquire(&bcache.lock[s]);
    for(b = bcache.head[s].prev; b != &bcache.head[s]; b = b->prev){
      if(b->refcnt == 0) {
        b->dev = dev;
        b->blockno = blockno;
        b->valid = 0;
        b->refcnt = 1;
        // steal it, holding two lock here
        b->prev->next = b->next;
        b->next->prev = b->prev;
        release(&bcache.lock[s]);
        b->next = bcache.head[i].next;
        b->prev = &bcache.head[i];
        bcache.head[i].next->prev = b;
        bcache.head[i].next = b;
        release(&bcache.lock[i]);
        // release(&bcache.glock);
        acquiresleep(&b->lock);
        return b;
      }
    }
    release(&bcache.lock[s]);
  }
  // no need to release lock, panic anyway
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  // acquire(&bcache.glock);
  uint i = HASH(b->blockno);
  acquire(&bcache.lock[i]);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.head[i].next;
    b->prev = &bcache.head[i];
    bcache.head[i].next->prev = b;
    bcache.head[i].next = b;
  }
  
  release(&bcache.lock[i]);
  // release(&bcache.glock);
}

void
bpin(struct buf *b) {
  // acquire(&bcache.glock);
  uint i = HASH(b->blockno);
  acquire(&bcache.lock[i]);
  b->refcnt++;
  release(&bcache.lock[i]);
  // release(&bcache.glock);
}

void
bunpin(struct buf *b) {
  // acquire(&bcache.glock);
  uint i = HASH(b->blockno);
  acquire(&bcache.lock[i]);
  b->refcnt--;
  release(&bcache.lock[i]);
  // release(&bcache.glock);
}


