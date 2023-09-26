// p = get a number from left neighbor
// print p
// loop:
//     n = get a number from left neighbor
//     if (p does not divide n)
//         send n to right neighbor

#include "kernel/types.h"
#include "user/user.h"

int main() {

  int fds[2];
  pipe(fds);

  // dummy head neighbor
  for (uint i = 2; i <= 35; ++i) {
    write(fds[1], &i, 4);
  }
  // eof
  write(fds[1], "\0\0\0\0", 4);

  int p, n;
  while (1) {
    // receive and print first
    read(fds[0], &p, 4);
    if (p == 0)
      break;
    printf("prime %d\n", p);

    // seive and sent others
    while (1) {
      read(fds[0], &n, 4);
      if (n == 0)
        break;
      if (n % p)
        write(fds[1], &n, 4);
    }

    write(fds[1], "\0\0\0\0", 4);
  }
  exit(0);
}
