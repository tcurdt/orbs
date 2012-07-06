#undef NDEBUG
#ifndef _MINUNIT_H
#define _MINUNIT_H

#include "common.h"
#include "unixy.h"
#include <assert.h>
#include <execinfo.h>
#include <signal.h>

#define mu_suite_start() char *message = NULL
#define mu_assert(condition, format, args...) if ((condition)) { printf("-"); } else { char* s = malloc(200); snprintf(s, 200, "line %d: " format, __LINE__, ##args); s[200] = 0; return s; }
#define mu_run_test(test) { printf("+"); message = test(); if (message) { printf("\n  ERROR in %s:\n    %s", #test, message); return message; }}

#define RUN_SUITE(suite) void handler(int sig) { \
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
  char *result = suite(); \
  printf("\n"); \
  return result != NULL ? 1 : 0; \
}

#endif
