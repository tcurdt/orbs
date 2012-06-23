#include <stdio.h>
#include <unistd.h> // sleep

#include "ringbuffer.h"

int main(int argc, char **argv) {

  ringbuffer_t b = calloc(1, sizeof(ringbuffer));

  ringbuffer_open("/Users/tcurdt/Desktop/kafka-native/cafka/src/fixtures", b);

  ringbuffer_close(b);
  
  if (b) free(b);

  // sleep(20);

  return 0;
}