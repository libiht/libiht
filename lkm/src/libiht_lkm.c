////////////////////////////////////////////////////////////////////////////////
//
//  File           : lkm/src/libiht_lkm.c
//  Description    : This is the main implementation for the Linux kernel module
//                   of libiht. This module is used to capture the last branch
//                   records of a given process. This file is very platform
//                   specific.
//
//   Author        : Thomason Zhao
//   Last Modified : Dec 03, 2023
//

#include "../include/libiht_lkm.h"

//
// Module information

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Thomason Zhao");
MODULE_DESCRIPTION("Intel Hardware Trace Library - Linux Kernel Module");

//
// Context switch handlers

////////////////////////////////////////////////////////////////////////////////
//
// Function     : sched_in_handler
// Description  : This function is the handler for the context switch in event.
//                It will be called when a process is scheduled in.
//
// Inputs       : notifier - the notifier block
//                cpu - the cpu id
// Outputs      : void

void sched_in_handler(struct preempt_notifier *notifier, int cpu)
{
    xprintdbg(KERN_INFO "LIBIHT_LKM: sched_in_handler, pid: %d, cpu: %d\n",
                current->pid, smp_processor_id());
    // get_lbr(current->pid);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : sched_out_handler
// Description  : This function is the handler for the context switch out event.
//                It will be called when a process is scheduled out.
//
// Inputs       : notifier - the notifier block
//                next - the next process
// Outputs      : void

void sched_out_handler(struct preempt_notifier *notifier, 
                        struct task_struct *next)
{
    xprintdbg(KERN_INFO "LIBIHT_LKM: sched_out_handler, pid: %d, cpu: %d\n",
                current->pid, smp_processor_id());
    // put_lbr(current->pid);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : pre_cswitch_handler
// Description  : This function is the handler for the context switch event. It
//                will be called before a process is switched out.
//
// Inputs       : probe - the kprobe block
//                regs - the register block
// Outputs      : int - status of the handler. 0 if success, -1 if fail.

int __kprobes pre_cswitch_handler(struct kprobe *probe, struct pt_regs *regs)
{
    xprintdbg(KERN_INFO "LIBIHT_LKM: pre_sched_in_handler, pid: %d, cpu: %d\n",
                current->pid, smp_processor_id());
    // Dump registers
    xprintdbg(KERN_INFO "LIBIHT_LKM: current: %llx\n", current);
    xprintdbg(KERN_INFO "LIBIHT_LKM: rax: %llx\n", regs->ax);
    xprintdbg(KERN_INFO "LIBIHT_LKM: rbx: %llx\n", regs->bx);
    xprintdbg(KERN_INFO "LIBIHT_LKM: rcx: %llx\n", regs->cx);
    xprintdbg(KERN_INFO "LIBIHT_LKM: rdx: %llx\n", regs->dx);
    xprintdbg(KERN_INFO "LIBIHT_LKM: rsi: %llx\n", regs->si);
    xprintdbg(KERN_INFO "LIBIHT_LKM: rdi: %llx\n", regs->di);
    xprintdbg(KERN_INFO "LIBIHT_LKM: rbp: %llx\n", regs->bp);
    xprintdbg(KERN_INFO "LIBIHT_LKM: rsp: %llx\n", regs->sp);
    xprintdbg(KERN_INFO "LIBIHT_LKM: r8: %llx\n", regs->r8);
    xprintdbg(KERN_INFO "LIBIHT_LKM: r9: %llx\n", regs->r9);
    xprintdbg(KERN_INFO "LIBIHT_LKM: r10: %llx\n", regs->r10);
    xprintdbg(KERN_INFO "LIBIHT_LKM: r11: %llx\n", regs->r11);
    xprintdbg(KERN_INFO "LIBIHT_LKM: r12: %llx\n", regs->r12);
    xprintdbg(KERN_INFO "LIBIHT_LKM: r13: %llx\n", regs->r13);
    xprintdbg(KERN_INFO "LIBIHT_LKM: r14: %llx\n", regs->r14);
    xprintdbg(KERN_INFO "LIBIHT_LKM: r15: %llx\n", regs->r15);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : post_cswitch_handler
// Description  : This function is the handler for the context switch event. It
//                will be called after a process is switched in.
//
// Inputs       : probe - the kprobe block
//                regs - the register block
//                flags - the flags
// Outputs      : void

void __kprobes post_cswitch_handler(struct kprobe *probe, struct pt_regs *regs,
                                        unsigned long flags)
{
    xprintdbg(KERN_INFO "LIBIHT_LKM: post_sched_in_handler, pid: %d, cpu: %d\n",
                current->pid, smp_processor_id());
    // Dump registers
    xprintdbg(KERN_INFO "LIBIHT_LKM: current: %llx\n", current);
    xprintdbg(KERN_INFO "LIBIHT_LKM: rax: %llx\n", regs->ax);
    xprintdbg(KERN_INFO "LIBIHT_LKM: rbx: %llx\n", regs->bx);
    xprintdbg(KERN_INFO "LIBIHT_LKM: rcx: %llx\n", regs->cx);
    xprintdbg(KERN_INFO "LIBIHT_LKM: rdx: %llx\n", regs->dx);
    xprintdbg(KERN_INFO "LIBIHT_LKM: rsi: %llx\n", regs->si);
    xprintdbg(KERN_INFO "LIBIHT_LKM: rdi: %llx\n", regs->di);
    xprintdbg(KERN_INFO "LIBIHT_LKM: rbp: %llx\n", regs->bp);
    xprintdbg(KERN_INFO "LIBIHT_LKM: rsp: %llx\n", regs->sp);
    xprintdbg(KERN_INFO "LIBIHT_LKM: r8: %llx\n", regs->r8);
    xprintdbg(KERN_INFO "LIBIHT_LKM: r9: %llx\n", regs->r9);
    xprintdbg(KERN_INFO "LIBIHT_LKM: r10: %llx\n", regs->r10);
    xprintdbg(KERN_INFO "LIBIHT_LKM: r11: %llx\n", regs->r11);
    xprintdbg(KERN_INFO "LIBIHT_LKM: r12: %llx\n", regs->r12);
    xprintdbg(KERN_INFO "LIBIHT_LKM: r13: %llx\n", regs->r13);
    xprintdbg(KERN_INFO "LIBIHT_LKM: r14: %llx\n", regs->r14);
    xprintdbg(KERN_INFO "LIBIHT_LKM: r15: %llx\n", regs->r15);
}

//
// Fork handlers

////////////////////////////////////////////////////////////////////////////////
//
// Function     : pre_fork_handler
// Description  : This function is the handler for the fork event. It will be
//                called before a new process is forked.
//
// Inputs       : probe - the kprobe block
//                regs - the register block
// Outputs      : int - status of the handler. 0 if success, -1 if fail.

int __kprobes pre_fork_handler(struct kprobe *probe, struct pt_regs *regs)
{
    // Reserved for future use
    // xprintdbg(KERN_INFO "LIBIHT_LKM: pre_fork_handler, pid: %d\n",
    //             current->pid);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : post_fork_handler
// Description  : This function is the handler for the fork event. It will be
//                called after a new process is forked.
//
// Inputs       : prob - the kprobe block
//                regs - the register block
//                flags - the flags
// Outputs      : void

void __kprobes post_fork_handler(struct kprobe *prob, struct pt_regs *regs,
                                    unsigned long flags)
{
    struct task_struct *task;
    struct list_head *list;
    struct lbr_state *parent_state, *child_state;

    // xprintdbg(KERN_INFO "LIBIHT_LKM: post_fork_handler, pid: %d\n",
    //             current->pid);

    // Examine parent process
    parent_state = find_lbr_state(current->pid);
    if (parent_state == NULL)
        return;
    
    xprintdbg(KERN_INFO "LIBIHT_LKM: parent pid: %d\n", parent_state->pid);

    // Examine child process
    task = current;
    list = task->children.next;
    while (list != &task->children)
    {
        task = list_entry(list, struct task_struct, sibling);
        list = task->sibling.next;

        child_state = find_lbr_state(task->pid);
        if (child_state == NULL)
            continue;

        xprintdbg(KERN_INFO "LIBIHT_LKM: child pid: %d\n", child_state->pid);

        // Set the field
        child_state->lbr_select = parent_state->lbr_select;
        child_state->parent = parent_state;
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
        state = create_lbr_state();
        if (state == NULL)
        {
            xprintdbg("LIBIHT-LKM: create lbr_state failed\n");
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

void lookup_tracepoints(struct tracepoint *tp, void *ignore) {
    int i;
    for (i = 0; i < sizeof(traces) / sizeof(struct tracepoint_table); i++) {
        if (strcmp(traces[i].name, tp->name) == 0) traces[i].tp = tp;
    }
}

void init_tracepoints(void) {
    int i;

    for_each_kernel_tracepoint(lookup_tracepoints, NULL);

    for (i = 0; i < sizeof(traces) / sizeof(struct tracepoint_table); i++) {
        if (traces[i].tp) {
            xprintdbg(KERN_INFO "LIBIHT_LKM: Registering tracepoint %s\n", traces[i].name);
            tracepoint_probe_register(traces[i].tp, traces[i].func, NULL);
        }
    }
}

void clear_tracepoints(void) {
    int i;
    for (i = 0; i < sizeof(traces) / sizeof(struct tracepoint_table); i++) {
        if (traces[i].tp) {
            tracepoint_probe_unregister(traces[i].tp, traces[i].func, NULL);
            traces[i].tp = NULL;
        }
    }
}

void tp_sched_switch_handler(void *data, bool preempt,
                                    struct task_struct *prev,
                                    struct task_struct *next)
{
    xprintdbg(KERN_INFO "Context switch: %s[%d] -> %s[%d]\n",
           prev->comm, prev->pid, next->comm, next->pid);
}

void tp_new_task_handler(void *data, struct task_struct *task)
{
    xprintdbg(KERN_INFO "New task: %s[%d]\n", task->comm, task->pid);
    // Print parent
    if (task->parent)
        xprintdbg(KERN_INFO "Parent: %s[%d]\n", task->parent->comm, task->parent->pid);
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
    proc_entry = proc_create("libiht-info", 0666, NULL, &libiht_ops);
    if (proc_entry == NULL) {
        xprintdbg(KERN_INFO "LIBIHT-LKM: Create proc failed\n");
        return -1;
    }

    // Register kprobe hooks on fork system call
    xprintdbg(KERN_INFO "LIBIHT_LKM: Registering fork kprobe...\n");
    // if (register_kprobe(&fork_kprobe)) {
    //     xprintdbg(KERN_INFO "LIBIHT_LKM: Register fork kprobe failed\n");
    //     proc_remove(proc_entry);
    //     return -1;
    // }

    // Register kprobe hooks on context switches
    xprintdbg(KERN_INFO "LIBIHT_LKM: Registering context switch hooks...\n");
    // if (register_kprobe(&cswitch_kprobe)) {
    //     xprintdbg(KERN_INFO "LIBIHT_LKM: Register cswitch kprobe failed\n");
    //     unregister_kprobe(&fork_kprobe);
    //     proc_remove(proc_entry);
    //     return -1;
    // }
    // preempt_notifier_init(&cswitch_notifier, &cswitch_ops);
    // preempt_notifier_register(&cswitch_notifier);

    // Register tracepoints
    xprintdbg(KERN_INFO "LIBIHT_LKM: Registering tracepoints...\n");
    init_tracepoints();


    // Init LBR
    lbr_init();

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
    clear_tracepoints();

    // Unregister kprobe hooks on context switches
    xprintdbg(KERN_INFO "LIBIHT_LKM: Unregistering cswitch hooks...\n");
    // preempt_notifier_unregister(&cswitch_notifier);
    // unregister_kprobe(&cswitch_kprobe);

    // Unregister kprobe hooks on fork system call
    xprintdbg(KERN_INFO "LIBIHT_LKM: Unregistering fork kprobe...\n");
    // unregister_kprobe(&fork_kprobe);

    // Remove the helper process if exist
    xprintdbg(KERN_INFO "LIBIHT_LKM: Removing helper process...\n");
    if (proc_entry != NULL)
        proc_remove(proc_entry);
    
    xprintdbg(KERN_INFO "LIBIHT_LKM: Exit complete\n");
}

module_init(libiht_lkm_init);
module_exit(libiht_lkm_exit);