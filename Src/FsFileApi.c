//
// Created by user on 13.03.2020.
//

#include <MiniFS/Config.h>
#include <MiniFS/FsFileApi.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static_assert(sizeof(inode) == INODE_SIZE, "INODE_SIZE mismatch");
static_assert(sizeof(super_block) == SUPER_BLOCK_SIZE,
              "SUPER_BLOCK_SIZE "
              "mismatch");

static super_block* fs_file_super_block = NULL;
static FILE* fs_file = NULL;

static inline void fs_file_goto_super_block() {
  fseek(fs_file, 0, SEEK_SET);
}

static inline void fs_file_goto_block_table() {
  fseek(fs_file, SUPER_BLOCK_SIZE + INODE_TABLE_SIZE, SEEK_SET);
}

static inline void fs_file_goto_inode(uint32_t inode_id) {
  fseek(fs_file, SUPER_BLOCK_SIZE + (inode_id - 1) * INODE_SIZE, SEEK_SET);
}

static inline void fs_file_goto_block(uint32_t block_id) {
  fseek(fs_file,
        SUPER_BLOCK_SIZE + INODE_TABLE_SIZE + BLOCK_TABLE_SIZE +
            (block_id - 1) * BLOCK_SIZE,
        SEEK_SET);
}

static inline void fs_file_read(void* ptr, size_t size, size_t n) {
  fread(ptr, size, n, fs_file);
}

static inline void fs_file_write(const void* ptr, size_t size, size_t n) {
  fwrite(ptr, size, n, fs_file);
}

static inline void fs_file_flush() {
  fflush(fs_file);
}

static inline void fs_file_read_block_table(char* block_table) {
  fs_file_goto_block_table();
  fs_file_read(block_table, 1, BLOCK_TABLE_SIZE);
}

static inline void fs_file_write_block_table(const char* block_table) {
  fs_file_goto_block_table();
  fs_file_write(block_table, 1, BLOCK_TABLE_SIZE);
  fs_file_flush();
}

static inline void init_super_block(super_block* sb) {
  sb->block_size = BLOCK_SIZE;
  sb->inode_size = INODE_SIZE;

  sb->block_total = BLOCK_NUMBER;
  sb->inode_total = INODE_SIZE;

  sb->block_used = 0;
  sb->inode_used = 0;
}

static void at_exit_action() {
  fs_file_goto_super_block();
  fs_file_write(fs_file_super_block, SUPER_BLOCK_SIZE, 1);
  fclose(fs_file);
  free(fs_file_super_block);
}

static void create_fs_file() {
  FILE* file = fopen(FS_FILE_NAME, "wb");
  ftruncate(fileno(file), FS_FILE_SIZE);
  fclose(file);
}

static void open_fs_file(int read_super_block) {
  fs_file = fopen(FS_FILE_NAME, "rb+");
  if (read_super_block) {
    fs_file_goto_super_block();
    fs_file_read(fs_file_super_block, SUPER_BLOCK_SIZE, 1);
  }
}

void fs_file_api_init() {
  fs_file_super_block = (super_block*)malloc(SUPER_BLOCK_SIZE);
  init_super_block(fs_file_super_block);

  if (access(FS_FILE_NAME, F_OK) == -1) {
    create_fs_file();
    open_fs_file(0);
  } else {
    open_fs_file(1);
  }

  atexit(at_exit_action);
}

void fs_file_api_acquire_inode(uint32_t* inode_id) {
  fs_file_goto_inode(1);
  inode i_node;
  memset(&i_node, 0, INODE_SIZE);

  for (uint32_t i = 0; i < INODE_NUMBER; ++i) {
    fs_file_read(&i_node, INODE_SIZE, 1);
    if (i_node.file_block == 0) {
      *inode_id = i + 1;
      fs_file_super_block->inode_used++;
      return;
    }
  }

  *inode_id = 0;
}

void fs_file_api_release_inode(uint32_t inode_id) {
  fs_file_goto_inode(inode_id);
  char buffer[INODE_SIZE];
  memset(buffer, 0, INODE_SIZE);
  fs_file_write(buffer, 1, INODE_SIZE);
  fs_file_super_block->inode_used--;
}

void fs_file_api_read_inode(uint32_t inode_id, inode* i_node) {
  fs_file_goto_inode(inode_id);
  fs_file_read(i_node, INODE_SIZE, 1);
}

void fs_file_api_write_inode(uint32_t inode_id, const inode* i_node) {
  fs_file_goto_inode(inode_id);
  fs_file_write(i_node, INODE_SIZE, 1);
}

void fs_file_api_try_acquire_blocks(uint32_t block_count, uint32_t* acquired,
                                    uint32_t* block_id) {
  char* block_table = malloc(BLOCK_TABLE_SIZE);
  fs_file_read_block_table(block_table);

  uint32_t counter = 0;
  for (uint32_t i = 0; counter < block_count && i < BLOCK_TABLE_SIZE; ++i) {
    if (block_table[i] == 0) {
      block_id[counter++] = i + 1;
    }
  }
  *acquired = counter;

  free(block_table);
}

void fs_file_api_acquire_blocks(uint32_t block_count,
                                const uint32_t* block_id) {
  char* block_table = malloc(BLOCK_TABLE_SIZE);
  fs_file_read_block_table(block_table);

  for (uint32_t i = 0; i < block_count; ++i) {
    block_table[block_id[i] - 1] = 1;
  }
  fs_file_super_block->block_used += block_count;

  fs_file_write_block_table(block_table);
  free(block_table);
}

void fs_file_api_release_blocks(uint32_t block_count,
                                const uint32_t* block_id) {
  char* block_table = malloc(BLOCK_TABLE_SIZE);
  fs_file_read_block_table(block_table);

  for (uint32_t i = 0; i < block_count; ++i) {
    block_table[block_id[i] - 1] = 0;
  }
  fs_file_super_block->block_used -= block_count;

  fs_file_write_block_table(block_table);
  free(block_table);
}

void fs_file_api_read_full_block(uint32_t block_id, char* block) {
  fs_file_goto_block(block_id);
  fs_file_read(block, 1, BLOCK_SIZE);
}

void fs_file_api_write_full_block(uint32_t block_id, const char* block) {
  fs_file_goto_block(block_id);
  fs_file_write(block, 1, BLOCK_SIZE);
}

void fs_file_api_read_next_block_id(uint32_t block_id,
                                    uint32_t* next_block_id) {
  fs_file_goto_block(block_id);
  fseek(fs_file, BLOCK_SPACE, SEEK_CUR);
  fs_file_read(next_block_id, sizeof(uint32_t), 1);
}
