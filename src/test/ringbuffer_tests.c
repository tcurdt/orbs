#include "minunit.h"
#include "ringbuffer.h"
#include "message.h"
#include "unixy.h"

char *test_ringbuffer_should_open_segments_from_disk() {

  const char *tmp_path;

  ringbuffer b;
  b.max_segment_size = 50;
  b.max_total_size = 3 * b.max_segment_size;
  b.sync_freq = 1;

  tmp_path = tmp_create();
  mu_assert(tmp_path, "failed to create tmp");
  b.base_path = strdup(tmp_path);
  mu_assert(ringbuffer_open(&b) == OK, "failed ringbuffer_open");
  mu_assert(ringbuffer_size(&b) == 0, "size is not zero (%d)", ringbuffer_size(&b));
  mu_assert(ringbuffer_close(&b) == OK, "failed ringbuffer_close");
  tmp_remove(tmp_path);

  tmp_path = tmp_create();
  mu_assert(tmp_path, "failed to create tmp");
  b.base_path = strdup(tmp_path);
  file_write(tmp_path, "1340423024", "1");
  mu_assert(ringbuffer_open(&b) == OK, "failed ringbuffer_open");
  mu_assert(ringbuffer_size(&b) == 1, "size is wrong (%d)", ringbuffer_size(&b));
  mu_assert(ringbuffer_close(&b) == OK, "failed ringbuffer_close");
  tmp_remove(tmp_path);

  tmp_path = tmp_create();
  mu_assert(tmp_path, "failed to create tmp");
  b.base_path = strdup(tmp_path);
  file_write(tmp_path, "1340423024", "1");
  file_write(tmp_path, "1340423025", "12");
  file_write(tmp_path, "1340423026", "123");
  mu_assert(ringbuffer_open(&b) == OK, "failed ringbuffer_open");
  mu_assert(ringbuffer_size(&b) == (1 + 2 + 3), "size is wrong(%d)", ringbuffer_size(&b));
  mu_assert(ringbuffer_close(&b) == OK, "failed ringbuffer_close");
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
  mu_assert(tmp_path, "failed to create tmp");

  ringbuffer b;
  b.max_segment_size = (m_size * fill) - 1; // cannot fit all fill messages into one segment
  b.max_total_size = 3 * b.max_segment_size;
  b.sync_freq = 1;
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
