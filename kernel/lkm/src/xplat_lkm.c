////////////////////////////////////////////////////////////////////////////////
//
//  File           : lkm/src/xplat_lkm.c
//  Description    : This is the cross-platform compatibility layer for the
//                   Linux kernel module.
//
//   Author        : Thomason Zhao
//   Last Modified : Jan 15, 2023
//

#include "../../commons/xplat.h"
#include "../include/headers_lkm.h"
#include <linux/uaccess.h>

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
// Outputs      : void * - pointer to the allocated memory.

void *xmalloc(u64 size)
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
// Outputs      : void * - pointer to the set memory.

void *xmemset(void *ptr, s32 c, u64 cnt)
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
// Outputs      : void * - pointer to the destination memory.

void *xmemcpy(void *dst, void *src, u64 cnt)
{
    return memcpy(dst, src, cnt);
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
// Function     : xrelease_core
// Description  : Cross platform unlock core function. Restore IRQL to
//                the new IRQL.
//
// Inputs       : new_irql - pointer to the new IRQL.
// Outputs      : void

void xrelease_core(void *new_irql)
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
    INIT_LIST_HEAD((struct list_head *)list);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xlist_add
// Description  : Cross platform list add function. Add a new entry after the
//                specified head.
//
// Inputs       : new - pointer to the new entry.
//                head - pointer to the list head.
// Outputs      : void

void xlist_add(void *new_entry, void *head)
{
    list_add((struct list_head *)new_entry, (struct list_head *)head);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xlist_del
// Description  : Cross platform list del function. Delete an entry from the
//                list.
//
// Inputs       : entry - pointer to the entry to be deleted.
// Outputs      : void

void xlist_del(void *entry)
{
    list_del((struct list_head *)entry);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xlist_next
// Description  : Cross platform list next function. Get the next entry in the
//                list. (Need to manipulate the pointer to the entry manually)
//
// Inputs       : entry - pointer to the entry.
// Outputs      : void* - pointer to the next entry.

void *xlist_next(void *entry)
{
    return (void *)((struct list_head *)entry)->next;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xlist_prev
// Description  : Cross platform list prev function. Get the previous entry in
//                the list. (Need to manipulate the pointer to the entry 
//                manually)
//
// Inputs       : entry - pointer to the entry.
// Outputs      : void* - pointer to the previous entry.

void *xlist_prev(void *entry)
{
    return (void *)((struct list_head *)entry)->prev;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xcopy_from_user
// Description  : Similar to the copy_from_user in ubuntu
//
// Inputs       : to - the target address (kernel space).
//                from - the source address (user space).
//                n - the lenght of content.
// Outputs      : u32 - return 0 if success

u32 xcopy_from_user(void *to, const void *from, u32 n){
    return copy_from_user(to, (const void __user *) from, n);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : xcopy_to_user
// Description  : Similar to the copy_to_user in ubuntu
//
// Inputs       : to - the source address (user space).
//                from - the source address (kernel space).
//                n - the lenght of content.
// Outputs      : u32 - return 0 if success

u32 xcopy_to_user(void *to, const void *from, u32 n){
    return copy_to_user((void __user *) to, from, n);
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

