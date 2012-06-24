#ifndef _MESSAGE_H
#define _MESSAGE_H

struct message {
  u_int8_t  type;
  void*     body;
  size_t    body_size;
  u_int32_t crc32;
};
typedef struct message message;

#endif