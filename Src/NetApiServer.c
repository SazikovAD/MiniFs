//
// Created by user on 26.03.2020.
//

#include <MiniFS/FsApi.h>
#include <MiniFS/NetApi.h>

#include <string.h>

#include <sys/socket.h>

static int server_client_fd = 0;

void server_net_api_init(int fd) {
  server_client_fd = fd;
}

void server_net_api_cat(const char* path) {
  char* content = NULL;
  size_t content_length = 0;

  int status = fs_api_read_file(path, &content, &content_length);

  send(server_client_fd, &status, sizeof(int), MSG_WAITALL);
  if (content_length != 0) {
    send(server_client_fd, &content_length, sizeof(size_t), MSG_WAITALL);
    send(server_client_fd, content, content_length, MSG_WAITALL);
  }
}

void server_net_api_dir(const char* path) {
  char* content = NULL;
  size_t content_length = 0;

  int status = fs_api_read_directory(path, &content, &content_length);

  send(server_client_fd, &status, sizeof(int), MSG_WAITALL);
  if (content_length != 0) {
    send(server_client_fd, &content_length, sizeof(size_t), MSG_WAITALL);
    send(server_client_fd, content, content_length, MSG_WAITALL);
  }
}

void server_net_api_del(const char* path) {
  int status = fs_api_remove_file(path);
  send(server_client_fd, &status, sizeof(int), MSG_WAITALL);
}

void server_net_api_rmdir(const char* path) {
  int status = fs_api_remove_directory(path);
  send(server_client_fd, &status, sizeof(int), MSG_WAITALL);
}

void server_net_api_mkdir(const char* path) {
  int status = fs_api_create_directory(path);
  send(server_client_fd, &status, sizeof(int), MSG_WAITALL);
}

void server_net_api_copy_from_local(size_t content_length, const char* content,
                                    const char* remote_path) {
  int status = fs_api_create_file(remote_path, content, content_length);
  send(server_client_fd, &status, sizeof(int), MSG_WAITALL);
}

void server_net_api_copy_to_local(const char* remote_path) {
  server_net_api_cat(remote_path);
}