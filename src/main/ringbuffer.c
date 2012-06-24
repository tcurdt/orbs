#include "common.h"
#include "ringbuffer.h"
#include "segments.h"
#include "crc.h"

#include <dirent.h>
#include <time.h>

static char* filename(const char* base_path, u_int32_t timestamp) {
  size_t len = strlen(base_path) + 1 + 20 + 1;
  char *ret = malloc(len);
  if(!ret) return NULL;
  snprintf(ret, len - 1, "%s/%d", base_path, timestamp);
  return ret;
}

static u_int32_t message_size(message* message) {
  return sizeof(message) - sizeof(message->body) + message->body_size;
}


int ringbuffer_open(const char *original_base_path, ringbuffer* buffer) {

  // copy base path
  const char* base_path = strdup(original_base_path);
  if (!base_path) reterr("failed to duplicate %s", original_base_path);

  // init segments
  segments* segments = &buffer->segments;
  if (segments_init(segments) != OK) {
    reterr("failed to init segments");
  }
  segments->base_path = base_path;

  // scan through dir
  DIR* dir = opendir(base_path);
  if (!dir) reterr("failed to open dir %s", base_path);

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    u_int32_t timestamp = atol(entry->d_name);
    if (timestamp > 0) {

      // found a segment file
      char* full_path = filename(base_path, timestamp);
      if (!full_path) reterr("failed to create filename");

      if (segments_add(segments, full_path) != OK) {
        reterr("failed to add segment");
      }
      
      free(full_path);
    }
  }

  closedir(dir);

  // if there is none - create one
  if (segments_count(segments) == 0) {

    char* full_path = filename(base_path, time(NULL));
    if (!full_path) reterr("failed to create filename");

    if (segments_add(segments, full_path) != OK) {
      reterr("failed to add segment");
    }

    free(full_path);
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
  if ((segments_head_size(segments) + message_size(message)) > buffer->max_segment_size) {

    // current segment full
    if ((segments_count(segments) + 1) > buffer->max_segment_count) {
      // remove old segment first
      if (segments_pop(segments) != OK) {
        reterr("failed to remove segment");
      }
    }

    char* full_path = filename(segments->base_path, time(NULL));
    if (!full_path) reterr("failed to create filename");

    if (segments_add(segments, full_path) != OK) {
      reterr("failed to add segment");
    }

    free(full_path);
  }

  if (segments_head_write(segments, message) != OK) {
    reterr("failed to write to segment");
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