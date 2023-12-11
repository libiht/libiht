////////////////////////////////////////////////////////////////////////////////
//
//  File           : lkm/src/xplat_lkm.c
//  Description    : This is the cross-platform compatibility layer for the
//                   Linux kernel module.
//
//   Author        : Thomason Zhao
//   Last Modified : Dec 03, 2023
//

#include "../../commons/xplat.h"
#include "../include/headers_lkm.h"

//
// Cross-platform functions

//
// Memory management functions

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xmalloc
// Description  : Cross platform kernel malloc function. Allocate memory from
//                the kernel heap.
//
// Inputs       : size - size of the memory to be allocated.
// Outputs      : void* - pointer to the allocated memory.

void* xmalloc(u64 size)
{
    return kmalloc(size, GFP_KERNEL);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xfree
// Description  : Cross platform kernel free function. Free memory from the
//                kernel heap.
//
// Inputs       : ptr - pointer to the memory to be freed.
// Outputs      : void

void xfree(void *ptr)
{
    kfree(ptr);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xmemset
// Description  : Cross platform kernel memset function. Set memory to a
//                specific value.
//
// Inputs       : ptr - pointer to the memory to be set.
//                c   - value to be set.
//                cnt - size of the memory to be set.

void xmemset(void *ptr, s32 c, u64 cnt)
{
    memset(ptr, c, cnt);
}

//
// CPU core, hardware, register read/write functions

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xlock_core
// Description  : Cross platform lock core function. Disable interrupts and
//                save the old IRQL.
//
// Inputs       : old_irql - pointer to the old IRQL.
// Outputs      : void

void xlock_core(void *old_irql)
{
    get_cpu();
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xunlock_core
// Description  : Cross platform unlock core function. Restore IRQL to
//                the old IRQL.
//
// Inputs       : old_irql - pointer to the old IRQL.
// Outputs      : void

void xunlock_core(void *old_irql)
{
    put_cpu();
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xwrmsr
// Description  : Cross platform write MSR function. Write a value to a MSR.
//
// Inputs       : msr - MSR to be written.
//                val - value to be written.
// Outputs      : void

void xwrmsr(u32 msr, u64 val)
{
    wrmsrl(msr, val);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xrdmsr
// Description  : Cross platform read MSR function. Read a value from a MSR.
//
// Inputs       : msr - MSR to be read.
//                val - pointer to the value to be read.
// Outputs      : void

void xrdmsr(u32 msr, u64 *val)
{
    rdmsrl(msr, *val);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xcoreid
// Description  : Cross platform get core id function. Get the current core id.
//
// Inputs       : void
// Outputs      : u32 - current core id.

u32 xcoreid(void)
{
    return smp_processor_id();
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xgetcurrent_pid
// Description  : Cross platform get current pid function. Get the current pid.
//
// Inputs       : void
// Outputs      : u32 - current pid.

u32 xgetcurrent_pid(void)
{
    return current->pid;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xcpuid
// Description  : Cross platform cpuid function. Get the cpuid information.
//
// Inputs       : func_id - function id.
// Outputs      : void

void xcpuid(u32 func_id, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx)
{
    cpuid(func_id, eax, ebx, ecx, edx);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xon_each_cpu
// Description  : Cross platform on each cpu function. Dispatch a function on
//                each cpu.
//
// Inputs       : func - function to be run.
// Outputs      : void

void xon_each_cpu(void (*func)(void))
{
    on_each_cpu((void *)(void *)func, NULL, 1);
}

//
// Lock functions

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xinit_lock
// Description  : Cross platform init lock function. Initialize a lock.
//
// Inputs       : lock - pointer to the lock to be initialized.
// Outputs      : void

void xinit_lock(void *lock)
{
    spin_lock_init((spinlock_t *)lock);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xacquire_lock
// Description  : Cross platform acquire lock function. Acquire a lock.
//
// Inputs       : lock - pointer to the lock to be acquired.
//                old_irql - pointer to the old IRQL.
// Outputs      : void

void xacquire_lock(void *lock, void *old_irql)
{
    spin_lock_irqsave((spinlock_t *)lock, *(unsigned long *)old_irql);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xrelease_lock
// Description  : Cross platform release lock function. Release a lock.
//
// Inputs       : lock - pointer to the lock to be released.
//                new_irql - pointer to the new IRQL.
// Outputs      : void

void xrelease_lock(void *lock, void *new_irql)
{
    spin_unlock_irqrestore((spinlock_t *)lock, *(unsigned long *)new_irql);
}

//
// Debug functions

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xprintdbg
// Description  : Cross platform print debug function. Print debug information.
//
// Inputs       : format - format string.
// Outputs      : void

void xprintdbg(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintk(format, args);
    va_end(args);
}

