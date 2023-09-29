// clang-format off
#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"
// clang-format on

#define BUF_SIZE 128

// skip all empty line, then read a non-empty line within `len` chars
// trans last '\n' to '\0' (if end with '\n'...)
int readline(int fd, char *buf, int len) {
  char *p = buf;

  // not overflow and not eof
  for (; p < buf + len && read(fd, p, 1) != 0; ++p) {
    if (*p == '\n') {
      *p = '\0';
      return p - buf;
    }
  }
  // eof or overflow
  return p - buf;
}

int is_empty_line(char *p) {

  for (; *p != '\0'; ++p) {
    if (*p != ' ' && *p != '\t') {
      return 0;
    }
  }
  return 1;
}

// accept no args, just exec cmd for echo line
int main(int argc, char *argv[]) {
  if (argc < 2)
    exit(-1);

  char *cmd_argv[MAXARG];
  memmove(cmd_argv, argv + 1, (argc - 1) * sizeof(char *));

  char buf[BUF_SIZE];
  *(cmd_argv + argc - 1) = buf;
  *(cmd_argv + argc) = 0;

  // exec cmd for each line
  while (1) {
    // update line buffer
    int len = readline(0, buf, BUF_SIZE);

    // overflow
    if (len == BUF_SIZE) {
      fprintf(2, "xargs: line %s overflow\n", argv[1]);
      exit(-1);
    }

    // readline ok (best to be ok...)
    if (buf[len] == '\0') {
      if (is_empty_line(buf))
        continue;
      if (fork() == 0) {
        exec(argv[1], cmd_argv);
        fprintf(2, "xargs: exec %s fail\n", argv[1]);
        exit(-1);
      }
      wait(0);
      continue;
    }

    // eof
    buf[len] = '\0';
    exec(argv[1], cmd_argv);
    fprintf(2, "xargs: exec %s fail\n", argv[1]);
    exit(-1);
  }
}
