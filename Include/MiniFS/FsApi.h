//
// Created by user on 13.03.2020.
//

#ifndef MINIFS_INCLUDE_MINIFS_FSAPI_H_
#define MINIFS_INCLUDE_MINIFS_FSAPI_H_

#include <stdlib.h>

void fs_api_init();

int fs_api_exists(const char* path);
int fs_api_is_file(const char* path);
int fs_api_is_directory(const char* path);

int fs_api_create_directory(const char* path);
int fs_api_remove_directory(const char* path);

int fs_api_create_file(const char* file, const char* content,
                       size_t content_length);
int fs_api_remove_file(const char* file);
int fs_api_read_file(const char* file, char** content, size_t* content_length);

int fs_api_read_directory(const char* directory, char** content,
                          size_t* content_length);

#endif  // MINIFS_INCLUDE_MINIFS_FSAPI_H_
