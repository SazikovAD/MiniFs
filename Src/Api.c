//
// Created by user on 19.03.2020.
//

#include <MiniFS/Api.h>
#include <MiniFS/FsApi.h>
#include <MiniFS/FsFileApi.h>

#include <stdio.h>
#include <string.h>

#define MAX_NAME_SIZE 32

void api_init() {
  fs_file_api_init();
  fs_api_init();
}

Status api_cat(const char* path) {
  char* content = NULL;
  size_t content_length = 0;

  int status = fs_api_read_file(path, &content, &content_length);

  switch (status) {
    case -1:
      return NOT_EXIST;
    case -2:
      return IS_DIRECTORY;
    default:
      break;
  }

  printf("%.*s", content_length, content);

  free(content);
  return OK;
}

Status api_dir(const char* path) {
  char* content = NULL;
  size_t content_length = 0;

  int status = fs_api_read_directory(path, &content, &content_length);

  switch (status) {
    case -1:
      return NOT_EXIST;
    case -2:
      return IS_FILE;
    default:
      break;
  }

  FILE* inp = fmemopen(content, content_length, "r");

  while (!feof(inp)) {
    uint32_t id = 0;
    char t = 'f';
    char name[MAX_NAME_SIZE];
    memset(name, 0, MAX_NAME_SIZE);

    fscanf(inp, "%d %c %s\n", &id, &t, &name);
    printf("< %c >\t%s\n", t, name);
  }

  fclose(inp);
  free(content);
  return OK;
}

Status api_del(const char* path) {
  switch (fs_api_remove_file(path)) {
    case -1:
      return NOT_EXIST;
    case -2:
      return IS_DIRECTORY;
    default:
      return OK;
  }
}

Status api_rmdir(const char* path) {
  switch (fs_api_remove_directory(path)) {
    case -1:
      return NOT_EXIST;
    case -2:
      return IS_FILE;
    case -3:
      return NOT_EMPTY;
    default:
      return OK;
  }
}

Status api_mkdir(const char* path) {
  switch (fs_api_create_directory(path)) {
    case -1:
      return INVALID_PATH;
    case -2:
      return NOT_ENOUGH_SPACE;
    default:
      return OK;
  }
}

Status api_copy_from_local(const char* local_path, const char* remote_path) {
  FILE* input = fopen(local_path, "rb");
  fseek(input, 0, SEEK_END);
  size_t file_size = (size_t)ftell(input);
  fseek(input, 0, SEEK_SET);

  char* buffer = malloc(file_size);
  fread(buffer, 1, file_size, input);
  fclose(input);

  int status = fs_api_create_file(remote_path, buffer, file_size);
  free(buffer);

  switch (status) {
    case -1:
      return INVALID_PATH;
    case -2:
      return NOT_EXIST;
    case -3:
      return NOT_ENOUGH_SPACE;
    default:
      return OK;
  }
}

Status api_copy_to_local(const char* local_path, const char* remote_path) {
  char* content = NULL;
  size_t content_length = 0;

  int status = fs_api_read_file(remote_path, &content, &content_length);

  switch (status) {
    case -1:
      return NOT_EXIST;
    case -2:
      return IS_DIRECTORY;
    default:
      break;
  }

  FILE* output = fopen(local_path, "rb");
  fwrite(content, 1, content_length, output);
  fclose(output);

  free(content);
  return OK;
}