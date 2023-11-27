#ifndef _XPLAT_H
#define _XPLAT_H

#include "types.h"
#include "debug.h"

/* cpp cross compile handler */
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Macros for cross platform
// This constrains the maximum length of the IRQL flag
#define MAX_IRQL_LEN	0x10
#define MAX_LOCK_LEN	0x100

/* Define all cross flatform function prototypes */
void*	xmalloc(u64 size);
void	xfree(void *ptr);
void	xmemset(void *ptr, s32 c, u64 cnt);

void	xlock_core(void *old_irql);
void	xrelease_core(void *new_irql);

void	xwrmsr(u32 msr, u64 val);
void	xrdmsr(u32 msr, u64 *val);

void	xinit_lock(void *lock);
void	xacquire_lock(void *lock, void *old_irql);
void	xrelease_lock(void *lock, void *new_irql);

u32		xcoreid(void);
void	xcpuid(u32 func_id, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx);
void	xon_each_cpu(void (*func)(void));
void	xprintdbg(const char *format, ...);

//u32		xplat_load(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _XPLAT_H
