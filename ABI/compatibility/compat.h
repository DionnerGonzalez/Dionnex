/*
 * Dionnex Backward Compatibility ABI
 */

#ifndef _DIONNEX_COMPAT_H
#define _DIONNEX_COMPAT_H

#include <ABI/kernel/internal/kabi.h>

struct compat_timeval {
	s32 tv_sec;
	s32 tv_usec;
};

#endif /* _DIONOS_COMPAT_H */
