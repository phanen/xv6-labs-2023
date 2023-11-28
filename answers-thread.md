
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
