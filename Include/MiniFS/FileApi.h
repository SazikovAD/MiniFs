//
// Created by user on 13.03.2020.
//

#ifndef MINIFS_INCLUDE_MINIFS_FILEAPI_H_
#define MINIFS_INCLUDE_MINIFS_FILEAPI_H_

#include <stdint.h>

typedef struct {
  uint64_t file_size;
  char* data;
} raw_file;

void file_api_remove(uint32_t inode_id);
void file_api_read(uint32_t inode_id, raw_file* file);
void file_api_write(uint32_t* inode_id, const raw_file* file);
void file_api_append(uint32_t inode_id, const raw_file* file, int* status);
void file_api_rewrite(uint32_t inode_id, const raw_file* file, int* status);

#endif  // MINIFS_INCLUDE_MINIFS_FILEAPI_H_
