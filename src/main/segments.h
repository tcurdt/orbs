#ifndef _SEGMENTS_H
#define _SEGMENTS_H

#include "common.h"
#include "message.h"

typedef struct position position;
struct position {
  u_int32_t timestamp;
  u_int32_t offset;
};

typedef struct segment segment;
struct segment {
  u_int32_t   timestamp;
  u_int32_t   size;
  u_int32_t   count;
  segment*    next;
};

typedef struct segments segments;
struct segments {
  const char* base_path;
  segment*    head; // oldest
  segment*    tail; // newest
};

char* segments_segment_path(segments* segments, segment* segment);

void segments_print(segments* segments);

int segments_init(segments* segments);
int segments_destroy(segments* segments);

u_int32_t segments_size(segments* segments);
u_int32_t segments_count(segments* segments);

segment* segments_pop(segments* segments);
int segments_add(segments* segments, u_int32_t timestamp);

#endif