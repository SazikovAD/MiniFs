//
// Created by user on 26.03.2020.
//

#ifndef MINIFS_INCLUDE_MINIFS_NETAPI_H_
#define MINIFS_INCLUDE_MINIFS_NETAPI_H_

#include "Api.h"

#include <stdlib.h>

typedef enum {
  CAT,
  DIR,
  DEL,
  RMDIR,
  MKDIR,
  COPY_FROM_LOCAL,
  COPY_TO_LOCAL,
  NONE
} NetOperations;

void server_net_api_init(int fd);
void client_net_api_init(int fd);

void server_net_api_cat(const char* path);
void server_net_api_dir(const char* path);

void server_net_api_del(const char* path);
void server_net_api_rmdir(const char* path);

void server_net_api_mkdir(const char* path);

void server_net_api_copy_from_local(size_t content_length, const char* content,
                                    const char* remote_path);
void server_net_api_copy_to_local(const char* remote_path);

Status client_net_api_cat(const char* path);
Status client_net_api_dir(const char* path);

Status client_net_api_del(const char* path);
Status client_net_api_rmdir(const char* path);

Status client_net_api_mkdir(const char* path);

Status client_net_api_copy_from_local(const char* local_path,
                                      const char* remote_path);
Status client_net_api_copy_to_local(const char* local_path,
                                    const char* remote_path);

#endif  // MINIFS_INCLUDE_MINIFS_NETAPI_H_
