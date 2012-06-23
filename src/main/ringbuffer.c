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


static char* join(const char* a, const char* b, const char* c) {
  size_t al = strlen(a), bl = strlen(b), cl = strlen(c);
  size_t len = al + bl + cl + 1;

  // potential overflow error but we don't care

  char *ret = malloc(len);
  if(ret == NULL) {
    return NULL;
  }

  strcpy(ret, a);
  strcpy(ret + al, b);
  strcpy(ret + al + bl, c);
  return ret;
}

// static char* join(const char* a, const char* b, const char* c) {
//   bstring bs = bfromcstr(a);
//   bconcat(bs, bfromcstr(b));
//   bconcat(bs, bfromcstr(c));
//   char* r = bstr2cstr(bs, '\0');
//   bdestroy(bs);
//   return r;
//   // return "/Users/tcurdt/Desktop/kafka-native/cafka/src/fixtures/1340423026-00000001";
// }

int ringbuffer_open(const char *base_path, ringbuffer_t buffer) {

  // struct tm tm;
  // time_t epoch;

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

            segment_file_t file = malloc(sizeof(segment_file));
            file->timestamp = timestamp;
            file->size = (long)st.st_size;
            file->previous_segment = NULL;

            // segment_file_t cf = buffer->current_segment;
            // while(cf != NULL && timestamp < cf->timestamp) {
            //   cf = cf->previous_segment;
            // }
            // 
            // if (cf) {
            // } else {
              buffer->current_segment = file;
            // }

            // insert into ringbuffer
          }
        }
        free(full_path);
      }
    }
  }
  closedir(dir);

  segment_file_t cf = buffer->current_segment;
  while(cf != NULL) {
    printf("segment %d\n", cf->timestamp);
    cf = cf->previous_segment;
  }


  return 0;
}

int ringbuffer_append(ringbuffer_t buffer, message_t message) {
  return 0;
}

int ringbuffer_close(ringbuffer_t buffer) {
  if (buffer->current_segment) free(buffer->current_segment);
  return 0;
}
