## primes
- <https://swtch.com/~rsc/thread/>

```
p = get a number from left neighbor
print p
loop:
    n = get a number from left neighbor
    if (p does not divide n)
        send n to right neighbor
```

执行单元
- recv prime
- recv remains, seive, send to right
```py
def func(l):
    r = create_pipe()
    p = recv_prime()
    if not p: exit()
    if fork(): seive_and_send(l, p, r)
    else: func(r)
```
细节
- 用一个 dummy head, 来 send 第一批 left
- fork 出的子进程要及时关掉不用的 fd, 不然资源用光只会卡住


多个进程实际串联成流水线
```
proc1 -> body
      -> proc2 -> body
               -> proc3 -> ...
```
