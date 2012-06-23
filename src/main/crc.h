#ifndef _CRC_H
#define _CRC_H

#include <sys/types.h>

u_int32_t crc32_buffer(const char *buffer, size_t len);

#endif
