#ifndef _COMMON_H
#define _COMMON_H

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <ftw.h>

#define _XOPEN_SOURCE 500
#define UNUSED(expr) { (void)(expr); }

#define OK 0
#define ERROR -1

#define MAX(a,b) (((a)>(b))?(a):(b))

#define reterr(fmt, ...) { fprintf(stderr, "[%s:%d]: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); return ERROR; }
#define check(condition, fmt, ...) { if (!(condition)) reterr(fmt, ##__VA_ARGS__); }
#define OOM "out of memory"

#endif