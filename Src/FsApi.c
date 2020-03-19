//
// Created by user on 13.03.2020.
//

#include <MiniFS/FileApi.h>
#include <MiniFS/FsApi.h>
#include <MiniFS/FsFileApi.h>

#include <stdio.h>
#include <string.h>

#define ROOT_INODE_ID 1
#define MAX_NAME_SIZE 32
#define MAX_DIR_LINE 66

char* bufferize_string(const char* str) {
  size_t s = strlen(str) + 1;
  char* buffer = malloc(s);
  memcpy(buffer, str, s);
  return buffer;
}

void get_file_inode(uint32_t dir_inode, const char* file, uint32_t* inode_id,
                    char* file_type) {
  raw_file dir;
  file_api_read(dir_inode, &dir);

  FILE* stream = fmemopen(dir.data, dir.file_size, "r");

  while (!feof(stream)) {
    uint32_t id = 0;
    char t = 0;
    char name[MAX_NAME_SIZE];
    fscanf(stream, "%d %c %s\n", &id, &t, &name);

    if (strcmp(name, file) == 0) {
      *inode_id = id;
      *file_type = t;
      fclose(stream);
      free(dir.data);
      return;
    }
  }

  fclose(stream);
  free(dir.data);

  *inode_id = 0;
  *file_type = 0;
}

void initialize_directory(uint32_t dir_inode_id, uint32_t parent_dir_inode_id) {
  char buffer[MAX_DIR_LINE];
  memset(buffer, 0, MAX_DIR_LINE);

  raw_file file;
  file.data = buffer;

  int status = 0;

  sprintf(buffer, "%d %c %s\n", dir_inode_id, 'd', ".");
  file.file_size = strlen(buffer);
  file_api_append(dir_inode_id, &file, &status);

  sprintf(buffer, "%d %c %s\n", parent_dir_inode_id, 'd', "..");
  file.file_size = strlen(buffer);
  file_api_append(dir_inode_id, &file, &status);
}

void fs_api_init() {
  inode i_node;
  fs_file_api_read_inode(ROOT_INODE_ID, &i_node);

  if (i_node.file_block == 0) {
    uint32_t block_id = 1;
    fs_file_api_acquire_blocks(1, &block_id);

    i_node.file_size = 0;
    i_node.file_block = block_id;

    fs_file_api_write_inode(ROOT_INODE_ID, &i_node);

    initialize_directory(ROOT_INODE_ID, ROOT_INODE_ID);
  }
}

int fs_api_exists(const char* path) {
  char* buffer = bufferize_string(path);
  char* token = strtok(buffer, "/");
  uint32_t current_id = ROOT_INODE_ID;
  char current_type = 'd';

  while (token != NULL) {
    if (current_type == 'f') {
      free(buffer);
      return 0;
    }

    uint32_t next_id = 0;
    char next_type = 'f';
    get_file_inode(current_id, token, &next_id, &next_type);

    if (next_id == 0) {
      free(buffer);
      return 0;
    }

    current_id = next_id;
    current_type = next_type;
    token = strtok(NULL, "/");
  }

  free(buffer);
  return 1;
}

int fs_api_is_file(const char* file) {
  if (!fs_api_exists(file)) {
    return -1;
  }

  char* buffer = bufferize_string(file);
  char* token = strtok(buffer, "/");
  uint32_t current_id = ROOT_INODE_ID;
  char current_type = 'd';

  while (token != NULL) {
    uint32_t next_id = 0;
    char next_type = 'f';
    get_file_inode(current_id, token, &next_id, &next_type);

    current_id = next_id;
    current_type = next_type;
    token = strtok(NULL, "/");
  }

  free(buffer);
  return (current_type == 'f' ? 1 : 0);
}

int fs_api_is_directory(const char* file) {
  if (!fs_api_exists(file)) {
    return -1;
  }

  char* buffer = bufferize_string(file);
  char* token = strtok(buffer, "/");
  uint32_t current_id = ROOT_INODE_ID;
  char current_type = 'd';

  while (token != NULL) {
    uint32_t next_id = 0;
    char next_type = 'f';
    get_file_inode(current_id, token, &next_id, &next_type);

    current_id = next_id;
    current_type = next_type;
    token = strtok(NULL, "/");
  }

  free(buffer);
  return (current_type == 'd' ? 1 : 0);
}

