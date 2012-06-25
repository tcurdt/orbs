#undef NDEBUG
#ifndef _MINUNIT_H
#define _MINUNIT_H

#include "common.h"
#include "unixy.h"
#include <assert.h>
#include <execinfo.h>
#include <signal.h>

#define mu_suite_start() char *message = NULL
#define mu_assert(test, message) if (!(test)) { return message; }
#define mu_run_test(test) message = test(); if (!message) { printf("."); } else { printf("\n  ERROR in %s: %s\n", #test, message); return message; }

#define RUN_TESTS(test) void handler(int sig) { \
  void *array[10]; \
  size_t size = backtrace(array, 10); \
  fprintf(stderr, "Error: signal %d:\n", sig); \
  backtrace_symbols_fd(array, size, 2); \
  exit(1); \
} \
int main(int argc, char *argv[]) { \
  argc = argc; argv = argv; \
  signal(SIGSEGV, handler); \
  printf(" %s ", basename(argv[0])); \
  char *result = test(); \
  if (!result) printf("\n"); \
  exit(result != NULL); \
  return result != NULL ? 1 : 0; \
}

#endif
