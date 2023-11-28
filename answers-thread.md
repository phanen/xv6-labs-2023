
## test struct
```c
struct A {
  int x;
  int y;
} a;

int main() {
  printf("%x\n", a); // 0
  printf("%x\n", &a); // 42c8020
  printf("%x\n", a.x); // 0
  printf("%x\n", (&a)->x); // 0
  printf("%x\n", &a.x); // 42c8020

  a.x = 1;
  printf("%x\n", a); // 1
}
```

## ph

Given random KV storage, then:
- each thread put some random KV
- join
- each thread get all KVs that should exist
- join
- result: missing in get...


Why are there missing keys with 2 threads, but not with 1 thread?
- insert is not thread-safe...

Identify a sequence of events with 2 threads that can lead to a key being missing.
```c
*p = e1;
*p = e2;
// what's more... memory leakage...
```

Pass `ph_safe`: zero missing
```c
pthread_mutex_lock(&lock);
insert(key, value, &table[i], table[i]);
pthread_mutex_unlock(&lock);
```

Pass `ph_fast`: two threads yield at least 1.25 times as many puts/second as one thread
- parallel some ops while maintaining correctness.
- the above impl is enough for my pc...

Intuition: lock a single bucket rather than the whole KV storage...
- but it didn't run faster in my pc...
- maybe more lock introduce more overhead

TODO: maybe we can test more threads....
- `./ph 4` -> about 4 rate
- `./ph 10` -> about 5~6 rate...


## barrier

difference from `pthread_join`?
- `pthread_join` will wait for exit...
- `barrier` just want to make sure sync

```c
pthread_cond_wait(&cond, &mutex);  // go to sleep on cond, releasing lock mutex, acquiring upon wake up
pthread_cond_broadcast(&cond);     // wake up every thread sleeping on cond
```

talk is cheap, see the code...
