////////////////////////////////////////////////////////////////////////////////
//
//  File           : kmd/src/xplat_kmd.c
//  Description    : This is the cross-platform compatibility layer for the
//                   kernel-mode driver.
//
//   Author        : Thomason Zhao
//   Last Modified : Jan 2, 2024
//

#include "../../commons/xplat.h"
#include "../include/headers_kmd.h"

//
// Cross-platform global variables
const unsigned long g_tag = 'XPLT';

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
    return ExAllocatePool2(POOL_FLAG_NON_PAGED, size, g_tag);
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
    ExFreePool(ptr);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xcopy_from_user
// Description  : Cross platform kernel copy from user function. Copy memory
//                from user space to kernel space.
//
// Inputs       : dst - pointer to the destination memory.
//                src - pointer to the source memory.
//                cnt - size of the memory to be copied.
// Outputs      : u64 - number of bytes not copied (0 on success).

u64 xcopy_from_user(void *dst, const void *src, u64 cnt)
{
    // WOW, windows kernel can access user space memory directly
	RtlCopyMemory(dst, src, cnt);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xcopy_to_user
// Description  : Cross platform kernel copy to user function. Copy memory from
//                kernel space to user space.
//
// Inputs       : dst - pointer to the destination memory.
//                src - pointer to the source memory.
//                cnt - size of the memory to be copied.
// Outputs      : u64 - number of bytes not copied (0 on success).

u64 xcopy_to_user(void *dst, const void *src, u64 cnt)
{
	RtlCopyMemory(dst, src, cnt);
    return 0;
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
// Outputs      : void* - pointer to the set memory.

void *xmemset(void* ptr, s32 c, u64 cnt)
{
    return memset(ptr, c, cnt);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xmemcpy
// Description  : Cross platform kernel memcpy function. Copy memory from one
//                location to another.
//
// Inputs       : dst - pointer to the destination memory.
//                src - pointer to the source memory.
//                cnt - size of the memory to be copied.
// Outputs      : void* - pointer to the destination memory.

void* xmemcpy(void* dst, void* src, u64 cnt)
{
    return memcpy(dst, src, cnt);
}

//
// CPU core, hardware, register read/write functions

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xlock_core
// Description  : Cross platform lock core function. Raise IRQL to
//                DISPATCH_LEVEL.
//
// Inputs       : old_irql - pointer to the old IRQL.
// Outputs      : void

void xlock_core(void *old_irql)
{
    KeRaiseIrql(DISPATCH_LEVEL, (PKIRQL)old_irql);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xrelease_core
// Description  : Cross platform release core function. Lower IRQL to the old
//                IRQL.
//
// Inputs       : new_irql - pointer to the new IRQL.
// Outputs      : void

void xrelease_core(void *new_irql)
{
    KeLowerIrql(*(PKIRQL)new_irql);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xwrmsr
// Description  : Cross platform write msr function. Write to a MSR.
//
// Inputs       : msr - MSR to be written.
//                val - value to be written.
// Outputs      : void

void xwrmsr(u32 msr, u64 val)
{
    __writemsr(msr, val);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xrdmsr
// Description  : Cross platform read msr function. Read from a MSR.
//
// Inputs       : msr - MSR to be read.
// Outputs      : val - pointer to the value read from the MSR.

void xrdmsr(u32 msr, u64 *val)
{
    *val = __readmsr(msr);
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
    return KeGetCurrentProcessorNumberEx(0);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xgetcurrent_pid
// Description  : Cross platform get current user process pid function. Get the
//                current user process pid.
//
// Inputs       : void
// Outputs      : u32 - current user process pid.

u32 xgetcurrent_pid(void)
{
    return (u32)(ULONG_PTR)PsGetCurrentProcessId();
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xcpuid
// Description  : Cross platform cpuid function. Get the cpuid information.
//
// Inputs       : func_id - cpuid function id.
// Outputs      : void

void xcpuid(u32 func_id, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx)
{
    s32 regs[4];
    __cpuid(regs, func_id);
    *eax = regs[0];
    *ebx = regs[1];
    *ecx = regs[2];
    *edx = regs[3];
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xon_each_cpu
// Description  : Cross platform on each cpu dispatch function. Dispatch a
//                function to each cpu.
//
// Inputs       : func - function to be dispatched.
// Outputs      : void

void xon_each_cpu(void (*func)(void))
{
    KeIpiGenericCall((PKIPI_BROADCAST_WORKER)func, 0);
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
    KeInitializeSpinLock((PKSPIN_LOCK)lock);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xacquire_lock
// Description  : Cross platform acquire lock function. Acquire a lock.
//
// Inputs       : lock     - pointer to the lock to be acquired.
//                old_irql - pointer to the old IRQL.
// Outputs      : void

void xacquire_lock(void *lock, void *old_irql)
{
    KeAcquireSpinLock((PKSPIN_LOCK)lock, (PKIRQL)old_irql);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xrelease_lock
// Description  : Cross platform release lock function. Release a lock.
//
// Inputs       : lock     - pointer to the lock to be released.
//                new_irql - pointer to the new IRQL.
// Outputs      : void

void xrelease_lock(void *lock, void *new_irql)
{
    KeReleaseSpinLock((PKSPIN_LOCK)lock, *(PKIRQL)new_irql);
}

//
// List functions

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xinit_list_head
// Description  : Cross platform init list head function. Initialize a list
//                head.
//
// Inputs       : list - pointer to the list head to be initialized.
// Outputs      : void

void xinit_list_head(void *list)
{
    InitializeListHead((PLIST_ENTRY)list);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xlist_add
// Description  : Cross platform list add function. Add an entry to a list.
//
// Inputs       : new_entry - pointer to the entry to be added.
//                head      - pointer to the list head.
// Outputs      : void

void xlist_add(void* new_entry, void* head)
{
    InsertHeadList((PLIST_ENTRY)head, (PLIST_ENTRY)new_entry);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xlist_del
// Description  : Cross platform list delete function. Delete an entry from a
//                list.
//
// Inputs       : entry - pointer to the entry to be deleted.
// Outputs      : void

void xlist_del(void* entry)
{
    RemoveEntryList((PLIST_ENTRY)entry);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xlist_next
// Description  : Cross platform list next function. Get the next entry in a
//                list.
//
// Inputs       : entry - pointer to the current entry.
// Outputs      : void* - pointer to the next entry.

void* xlist_next(void* entry)
{
    return (void*)((PLIST_ENTRY)entry)->Flink;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xlist_prev
// Description  : Cross platform list prev function. Get the previous entry in a
//                list.
//
// Inputs       : entry - pointer to the current entry.
// Outputs      : void* - pointer to the previous entry.

void* xlist_prev(void* entry)
{
    return (void*)((PLIST_ENTRY)entry)->Blink;
}

//
// Debug functions

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xprintdbg
// Description  : Cross platform print kernel debug message function. Print a
//                debug message to the kernel debug output.
//
// Inputs       : format - format string.
// Outputs      : void

void xprintdbg(const char *format, ...)
{
    va_list args;
    _crt_va_start(args, format);
    vDbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, format, args);
}