int fs_api_create_directory(const char* path) {
  char* buffer = bufferize_string(path);
  char* token = strtok(buffer, "/");
  uint32_t current_id = ROOT_INODE_ID;
  char current_type = 'd';

  while (token != NULL) {
    if (current_type == 'f') {
      free(buffer);
      return -1;
    }

    uint32_t next_id = 0;
    char next_type = 'f';
    get_file_inode(current_id, token, &next_id, &next_type);

    if (next_id == 0) {
      break;
    }

    current_id = next_id;
    current_type = next_type;
    token = strtok(NULL, "/");
  }

  while (token != NULL) {
    inode i_node;
    uint32_t inode_id = 0;
    uint32_t acquired = 0;
    uint32_t block_id = 0;

    fs_file_api_acquire_inode(&inode_id);
    fs_file_api_try_acquire_blocks(1, &acquired, &block_id);

    if (inode_id == 0 || acquired == 0) {
      free(buffer);
      return -2;
    }

    fs_file_api_acquire_blocks(1, &block_id);

    i_node.file_size = 0;
    i_node.file_block = block_id;
    fs_file_api_write_inode(inode_id, &i_node);

    initialize_directory(inode_id, current_id);

    raw_file file;
    int status = 0;
    char tmp[MAX_DIR_LINE];
    memset(tmp, 0, MAX_DIR_LINE);
    file.data = tmp;

    sprintf(tmp, "%d %c %s\n", inode_id, 'd', token);
    file.file_size = strlen(tmp);

    file_api_append(current_id, &file, &status);

    current_id = inode_id;
    current_type = 'd';
    token = strtok(NULL, "/");
  }

  free(buffer);
  return 1;
}

int fs_api_remove_directory(const char* path) {
  if (!fs_api_exists(path)) {
    return -1;
  }

  char* buffer = bufferize_string(path);
  char* token = strtok(buffer, "/");

  uint32_t current_id = ROOT_INODE_ID;
  uint32_t prev_id = ROOT_INODE_ID;

  char cur_type = 'd';

  while (token != NULL) {
    uint32_t next_id = 0;
    char next_type = 'd';

    get_file_inode(current_id, token, &next_id, &next_type);

    prev_id = current_id;
    current_id = next_id;
    cur_type = next_type;
    token = strtok(NULL, "/");
  }

  if (cur_type == 'f') {
    free(buffer);
    return -2;
  }

  raw_file dir;
  file_api_read(current_id, &dir);
  char is_empty_dir = 1;
  FILE* in = fmemopen(dir.data, dir.file_size, "r");
  while (!feof(in)) {
    uint32_t id = 0;
    char t = 0;
    char name[MAX_NAME_SIZE];
    memset(name, 0, MAX_NAME_SIZE);

    fscanf(in, "%d %c %s\n", &id, &t, &name);

    if ((strcmp(name, ".") != 0) && (strcmp(name, "..") != 0)) {
      is_empty_dir = 0;
      break;
    }
  }
  fclose(in);

  free(dir.data);

  if (!is_empty_dir) {
    free(buffer);
    return -3;
  }

  file_api_remove(current_id);

  raw_file old, new;

  file_api_read(prev_id, &old);

  new.file_size = old.file_size;
  new.data = malloc(old.file_size);
  memset(new.data, 0, old.file_size);

  in = fmemopen(old.data, old.file_size, "r");
  FILE* out = fmemopen(new.data, new.file_size, "w");

  while (!feof(in)) {
    uint32_t id = 0;
    char t = 0;
    char name[MAX_NAME_SIZE];
    memset(name, 0, MAX_NAME_SIZE);

    fscanf(in, "%d %c %s\n", &id, &t, &name);
    if (id != current_id) {
      fprintf(out, "%d %c %s\n", id, t, name);
    }
  }

  fclose(in);
  fclose(out);

  int status = 0;
  new.file_size = strlen(new.data);
  file_api_rewrite(prev_id, &new, &status);

  free(old.data);
  free(new.data);
  free(buffer);
  return 1;
}

