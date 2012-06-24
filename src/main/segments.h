#ifndef _SEGMENTS_H
#define _SEGMENTS_H

#include "common.h"
#include "message.h"

struct position {
  u_int32_t timestamp;
  u_int32_t offset;
};
typedef struct position position;

struct segment {
  const char*     full_path;
  u_int32_t       timestamp;
  u_int32_t       size;
  struct segment* previous_segment;
};
typedef struct segment segment;

struct segments {
  const char* base_path;
  segment*    head;
  FILE*       file;
};
typedef struct segments segments;

int segments_init(segments* segments);
int segments_destroy(segments* segments);
int segments_head_write(segments* segments, message* message);
u_int32_t segments_head_size(segments* segments);
u_int32_t segments_count(segments* segments);
int segments_pop(segments* segments);
int segments_add(segments* segments, const char *original_full_path);


#endif