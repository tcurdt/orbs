#include "common.h"
#include "ringbuffer.h"
#include "segments.h"
#include "crc.h"

#include <sys/stat.h>
#include <dirent.h>
#include <time.h>


// potential overflow bug but we don't care
// strings are coming from the filesystem
static char* join(const char* a, const char* b, const char* c) {
  size_t al = strlen(a), bl = strlen(b), cl = strlen(c);
  size_t len = al + bl + cl + 1;

  char *ret = malloc(len);
  if(!ret) return NULL;

  strcpy(ret, a);
  strcpy(ret + al, b);
  strcpy(ret + al + bl, c);
  return ret;
}

static u_int32_t message_size(message* message) {
  return sizeof(message) - sizeof(message->body) + message->body_size;
}


int ringbuffer_open(const char *base_path, ringbuffer* buffer) {

  segments* segments = &buffer->segments;
  if (segments_init(segments) != OK) {
    reterr("failed to init segments");
  }

  DIR* dir = opendir(base_path);
  if (!dir) reterr("failed to open dir %s", base_path);

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    u_int32_t timestamp = atol(entry->d_name);
    if (timestamp > 0) {
      // found a segment file
      char* full_path = join(base_path, "/", entry->d_name);
      if (!full_path) reterr("failed to join paths");
      struct stat st;
      if (stat(full_path, &st) != OK) {
        reterr("failed to get file information %s", full_path);
      }
      if (S_ISREG(st.st_mode)) {
        if (segments_add(segments, timestamp, (long)st.st_size) != OK) {
          reterr("failed to add segment");
        }
      }
      free(full_path);
    }
  }
  closedir(dir);

  if (segments_count(segments) == 0) {
    if (segments_add(segments, time(NULL), 0) != OK) {
      reterr("failed to add segment");
    }
  }

  return OK;
}

int ringbuffer_close(ringbuffer* buffer) {
  if (segments_destroy(&buffer->segments) != OK) {
    reterr("failed to destroy segments");
  }
  return OK;
}

int ringbuffer_append(ringbuffer* buffer, message* message) {

  message->crc32 = crc32_buffer((const char *)message->body, message->body_size);

  segments* segments = &buffer->segments;
  if ((segments_head_size(segments) + message_size(message)) < buffer->max_segment_size) {
    if (segments_head_write(segments, message) != OK) {
      reterr("failed to write to segment");
    }
  } else {
    // current segment full
    if ((segments_count(segments) + 1) > buffer->max_segment_count) {
      // remove old segment first
      if (segments_pop(segments) != OK) {
        reterr("failed to remove segment");
      }
    }
    if (segments_add(segments, time(NULL), 0) != OK) {
      reterr("failed to add segment");
    }
  }
  return OK;
}

int ringbuffer_read(ringbuffer* buffer, position* position, message* message) {
  return OK;
}


u_int32_t ringbuffer_size(ringbuffer* buffer) {
  u_int32_t size = 0;
  segment* curr = (&buffer->segments)->head;
  while(curr != NULL) {
    size += curr->size;
    curr = curr->previous_segment;
  }
  return size;
}