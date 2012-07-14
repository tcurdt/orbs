#ifndef _COMMON_H
#define _COMMON_H

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <fts.h>

#define UNUSED(expr) { (void)(expr); }

#ifdef __linux__
#define FSYNC(fd) fdatasync(fd)
#else
#define FSYNC(fd) fsync(fd)
#endif

#define OK 0
#define ERROR -1

#define MAX(a,b) (((a)>(b))?(a):(b))

#define reterr(fmt, ...) { fprintf(stderr, "[%s:%d]: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); return ERROR; }
#define check(condition, fmt, ...) { if (!(condition)) reterr(fmt, ##__VA_ARGS__); }
#define OOM "out of memory"

#endif