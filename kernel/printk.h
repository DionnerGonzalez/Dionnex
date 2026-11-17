/* ============================================================================
 * Dionnex Monolithic Kernel - Kernel Output Printk Header
 * ============================================================================
 */
#ifndef _DIONNEX_PRINTK_H_
#define _DIONNEX_PRINTK_H_

#include <types.h>

void printk(const char *fmt, ...);

#define pr_info(fmt, ...)  printk("[INFO] " fmt, ##__VA_ARGS__)
#define pr_warn(fmt, ...)  printk("[WARN] " fmt, ##__VA_ARGS__)
#define pr_err(fmt, ...)   printk("[ERR]  " fmt, ##__VA_ARGS__)
#define pr_debug(fmt, ...) printk("[DBG]  " fmt, ##__VA_ARGS__)

#endif /* _DIONNEX_PRINTK_H_ */
