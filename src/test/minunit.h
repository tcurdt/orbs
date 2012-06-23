#undef NDEBUG
#ifndef _MINUNIT_H
#define _MINUNIT_H

#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <signal.h>

#define mu_suite_start() char *message = NULL
#define mu_assert(test, message) if (!(test)) { return message; }
#define mu_run_test(test) message = test(); printf(" - %s %s %s\n", (message)?"ERROR":"OK", #test, (message)?message:""); tests_run++; if (message) return message;


#define RUN_TESTS(name) void handler(int sig) { \
  void *array[10]; \
  size_t size = backtrace(array, 10); \
  fprintf(stderr, "Error: signal %d:\n", sig); \
  backtrace_symbols_fd(array, size, 2); \
  exit(1); \
} \
int main(int argc, char *argv[]) { \
  argc = argc; argv = argv; \
  signal(SIGSEGV, handler); \
  FILE *log_file = fopen("src/test/tests.log", "a+"); \
  if(!log_file) { printf("CAN'T OPEN TEST LOG\n"); exit(1); } \
  char *result = name(); \
  printf(" Tests run: %d\n", tests_run); \
  exit(result != NULL); \
  return result != NULL ? 1 : 0; \
}

/*
#define RUN_TESTS(name) int main(int argc, char *argv[]) { \
  argc = argc; argv = argv; \
  FILE *log_file = fopen("src/test/tests.log", "a+"); \
  if(!log_file) { printf("CAN'T OPEN TEST LOG\n"); exit(1); } \
  char *result = name(); \
  printf(" Tests run: %d\n", tests_run); \
  exit(result != NULL); \
  return result != NULL ? 1 : 0; \
}
*/

int tests_run;

#endif
