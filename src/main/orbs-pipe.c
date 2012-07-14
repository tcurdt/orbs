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


static void usage(char **argv) {
  fprintf(stderr, "Usage: %s [-t max_total_size] [-s max_segment_size] [-f sync_freq] dir\n", argv[0]);
  exit(-1);
}

int main(int argc, char **argv) {

  int opt;

  // default parameters
  u_int32_t max_segment_size = 10000000;
  u_int32_t max_total_size = 4 * max_segment_size;
  u_int32_t sync_freq = 1;
  int       sync_type = RINGBUFFER_SYNC_ALWAYS;
  char*     base_path = "buffer";

  while ((opt = getopt(argc, argv, "f:s:t:")) != -1) {
    switch (opt) {
      case 'f': {
        int len = strlen(optarg);
        switch(optarg[len - 1]) {
          case 's': sync_type = RINGBUFFER_SYNC_SECONDS; break;
          case 'x': sync_type = RINGBUFFER_SYNC_COUNTS; break;
          default: usage(argv);
        }
        optarg[len - 1] = 0;
        sync_freq = atoi(optarg);
        break;
      }
      case 't':
        max_total_size = atoi(optarg);
        break;
      case 's':
        max_segment_size = atoi(optarg);
        break;
      default: usage(argv);
    }
  }

  if (optind >= argc) {
    usage(argv);
  }

  base_path = argv[optind];

  printf("max_total_size=%d; max_segment_size=%d; sync_freq=%d; sync_type=%d; base_path=%s\n",
    max_total_size, max_segment_size, sync_freq, sync_type, base_path);


  // for benchmarking
  size_t total_bytes = 0;
  size_t total_messages = 0;
  struct timeval start, end;


  // setup ringbuffer
  ringbuffer rb;
  rb.base_path = strdup(base_path);
  rb.max_segment_size = max_segment_size;
  rb.max_total_size = max_total_size;
  rb.sync_freq = sync_freq;
  rb.sync_type = sync_type;

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

        ringbuffer_append(&rb, message_buffer, message_len);

        message_len = 0;

        total_messages += 1;
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

  // printf("%zd messages, %zd bytes in %1.1fs, %1.1f bytes/s\n", total_messages, total_bytes, (double)duration/1000000, (double)total_bytes*1000000/duration);
  printf("%1.1f\n", (double)total_bytes*1000000/duration);

  return 0;
}