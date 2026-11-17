/* ============================================================================
 * Dionnex Monolithic Kernel - Kernel Pipe IPC Header
 * ============================================================================
 */
#ifndef _DIONNEX_PIPE_H_
#define _DIONNEX_PIPE_H_

#include <types.h>
#include <fs/vfs.h>
#include <kernel/sync.h>

#define PIPE_BUF_SIZE 4096

typedef struct pipe_buffer {
    uint8_t    data[PIPE_BUF_SIZE];
    uint32_t   read_pos;
    uint32_t   write_pos;
    uint32_t   count;
    uint32_t   readers;
    uint32_t   writers;
    spinlock_t lock;
} pipe_buffer_t;

int pipe_create(vfs_node_t **read_end, vfs_node_t **write_end);

#endif /* _DIONNEX_PIPE_H_ */
