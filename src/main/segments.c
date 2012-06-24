#include "common.h"
#include "segments.h"

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
  // write to file
  segments->head->size += message->body_size; // FIXME
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
    // remove file
    free(curr);
  }

  return OK;
}

int segments_add(segments* segments, u_int32_t timestamp, u_int32_t size) {

  segment* new_segment = malloc(sizeof(segment));
  if (!new_segment) reterr("out of memory");
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