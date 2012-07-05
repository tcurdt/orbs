#include "common.h"
#include "ringbuffer.h"

static void usage() {
  printf("usage\n");
  exit(-1);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    usage();
  }

  ringbuffer rb;
  rb.base_path = strdup(argv[1]);
  rb.max_segment_size = 100000;
  rb.max_total_size = 3 * rb.max_segment_size;
  
  if (ringbuffer_open(&rb) != OK) {
    fprintf(stderr, "failed to open ringbuffer");
    exit(-1);
  }

  size_t read_buffer_len = 16;
  char*  read_buffer = malloc(read_buffer_len);

  size_t message_buffer_len = 16;
  char*  message_buffer = malloc(message_buffer_len);
  size_t message_len = 0;
  size_t message_len_max = 0;

  while(1) {

    ssize_t bytes_read = read(STDIN_FILENO, read_buffer, read_buffer_len);

    if (bytes_read <= 0) break;

    if ((message_len + bytes_read) > message_buffer_len) {
      message_buffer_len <<= 1;
      message_buffer = realloc(message_buffer, message_buffer_len);
    }

    ssize_t i = 0;
    while(i<bytes_read) {
      char c = read_buffer[i++];
      if (c == '\n') {
        message_buffer[message_len] = 0;

        if (message_len > message_len_max) {
          message_len_max = message_len;
        }

        if (message_len_max > read_buffer_len) {
          read_buffer_len = message_len_max;
          read_buffer = realloc(read_buffer, read_buffer_len);
        }

        // printf("message: %s\n", message_buffer);
        printf("len=%zd max=%zd\n", message_len, message_len_max);

        ringbuffer_append(&rb, message_buffer, message_len);

        message_len = 0;
      } else {
        message_buffer[message_len++] = c;
      }
    }
  }

  if (ringbuffer_close(&rb) != OK) {
    fprintf(stderr, "failed to close ringbuffer");
    exit(-1);
  }

  return 0;
}