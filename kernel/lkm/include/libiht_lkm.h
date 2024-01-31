#ifndef _LIBIHT_LKM_H
#define _LIBIHT_LKM_H

////////////////////////////////////////////////////////////////////////////////
//
//  File           : lkm/include/libiht_lkm.h
//  Description    : This is the header file for the Linux kernel module of
//                   libiht.
//
//   Author        : Thomason Zhao
//   Last Modified : Jan 10, 2023

//
// Include Files
#include "headers_lkm.h"
#include "../../commons/lbr.h"
#include "../../commons/bts.h"
#include "../../commons/types.h"
#include "../../commons/debug.h"

//
// Library constants

// Check Linux kernel version.
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#define HAVE_PROC_OPS
#endif

// Device name
#define DEVICE_NAME "libiht-info"

// I/O control macros
#define LIBIHT_LKM_IOCTL_MAGIC 'l'
#define LIBIHT_LKM_IOCTL_BASE       _IO(LIBIHT_LKM_IOCTL_MAGIC, 0)

//
// Type definitions

// Tracepoint table
struct tracepoint_table
{
    const char *name;
    void *func;
    struct tracepoint *tp;
};

//
// Global variables

struct proc_dir_entry *proc_entry;

//
// Function prototypes

void lookup_tracepoints(struct tracepoint *tp, void *ignore);
// This function is used to lookup tracepoints.

void register_tracepoints(void);
// This function is used to register tracepoints.

void unregister_tracepoints(void);
// This function is used to unregister tracepoints.

void tp_sched_switch_handler(void *data, bool preempt,
                                struct task_struct *prev,
                                struct task_struct *next);
// This function is called when the sched_switch tracepoint is hit.

void tp_new_task_handler(void *data, struct task_struct *task);
// This function is called when the task_newtask tracepoint is hit.

int device_open(struct inode *inode, struct file *file_ptr);
// This function is used to open the device.

int device_release(struct inode *inode, struct file *file_ptr);
// This function is used to close the device.

ssize_t device_read(struct file *file_ptr, char *buffer, size_t length,
                        loff_t *offset);
// This function is used to read from the device.

ssize_t device_write(struct file *file_ptr, const char *buffer, size_t length,
                        loff_t *offset);
// This function is used to write to the device.

long device_ioctl(struct file *file_ptr, unsigned int ioctl_cmd,
                    unsigned long ioctl_param);
// This function is used to handle IOCTL requests.

int __init libiht_lkm_init(void);
// This function is called when the module is loaded.

void __exit libiht_lkm_exit(void);
// This function is called when the module is unloaded.

//
// Global variables require function prototypes
// Due to differnt kernel version, determine which struct going to use
#ifdef HAVE_PROC_OPS
static struct proc_ops libiht_ops = {
    .proc_open = device_open,
    .proc_release = device_release,
    .proc_read = device_read,
    .proc_write = device_write,
    .proc_ioctl = device_ioctl};
#else
static struct file_operations libiht_ops = {
    .open = device_open,
    .release = device_release,
    .read = device_read,
    .write = device_write,
    .unlocked_ioctl = device_ioctl};
#endif

// Structures for installing the tracepoint hooks.
struct tracepoint_table traces[] = {
    {.name = "sched_switch", .func = tp_sched_switch_handler},
    {.name = "task_newtask", .func = tp_new_task_handler}
};


#endif // _LIBIHT_LKM_H