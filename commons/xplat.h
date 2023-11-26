#ifndef _COMMONS_XPLAT_H
#define _COMMONS_XPLAT_H

////////////////////////////////////////////////////////////////////////////////
//
//  File           : commons/xplat.h
//  Description    : This is the header file for the cross platform functions
//                   used for libiht library. Details of the functions are
//                   implemented in the corresponding platform specific files.
//                   i.e. `xplat_kmd.c` and `xplat_lkm.c` for windows and linux.
//
//   Author        : Thomason Zhao
//   Last Modified : Nov 25, 2023
//

#include "types.h"
#include "debug.h"

// cpp cross compile handler
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//
// Library constants

#define MAX_IRQL_LEN	0x10
#define MAX_LOCK_LEN	0x100

//
// Function Prototypes

void* xmalloc(u64 size);
// Cross platform kernel malloc function.

void xfree(void *ptr);
// Cross platform kernel free function.

void xmemset(void *ptr, s32 c, u64 cnt);
// Cross platform kernel memset function.

void xlock_core(void *old_irql);
// Cross platform lock core function.

void xrelease_core(void *new_irql);
// Cross platform release core function.

void xwrmsr(u32 msr, u64 val);
// Cross platform write msr function.

void xrdmsr(u32 msr, u64 *val);
// Cross platform read msr function.

void xinit_lock(void *lock);
// Cross platform init lock function.

void xacquire_lock(void *lock, void *old_irql);
// Cross platform acquire lock function.

void xrelease_lock(void *lock, void *new_irql);
// Cross platform release lock function.

u32  xcoreid(void);
// Cross platform get core id function.

void xcpuid(u32 func_id, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx);
// Cross platform cpuid function.

void xon_each_cpu(void (*func)(void));
// Cross platform on each cpu dispatch function.

void xprintdbg(const char *format, ...);
// Cross platform print kernel debug message function.

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _COMMONS_XPLAT_H
