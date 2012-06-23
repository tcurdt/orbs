#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#import "ringbuffer.h"

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

int ringbuffer_open(const char *base_path, ringbuffer_t buffer) {

  buffer->current_segment = NULL;

  u_int32_t timestamp;
  struct dirent *entry;
  struct stat st;
  DIR* dir = opendir(base_path);
  char* full_path;
  while ((entry = readdir(dir)) != NULL) {
    timestamp = atol(entry->d_name);
    if (timestamp > 0) {
      full_path = join(base_path, "/", entry->d_name);
      if (full_path) {
        if (stat(full_path, &st) == 0) {
          if (S_ISREG(st.st_mode)) {

            segment_file_t head = buffer->current_segment;

            segment_file_t file = malloc(sizeof(segment_file));
            file->timestamp = timestamp;
            file->size = (long)st.st_size;

            segment_file_t prev = NULL;
            segment_file_t curr = head;
            while(curr != NULL && timestamp < curr->timestamp) {
              prev = curr;
              curr = curr->previous_segment;
            }

            if (prev) {
              file->previous_segment = prev->previous_segment;
              prev->previous_segment = file;
            } else {
              buffer->current_segment = file;
              file->previous_segment = head;
            }
          }
        }
        free(full_path);
      }
    }
  }
  closedir(dir);

  return 0;
}

u_int32_t ringbuffer_size(ringbuffer_t buffer) {
  u_int32_t size = 0;
  segment_file_t curr = buffer->current_segment;
  while(curr != NULL) {
    size += curr->size;
    curr = curr->previous_segment;
  }
  return size;
}

int ringbuffer_append(ringbuffer_t buffer, message_t message) {
  // struct tm tm;
  // time_t epoch;
  return 0;
}

int ringbuffer_close(ringbuffer_t buffer) {
  if (buffer->current_segment) free(buffer->current_segment);
  return 0;
}
