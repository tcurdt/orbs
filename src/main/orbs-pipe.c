#include "common.h"
#include "ringbuffer.h"
#include <sys/time.h>

u_int64_t timeval_diff(struct timeval* e, struct timeval* s) {
  struct timeval d;

  d.tv_sec  = e->tv_sec  - s->tv_sec;
  d.tv_usec = e->tv_usec - s->tv_usec;

  while(d.tv_usec < 0) {
    d.tv_usec += 1000000;
    d.tv_sec  -= 1;
  }

  return 1000000LL * d.tv_sec + d.tv_usec;
}


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
  rb.max_segment_size = 10000000;
  rb.max_total_size = 3 * rb.max_segment_size;
  rb.sync_freq = 100;
  
  if (ringbuffer_open(&rb) != OK) {
    fprintf(stderr, "failed to open ringbuffer\n");
    exit(-1);
  }

  size_t read_buffer_len = 16;
  char*  read_buffer = malloc(read_buffer_len);

  size_t message_buffer_len = 16;
  char*  message_buffer = malloc(message_buffer_len);
  size_t message_len = 0;
  size_t message_len_max = 0;

  size_t total_bytes = 0;
  size_t total_messages = 0;

  struct timeval start, end;
  gettimeofday(&start, NULL);

  while(1) {

    ssize_t bytes_read = read(STDIN_FILENO, read_buffer, read_buffer_len);

    if (bytes_read <= 0) break;

    total_bytes += bytes_read;

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
        // printf("len=%zd max=%zd\n", message_len, message_len_max);

        ringbuffer_append(&rb, message_buffer, message_len);

        total_messages += 1;

        message_len = 0;
      } else {
        message_buffer[message_len++] = c;
      }
    }
  }

  gettimeofday(&end, NULL);

  if (ringbuffer_close(&rb) != OK) {
    fprintf(stderr, "failed to close ringbuffer\n");
    exit(-1);
  }

  u_int64_t duration = timeval_diff(&end, &start);

  printf("%zd messages, %zd bytes in %1.1fs, %1.1f bytes/s\n", total_messages, total_bytes, (double)duration/1000000, (double)total_bytes*1000000/duration);

  return 0;
}