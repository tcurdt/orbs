#include "common.h"
#include "segments.h"

static char* filename(const char *base_path, u_int32_t timestamp) {
  size_t len = strlen(base_path) + 1 + 20 + 1;
  char *ret = malloc(len);
  if(!ret) return NULL;
  snprintf(ret, len - 1, "%s/%d", base_path, timestamp);
  return ret;
}

char* segments_segment_path(segments* segments, segment* segment) {
  return filename(segments->base_path, segment->timestamp);
}

int segments_init(segments* segments) {
  segments->head = NULL;
  return OK;
}

int segments_destroy(segments* segments) {
  if (segments->head) {
    segment* curr = segments->head;
    segment* next;
    while(curr != NULL) {
      next = curr->next;
      free(curr);
      curr = next;
    }
    segments->head = NULL;
  }
  return OK;
}

u_int32_t segments_size(segments* segments) {
  u_int32_t size = 0;

  segment* curr = segments->head;
  while(curr != NULL) {
    size += curr->size;
    curr = curr->next;
  }

  return size;
}

u_int32_t segments_count(segments* segments) {
  u_int32_t count = 0;

  segment* curr = segments->head;
  while(curr != NULL) {
    count += 1;
    curr = curr->next;
  }

  return count;
}

segment* segments_pop(segments* segments) {

  segment* head = segments->head;
  segment* prev = NULL;
  segment* curr = head;
  while(curr != NULL && curr->next != NULL) {
    prev = curr;
    curr = curr->next;
  }

  if (prev) {
    // popping an element from the chain
    prev->next = NULL;
  } else {
    // popping the last element
    segments->head = NULL;
  }

  return curr;
}

int segments_add(segments* segments, u_int32_t timestamp) {

  // continously increasing
  if (segments->head) timestamp = MAX(timestamp, segments->head->timestamp + 1);

  char* path = filename(segments->base_path, timestamp);
  check(path, OOM);

  // check length
  u_int32_t size = 0;
  struct stat st;
  if (stat(path, &st) == OK) {
    check(S_ISREG(st.st_mode), "not a regular file %s", path);
    size = st.st_size;
    // FIXME maybe check a magic header?
  }
  free(path);

  segment* new_segment = malloc(sizeof(segment));
  check(new_segment, OOM);
  new_segment->timestamp = timestamp;
  new_segment->size = size;
  new_segment->count = 0;

  segment* head = segments->head;
  segment* prev = NULL;
  segment* curr = head;
  while(curr != NULL && new_segment->timestamp < curr->timestamp) {
    prev = curr;
    curr = curr->next;
  }

  if (prev) {
    // insert into chain
    new_segment->next = prev->next;
    prev->next = new_segment;
  } else {
    // new segments head
    new_segment->next = head;
    segments->head = new_segment;
  }

  return OK;
}