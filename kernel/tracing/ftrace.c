/*
 * Function Tracer (ftrace) Subsystem
 */

#include <ABI/kernel/internal/kabi.h>

void __cyg_profile_func_enter(void *this_fn, void *call_site)
{
	(void)this_fn;
	(void)call_site;
}

void __cyg_profile_func_exit(void *this_fn, void *call_site)
{
	(void)this_fn;
	(void)call_site;
}
