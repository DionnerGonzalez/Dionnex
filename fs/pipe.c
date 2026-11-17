/* ============================================================================
 * Dionnex Monolithic Kernel - Kernel Pipe IPC Implementation
 * ============================================================================
 */
#include <fs/pipe.h>
#include <mm/heap.h>
#include <string.h>

typedef struct pipe_endpoint {
    pipe_buffer_t *pipe;
    uint8_t is_write;
} pipe_endpoint_t;

static int pipe_vfs_read(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buf) {
    (void)offset;
    if (!node || !buf || size == 0) return 0;
    pipe_endpoint_t *ep = (pipe_endpoint_t*)node->private_data;
    if (!ep || !ep->pipe || ep->is_write) return -1;

    pipe_buffer_t *p = ep->pipe;
    uint32_t flags = spin_lock_irqsave(&p->lock);
    uint32_t read_bytes = 0;
    while (read_bytes < size && p->count > 0) {
        buf[read_bytes++] = p->data[p->read_pos];
        p->read_pos = (p->read_pos + 1U) % PIPE_BUF_SIZE;
        p->count--;
    }
    spin_unlock_irqrestore(&p->lock, flags);
    return (int)read_bytes;
}

static int pipe_vfs_write(vfs_node_t *node, uint32_t offset, uint32_t size, const uint8_t *buf) {
    (void)offset;
    if (!node || !buf || size == 0) return 0;
    pipe_endpoint_t *ep = (pipe_endpoint_t*)node->private_data;
    if (!ep || !ep->pipe || !ep->is_write) return -1;

    pipe_buffer_t *p = ep->pipe;
    uint32_t flags = spin_lock_irqsave(&p->lock);
    if (p->readers == 0) {
        spin_unlock_irqrestore(&p->lock, flags);
        return -1; /* Broken pipe (EPIPE) */
    }
    uint32_t written = 0;
    while (written < size && p->count < PIPE_BUF_SIZE) {
        p->data[p->write_pos] = buf[written++];
        p->write_pos = (p->write_pos + 1U) % PIPE_BUF_SIZE;
        p->count++;
    }
    spin_unlock_irqrestore(&p->lock, flags);
    return (int)written;
}

static int pipe_vfs_close(vfs_node_t *node) {
    if (!node) return -1;
    pipe_endpoint_t *ep = (pipe_endpoint_t*)node->private_data;
    if (!ep || !ep->pipe) return -1;

    pipe_buffer_t *p = ep->pipe;
    uint32_t flags = spin_lock_irqsave(&p->lock);
    if (ep->is_write) {
        if (p->writers > 0) p->writers--;
    } else {
        if (p->readers > 0) p->readers--;
    }
    int free_pipe = (p->readers == 0 && p->writers == 0);
    spin_unlock_irqrestore(&p->lock, flags);

    if (free_pipe) {
        kfree(p);
    }
    kfree(ep);
    kfree(node);
    return 0;
}

static vfs_ops_t pipe_read_ops = {
    .read = pipe_vfs_read,
    .write = NULL,
    .open = NULL,
    .close = pipe_vfs_close,
    .readdir = NULL,
    .finddir = NULL,
    .create = NULL,
    .delete = NULL
};

static vfs_ops_t pipe_write_ops = {
    .read = NULL,
    .write = pipe_vfs_write,
    .open = NULL,
    .close = pipe_vfs_close,
    .readdir = NULL,
    .finddir = NULL,
    .create = NULL,
    .delete = NULL
};

int pipe_create(vfs_node_t **read_end, vfs_node_t **write_end) {
    if (!read_end || !write_end) return -1;

    pipe_buffer_t *p = (pipe_buffer_t*)kcalloc(1, sizeof(pipe_buffer_t));
    vfs_node_t *rnode = (vfs_node_t*)kcalloc(1, sizeof(vfs_node_t));
    vfs_node_t *wnode = (vfs_node_t*)kcalloc(1, sizeof(vfs_node_t));
    pipe_endpoint_t *rep = (pipe_endpoint_t*)kcalloc(1, sizeof(pipe_endpoint_t));
    pipe_endpoint_t *wep = (pipe_endpoint_t*)kcalloc(1, sizeof(pipe_endpoint_t));

    if (!p || !rnode || !wnode || !rep || !wep) {
        if (p) kfree(p);
        if (rnode) kfree(rnode);
        if (wnode) kfree(wnode);
        if (rep) kfree(rep);
        if (wep) kfree(wep);
        return -1;
    }

    spinlock_init(&p->lock);
    p->readers = 1;
    p->writers = 1;

    rep->pipe = p;
    rep->is_write = 0;
    wep->pipe = p;
    wep->is_write = 1;

    strcpy(rnode->name, "pipe_r");
    rnode->flags = VFS_CHARDEV;
    rnode->private_data = rep;
    rnode->ops = &pipe_read_ops;

    strcpy(wnode->name, "pipe_w");
    wnode->flags = VFS_CHARDEV;
    wnode->private_data = wep;
    wnode->ops = &pipe_write_ops;

    *read_end = rnode;
    *write_end = wnode;
    return 0;
}
