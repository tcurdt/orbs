#include "minunit.h"
#include "message.h"

char *test_message_size() {

  message m;
  m.type = 1;
  m.body = "01234567890123456789";
  m.body_size = strlen(m.body);
  m.crc32 = 0;

  u_int32_t len = 0;
  len += sizeof(u_int8_t);
  len += sizeof(size_t);
  len += m.body_size;
  len += sizeof(u_int32_t);

  mu_assert(message_size(&m) == len, "message size should match memory layout");

  return NULL;
}

char *suite() {
  mu_suite_start();

  mu_run_test(test_message_size);

  return NULL;
}

RUN_SUITE(suite);
