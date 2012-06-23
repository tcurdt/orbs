#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#import "ringbuffer.h"
#include "crc.h"

#define MAX(a,b) ((a)>(b))?(a):(b)

// #import "bstring.h"
// #include <stdbool.h>
// static bool filename_matches(const char *filename) {
//   return true;
// }

// potential overflow bug but we don't care
// strings are coming from the filesystem
static char* join(const char* a, const char* b, const char* c) {
  size_t al = strlen(a), bl = strlen(b), cl = strlen(c);
  size_t len = al + bl + cl + 1;

  char *ret = malloc(len);
  if(ret == NULL) {
    return NULL;
  }

  strcpy(ret, a);
  strcpy(ret + al, b);
  strcpy(ret + al + bl, c);
  return ret;
}

u_int32_t ringbuffer_segment_count(ringbuffer_t buffer) {
  u_int32_t count = 0;
  ringebuffer_segment_t head = buffer->current_segment;
  ringebuffer_segment_t curr = head;
  while(curr != NULL) {
    count += 1;
    curr = curr->previous_segment;
  }
  return count;
}

static void ringbuffer_segment_push(ringbuffer_t buffer, ringebuffer_segment_t segment) {
  ringebuffer_segment_t head = buffer->current_segment;
  ringebuffer_segment_t prev = NULL;
  ringebuffer_segment_t curr = head;
  while(curr != NULL && segment->timestamp < curr->timestamp) {
    prev = curr;
    curr = curr->previous_segment;
  }
  if (prev) {
    segment->previous_segment = prev->previous_segment;
    prev->previous_segment = segment;
  } else {
    segment->previous_segment = head;
    buffer->current_segment = segment;
  }
}

static ringebuffer_segment_t ringbuffer_segment_pop(ringbuffer_t buffer) {
  ringebuffer_segment_t head = buffer->current_segment;
  ringebuffer_segment_t prev = NULL;
  ringebuffer_segment_t curr = head;
  while(curr != NULL && curr->previous_segment != NULL) {
    prev = curr;
    curr = curr->previous_segment;
  }
  if (prev) {
    prev->previous_segment = NULL;
  } else {
    buffer->current_segment = NULL;
  }
  return curr;
}

u_int32_t ringbuffer_size(ringbuffer_t buffer) {
  u_int32_t size = 0;
  ringebuffer_segment_t curr = buffer->current_segment;
  while(curr != NULL) {
    size += curr->size;
    curr = curr->previous_segment;
  }
  return size;
}

int ringbuffer_open(const char *base_path, ringbuffer_t buffer) {

  buffer->current_segment = NULL;

  u_int32_t timestamp;
  struct dirent *entry;
  struct stat st;
  DIR* dir = opendir(base_path);
  if (dir != NULL) {
    char* full_path;
    while ((entry = readdir(dir)) != NULL) {
      timestamp = atol(entry->d_name);
      if (timestamp > 0) {
        full_path = join(base_path, "/", entry->d_name);
        if (full_path) {
          if (stat(full_path, &st) == 0) {
            if (S_ISREG(st.st_mode)) {
              ringebuffer_segment_t segment = malloc(sizeof(ringebuffer_segment));
              segment->timestamp = timestamp;
              segment->size = (long)st.st_size;
              ringbuffer_segment_push(buffer, segment);
            }
          }
          free(full_path);
        }
      }
    }
    closedir(dir);
    return 0;
  }

  return -1;
}

int ringbuffer_append(ringbuffer_t buffer, message_t message) {
  message->crc32 = crc32_buffer((const char *)message->body, message->body_size);

  ringebuffer_segment_t curr = buffer->current_segment;

  if (curr == NULL || (curr->size + message->body_size) > buffer->max_segment_size) {
    // segment too large, new one
    time_t timestamp = time(NULL);
    ringebuffer_segment_t segment = malloc(sizeof(ringebuffer_segment));
    segment->timestamp = MAX(timestamp, (curr)?curr->timestamp:0);
    segment->size = 0;
    ringbuffer_segment_push(buffer, segment);
  }
  
  if (ringbuffer_segment_count(buffer) > buffer->max_segment_count) {
    // drop the oldest segment
    ringebuffer_segment_t oldest = ringbuffer_segment_pop(buffer);
    if (oldest != NULL){
       free(oldest);
    }
  }
  
  // write
  buffer->current_segment->size += message->body_size;

  return 0;
}

int ringbuffer_close(ringbuffer_t buffer) {
  if (buffer->current_segment) {
    free(buffer->current_segment);
  }
  return 0;
}
