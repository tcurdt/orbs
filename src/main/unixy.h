#ifndef _UNIXY_H
#define _UNIXY_H

const char* tmp_create();
void tmp_remove(const char* path);
int file_exists(const char* path);
int file_size(const char* path);
const char* basename(const char *path);
void file_write(const char* path, const char* name, const char* data);

#endif