//
// Created by user on 26.03.2020.
//

#include <MiniFS/NetApi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>

static inline void process_cat(int fd) {
  size_t path_size;
  recv(fd, &path_size, sizeof(size_t), MSG_WAITALL);
  char* path = malloc(path_size);
  recv(fd, path, path_size, MSG_WAITALL);

  server_net_api_cat(path);
  free(path);
}

static inline void process_dir(int fd) {
  size_t path_size;
  recv(fd, &path_size, sizeof(size_t), MSG_WAITALL);
  char* path = malloc(path_size);
  recv(fd, path, path_size, MSG_WAITALL);

  server_net_api_dir(path);
  free(path);
}

static inline void process_del(int fd) {
  size_t path_size;
  recv(fd, &path_size, sizeof(size_t), MSG_WAITALL);
  char* path = malloc(path_size);
  recv(fd, path, path_size, MSG_WAITALL);

  server_net_api_del(path);
  free(path);
}

static inline void process_rmdir(int fd) {
  size_t path_size;
  recv(fd, &path_size, sizeof(size_t), MSG_WAITALL);
  char* path = malloc(path_size);
  recv(fd, path, path_size, MSG_WAITALL);

  server_net_api_rmdir(path);
  free(path);
}

static inline void process_mkdir(int fd) {
  size_t path_size;
  recv(fd, &path_size, sizeof(size_t), MSG_WAITALL);
  char* path = malloc(path_size);
  recv(fd, path, path_size, MSG_WAITALL);

  server_net_api_mkdir(path);
  free(path);
}

static inline void process_copy_to_local(int fd) {
  size_t path_size;
  recv(fd, &path_size, sizeof(size_t), MSG_WAITALL);
  char* path = malloc(path_size);
  recv(fd, path, path_size, MSG_WAITALL);

  server_net_api_copy_to_local(path);
  free(path);
}

static inline void process_copy_from_local(int fd) {
  size_t path_size;
  recv(fd, &path_size, sizeof(size_t), MSG_WAITALL);
  char* path = malloc(path_size);
  recv(fd, path, path_size, MSG_WAITALL);

  size_t content_length;
  recv(fd, &content_length, sizeof(size_t), MSG_WAITALL);
  char* content = malloc(content_length);
  recv(fd, content, content_length, MSG_WAITALL);

  server_net_api_copy_from_local(content_length, content, path);

  free(path);
  free(content);
}

int main() {
  daemon(0, 0);
  api_init();
  int sock, listener;
  struct sockaddr_in addr;

  listener = socket(AF_INET, SOCK_STREAM, 0);
  if (listener < 0) {
    perror("socket");
    exit(1);
  }

  addr.sin_family = AF_INET;
  addr.sin_port = htons(3245);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(listener, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    perror("bind");
    exit(2);
  }

  listen(listener, 1);

  for (;;) {
    sock = accept(listener, NULL, NULL);
    server_net_api_init(sock);

    NetOperations operation = NONE;

    recv(sock, &operation, sizeof(NetOperations), MSG_WAITALL);

    switch (operation) {
      case CAT:
        process_cat(sock);
        break;
      case DIR:
        process_dir(sock);
        break;
      case DEL:
        process_del(sock);
        break;
      case RMDIR:
        process_rmdir(sock);
        break;
      case MKDIR:
        process_mkdir(sock);
        break;
      case COPY_FROM_LOCAL:
        process_copy_from_local(sock);
        break;
      case COPY_TO_LOCAL:
        process_copy_to_local(sock);
        break;
      default:
        break;
    }

    close(sock);
    sleep(1);
  }

  return 0;
}