int fs_api_create_file(const char* path, const char* content,
                       size_t content_length) {
  int status = fs_api_is_file(path);

  if (status != -1) {
    return -1;
  }

  char* buffer = bufferize_string(path);
  char* token = strtok(buffer, "/");
  uint32_t current_id = ROOT_INODE_ID;

  while (token != NULL) {
    uint32_t next_id = 0;
    char next_type = 'f';

    get_file_inode(current_id, token, &next_id, &next_type);

    if (next_type == 'f') {
      free(buffer);
      return -2;
    }

    if (next_id == 0) {
      break;
    }

    current_id = next_id;
    token = strtok(NULL, "/");
  }

  if (strtok(NULL, "/") != NULL) {
    free(buffer);
    return -2;
  }

  raw_file file;
  uint32_t inode_id = 0;
  file.file_size = content_length;
  file.data = malloc(content_length);
  memcpy(file.data, content, content_length);

  file_api_write(&inode_id, &file);

  if (inode_id == 0) {
    free(file.data);
    free(buffer);
    return -3;
  }

  raw_file dir;
  char tmp[MAX_DIR_LINE];
  memset(tmp, 0, MAX_DIR_LINE);
  dir.data = tmp;

  sprintf(tmp, "%d %c %s\n", inode_id, 'f', token);
  dir.file_size = strlen(tmp);

  file_api_append(current_id, &dir, &status);

  return 1;
}

int fs_api_remove_file(const char* path) {
  if (!fs_api_exists(path)) {
    return -1;
  }

  char* buffer = bufferize_string(path);
  char* token = strtok(buffer, "/");

  uint32_t current_id = ROOT_INODE_ID;
  uint32_t prev_id = ROOT_INODE_ID;

  char cur_type = 'd';

  while (token != NULL) {
    uint32_t next_id = 0;
    char next_type = 'd';

    get_file_inode(current_id, token, &next_id, &next_type);

    prev_id = current_id;
    current_id = next_id;
    cur_type = next_type;
    token = strtok(NULL, "/");
  }

  if (cur_type == 'd') {
    free(buffer);
    return -2;
  }

  file_api_remove(current_id);

  raw_file old, new;

  file_api_read(prev_id, &old);

  new.file_size = old.file_size;
  new.data = malloc(old.file_size);
  memset(new.data, 0, old.file_size);

  FILE* in = fmemopen(old.data, old.file_size, "r");
  FILE* out = fmemopen(new.data, new.file_size, "w");

  while (!feof(in)) {
    uint32_t id = 0;
    char t = 0;
    char name[MAX_NAME_SIZE];
    memset(name, 0, MAX_NAME_SIZE);

    fscanf(in, "%d %c %s\n", &id, &t, &name);
    if (id != current_id) {
      fprintf(out, "%d %c %s\n", id, t, name);
    }
  }

  fclose(in);
  fclose(out);

  int status = 0;
  new.file_size = strlen(new.data);
  file_api_rewrite(prev_id, &new, &status);

  free(old.data);
  free(new.data);
  free(buffer);
  return 1;
}

int fs_api_read_file(const char* path, char** content, size_t* content_length) {
  int status = fs_api_is_file(path);

  if (status == -1) {
    *content = NULL;
    *content_length = 0;
    return -1;
  }

  if (status == 0) {
    *content = NULL;
    *content_length = 0;
    return -2;
  }

  char* buffer = bufferize_string(path);
  char* token = strtok(buffer, "/");
  uint32_t current_id = ROOT_INODE_ID;

  while (token != NULL) {
    uint32_t next_id = 0;
    char next_type = 'f';
    get_file_inode(current_id, token, &next_id, &next_type);

    current_id = next_id;
    token = strtok(NULL, "/");
  }

  free(buffer);

  raw_file file;
  file_api_read(current_id, &file);

  *content = file.data;
  *content_length = file.file_size;

  return 1;
}

int fs_api_read_directory(const char* directory, char** content,
                          size_t* content_length) {
  int status = fs_api_is_directory(directory);

  if (status == -1) {
    *content = NULL;
    *content_length = 0;
    return -1;
  }

  if (status == 0) {
    *content = NULL;
    *content_length = 0;
    return -2;
  }

  char* buffer = bufferize_string(directory);
  char* token = strtok(buffer, "/");
  uint32_t current_id = ROOT_INODE_ID;

  while (token != NULL) {
    uint32_t next_id = 0;
    char next_type = 'f';
    get_file_inode(current_id, token, &next_id, &next_type);

    current_id = next_id;
    token = strtok(NULL, "/");
  }

  free(buffer);

  raw_file file;
  file_api_read(current_id, &file);

  *content = file.data;
  *content_length = file.file_size;

  return 1;
}