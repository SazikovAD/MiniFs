//
// Created by user on 26.03.2020.
//

#include <MiniFS/NetApi.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>

static inline int is_valid_path(const char* path) {
  size_t s = strlen(path) + 1;
  char* buffer = malloc(s);
  memcpy(buffer, path, s);
  char is_valid = 1;

  char* token = strtok(buffer, "/");

  while (token != NULL) {
    if (strcmp(token, ".") == 0 || strcmp(token, "..") == 0) {
      is_valid = 0;
      break;
    }
    token = strtok(NULL, "/");
  }

  free(buffer);
  return is_valid;
}

static inline void minifs_cat(const char* path) {
  if (!is_valid_path(path)) {
    printf("MiniFs can only work with canonized paths!\n");
    return;
  }

  switch (client_net_api_cat(path)) {
    case NOT_EXIST:
      printf("file not exists\n");
      break;
    case IS_DIRECTORY:
      printf("trying to read directory\n");
      break;
    default:
      break;
  }
}

static inline void minifs_dir(const char* path) {
  if (!is_valid_path(path)) {
    printf("MiniFs can only work with canonized paths!\n");
    return;
  }

  switch (client_net_api_dir(path)) {
    case NOT_EXIST:
      printf("directory not exists\n");
      break;
    case IS_FILE:
      printf("trying to read file\n");
      break;
    default:
      break;
  }
}

static inline void minifs_del(const char* path) {
  if (!is_valid_path(path)) {
    printf("MiniFs can only work with canonized paths!\n");
    return;
  }

  switch (client_net_api_del(path)) {
    case NOT_EXIST:
      printf("file not exists\n");
      break;
    case IS_DIRECTORY:
      printf("trying to delete directory\n");
      break;
    default:
      break;
  }
}

static inline void minifs_rmdir(const char* path) {
  if (!is_valid_path(path)) {
    printf("MiniFs can only work with canonized paths!\n");
    return;
  }

  switch (client_net_api_rmdir(path)) {
    case NOT_EXIST:
      printf("directory not exists\n");
      break;
    case IS_FILE:
      printf("trying to delete file\n");
      break;
    case NOT_EMPTY:
      printf("trying to delete not empty directory\n");
      break;
    default:
      break;
  }
}

static inline void minifs_mkdir(const char* path) {
  if (!is_valid_path(path)) {
    printf("MiniFs can only work with canonized paths!\n");
    return;
  }

  switch (client_net_api_mkdir(path)) {
    case INVALID_PATH:
      printf("inputed path is invalid\n");
      break;
    case NOT_ENOUGH_SPACE:
      printf("cannot make directory due to lack of space\n");
      break;
    default:
      break;
  }
}

static inline void minifs_copy_from_local(const char* local,
                                          const char* remote) {
  if (!is_valid_path(remote)) {
    printf("MiniFs can only work with canonized paths!\n");
    return;
  }

  switch (client_net_api_copy_from_local(local, remote)) {
    case INVALID_PATH:
      printf("path already exists\n");
      break;
    case NOT_EXIST:
      printf("parent directory doesn't exists\n");
      break;
    case NOT_ENOUGH_SPACE:
      printf("cannot create file due to lack of space\n");
      break;
    default:
      break;
  }
}

static inline void minifs_copy_to_local(const char* local, const char* remote) {
  if (!is_valid_path(remote)) {
    printf("MiniFs can only work with canonized paths!\n");
    return;
  }

  switch (client_net_api_copy_to_local(local, remote)) {
    case NOT_EXIST:
      printf("file not exists\n");
      break;
    case IS_DIRECTORY:
      printf("trying to copy directory\n");
      break;
    default:
      break;
  }
}

static inline void print_help() {
  printf("MiniFs usage:\n");
  printf(" help                                       show help\n");
  printf(" cat             <file>                     prints file content\n");
  printf(" dir             <directory>                prints dir content\n");
  printf(" del             <file>                     delete file\n");
  printf(" rmdir           <directory>                delete directory\n");
  printf(" mkdir           <directory>                create directory\n");
  printf(
      " copy_from_local <local file> <remote file> copy file from local fs\n");
  printf(
      " copy_to_local   <remote file> <local file> copy file to local "
      "fs\n");
}

static inline void parse_args(int argc, char** argv) {
  switch (argc) {
    case 2:
      if (strcmp(argv[1], "help") == 0) {
        print_help();
        break;
      }
      goto help;
    case 3:
      if (strcmp(argv[1], "cat") == 0) {
        minifs_cat(argv[2]);
        break;
      }
      if (strcmp(argv[1], "dir") == 0) {
        minifs_dir(argv[2]);
        break;
      }
      if (strcmp(argv[1], "del") == 0) {
        minifs_del(argv[2]);
        break;
      }
      if (strcmp(argv[1], "rmdir") == 0) {
        minifs_rmdir(argv[2]);
        break;
      }
      if (strcmp(argv[1], "mkdir") == 0) {
        minifs_mkdir(argv[2]);
        break;
      }
      goto help;
    case 4:
      if (strcmp(argv[1], "copy_from_local") == 0) {
        minifs_copy_from_local(argv[2], argv[3]);
        break;
      }
      if (strcmp(argv[1], "copy_to_local") == 0) {
        minifs_copy_to_local(argv[3], argv[2]);
        break;
      }
      goto help;
    default:
    help:
      print_help();
      break;
  }
}

int main(int argc, char** argv) {
  int sock;
  struct sockaddr_in addr;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("socket");
    exit(1);
  }

  addr.sin_family = AF_INET;
  addr.sin_port = htons(3245);
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    perror("connect");
    exit(2);
  }

  client_net_api_init(sock);
  parse_args(argc, argv);

  close(sock);
  return 0;
}
