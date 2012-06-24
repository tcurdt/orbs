#include "common.h"
#include "segments.h"
#include <sys/stat.h>
#include <unistd.h>

int segments_init(segments* segments) {
  segments->head = NULL;
  segments->file = NULL;
  return OK;
}

int segments_destroy(segments* segments) {
  if (segments->head) {
    segments->head = NULL;
  }
  if (segments->file) {
    segments->file = NULL;
  }
  return OK;
}

int segments_head_write(segments* segments, message* message) {
  FILE *file = segments->file;
  if (!file) {
    file = fopen(segments->head->full_path, "a");
    if (!file) {
      reterr("failed to open %s", segments->head->full_path);
    }
    segments->file = file;
  }
  size_t written, len, total = 0;

  len = sizeof(u_int8_t);
  written = fwrite((void*)&message->type, len, 1, file);
  if (written != 1) reterr("failed to write %zd", written);
  total += written;

  len = sizeof(size_t);
  written = fwrite((void*)&message->body_size, len, 1, file);
  if (written != 1) reterr("failed to write %zd", written);
  total += written;

  len = message->body_size;
  written = fwrite((void*)message->body, len, 1, file);
  if (written != 1) reterr("failed to write %zd", written);
  total += written;

  len = sizeof(u_int32_t);
  written = fwrite((void*)&message->crc32, len, 1, file);
  if (written != 1) reterr("failed to write %zd", written);
  total += written;

  fflush(file);
  segments->head->size += total;

  return OK;
}

u_int32_t segments_head_size(segments* segments) {
  u_int32_t size = 0;
  segment* curr = segments->head;
  while(curr != NULL) {
    size += curr->size;
    curr = curr->previous_segment;
  }
  return size;
}

u_int32_t segments_count(segments* segments) {
  u_int32_t count = 0;
  segment* head = segments->head;
  segment* curr = head;
  while(curr != NULL) {
    count += 1;
    curr = curr->previous_segment;
  }
  return count;
}

int segments_pop(segments* segments) {

  segment* head = segments->head;
  segment* prev = NULL;
  segment* curr = head;
  while(curr != NULL && curr->previous_segment != NULL) {
    prev = curr;
    curr = curr->previous_segment;
  }
  if (prev) {
    prev->previous_segment = NULL;
  } else {
    segments->head = NULL;
  }

  if (curr) {
    // close file
    if (segments->file) {
      fclose(segments->file);
      segments->file = NULL;
    }

    // remove file
    if (unlink(curr->full_path) != OK) {
      reterr("failed to delete %s", curr->full_path);
    }

    free((void*)curr->full_path);
    free(curr);
  }

  return OK;
}

static const char* basename(const char *path) {
  const char *base = strrchr(path, '/');
  return base ? base + 1 : path;
}

int segments_add(segments* segments, const char *original_full_path) {

  const char* full_path = strdup(original_full_path);
  if (!full_path) reterr("failed to duplicate %s", original_full_path);

  // check length
  u_int32_t size = 0;
  struct stat st;
  if (stat(full_path, &st) == OK) {
    if (!S_ISREG(st.st_mode)) {
      reterr("not a regular file %s", full_path);
    }
    size = st.st_size;
    // maybe check a magic header?
  }

  // check timestamp
  u_int32_t timestamp = atol(basename(full_path));
  if (timestamp <= 0) {
    reterr("not a segment file %s", full_path);
  }

  segment* new_segment = malloc(sizeof(segment));
  if (!new_segment) reterr("out of memory");
  new_segment->full_path = full_path;
  new_segment->timestamp = timestamp;
  new_segment->size = size;

  segment* head = segments->head;
  segment* prev = NULL;
  segment* curr = head;
  while(curr != NULL && new_segment->timestamp < curr->timestamp) {
    prev = curr;
    curr = curr->previous_segment;
  }
  if (prev) {
    new_segment->previous_segment = prev->previous_segment;
    prev->previous_segment = new_segment;
  } else {
    new_segment->previous_segment = head;
    segments->head = new_segment;
  }
  return OK;
}