// p = get a number from left neighbor
// print p
// loop:
//     n = get a number from left neighbor
//     if (p does not divide n)
//         send n to right neighbor

#include "kernel/types.h"
#include "user/user.h"

#define SEIVE_BOUND 35

// seive from left, send to right
__attribute__((noreturn)) void pass(int l) {

  int p, n;
  int r[2];

  // recv from left
  if (!read(l, &p, sizeof(int)))
    exit(0);
  printf("prime %d\n", p);

  pipe(r);

  if (fork()) {
    // seive
    while (read(l, &n, sizeof(int))) {
      if (n % p) {
        write(r[1], &n, sizeof(int));
      }
    }
    close(l);
    close(r[1]);
    wait(0);

  } else {
    // send to right
    close(r[1]);
    pass(r[0]);
  }
  exit(0);
}

int main() {
  int r[2];
  pipe(r);

  // dummy left
  for (int i = 2; i <= SEIVE_BOUND; ++i) {
    write(r[1], &i, sizeof(int));
  }
  close(r[1]);
  pass(r[0]);

  exit(0);
}
