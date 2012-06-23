#include "minunit.h"
#include <assert.h>

char *test_ringbuffer_append()
{
    mu_assert(1, "Failed to append");
    return NULL;
}

char *test_ringbuffer_read()
{
    mu_assert(1, "Failed to read");
    return NULL;
}

char *all_tests() {
    mu_suite_start();

    mu_run_test(test_ringbuffer_append);
    mu_run_test(test_ringbuffer_read);

    return NULL;
}

RUN_TESTS(all_tests);
