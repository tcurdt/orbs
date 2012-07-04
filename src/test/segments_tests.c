#include "minunit.h"
#include "segments.h"

char *test_segments_should_maintain_order_and_be_monotonic() {
  segments s;
  mu_assert(segments_init(&s) == OK, "failed to init");

  mu_assert(segments_add(&s, 4, 0) == OK, "failed to add");
  mu_assert(segments_add(&s, 1, 0) == OK, "failed to add");
  mu_assert(segments_add(&s, 3, 0) == OK, "failed to add");
  mu_assert(segments_add(&s, 2, 0) == OK, "failed to add");

  segment *e;

  e = segments_pop(&s);
  mu_assert(e != NULL, "nothing from pop");
  mu_assert(e->timestamp == 4, "wrong timestamp");
  free(e);

  e = segments_pop(&s);
  mu_assert(e != NULL, "nothing from pop");
  mu_assert(e->timestamp == 5, "wrong timestamp");
  free(e);

  e = segments_pop(&s);
  mu_assert(e != NULL, "nothing from pop");
  mu_assert(e->timestamp == 6, "wrong timestamp");
  free(e);

  e = segments_pop(&s);
  mu_assert(e != NULL, "nothing from pop");
  mu_assert(e->timestamp == 7, "wrong timestamp");
  free(e);

  e = segments_pop(&s);
  mu_assert(e == NULL, "should be empty");

  mu_assert(segments_destroy(&s) == OK, "failed to destroy");
  return NULL;
}


char *suite() {
  mu_suite_start();

  mu_run_test(test_segments_should_maintain_order_and_be_monotonic);

  return NULL;
}

RUN_SUITE(suite);
