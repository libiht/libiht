#ifndef _LIBIHT_LKM_H
#define _LIBIHT_LKM_H

////////////////////////////////////////////////////////////////////////////////
//
//  File           : lkm/include/libiht_lkm.h
//  Description    : This is the header file for the Linux kernel module of
//                   libiht.
//
//   Author        : Thomason Zhao
//   Last Modified : Dec 03, 2023

//
// Include Files
#include "headers_lkm.h"
#include "../../commons/lbr.h"
#include "../../commons/cpu.h"
#include "../../commons/types.h"
#include "../../commons/debug.h"

//
// Library constants

// Check Linux kernel version.
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#define HAVE_PROC_OPS
#endif


// I/O control table
#define LIBIHT_LKM_IOC_MAGIC 'l'
#define LIBIHT_LKM_IOC_ENABLE_TRACE     _IO(LIBIHT_LKM_IOC_MAGIC, 1)
#define LIBIHT_LKM_IOC_DISABLE_TRACE    _IO(LIBIHT_LKM_IOC_MAGIC, 2)
#define LIBIHT_LKM_IOC_DUMP_LBR         _IO(LIBIHT_LKM_IOC_MAGIC, 3)
#define LIBIHT_LKM_IOC_SELECT_LBR       _IO(LIBIHT_LKM_IOC_MAGIC, 4)

//
// Type definitions

// IOCTL request structure
struct ioctl_request{
    uint64_t lbr_select;
    pid_t pid;
};

//
// Global variables

struct proc_dir_entry *proc_entry;

//
// Function prototypes

void sched_in_handler(struct preempt_notifier *notifier, int cpu);
// Function hook for task scheduling in.

void sched_out_handler(struct preempt_notifier *notifier, 
                        struct task_struct *next);
// Function hook for task scheduling out.

int __kprobes pre_fork_handler(struct kprobe *probe, struct pt_regs *regs);
// Function hook before forking a new process.

void __kprobes post_fork_handler(struct kprobe *prob, struct pt_regs *regs,
                                    unsigned long flags);
// Function hook after forking a new process.

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

// Structures for installing the context switch hooks.
static struct preempt_notifier cswitch_notifier;
static struct preempt_ops cswitch_ops = {
    .sched_in = sched_in_handler,
    .sched_out = sched_out_handler};

// Structures for installing the syscall (fork) hooks.
static struct kprobe fork_kprobe = {
    .symbol_name = "kernel_clone",
    .pre_handler = pre_fork_handler,
    .post_handler = post_fork_handler};

#endif // _LIBIHT_LKM_H