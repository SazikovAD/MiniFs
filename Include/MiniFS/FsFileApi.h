//
// Created by user on 13.03.2020.
//

#ifndef MINIFS_INCLUDE_MINIFS_FSFILEAPI_H_
#define MINIFS_INCLUDE_MINIFS_FSFILEAPI_H_

#include <stdint.h>

typedef struct {
  uint64_t file_size;
  uint32_t file_block;
} inode;

typedef struct {
  uint32_t block_total;
  uint32_t inode_total;

  uint32_t block_used;
  uint32_t inode_used;

  uint32_t block_size;
  uint32_t inode_size;
} super_block;

void fs_file_api_init();

void fs_file_api_acquire_inode(uint32_t* inode_id);
void fs_file_api_release_inode(uint32_t inode_id);

void fs_file_api_read_inode(uint32_t inode_id, inode* i_node);
void fs_file_api_write_inode(uint32_t inode_id, const inode* i_node);

void fs_file_api_try_acquire_blocks(uint32_t block_count, uint32_t* acquired,
                                    uint32_t* block_id);
void fs_file_api_acquire_blocks(uint32_t block_count, const uint32_t* block_id);
void fs_file_api_release_blocks(uint32_t block_count, const uint32_t* block_id);

void fs_file_api_read_full_block(uint32_t block_id, char* block);
void fs_file_api_write_full_block(uint32_t block_id, const char* block);

void fs_file_api_read_next_block_id(uint32_t block_id, uint32_t* next_block_id);

#endif  // MINIFS_INCLUDE_MINIFS_FSFILEAPI_H_
