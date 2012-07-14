#include "common.h"
#include "ringbuffer.h"
#include "segments.h"

#define FILE_OPEN   O_CREAT | O_EXCL | O_WRONLY | O_APPEND
#define FILE_PERMS  0640

static char* filename(const char *base_path, u_int32_t timestamp) {
  size_t len = strlen(base_path) + 1 + 20 + 1;
  char *ret = malloc(len);
  if(!ret) return NULL;
  snprintf(ret, len - 1, "%s/%d", base_path, timestamp);
  ret[len-1] = 0;
  return ret;
}

static int ringbuffer_add_segment(ringbuffer* buffer, u_int32_t timestamp) {
  check(buffer->base_path, "needs base_path");

  u_int32_t size = 0;

  // check length
  char* path = filename(buffer->base_path, timestamp);
  check(path, OOM);
  struct stat st;
  if (stat(path, &st) == OK) {
    check(S_ISREG(st.st_mode), "not a regular file %s", path);
    size = st.st_size;
  }
  free(path);

  buffer->total_size += size;

  return segments_add(&buffer->segments, timestamp, size);
}

void ringbuffer_print(ringbuffer* buffer) {
  return segments_print(&buffer->segments);
}

int ringbuffer_open(ringbuffer* buffer) {
  check(buffer->max_total_size > 0, "needs max_total_size");
  check(buffer->max_segment_size > 0, "needs max_segment_size");
  check(buffer->max_total_size >= (3 * buffer->max_segment_size), "max_total_size must be 3x larger than max_segment_size (%d >= 3 * %d)", buffer->max_total_size, buffer->max_segment_size);
  check(buffer->base_path, "needs base_path");

  // init
  segments* segments = &buffer->segments;
  check(segments_init(segments) == OK, "segments_init failed");
  buffer->total_size = 0;

  // scan through dir
  DIR* dir = opendir(buffer->base_path);
  check(dir, "failed to open dir %s", buffer->base_path);
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    u_int32_t timestamp = atol(entry->d_name);
    if (timestamp > 0) {
      check(ringbuffer_add_segment(buffer, timestamp) == OK, "ringbuffer_add_segment failed");
    }
  }
  closedir(dir);

  // always create a new empty segment after restart
  u_int32_t timestamp = time(NULL);
  check(ringbuffer_add_segment(buffer, timestamp) == OK, "ringbuffer_add_segment failed");
  check(segments->tail->timestamp >= timestamp, "new segment should be newest");
  check(segments->tail->size == 0, "new segment should be empty");

  // open the file
  char* path = filename(buffer->base_path, segments->tail->timestamp);
  buffer->fd = open(path, (buffer->sync_type == RINGBUFFER_SYNC_ALWAYS) ? FILE_OPEN | O_SYNC : FILE_OPEN, FILE_PERMS);
  check(buffer->fd > 0, "failed to open segment file %s", path);
  free(path);

  return OK;
}

int ringbuffer_close(ringbuffer* buffer) {
  check(segments_destroy(&buffer->segments) == OK, "segments_destroy failed");
  check(fsync(buffer->fd) == OK, "failed to fsync");
  check(close(buffer->fd) == OK, "failed to close");
  buffer->fd = ERROR;
  if (buffer->base_path) free((char*)buffer->base_path);
  return OK;
}

int ringbuffer_append(ringbuffer* buffer, void* message, u_int32_t message_length) {
  segments* segments = &buffer->segments;

  segment* newest = segments->tail;
  check(newest, "should have a newest segment");

  // check if we have reached the maximum ringbuffer size
  if ((buffer->total_size + message_length) >= buffer->max_total_size) {

    // remove oldest
    segment* oldest = segments_pop(segments);
    check(oldest, "should have an oldest segment");

    // remove file
    char* path = filename(buffer->base_path, oldest->timestamp);
    check(unlink(path) == OK, "failed to delete %s", path);
    free(path);

    buffer->total_size -= oldest->size;

    free(oldest);
  }

  // check if message fits into current segment file
  if ((newest->size + message_length) >= buffer->max_segment_size) {
    // newest segment full

    // close the current file
    check(buffer->fd > 0, "no file");
    check(close(buffer->fd) == OK, "failed to close");
    buffer->fd = ERROR;

    // create new segment
    check(segments_add(segments, time(NULL), 0) == OK, "segments_add failed");

    // open the file
    char* path = filename(buffer->base_path, segments->tail->timestamp);
    buffer->fd = open(path, (buffer->sync_type == RINGBUFFER_SYNC_ALWAYS) ? FILE_OPEN | O_SYNC : FILE_OPEN, FILE_PERMS);
    check(buffer->fd > 0, "failed to open segment file %s", path);
    free(path);
  }

  check(write(buffer->fd, message, message_length) == message_length,
    "failed to write %d bytes", message_length);

  if (buffer->sync_type == RINGBUFFER_SYNC_COUNTS) {
    // fsync every n-th message
    if (segments->tail->messages % buffer->sync_freq == 0) {
      FSYNC(buffer->fd);
    }
  }

  segments->tail->size += message_length;
  segments->tail->messages += 1;
  buffer->total_size += message_length;

  return OK;
}

u_int32_t ringbuffer_size(ringbuffer* buffer) {
  return buffer->total_size;
}

