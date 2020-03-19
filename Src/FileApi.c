//
// Created by user on 13.03.2020.
//

#include <MiniFS/Config.h>
#include <MiniFS/FileApi.h>
#include <MiniFS/FsFileApi.h>

#include <math.h>
#include <stdlib.h>
#include <string.h>

#define MIN(X, Y) (X < Y ? X : Y)

void file_api_remove(uint32_t inode_id) {
  inode i_node;
  fs_file_api_read_inode(inode_id, &i_node);

  uint64_t size = i_node.file_size;
  uint32_t block_count = (uint32_t)ceil((double)size / (double)BLOCK_SPACE);

  uint32_t* blocks = (uint32_t*)malloc(sizeof(uint32_t) * block_count);
  uint32_t block_id = i_node.file_block;

  for (uint32_t i = 0; i < block_count; ++i) {
    blocks[i] = block_id;
    fs_file_api_read_next_block_id(block_id, &block_id);
  }

  fs_file_api_release_inode(inode_id);
  fs_file_api_release_blocks(block_count, blocks);

  free(blocks);
}

void file_api_read(uint32_t inode_id, raw_file* file) {
  inode i_node;
  fs_file_api_read_inode(inode_id, &i_node);

  uint32_t block_id = i_node.file_block;
  uint64_t size = i_node.file_size;
  uint64_t offset = 0;

  char buffer[BLOCK_SIZE];
  memset(buffer, 0, BLOCK_SIZE);

  file->file_size = size;
  file->data = malloc(size);

  while (block_id != 0) {
    fs_file_api_read_full_block(block_id, buffer);
    memcpy(&block_id, buffer + BLOCK_SPACE, sizeof(uint32_t));
    memcpy(file->data + offset, buffer, MIN(BLOCK_SPACE, size));
    offset += BLOCK_SPACE;
    size -= BLOCK_SPACE;
  }
}

void file_api_write(uint32_t* inode_id, const raw_file* file) {
  uint64_t size = file->file_size;
  uint32_t block_count = (uint32_t)ceil((double)size / (double)BLOCK_SPACE);

  uint32_t acquired_inode = 0;
  inode i_node;

  fs_file_api_acquire_inode(&acquired_inode);

  if (acquired_inode == 0) {
    *inode_id = 0;
    return;
  }

  uint32_t acquired_blocks = 0;
  uint32_t* blocks = (uint32_t*)malloc(sizeof(uint32_t) * block_count);
  fs_file_api_try_acquire_blocks(block_count, &acquired_blocks, blocks);

  if (acquired_blocks != block_count) {
    free(blocks);
    *inode_id = 0;
    return;
  }

  fs_file_api_acquire_blocks(block_count, blocks);

  uint64_t offset = 0;
  char buffer[BLOCK_SIZE];

  for (uint32_t i = 0; i < block_count; ++i) {
    memset(buffer, 0, BLOCK_SIZE);
    memcpy(buffer, file->data + offset, MIN(BLOCK_SPACE, size));

    if (i < block_count - 1) {
      memcpy(buffer + BLOCK_SPACE, &blocks[i + 1], sizeof(uint32_t));
    }

    offset += BLOCK_SPACE;
    size -= BLOCK_SPACE;

    fs_file_api_write_full_block(blocks[i], buffer);
  }

  *inode_id = acquired_inode;
  i_node.file_size = file->file_size;
  i_node.file_block = blocks[0];

  fs_file_api_write_inode(acquired_inode, &i_node);

  free(blocks);
}

void file_api_append(uint32_t inode_id, const raw_file* file, int* status) {
  inode i_node;
  fs_file_api_read_inode(inode_id, &i_node);

  uint64_t old_size = i_node.file_size;
  uint32_t old_block_num = 0;

  uint32_t block_id = 0;
  uint32_t next_block_id = i_node.file_block;

  do {
    block_id = next_block_id;
    fs_file_api_read_next_block_id(block_id, &next_block_id);
    old_block_num++;
  } while (next_block_id != 0);

  uint64_t space_left =
      (uint64_t)BLOCK_SPACE * (uint64_t)old_block_num - old_size;

  uint32_t new_block_num = (uint32_t)ceil(
      (double)(file->file_size - space_left) / (double)BLOCK_SPACE);
  uint32_t acquired_blocks = 0;
  uint32_t* blocks = (uint32_t*)malloc(sizeof(uint32_t) * new_block_num);

  fs_file_api_try_acquire_blocks(new_block_num, &acquired_blocks, blocks);

  if (acquired_blocks != new_block_num) {
    *status = 1;
    return;
  }

  fs_file_api_acquire_blocks(new_block_num, blocks);

  char buffer[BLOCK_SIZE];

  fs_file_api_read_full_block(block_id, buffer);
  memcpy(buffer + BLOCK_SPACE - space_left, file->data,
         MIN(space_left, file->file_size));
  if (new_block_num != 0) {
    memcpy(buffer + BLOCK_SPACE, &blocks[0], sizeof(uint32_t));
  }
  fs_file_api_write_full_block(block_id, buffer);

  uint64_t offset = space_left;
  uint64_t size_left = file->file_size - space_left;

  for (uint32_t i = 0; i < new_block_num; ++i) {
    memset(buffer, 0, BLOCK_SIZE);
    memcpy(buffer, file->data + offset, MIN(BLOCK_SPACE, size_left));
    offset += BLOCK_SPACE;
    size_left -= BLOCK_SIZE;

    if (i < new_block_num - 1) {
      memcpy(buffer + BLOCK_SPACE, &blocks[i + 1], sizeof(uint32_t));
    }

    fs_file_api_write_full_block(blocks[i], buffer);
  }

  i_node.file_size += file->file_size;
  fs_file_api_write_inode(inode_id, &i_node);
  *status = 0;
}

void file_api_rewrite(uint32_t inode_id, const raw_file* file, int* status) {
  inode i_node;
  fs_file_api_read_inode(inode_id, &i_node);

  uint64_t size = i_node.file_size;
  uint32_t block_count = (uint32_t)ceil((double)size / (double)BLOCK_SPACE);

  uint32_t* blocks = (uint32_t*)malloc(sizeof(uint32_t) * block_count);
  uint32_t block_id = i_node.file_block;

  for (uint32_t i = 0; i < block_count; ++i) {
    blocks[i] = block_id;
    fs_file_api_read_next_block_id(block_id, &block_id);
  }

  fs_file_api_release_blocks(block_count - 1, blocks);

  i_node.file_size = 0;
  i_node.file_block = blocks[block_count - 1];

  fs_file_api_write_inode(inode_id, &i_node);

  free(blocks);

  file_api_append(inode_id, file, status);
}