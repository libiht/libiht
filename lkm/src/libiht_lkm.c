////////////////////////////////////////////////////////////////////////////////
//
//  File           : lkm/src/libiht_lkm.c
//  Description    : This is the main implementation for the Linux kernel module
//                   of libiht. This module is used to capture the last branch
//                   records of a given process. This file is very platform
//                   specific.
//
//   Author        : Thomason Zhao
//   Last Modified : Dec 25, 2023
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
//                prev - the previous process
//                next - the next process
// Outputs      : void

void tp_sched_switch_handler(void *data, bool preempt,
                                    struct task_struct *prev,
                                    struct task_struct *next)
{
    // xprintdbg(KERN_INFO "LIBIHT_LKM: Context switch: %s[%d] -> %s[%d]\n",
    //        prev->comm, prev->pid, next->comm, next->pid);

    // Dump/restore registers
    get_lbr(prev->pid);
    put_lbr(next->pid);
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
    struct lbr_state *parent_state, *child_state;

    // Check if parent is enabled
    parent_state = find_lbr_state(task->real_parent->pid);
    if (parent_state == NULL)
        // Parent is not enabled
        return;
    
    xprintdbg(KERN_INFO "LIBIHT_LKM: Process %s[%d] created, parent %s[%d]\n", 
                        task->comm, task->pid, 
                        task->real_parent->comm, task->real_parent->pid);

    // Check if child is enabled
    child_state = create_lbr_state();
    if (child_state != NULL) {
        // Child is enabled
        child_state->lbr_select = parent_state->lbr_select;
        child_state->pid = task->pid;
        child_state->parent = parent_state;
        insert_lbr_state(child_state);
    }
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
    struct lbr_state *state;
    struct ioctl_request request;
    unsigned long request_size_left;
    long ret_val = 0;

    // Copy user request
    request_size_left = copy_from_user(&request, (struct ioctl_request *)ioctl_param, sizeof(struct ioctl_request));
    if (request_size_left != 0)
    {
        // Partial copy
        xprintdbg(KERN_INFO "LIBIHT-LKM: Remaining size %ld\n", request_size_left);
        return -EIO;
    }

    xprintdbg(KERN_INFO "LIBIHT-LKM: Got ioctl argument %#x!", ioctl_cmd);
    xprintdbg(KERN_INFO "LIBIHT-LKM: request select bits: %lld", request.lbr_select);
    xprintdbg(KERN_INFO "LIBIHT-LKM: request pid: %d", request.pid);

    switch (ioctl_cmd)
    {
    case LIBIHT_LKM_IOC_ENABLE_TRACE:
        xprintdbg(KERN_INFO "LIBIHT-LKM: ENABLE_TRACE\n");
        // Enable trace for assigned process
        state = find_lbr_state(request.pid);
        if (state)
        {
            xprintdbg("LIBIHT-LKM: Process %d already enabled\n", request.pid);
            ret_val = -EINVAL;
            break;
        }
        state = create_lbr_state();
        if (state == NULL)
        {
            xprintdbg("LIBIHT-KMD: create lbr_state failed\n");
            ret_val = -EINVAL;
            break;
        }

        // Set the field
        state->lbr_select = request.lbr_select ? request.lbr_select : LBR_SELECT;
        state->pid = request.pid ? request.pid : current->pid;
        state->parent = NULL;

        insert_lbr_state(state);
        break;

    case LIBIHT_LKM_IOC_DISABLE_TRACE:
        xprintdbg(KERN_INFO "LIBIHT-LKM: DISABLE_TRACE\n");
        // Disable trace for assigned process (and its children)
        state = find_lbr_state(request.pid);
        if (state == NULL)
        {
            xprintdbg("LIBIHT-LKM: find lbr_state failed\n");
            ret_val = -EINVAL;
            break;
        }

        remove_lbr_state(state);
        break;

    case LIBIHT_LKM_IOC_DUMP_LBR:
        xprintdbg(KERN_INFO "LIBIHT-LKM: DUMP_LBR\n");
        // Dump LBR info for assigned process
        dump_lbr(request.pid);
        break;

    case LIBIHT_LKM_IOC_SELECT_LBR:
        xprintdbg(KERN_INFO "LIBIHT-LKM: SELECT_LBR\n");
        // Update the select bits for assigned process
        state = find_lbr_state(request.pid);
        if (state == NULL)
        {
            xprintdbg("LIBIHT-LKM: find lbr_state failed\n");
            ret_val = -EINVAL;
            break;
        }

        state->lbr_select = request.lbr_select;
        break;

    default:
        // Error command code
        xprintdbg(KERN_INFO "LIBIHT-LKM: Error IOCTL command\n");
        ret_val = -EINVAL;
        break;
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

    // Check availability of the CPU
    if(identify_cpu() < 0) {
        xprintdbg(KERN_ERR "LIBIHT_LKM: Identify CPU failed\n");
        return -1;
    }

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
    lbr_init();

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

    // Exit LBR
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