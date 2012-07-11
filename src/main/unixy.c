#include "unixy.h"
#include "common.h"
#include <fts.h>
#include <string.h>

const char* tmp_create() {
  char *path = mkdtemp(strdup("/tmp/orbs-XXXXXX"));
  assert(path);
  return path;
}

int tmp_remove(const char* path) {
  if (path == NULL) return ERROR;

  char* const paths[] = { (char*)path, NULL };

  FTS *tree = fts_open(paths, FTS_NOCHDIR, 0);
  check(tree, "fts_open");

  FTSENT *node;
  while ((node = fts_read(tree))) {
    if (node->fts_level > 0 && node->fts_name[0] == '.') {
      fts_set(tree, node, FTS_SKIP);
    } else if (node->fts_info & FTS_F) {
      check(remove(node->fts_path) == OK, "failed to remove file %s", node->fts_path);
    } else if (node->fts_info & FTS_DP) {
      check(remove(node->fts_path) == OK, "failed to remove dir %s", node->fts_path);
    }
  }

  check(fts_close(tree) == OK, "fts_close");

  free((void*)path);
  return OK;
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