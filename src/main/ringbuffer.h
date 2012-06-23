#ifndef _RINGBUFFER_H
#define _RINGBUFFER_H

#include <sys/types.h>

typedef struct message {
  u_int8_t  type;
  void*     body;
  size_t    body_size;
  u_int32_t crc32;
} message, *message_t;

typedef struct position {
  u_int32_t timestamp;
  u_int32_t offset;
} position, *position_t;

typedef struct ringebuffer_segment {
  u_int32_t                   timestamp;
  u_int32_t                   size;
  struct ringebuffer_segment* previous_segment;
} ringebuffer_segment, *ringebuffer_segment_t;

typedef struct ringbuffer {
  u_int32_t             max_segment_count;
  u_int32_t             max_segment_size;
  ringebuffer_segment_t current_segment;
  FILE*                 current_file;
} ringbuffer, *ringbuffer_t;


int ringbuffer_open(const char* base_path, ringbuffer_t buffer);
int ringbuffer_append(ringbuffer_t buffer, message_t message);
int ringbuffer_read(ringbuffer_t buffer, position_t position, message_t message);
int ringbuffer_close(ringbuffer_t buffer);
u_int32_t ringbuffer_size(ringbuffer_t buffer);
u_int32_t ringbuffer_segment_count(ringbuffer_t buffer);

#endif