/*
 * Userspace libc basic types definitions for DionOS
 */

#ifndef _DIONOS_SYS_TYPES_H
#define _DIONOS_SYS_TYPES_H

#include <ABI/kernel/internal/kabi.h>

typedef s64 off_t;
typedef s64 time_t;
typedef u32 mode_t;
typedef u32 uid_t;
typedef u32 gid_t;
typedef u64 dev_t;
typedef u64 ino_t;

struct timespec {
	time_t tv_sec;
	s64 tv_nsec;
};

#endif /* _DIONOS_SYS_TYPES_H */
