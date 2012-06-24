#include "minunit.h"
#include <assert.h>
// #include <stdio.h>
#include "string.h"

#include "ringbuffer.h"

static u_int32_t sum_timestamps(ringbuffer* buffer) {
  u_int32_t sum = 0;
  int l = 0;
  int order = 0;
  segments* segments = &buffer->segments;
  segment* curr = segments->head;
  while(curr != NULL) {
    sum += curr->timestamp << order;
    curr = curr->previous_segment;
    l += 1;
    order += 1;
  }
  return sum + l;
}

char *test_ringbuffer_open() {

  ringbuffer b;
  b.max_segment_count = 3;
  b.max_segment_size = 50;

  ringbuffer_open("src/fixtures/segments/empty", &b);
  mu_assert(ringbuffer_size(&b) == 0, "segments size should be zero");
  mu_assert(sum_timestamps(&b) > 0, "there always is a segments");
  ringbuffer_close(&b);

  ringbuffer_open("src/fixtures/segments/single", &b);
  mu_assert(ringbuffer_size(&b) == 1, "segments should have the correct size");
  mu_assert(sum_timestamps(&b) == ((1340423024<<0) + 1), "there should only be one segment");
  ringbuffer_close(&b);

  ringbuffer_open("src/fixtures/segments/order", &b);
  mu_assert(ringbuffer_size(&b) == (1 + 2 + 3), "segments should have the correct size");
  mu_assert(sum_timestamps(&b) == ((1340423026<<0) + (1340423025<<1) + (1340423024<<2) + 3), "the segments should have the right order");
  ringbuffer_close(&b);

  return NULL;
}

char *test_ringbuffer_append() {

  ringbuffer b;
  b.max_segment_count = 3;
  b.max_segment_size = 1;

  message m;
  m.type = 1;
  m.body = "body";
  m.body_size = strlen(m.body);
  m.crc32 = 0;

  ringbuffer_open("src/fixtures/segments/empty", &b);
  mu_assert(ringbuffer_append(&b, &m) == 0, "failed to append");
  mu_assert(ringbuffer_append(&b, &m) == 0, "failed to append");
  mu_assert(ringbuffer_append(&b, &m) == 0, "failed to append");
  mu_assert(segments_count(&b.segments) == 3, "each append should have started a new segment file (body > max_segment_size)");
  mu_assert(ringbuffer_append(&b, &m) == 0, "failed to append");
  mu_assert(segments_count(&b.segments) == 3, "max_segment_count should never be exceeded");
  ringbuffer_close(&b);

  return NULL;
}

char *test_ringbuffer_read() {
  mu_assert(1, "Failed to read");
  return NULL;
}

char *all_tests() {
  mu_suite_start();

  mu_run_test(test_ringbuffer_open);
  mu_run_test(test_ringbuffer_append);
  mu_run_test(test_ringbuffer_read);

  return NULL;
}

RUN_TESTS(all_tests);
