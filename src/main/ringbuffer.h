#ifndef _RINGBUFFER_H
#define _RINGBUFFER_H

#include "common.h"
#include "segments.h"

#define RINGBUFFER_SYNC_ALWAYS  0
#define RINGBUFFER_SYNC_COUNTS  1
#define RINGBUFFER_SYNC_SECONDS 2

typedef struct ringbuffer ringbuffer;
struct ringbuffer {
  u_int32_t    max_total_size;
  u_int32_t    max_segment_size;
  u_int32_t    sync_freq;
  int          sync_type;
  const char*  base_path;

  // private
  int          fd;
  segments     segments;
  u_int32_t    total_size;
};

int ringbuffer_open(ringbuffer* buffer);
int ringbuffer_close(ringbuffer* buffer);
int ringbuffer_append(ringbuffer* buffer, void* message, u_int32_t message_length);

u_int32_t ringbuffer_size(ringbuffer* buffer);

void ringbuffer_print(ringbuffer* buffer);

#endif