#ifndef _XPLAT_H
#define _XPLAT_H

#include "types.h"
#include "debug.h"

/* cpp cross compile handler */
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// TODO: consider remove this
///* Define all cross flatform function prototype pointers */
//typedef void*	(*xmalloc_t)		(u64 size);
//typedef void	(*xfree_t)			(void *ptr);
//typedef void	(*xlock_core_t)		(void);
//typedef void	(*xrelease_core_t)	(void);
//typedef void	(*xwrmsr_t)			(u32 msr, u64 val);
//typedef u64		(*xrdmsr_t)			(u32 msr);
//typedef void	(*xacquire_lock_t)	(void *lock);
//typedef void	(*xrelease_lock_t)	(void *lock);
//typedef u32		(*xcoreid_t)		(void);
//typedef void	(*xcpuid_t)			(u32 cpuinfo[4], u32 func_id);
//
//typedef struct _xplat {
//	xmalloc_t		xmalloc;
//	xfree_t			xfree;
//	xlock_core_t	xlock_core;
//	xrelease_core_t	xrelease_core;
//	xwrmsr_t		xwrmsr;
//	xrdmsr_t		xrdmsr;
//	xacquire_lock_t	xacquire_lock;
//	xrelease_lock_t	xrelease_lock;
//	xcoreid_t		xcoreid;
//	xcpuid_t		xcpuid;
//} xplat_t;
//
//xplat_t xplat;

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
