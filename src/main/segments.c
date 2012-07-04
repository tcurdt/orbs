#include "common.h"
#include "segments.h"

void segments_print(segments* segments) {
  printf("segments: %d = { ", segments_count(segments));
  segment* segment = segments->head;
  while(segment != NULL) {
    printf("%d:{ %d/%d } ", segment->timestamp, segment->size, segment->messages);
    segment = segment->next;
  }
  printf("}\n");
}

int segments_init(segments* segments) {
  segments->head = NULL;
  segments->tail = NULL;
  return OK;
}

int segments_destroy(segments* segments) {
  segment* curr = segments->head;
  segment* next;
  while(curr != NULL) {
    next = curr->next;
    free(curr);
    curr = next;
  }
  segments->head = NULL;
  segments->tail = NULL;
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
  if (head) {
    segments->head = head->next;
    // set tail to null if last element
    if (segments->head == NULL) {
      segments->tail = NULL;
    }
  }
  return head;
}

int segments_add(segments* segments, u_int32_t timestamp, u_int32_t size) {

  // continously increasing
  if (segments->tail) timestamp = MAX(timestamp, segments->tail->timestamp + 1);

  // create segment
  segment* new_segment = malloc(sizeof(segment));
  check(new_segment, OOM);
  new_segment->timestamp = timestamp;
  new_segment->size = size;
  new_segment->messages = 0;

  segment* head = segments->head;
  segment* prev = NULL;
  segment* curr = head;
  while(curr != NULL && new_segment->timestamp > curr->timestamp) {
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

  if (new_segment->next == NULL) {
    segments->tail = new_segment;
  }

  return OK;
}