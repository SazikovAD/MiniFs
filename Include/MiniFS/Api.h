//
// Created by user on 19.03.2020.
//

#ifndef MINIFS_INCLUDE_MINIFS_API_H_
#define MINIFS_INCLUDE_MINIFS_API_H_

typedef enum {
  OK,
  NOT_EXIST,
  IS_FILE,
  IS_DIRECTORY,
  NOT_EMPTY,
  INVALID_PATH,
  NOT_ENOUGH_SPACE
} Status;

void api_init();

Status api_cat(const char* path);
Status api_dir(const char* path);

Status api_del(const char* path);
Status api_rmdir(const char* path);

Status api_mkdir(const char* path);
Status api_copy_from_local(const char* local_path, const char* remote_path);
Status api_copy_to_local(const char* local_path, const char* remote_path);

#endif  // MINIFS_INCLUDE_MINIFS_API_H_
