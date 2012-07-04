#include "common.h"
#include "ringbuffer.h"
#include <poll.h>

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

  // // buffering
  // setvbuf(stdin, NULL, _IOLBF, 0);
  // // setvbuf(stdin, NULL, _IONBF, 0);
  // // setvbuf(stdout, NULL, _IONBF, 0);

  // polling
  int timeout = 1000;
  struct pollfd pfd;
  pfd.fd = STDIN_FILENO;
  pfd.events = POLLIN;
  pfd.revents = 0;

  size_t buffer_len = 1024;
  unsigned char *buffer = malloc(sizeof(unsigned char) * buffer_len);

  while(1) {
    int ret = poll(&pfd, 1, timeout);
    if (ret > 0 && ((pfd.revents & POLLIN) != 0)) {
      ssize_t bytes_read = read(pfd.fd, buffer, buffer_len);
      printf("read %zd\n", bytes_read);

      // ringbuffer_append(&rb, &m);
    }
    printf("loop\n");
  }

  if (ringbuffer_close(&rb) != OK) {
    fprintf(stderr, "failed to close ringbuffer");
    exit(-1);
  }

  return 0;
}