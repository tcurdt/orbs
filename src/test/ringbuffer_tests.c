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
  b.max_segment_count = 3;
  b.max_segment_size = 50;

  tmp_path = tmp_create();
  ringbuffer_open(tmp_path, &b);
  mu_assert(ringbuffer_size(&b) == 0, "segments size should be zero");
  mu_assert(sum_timestamps(&b) > 0, "there always is a segments");
  ringbuffer_close(&b);
  tmp_remove(tmp_path);

  tmp_path = tmp_create();
  file_write(tmp_path, "1340423024", "1");
  ringbuffer_open(tmp_path, &b);
  mu_assert(ringbuffer_size(&b) == 1, "segments should have the correct size");
  // mu_assert(sum_timestamps(&b) == ((1340423024<<0) + 1), "there should only be one segment");
  ringbuffer_close(&b);
  tmp_remove(tmp_path);

  tmp_path = tmp_create();
  file_write(tmp_path, "1340423024", "1");
  file_write(tmp_path, "1340423025", "12");
  file_write(tmp_path, "1340423026", "123");
  ringbuffer_open(tmp_path, &b);
  mu_assert(ringbuffer_size(&b) == (1 + 2 + 3), "segments should have the correct size");
  // mu_assert(sum_timestamps(&b) == ((1340423026<<0) + (1340423025<<1) + (1340423024<<2) + 3), "the segments should have the right order");
  ringbuffer_close(&b);
  tmp_remove(tmp_path);

  return NULL;
}

char *test_ringbuffer_should_not_grow_beyond_limits() {

  const char *tmp_path;
  int i;

  message m;
  m.type = 1;
  m.body = "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";
  m.body_size = strlen(m.body);
  m.crc32 = 0;

  u_int32_t m_size = message_size(&m);

  ringbuffer b;
  b.max_segment_count = 3;
  b.max_segment_size = m_size * 100 - 1; // after 100 messages we need a new segment

  tmp_path = tmp_create();
  ringbuffer_open(tmp_path, &b);

  mu_assert(segments_count(&b.segments) == 1, "segment count should be 1");
  for(i=0;i<100;i++) {
    mu_assert(ringbuffer_append(&b, &m) == OK, "failed to append");
  }

  mu_assert(segments_count(&b.segments) == 2, "segment count should be 2");
  for(i=0;i<100;i++) {
    mu_assert(ringbuffer_append(&b, &m) == OK, "failed to append");
  }

  mu_assert(segments_count(&b.segments) == 3, "segment count should be 3");
  for(i=0;i<100;i++) {
    mu_assert(ringbuffer_append(&b, &m) == OK, "failed to append");
  }

  mu_assert(segments_count(&b.segments) == 3, "segment count should be still 3");
  for(i=0;i<100;i++) {
    mu_assert(ringbuffer_append(&b, &m) == OK, "failed to append");
  }
  mu_assert(segments_count(&b.segments) == 3, "segment count should be still 3");
  mu_assert(ringbuffer_size(&b) == (4+99+99+99 - 99) * m_size, "should have dropped 99");

  // check each segment file exists
  segments* segments = &b.segments;
  segment* curr = segments->head;
  while(curr != NULL) {
    char* path = segments_segment_path(segments, curr);
    mu_assert(file_exists(path) == true, "file does not exist");
    free(path);
    curr = curr->next;
  }

  ringbuffer_close(&b);
  tmp_remove(tmp_path);

  return NULL;
}

// char *test_ringbuffer_read() {
//   mu_assert(1, "Failed to read");
//   return NULL;
// }

char *all_tests() {
  mu_suite_start();

  mu_run_test(test_ringbuffer_should_open_segments_from_disk);
  mu_run_test(test_ringbuffer_should_not_grow_beyond_limits);

  return NULL;
}

RUN_TESTS(all_tests);
