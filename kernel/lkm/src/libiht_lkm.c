////////////////////////////////////////////////////////////////////////////////
//
//  File           : kernel/lkm/src/libiht_lkm.c
//  Description    : This is the main implementation for the Linux kernel module
//                   of libiht. This module is used to capture the last branch
//                   records of a given process. This file is very platform
//                   specific.
//
//   Author        : Thomason Zhao
//   Last Modified : July 10, 2024
//

#include "../include/libiht_lkm.h"

//
// Module information

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Thomason Zhao");
MODULE_DESCRIPTION("Intel Hardware Trace Library - Linux Kernel Module");

//
// Tracepoint table helpers

////////////////////////////////////////////////////////////////////////////////
//
// Function     : lookup_tracepoints
// Description  : This function is used to lookup tracepoints.
//
// Inputs       : tp - the tracepoint
//                ignore - the ignore pointer
// Outputs      : void

void lookup_tracepoints(struct tracepoint *tp, void *ignore) {
    int i;

    for (i = 0; i < sizeof(traces) / sizeof(struct tracepoint_table); i++) {
        // xprintdbg(KERN_INFO "LIBIHT_LKM: Lookup tracepoint: %s\n", tp->name);
        if (strcmp(traces[i].name, tp->name) == 0)
            traces[i].tp = tp;
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : register_tracepoints
// Description  : This function is used to register tracepoints.
//
// Inputs       : void
// Outputs      : void

void register_tracepoints(void) {
    int i;

    for_each_kernel_tracepoint(lookup_tracepoints, NULL);

    // Register tracepoint handlers in the table
    for (i = 0; i < sizeof(traces) / sizeof(struct tracepoint_table); i++) {
        if (traces[i].tp) {
            xprintdbg(KERN_INFO "LIBIHT_LKM: Registering tracepoint %s\n", traces[i].name);
            tracepoint_probe_register(traces[i].tp, traces[i].func, NULL);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : unregister_tracepoints
// Description  : This function is used to unregister tracepoints.
//
// Inputs       : void
// Outputs      : void

void unregister_tracepoints(void) {
    int i;

    // Unregister tracepoint handlers in the table
    for (i = 0; i < sizeof(traces) / sizeof(struct tracepoint_table); i++) {
        if (traces[i].tp) {
            tracepoint_probe_unregister(traces[i].tp, traces[i].func, NULL);
            traces[i].tp = NULL;
        }
    }
}

//
// Tracepoint handlers

////////////////////////////////////////////////////////////////////////////////
//
// Function     : tp_sched_switch_handler
// Description  : This function is the handler for the sched_switch event. It
//                will be called when a process is switched in.
//
// Inputs       : data - the data
//                preempt - the preempt flag
//                prev_task - the previous process
//                next_task - the next_task process
// Outputs      : void

void tp_sched_switch_handler(void *data, bool preempt,
                                    struct task_struct *prev_task,
                                    struct task_struct *next_task)
{
    lbr_cswitch_handler(prev_task->pid, next_task->pid);
    bts_cswitch_handler(prev_task->pid, next_task->pid);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : tp_new_task_handler
// Description  : This function is the handler for the new_task event. It will
//                be called when a new process is created.
//
// Inputs       : data - the data
//                task - the task
// Outputs      : void

void tp_new_task_handler(void *data, struct task_struct *task)
{
    lbr_newproc_handler(task->real_parent->pid, task->pid);
    bts_newproc_handler(task->real_parent->pid, task->pid);
}

//
// Device proc handlers

////////////////////////////////////////////////////////////////////////////////
//
// Function     : device_open
// Description  : This function is used to handle open request for the device
//                process.
//
// Inputs       : inode - the inode
//                file_ptr - the file pointer
// Outputs      : int - status of the open. 0 if success, -1 if fail.

int device_open(struct inode *inode, struct file *file_ptr)
{
    // Reserved for future use
    xprintdbg(KERN_INFO "LIBIHT_LKM: device_open\n");
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : device_release
// Description  : This function is used to handle close request for the device
//                process.
//
// Inputs       : inode - the inode
//                file_ptr - the file pointer
// Outputs      : int - status of the close. 0 if success, -1 if fail.

int device_release(struct inode *inode, struct file *file_ptr)
{
    // Reserved for future use
    xprintdbg(KERN_INFO "LIBIHT_LKM: device_release\n");
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : device_read
// Description  : This function is used to read handle request for the device
//                process.
//
// Inputs       : file_ptr - the file pointer
//                buffer - the buffer
//                length - the length
//                offset - the offset
// Outputs      : ssize_t - the size of the read

ssize_t device_read(struct file *file_ptr, char *buffer, size_t length,
                        loff_t *offset)
{
    // Reserved for future use
    xprintdbg(KERN_INFO "LIBIHT_LKM: device_read\n");
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : device_write
// Description  : This function is used to write handle request for the device
//                process.
//
// Inputs       : file_ptr - the file pointer
//                buffer - the buffer
//                length - the length
//                offset - the offset
// Outputs      : ssize_t - the size of the write

ssize_t device_write(struct file *file_ptr, const char *buffer, size_t length,
                        loff_t *offset)
{
    // Reserved for future use
    xprintdbg(KERN_INFO "LIBIHT_LKM: device_write\n");
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : device_ioctl
// Description  : This function is used to handle ioctl request for the device
//                process.
//
// Inputs       : file_ptr - the file pointer
//                ioctl_num - the ioctl number
//                ioctl_param - the ioctl parameter
// Outputs      : long - the status of the ioctl

long device_ioctl(struct file *file_ptr, unsigned int ioctl_cmd,
                    unsigned long ioctl_param)
{
    struct xioctl_request request;
    unsigned long request_size_left;
    long ret_val = 0;

    // Copy user request
    request_size_left = copy_from_user(&request, 
                        (struct xioctl_request *)ioctl_param,
                        sizeof(struct xioctl_request));
    if (request_size_left != 0)
    {
        // Partial copy
        xprintdbg(KERN_INFO "LIBIHT-LKM: Remaining size %ld\n", request_size_left);
        return -EIO;
    }

    // Process request
    if (request.cmd <= LIBIHT_IOCTL_LBR_END)
    {
        // LBR request
        xprintdbg(KERN_INFO "LIBIHT-LKM: LBR request\n");
        ret_val = lbr_ioctl_handler(&request);
    }
    else if (request.cmd <= LIBIHT_IOCTL_BTS_END)
    {
        // BTS request
        xprintdbg(KERN_INFO "LIBIHT-LKM: BTS request\n");
        ret_val = bts_ioctl_handler(&request);
    }
    else
    {
        // Unknown request
        xprintdbg(KERN_INFO "LIBIHT-LKM: Unknown request\n");
        ret_val = -EINVAL;
    }

    return ret_val;
}

//
// Module initialization and cleanup functions

////////////////////////////////////////////////////////////////////////////////
//
// Function     : libiht_lkm_init
// Description  : This function is the main entry point for the module. It will
//                initialize the driver and register all the required services.

//
// Inputs       : void
// Outputs      : int - status of the initialization. 0 if success, -1 if fail.

int __init libiht_lkm_init(void)
{
    xprintdbg(KERN_INFO "LIBIHT_LKM: Initializing...\n");

    // Create user interactive helper process
    xprintdbg(KERN_INFO "LIBIHT-LKM: Creating helper process...\n");
    proc_entry = proc_create(DEVICE_NAME, 0666, NULL, &libiht_ops);
    if (proc_entry == NULL) {
        xprintdbg(KERN_INFO "LIBIHT-LKM: Create proc failed\n");
        return -1;
    }

    // Register tracepoint hooks for context swtich and fork
    xprintdbg(KERN_INFO "LIBIHT_LKM: Registering tracepoints...\n");
    register_tracepoints();

    // Init LBR
    xprintdbg(KERN_INFO "LIBIHT_LKM: Initilizing LBR...\n");
    lbr_init();

    // Init BTS
    xprintdbg(KERN_INFO "LIBIHT_LKM: Initilizing BTS...\n");
    bts_init();

    xprintdbg(KERN_INFO "LIBIHT_LKM: Initilized\n");
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : libiht_lkm_exit
// Description  : This function is the main exit point for the module. It will
//                unregister all the registered services and clean up the
//                module.
//
// Inputs       : void
// Outputs      : void

void __exit libiht_lkm_exit(void)
{
    xprintdbg(KERN_INFO "LIBIHT_LKM: Exiting...\n");

    // Exit BTS
    xprintdbg(KERN_INFO "LIBIHT_LKM: Exiting BTS...\n");
    bts_exit();

    // Exit LBR
    xprintdbg(KERN_INFO "LIBIHT_LKM: Exiting LBR...\n");
    lbr_exit();

    // Unregister tracepoints
    xprintdbg(KERN_INFO "LIBIHT_LKM: Unregistering tracepoints...\n");
    unregister_tracepoints();

    // Remove the helper process if exist
    xprintdbg(KERN_INFO "LIBIHT_LKM: Removing helper process...\n");
    if (proc_entry != NULL)
        proc_remove(proc_entry);

    xprintdbg(KERN_INFO "LIBIHT_LKM: Exit complete\n");
}

module_init(libiht_lkm_init);
module_exit(libiht_lkm_exit);