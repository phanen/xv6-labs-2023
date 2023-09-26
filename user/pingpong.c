#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  int p[2];
  pipe(p);
  int pid = fork();
  void *one_byte = malloc(1);
  if (pid == 0) {
    read(p[0], one_byte, 1);
    printf("%d: received ping\n", getpid());
    write(p[1], one_byte, 1);
  } else {
    write(p[1], "1", 1);
    read(p[0], one_byte, 1);
    printf("%d: received pong\n", getpid());
    wait(0);
    // sleep(1);
  }
  exit(0);
}
