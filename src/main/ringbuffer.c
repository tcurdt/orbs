#include "common.h"
#include "ringbuffer.h"
#include "segments.h"
#include "crc.h"

void ringbuffer_print(ringbuffer* buffer) {
  return segments_print(&buffer->segments);
}

int ringbuffer_open(const char *original_base_path, ringbuffer* buffer) {

  // copy base path
  const char* base_path = strdup(original_base_path);
  check(base_path, OOM);

  // init segments
  segments* segments = &buffer->segments;
  check(segments_init(segments) == OK, "segments_init failed");
  segments->base_path = base_path;

  // scan through dir
  DIR* dir = opendir(base_path);
  check(dir, "failed to open dir %s", base_path);
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    u_int32_t timestamp = atol(entry->d_name);
    if (timestamp > 0) {
      check(segments_add(segments, timestamp) == OK, "segments_add failed");
    }
  }
  closedir(dir);

  // always create a new segment after restart
  check(segments_add(segments, time(NULL)) == OK, "segments_add failed");

  // open the file
  char* path = segments_segment_path(segments, segments->head);
  buffer->file = fopen(path, "a");
  check(buffer->file, "failed to open segment file %s", path);
  free(path);

  return OK;
}

int ringbuffer_close(ringbuffer* buffer) {
  check(segments_destroy(&buffer->segments) == OK, "segments_destroy failed");
  return OK;
}

u_int32_t ringbuffer_size(ringbuffer* buffer) {
  return segments_size(&buffer->segments);
}

int ringbuffer_append(ringbuffer* buffer, message* message) {
  segments* segments = &buffer->segments;

  segment* newest = segments->tail;
  check(newest, "should have a newest segment");

  // check if message fits into current segment file
  if ((newest->size + message_size(message)) >= buffer->max_segment_size) {
    // newest segment full

    // close the current file
    check(buffer->file, "should have a file");
    fclose(buffer->file), buffer->file = NULL;

    // check if we have reached the maximum ringbuffer size
    if ((segments_count(segments) + 1) > buffer->max_segment_count) {

      // remove oldest
      segment* oldest = segments_pop(segments);
      check(oldest, "should have an oldest segment");

      // remove file
      char* path = segments_segment_path(segments, oldest);
      check(unlink(path) == OK, "failed to delete %s", path);
      free(path);

      free(oldest);
    }

    // create new segment
    check(segments_add(segments, time(NULL)) == OK, "segments_add failed");

    // open the file
    char* path = segments_segment_path(segments, segments->tail);
    buffer->file = fopen(path, "a");
    check(buffer->file, "failed to open segment file %s", path);
    free(path);
  }

  // calculate checksum
  message->crc32 = crc32_buffer((const char *)message->body, message->body_size);

  // write message to file
  size_t len, total = 0;
  FILE* file = buffer->file;

  len = sizeof(u_int8_t);
  check(fwrite((void*)&message->type, len, 1, file) == 1,
    "failed to write %zd bytes", len);
  total += len;

  len = sizeof(size_t);
  check(fwrite((void*)&message->body_size, len, 1, file) == 1,
    "failed to write %zd bytes", len);
  total += len;

  len = message->body_size;
  check(fwrite((void*)message->body, len, 1, file) == 1,
    "failed to write %zd bytes", len);
  total += len;

  len = sizeof(u_int32_t);
  check(fwrite((void*)&message->crc32, len, 1, file) == 1,
    "failed to write %zd bytes", len);
  total += len;

  fflush(file);
  fsync(fileno(file));
  segments->tail->size += total;
  segments->tail->count += 1;

  return OK;
}

int ringbuffer_read(ringbuffer* buffer, position* position, message* message) {
  UNUSED(buffer);
  UNUSED(position);
  UNUSED(message);
  return OK;
}