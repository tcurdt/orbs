#ifndef _RINGBUFFER_H
#define _RINGBUFFER_H

#include "common.h"
#include "message.h"
#include "segments.h"

typedef struct ringbuffer ringbuffer;
struct ringbuffer {
  u_int32_t   max_segment_count;
  u_int32_t   max_segment_size;
  segments    segments;
  FILE*       file;
};

void ringbuffer_print(ringbuffer* buffer);
  
int ringbuffer_open(const char* base_path, ringbuffer* buffer);
int ringbuffer_close(ringbuffer* buffer);

u_int32_t ringbuffer_size(ringbuffer* buffer);

int ringbuffer_append(ringbuffer* buffer, message* message);
int ringbuffer_read(ringbuffer* buffer, position* position, message* message);

#endif