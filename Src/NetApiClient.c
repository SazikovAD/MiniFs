//
// Created by user on 26.03.2020.
//

#include <MiniFS/NetApi.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>

#define MAX_NAME_SIZE 32

static int client_server_fd = 0;

void client_net_api_init(int fd) {
  client_server_fd = fd;
}

Status client_net_api_cat(const char* path) {
  NetOperations op = CAT;
  size_t size = strlen(path) + 1;

  send(client_server_fd, &op, sizeof(NetOperations), MSG_WAITALL);
  send(client_server_fd, &size, sizeof(size_t), MSG_WAITALL);
  send(client_server_fd, path, size, MSG_WAITALL);

  int status = 0;
  recv(client_server_fd, &status, sizeof(int), MSG_WAITALL);

  switch (status) {
    case -1:
      return NOT_EXIST;
    case -2:
      return IS_DIRECTORY;
    default:
      break;
  }

  size_t content_length = 0;
  recv(client_server_fd, &content_length, sizeof(size_t), MSG_WAITALL);
  char* content = malloc(content_length);
  recv(client_server_fd, content, content_length, MSG_WAITALL);

  printf("%.*s", content_length, content);

  free(content);
  return OK;
}

Status client_net_api_dir(const char* path) {
  NetOperations op = DIR;
  size_t size = strlen(path) + 1;

  send(client_server_fd, &op, sizeof(NetOperations), MSG_WAITALL);
  send(client_server_fd, &size, sizeof(size_t), MSG_WAITALL);
  send(client_server_fd, path, size, MSG_WAITALL);

  int status = 0;
  recv(client_server_fd, &status, sizeof(int), MSG_WAITALL);

  switch (status) {
    case -1:
      return NOT_EXIST;
    case -2:
      return IS_FILE;
    default:
      break;
  }

  size_t content_length = 0;
  recv(client_server_fd, &content_length, sizeof(size_t), MSG_WAITALL);
  char* content = malloc(content_length);
  recv(client_server_fd, content, content_length, MSG_WAITALL);

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

Status client_net_api_del(const char* path) {
  NetOperations op = DEL;
  size_t size = strlen(path) + 1;

  send(client_server_fd, &op, sizeof(NetOperations), MSG_WAITALL);
  send(client_server_fd, &size, sizeof(size_t), MSG_WAITALL);
  send(client_server_fd, path, size, MSG_WAITALL);

  int status = 0;
  recv(client_server_fd, &status, sizeof(int), MSG_WAITALL);

  switch (status) {
    case -1:
      return NOT_EXIST;
    case -2:
      return IS_DIRECTORY;
    default:
      return OK;
  }
}

Status client_net_api_rmdir(const char* path) {
  NetOperations op = RMDIR;
  size_t size = strlen(path) + 1;

  send(client_server_fd, &op, sizeof(NetOperations), MSG_WAITALL);
  send(client_server_fd, &size, sizeof(size_t), MSG_WAITALL);
  send(client_server_fd, path, size, MSG_WAITALL);

  int status = 0;
  recv(client_server_fd, &status, sizeof(int), MSG_WAITALL);

  switch (status) {
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

Status client_net_api_mkdir(const char* path) {
  NetOperations op = MKDIR;
  size_t size = strlen(path) + 1;

  send(client_server_fd, &op, sizeof(NetOperations), MSG_WAITALL);
  send(client_server_fd, &size, sizeof(size_t), MSG_WAITALL);
  send(client_server_fd, path, size, MSG_WAITALL);

  int status = 0;
  recv(client_server_fd, &status, sizeof(int), MSG_WAITALL);

  switch (status) {
    case -1:
      return INVALID_PATH;
    case -2:
      return NOT_ENOUGH_SPACE;
    default:
      return OK;
  }
}

Status client_net_api_copy_from_local(const char* local_path,
                                      const char* remote_path) {
  NetOperations op = COPY_FROM_LOCAL;
  size_t size = strlen(remote_path) + 1;

  FILE* input = fopen(local_path, "rb");
  fseek(input, 0, SEEK_END);
  size_t file_size = (size_t)ftell(input);
  fseek(input, 0, SEEK_SET);

  char* buffer = malloc(file_size);
  fread(buffer, 1, file_size, input);
  fclose(input);

  send(client_server_fd, &op, sizeof(NetOperations), MSG_WAITALL);
  send(client_server_fd, &size, sizeof(size_t), MSG_WAITALL);
  send(client_server_fd, remote_path, size, MSG_WAITALL);
  send(client_server_fd, &file_size, sizeof(size_t), MSG_WAITALL);
  send(client_server_fd, buffer, file_size, MSG_WAITALL);

  free(buffer);

  int status = 0;
  recv(client_server_fd, &status, sizeof(int), MSG_WAITALL);

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

Status client_net_api_copy_to_local(const char* local_path,
                                    const char* remote_path) {
  NetOperations op = COPY_TO_LOCAL;
  size_t size = strlen(remote_path) + 1;

  send(client_server_fd, &op, sizeof(NetOperations), MSG_WAITALL);
  send(client_server_fd, &size, sizeof(size_t), MSG_WAITALL);
  send(client_server_fd, remote_path, size, MSG_WAITALL);

  int status = 0;
  recv(client_server_fd, &status, sizeof(int), MSG_WAITALL);

  switch (status) {
    case -1:
      return NOT_EXIST;
    case -2:
      return IS_DIRECTORY;
    default:
      break;
  }

  size_t content_length;
  recv(client_server_fd, &content_length, sizeof(size_t), MSG_WAITALL);
  char* content = malloc(content_length);
  recv(client_server_fd, content, content_length, MSG_WAITALL);

  FILE* output = fopen(local_path, "wb");
  fwrite(content, 1, content_length, output);
  fclose(output);

  free(content);
}
