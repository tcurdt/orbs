#ifndef _COMMON_H
#define _COMMON_H

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define OK 0
#define ERROR -1

#define reterr(F, ...) { fprintf(stderr, "[%s:%d]: " F, __FILE__, __LINE__, ##__VA_ARGS__); return ERROR; }

#endif