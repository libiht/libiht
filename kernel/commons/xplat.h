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
//   Last Modified : Mar 27, 2024
//

#include "types.h"
#include "debug.h"

// cpp cross compile handler
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//
// Library constants

#define MAX_IRQL_LEN    0x10    // Maximum length of OS irql struct
#define MAX_LOCK_LEN    0x20    // Maximum length of OS lock struct
#define MAX_LIST_LEN    0x20    // Maximum length of OS list struct

//
// Function Prototypes

//
// Memory management functions

void *xmalloc(u64 size);
// Cross platform kernel malloc function.

void xfree(void *ptr);
// Cross platform kernel free function.

u64 xcopy_from_user(void *dst, void *src, u64 cnt);
// Cross platform kernel copy from user function.

u64 xcopy_to_user(void *dst, void *src, u64 cnt);
// Cross platform kernel copy to user function.

void *xmemset(void *ptr, s32 c, u64 cnt);
// Cross platform kernel memset function.

void *xmemcpy(void *dst, void *src, u64 cnt);
// Cross platform kernel memcpy function.

//
// CPU core, hardware, register read/write functions

void xlock_core(void *old_irql);
// Cross platform lock core function.

void xrelease_core(void *new_irql);
// Cross platform release core function.

void xwrmsr(u32 msr, u64 val);
// Cross platform write msr function.

void xrdmsr(u32 msr, u64 *val);
// Cross platform read msr function.

u32  xcoreid(void);
// Cross platform get core id function.

u32 xgetcurrent_pid(void);
// Cross platform get current user process pid function.

void xcpuid(u32 func_id, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx);
// Cross platform cpuid function.

void xon_each_cpu(void (*func)(void));
// Cross platform on each cpu dispatch function.

//
// Lock functions

void xinit_lock(void *lock);
// Cross platform init lock function.

void xacquire_lock(void *lock, void *old_irql);
// Cross platform acquire lock function.

void xrelease_lock(void *lock, void *new_irql);
// Cross platform release lock function.

//
// List functions

void xinit_list_head(void *list);
// Cross platform init list head function.

void xlist_add(void *new_entry, void *head);
// Cross platform list add function.

void xlist_del(void *entry);
// Cross platform list del function.

void *xlist_next(void *entry);
// Cross platform list next function.

void *xlist_prev(void *entry);
// Cross platform list prev function.

//
// Debug functions (will be moved to debug.h)

void xprintdbg(const char *format, ...);
// Cross platform print kernel debug message function.

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _COMMONS_XPLAT_H
