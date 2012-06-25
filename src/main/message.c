#include "common.h"
#include "message.h"

u_int32_t message_size(message* message) {
  u_int32_t len = 0;
  len += sizeof(u_int8_t);
  len += sizeof(size_t);
  len += message->body_size;
  len += sizeof(u_int32_t);
  return len;
}
