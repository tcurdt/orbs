#include "minunit.h"
#include "ringbuffer.h"
#include "message.h"
#include "unixy.h"

static u_int32_t sum_timestamps(ringbuffer* buffer) {
  u_int32_t sum = 0;
  int l = 0;
  int order = 0;
  segments* segments = &buffer->segments;
  segment* curr = segments->head;
  while(curr != NULL) {
    sum += curr->timestamp << order;
    curr = curr->next;
    l += 1;
    order += 1;
  }
  return sum + l;
}

char *test_ringbuffer_should_open_segments_from_disk() {

  const char *tmp_path;

  ringbuffer b;
  b.max_segment_size = 50;
  b.max_total_size = 3 * b.max_segment_size;

  tmp_path = tmp_create();
  b.base_path = strdup(tmp_path);
  ringbuffer_open(&b);
  mu_assert(ringbuffer_size(&b) == 0, "size is not zero (%d)", ringbuffer_size(&b));
  mu_assert(sum_timestamps(&b) > 0, "there always is a segments");
  ringbuffer_close(&b);
  tmp_remove(tmp_path);

  tmp_path = tmp_create();
  b.base_path = strdup(tmp_path);
  file_write(tmp_path, "1340423024", "1");
  ringbuffer_open(&b);
  mu_assert(ringbuffer_size(&b) == 1, "size is wrong (%d)", ringbuffer_size(&b));
  // mu_assert(sum_timestamps(&b) == ((1340423024<<0) + 1), "there should only be one segment");
  ringbuffer_close(&b);
  tmp_remove(tmp_path);

  tmp_path = tmp_create();
  b.base_path = strdup(tmp_path);
  file_write(tmp_path, "1340423024", "1");
  file_write(tmp_path, "1340423025", "12");
  file_write(tmp_path, "1340423026", "123");
  ringbuffer_open(&b);
  mu_assert(ringbuffer_size(&b) == (1 + 2 + 3), "size is wrong(%d)", ringbuffer_size(&b));
  // mu_assert(sum_timestamps(&b) == ((1340423026<<0) + (1340423025<<1) + (1340423024<<2) + 3), "the segments should have the right order");
  ringbuffer_close(&b);
  tmp_remove(tmp_path);

  return NULL;
}

char *test_ringbuffer_should_not_grow_beyond_limits() {

  const char *tmp_path;
  int i;

  char *m = "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";
  u_int32_t m_size = strlen(m);

  int fill = 10;

  tmp_path = tmp_create();

  ringbuffer b;
  b.max_segment_size = (m_size * fill) - 1; // cannot fit all fill messages into one segment
  b.max_total_size = 3 * b.max_segment_size;
  b.base_path = strdup(tmp_path);

  ringbuffer_open(&b);

  mu_assert(segments_count(&b.segments) == 1, "segment count should be 1 (%d)", segments_count(&b.segments));
  for(i=0;i<fill;i++) {
    mu_assert(ringbuffer_append(&b, m, m_size) == OK, "failed to append");
  }

  mu_assert(segments_count(&b.segments) == 2, "segment count should be 2 (%d)", segments_count(&b.segments));
  for(i=0;i<fill;i++) {
    mu_assert(ringbuffer_append(&b, m, m_size) == OK, "failed to append");
  }

  mu_assert(segments_count(&b.segments) == 3, "segment count should be 3 (%d)", segments_count(&b.segments));
  for(i=0;i<fill;i++) {
    mu_assert(ringbuffer_append(&b, m, m_size) == OK, "failed to append");
  }

  mu_assert(segments_count(&b.segments) == 3, "segment count should be still 3 (%d)", segments_count(&b.segments));
  for(i=0;i<fill;i++) {
    mu_assert(ringbuffer_append(&b, m, m_size) == OK, "failed to append");
  }

  mu_assert(segments_count(&b.segments) == 3, "segment count should be still 3 (%d)", segments_count(&b.segments));
  mu_assert(ringbuffer_size(&b) == (4 + 2 * (fill-1)) * m_size, "should have dropped fill-1 messages");

  ringbuffer_close(&b);
  tmp_remove(tmp_path);

  return NULL;
}

char *suite() {
  mu_suite_start();

  mu_run_test(test_ringbuffer_should_open_segments_from_disk);
  mu_run_test(test_ringbuffer_should_not_grow_beyond_limits);

  return NULL;
}

RUN_SUITE(suite);
