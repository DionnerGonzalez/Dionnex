/* ============================================================================
 * Dionnex Monolithic Kernel - Kernel Logging Subsystem Header
 * ============================================================================
 */
#ifndef _DIONNEX_KLOG_H_
#define _DIONNEX_KLOG_H_

#include <types.h>
#include <config.h>

#define KLOG_DEBUG 0
#define KLOG_INFO  1
#define KLOG_WARN  2
#define KLOG_ERROR 3
#define KLOG_PANIC 4

void klog_init(void);
void klog(uint8_t level, const char *fmt, ...);
void klog_dump(void);
void klog_clear(void);
uint32_t klog_get_count(void);
int klog_read_bytes(char *buf, uint32_t max_len);

#endif /* _DIONNEX_KLOG_H_ */
