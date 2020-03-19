//
// Created by user on 13.03.2020.
//

#include <MiniFS/Config.h>

#include <assert.h>
#include <stdint.h>

static_assert((uint64_t)FS_FILE_SIZE ==
                  (uint64_t)SUPER_BLOCK_SIZE + (uint64_t)INODE_TABLE_SIZE +
                      (uint64_t)BLOCK_TABLE_SIZE +
                      (uint64_t)BLOCK_SIZE * (uint64_t)BLOCK_NUMBER,
              "FS_FILE_SIZE mismatch");

static_assert(INODE_TABLE_SIZE == INODE_NUMBER * INODE_SIZE,
              "INODE_TABLE_SIZE mismatch");

static_assert(BLOCK_TABLE_SIZE == BLOCK_NUMBER, "BLOCK_TABLE_SIZE mismatch");
