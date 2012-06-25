#include "common.h"
#include "ringbuffer.h"
#include "segments.h"
#include "crc.h"

void ringbuffer_print(ringbuffer* buffer) {
  printf("ringbuffer: %d = { ", ringbuffer_size(buffer));
  segments* segments = &buffer->segments;
  if (segments->head) {
    segment* curr = segments->head;
    while(curr != NULL) {
      printf("%d/%d ", curr->size, curr->count);
      curr = curr->next;
    }
  }
  printf("}\n");
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
  u_int32_t size = 0;

  segments* segments = &buffer->segments;
  segment* curr = segments->head;
  while(curr != NULL) {
    size += curr->size;
    curr = curr->next;
  }

  return size;
}

int ringbuffer_append(ringbuffer* buffer, message* message) {

  segments* segments = &buffer->segments;
  segment* head = segments->head;
  check(head, "should have a head");

  // check if message fits into current segment file
  if ((head->size + message_size(message)) >= buffer->max_segment_size) {
    // current segment full

    // close the current file
    check(buffer->file, "should have a file");
    fclose(buffer->file), buffer->file = NULL;

    // check if we have reached the maximum ringbuffer size
    if ((segments_count(segments) + 1) > buffer->max_segment_count) {

      // remove tail
      segment* tail = segments_pop(segments);
      check(tail, "should have a tail");

      // remove file
      char* path = segments_segment_path(segments, tail);
      check(unlink(path) == OK, "failed to delete %s", path);
      free(path);

      free(tail);
    }

    // create new segment
    check(segments_add(segments, time(NULL)) == OK, "segments_add failed");

    // open the file
    char* path = segments_segment_path(segments, segments->head);
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
  segments->head->size += total;
  segments->head->count += 1;

  return OK;
}

int ringbuffer_read(ringbuffer* buffer, position* position, message* message) {
  UNUSED(buffer);
  UNUSED(position);
  UNUSED(message);
  return OK;
}