#ifndef _RINGBUFFER_H
#define _RINGBUFFER_H

#include <sys/types.h>

typedef struct message {
  u_int8_t  type;
  void*     body;
  size_t    body_length;
  u_int32_t crc32;
} message, *message_t;

typedef struct position {
  u_int32_t timestamp;
  u_int32_t offset;
} position, *position_t;

typedef struct segment_file {
  u_int32_t            timestamp;
  u_int32_t            size;
  struct segment_file* previous_segment;
} segment_file, *segment_file_t;

typedef struct ringbuffer {
  u_int32_t      max_segment_count;
  u_int32_t      max_segment_size;
  segment_file_t current_segment;
} ringbuffer, *ringbuffer_t;


int ringbuffer_open(const char* base_path, ringbuffer_t buffer);
int ringbuffer_append(ringbuffer_t buffer, message_t message);
int ringbuffer_read(ringbuffer_t buffer, position_t position, message_t message);
int ringbuffer_close(ringbuffer_t buffer);
u_int32_t ringbuffer_size(ringbuffer_t buffer);

#endif