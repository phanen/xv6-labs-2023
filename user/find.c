// clang-format off
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
// clang-format on

void find(char *base, char *name) {

  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if ((fd = open(base, O_RDONLY)) < 0) {
    fprintf(2, "find: cannot open %s\n", base);
    return;
  }

  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", base);
    close(fd);
    return;
  }

  if (st.type != T_DIR) {
    fprintf(2, "find: root path should be dir\n");
    close(fd);
    return;
  }

  // path + '/' + 'name' + '\0'
  if (strlen(base) + 1 + DIRSIZ + 1 > sizeof buf) {
    printf("ls: path too long\n");
    close(fd);
    return;
  }

  // "base/"
  strcpy(buf, base);
  p = buf + strlen(buf);
  *p++ = '/';
  while (read(fd, &de, sizeof(de)) == sizeof(de)) {
    if (de.inum == 0)
      continue;
    memmove(p, de.name, DIRSIZ);
    p[DIRSIZ] = 0;
    if (stat(buf, &st) < 0) {
      printf("find: cannot stat %s\n", buf);
      continue;
    }

    switch (st.type) {
    case T_FILE:
    case T_DEVICE:
      // match name
      if (!strcmp(p, name))
        printf("%s\n", buf);

      break;
    case T_DIR:
      if (!strcmp(p, ".") || !strcmp(p, ".."))
        continue;
      find(buf, name);
      break;
    }
  }
  close(fd);
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    exit(0);
  }
  find(argv[1], argv[2]);
  exit(0);
}
