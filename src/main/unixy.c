#include "unixy.h"
#include "common.h"
#include <ftw.h>
#include <fcntl.h>

#define _XOPEN_SOURCE 500

const char* tmp_create() {
  char *pattern = mktemp(strdup("orbs-XXXXXXXX"));
  if (mkdir(pattern, 0777) != OK) {
    return NULL;
  }
  return pattern;
}

static int tmp_remove_file(const char* fpath, const struct stat* sb, int typeflag, struct FTW* ftwbuf) {
  UNUSED(sb);
  UNUSED(typeflag);
  UNUSED(ftwbuf);
  return remove(fpath);
}

void tmp_remove(const char* path) {
  if (path == NULL) return;
  nftw(path, tmp_remove_file, 64, FTW_DEPTH | FTW_PHYS);
  free((void*)path);
}

int file_exists(const char* path) {
  return access(path, F_OK) != -1;
}

int file_size(const char* path) {
  UNUSED(path);
  return 0;
}

void file_write(const char* path, const char* name, const char* data) {
  int len = strlen(path) + strlen(name) + 1 + 1;
  char *filename = malloc(len);
  snprintf(filename, len, "%s/%s", path, name);
  int fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0666);
  write(fd, data, strlen(data));
  close(fd);
  free(filename);
}

const char* basename(const char *path) {
  const char *base = strrchr(path, '/');
  return base ? base + 1 : path;
}