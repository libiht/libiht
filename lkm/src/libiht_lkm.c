#include <linux/cpumask.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/preempt.h>
#include <linux/ioctl.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <asm/msr.h>
#include <asm/processor.h>

#include "../include/libiht_lkm.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Thomason Zhao");
MODULE_DESCRIPTION("Intel Hardware Trace Library - Linux Kernel Module");

/************************************************
 * Global variables
 ************************************************/

/*
 * Due to differnt kernel version, determine which struct going to use
 */
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
    .release = device_release
                   .read = device_read,
    .write = device_write,
    .unlocked_ioctl = device_ioctl};
#endif

/*
 * Structures for installing the context switch hooks.
 */
static struct preempt_notifier notifier;
static struct preempt_ops ops = {
    .sched_in = sched_in,
    .sched_out = sched_out};

static struct lbr_state *lbr_state_list;
uint64_t lbr_capacity;
static spinlock_t lbr_cache_lock;
static struct proc_dir_entry *proc_entry;

/************************************************
 * LBR helper functions
 *
 * Help to manage LBR stack/registers
 ************************************************/
/*
 * Flush the LBR registers. Caller should ensure this function run on
 * single cpu (by wrapping get_cpu() and put_cpu())
 */
static void flush_lbr(bool enable)
{
    int i;

    wrmsrl(MSR_LBR_TOS, 0);
    for (i = 0; i < lbr_capacity; i++)
    {
        wrmsrl(MSR_LBR_NHM_FROM + i, 0);
        wrmsrl(MSR_LBR_NHM_TO + i, 0);
    }

    if (enable)
        wrmsrl(MSR_IA32_DEBUGCTLMSR, DEBUGCTLMSR_LBR);
    else
        wrmsrl(MSR_IA32_DEBUGCTLMSR, 0);
}

/*
 * Store the LBR registers to kernel maintained datastructure
 */
static void get_lbr(void)
{
    int i;

    rdmsrl(MSR_IA32_DEBUGCTLMSR, lbr_state_list.lbr_ctl);
    rdmsrl(MSR_LBR_SELECT, lbr_state_list.lbr_select);
    rdmsrl(MSR_LBR_TOS, lbr_state_list.lbr_tos);

    for (i = 0; i < lbr_capacity; i++)
    {
        rdmsrl(MSR_LBR_NHM_FROM + i, lbr_state_list.entries[i].from);
        rdmsrl(MSR_LBR_NHM_TO + i, lbr_state_list.entries[i].to);
    }
}

/*
 * Write the LBR registers from kernel maintained datastructure
 */
static void put_lbr(void)
{
    int i;

    wrmsrl(MSR_IA32_DEBUGCTLMSR, lbr_state_list.lbr_ctl);
    wrmsrl(MSR_LBR_SELECT, lbr_state_list.lbr_select);
    wrmsrl(MSR_LBR_TOS, lbr_state_list.lbr_tos);

    for (i = 0; i < lbr_capacity; i++)
    {
        wrmsrl(MSR_LBR_NHM_FROM + i, lbr_state_list.entries[i].from);
        wrmsrl(MSR_LBR_NHM_TO + i, lbr_state_list.entries[i].to);
    }
}

/*
 * Dump out the LBR registers to kernel message
 */
static void dump_lbr(void)
{
    int i;

    get_cpu();
    if (lbr_state_list.pid == current->pid)
        get_lbr();

    printk(KERN_INFO "MSR_IA32_DEBUGCTLMSR: 0x%llx\n", lbr_state_list.lbr_ctl);
    printk(KERN_INFO "MSR_LBR_SELECT:       0x%llx\n", lbr_state_list.lbr_select);
    printk(KERN_INFO "MSR_LBR_TOS:          %lld\n", lbr_state_list.lbr_tos);

    for (i = 0; i < lbr_capacity; i++)
    {
        printk(KERN_INFO "MSR_LBR_NHM_FROM[%2d]: 0x%llx\n", i, lbr_state_list.entries[i].from);
        printk(KERN_INFO "MSR_LBR_NHM_TO  [%2d]: 0x%llx\n", i, lbr_state_list.entries[i].to);
    }

    printk(KERN_INFO "LIBIHT-LKM: LBR info for cpuid: %d\n", smp_processor_id());

    put_cpu();
}

/*
 * Enable the LBR feature for the current CPU. *info may be NULL (it is required
 * by on_each_cpu()).
 */
static void enable_lbr(void *info)
{

    get_cpu();

    printk(KERN_INFO "LIBIHT-LKM: Enable LBR on cpu core: %d\n", smp_processor_id());

    /* Apply the filter (what kind of branches do we want to track?) */
    // wrmsrl(MSR_LBR_SELECT, lbr_state_list.lbr_select);

    /* Flush the LBR and enable it */
    flush_lbr(true);

    put_cpu();
}

/*
 * Disable the LBR feature for the current CPU. *info may be NULL (it is required
 * by on_each_cpu()).
 */
static void disable_lbr(void *info)
{

    get_cpu();

    printk(KERN_INFO "LIBIHT-LKM: Disable LBR on cpu core: %d\n", smp_processor_id());

    /* Remove the filter */
    wrmsrl(MSR_LBR_SELECT, 0);

    /* Flush the LBR and disable it */
    flush_lbr(false);

    put_cpu();
}

/************************************************
 * LBR state helper functions
 *
 * Help to manage kernel LBR state datastructure
 ************************************************/
/*
 * Insert new LBR state into the back of the list
 */
void insert_lbr_state(struct lbr_state *new_state)
{
    struct lbr_state *head = lbr_state_list; 
    if (head == NULL)
    {
        new_state->prev = NULL;
        new_state->next = NULL;
        head = new_state;
    }
    else
    {
        head->prev->next = new_state;
        new_state->prev = head->prev;
        head->prev = new_state;
        new_state->next = head;
    }
}

/*
 * Find the LBR state by given the pid. (Try to do as fast as possiable)
 */
struct lbr_state *find_lbr_state(pid_t pid)
{
    // Perform a backward traversal (typically, newly created processes are
    // more likely to be find)
    struct lbr_state *tmp = lbr_state_list;
    while (tmp != NULL)
    {
        if (tmp->pid == pid)
            return tmp;
        tmp = tmp->prev;
    }

    return NULL;
}

/************************************************
 * Save and Restore the LBR during context switches.
 *
 * Should be done as fast as possiable to minimize the overhead of context switches.
 ************************************************/

/*
 * Save LBR
 */
static void save_lbr(void)
{
    unsigned long lbr_cache_flags;

    // Save when target process being preempted
    printk(KERN_INFO "LIBIHT-LKM: Leave, saving LBR status for pid: %d\n", current->pid);
    spin_lock_irqsave(&lbr_cache_lock, lbr_cache_flags);
    get_lbr();
    spin_unlock_irqrestore(&lbr_cache_lock, lbr_cache_flags);
}

/*
 * Restore LBR
 */
static void restore_lbr(void)
{
    unsigned long lbr_cache_flags;

    // Restore when target process being rescheduled
    if (lbr_state_list.pid == current->pid)
    {
        printk(KERN_INFO "LIBIHT-LKM: Enter, restoring LBR status for pid: %d\n", current->pid);
        spin_lock_irqsave(&lbr_cache_lock, lbr_cache_flags);
        put_lbr();
        spin_unlock_irqrestore(&lbr_cache_lock, lbr_cache_flags);
    }
}

/************************************************
 * Context switch hook functions
 ************************************************/

/*
 * Entering into scheduler
 */
static void sched_in(struct preempt_notifier *pn, int cpu)
{
    restore_lbr();
}

/*
 * Exiting from scheduler
 */
static void sched_out(struct preempt_notifier *pn, struct task_struct *next)
{
    save_lbr();
}

/************************************************
 * Device hook functions
 *
 * Maintain functionality of the libiht-info helper process
 ************************************************/

/*
 * Hooks for opening the device
 */
static int device_open(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "LIBIHT-LKM: Device opened.\n");
    return 0;
}

/*
 * Hooks for releasing the device
 */
static int device_release(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "LIBIHT-LKM: Device closed.\n");
    return 0;
}

/*
 * Hooks for reading the device
 */
static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
    printk(KERN_INFO "LIBIHT-LKM: Device read.\n");
    dump_lbr();
    return 0;
}

/*
 * Hooks for writting the device
 */
static ssize_t device_write(struct file *filp, const char *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "LIBIHT-LKM: doesn't support write.\n");
    return -EINVAL;
}

/*
 * Hooks for I/O controling the device
 */
static long device_ioctl(struct file *filp, unsigned int ioctl_cmd, unsigned long ioctl_param)
{
    printk(KERN_INFO "LIBIHT-LKM: Got ioctl argument %#x!", ioctl_cmd);

    switch (ioctl_cmd)
    {
    case LIBIHT_IOC_INIT_LBR:
        // Initialize LBR feature, auto trace current process
        struct lbr_state *state = kmalloc(sizeof(struct lbr_state), GFP_KERNEL);
        if (state == NULL)
            return -EINVAL;

        memset(state, 0, sizeof(struct lbr_state));
        state->lbr_select = LBR_SELECT;
        state->pid = current->pid;

        insert_lbr_state(state);

        // Set the select bit to default
        state->lbr_select = LBR_SELECT;
        state->pid = current->pid;
        wrmsrl(MSR_LBR_SELECT, state->lbr_select);

        // Register preemption hooks
        printk(KERN_INFO "LIBIHT-LKM: Applying preemption hook\n");
        preempt_notifier_inc();
        preempt_notifier_register(&notifier);
        break;

    case LIBIHT_IOC_ENABLE_LBR:
        wrmsrl(MSR_IA32_DEBUGCTLMSR, DEBUGCTLMSR_LBR);
        break;

    case LIBIHT_IOC_DISABLE_LBR:
        wrmsrl(MSR_IA32_DEBUGCTLMSR, 0);
        break;

    case LIBIHT_IOC_DUMP_LBR:
        dump_lbr();
        break;

    case LIBIHT_IOC_SELECT_LBR:
        // Update select bits
        lbr_state_list.lbr_select = ioctl_param;
        on_each_cpu(enable_lbr, NULL, 1);
        break;

    default:
        // Error command code
        return -EINVAL;
    }

    return 0;
}

/************************************************
 * Kernel module functions
 ************************************************/

static int identify_cpu(void)
{
    unsigned int info;
    unsigned int ext_model;
    unsigned int model;
    unsigned int family;
    register unsigned int eax asm("eax");

    asm(
        "push %rbx;"
        "push %rcx;"
        "push %rdx;"
        "movl $1, %eax;"
        "cpuid;"
        "pop %rdx;"
        "pop %rcx;"
        "pop %rbx;");

    info = eax;
    family = (info >> 8) & 0xf;

    // Identify CPU family
    if (family != 6)
        return -1;

    model = (info >> 4) & 0xf;
    ext_model = (info >> 16) & 0xf;
    model = (ext_model << 4) + model;

    // Identify CPU modle
    switch (model)
    {
    case 0x5c:
    case 0x5f:
    case 0x4e:
    case 0x5e:
    case 0x8e:
    case 0x9e:
    case 0x55:
    case 0x66:
    case 0x7a:
    case 0x67:
    case 0x6a:
    case 0x6c:
    case 0x7d:
    case 0x7e:
    case 0x8c:
    case 0x8d:
    case 0xa5:
    case 0xa6:
    case 0xa7:
    case 0xa8:
    case 0x86:
    case 0x8a:
    case 0x96:
    case 0x9c:
        lbr_capacity = 32;
        break;

    case 0x3d:
    case 0x47:
    case 0x4f:
    case 0x56:
    case 0x3c:
    case 0x45:
    case 0x46:
    case 0x3f:
    case 0x2a:
    case 0x2d:
    case 0x3a:
    case 0x3e:
    case 0x1a:
    case 0x1e:
    case 0x1f:
    case 0x2e:
    case 0x25:
    case 0x2c:
    case 0x2f:
        lbr_capacity = 16;
        break;

    case 0x17:
    case 0x1d:
    case 0x0f:
        lbr_capacity = 4;
        break;

    case 0x37:
    case 0x4a:
    case 0x4c:
    case 0x4d:
    case 0x5a:
    case 0x5d:
    case 0x1c:
    case 0x26:
    case 0x27:
    case 0x35:
    case 0x36:
        lbr_capacity = 8;
        break;

    default:
        // Model name not found
        return -1;
    }

    printk(KERN_INFO "LIBIHT-LKM: DisplayFamily_DisplayModel - %x_%xH\n", family, model);

    return 0;
}

static int __init libiht_lkm_init(void)
{
    printk(KERN_INFO "LIBIHT-LKM: Initializing...\n");

    // Check availability of the cpu
    printk(KERN_INFO "LIBIHT-LKM: Identifying CPU for LBR availability...\n");
    if (identify_cpu() < 0)
    {
        printk(KERN_INFO "LIBIHT-LKM: Identify CPU failed\n");
        return -1;
    }

    // Create user interactive helper process
    printk(KERN_INFO "LIBIHT-LKM: Creating helper process...\n");
    proc_entry = proc_create("libiht-info", 0666, NULL, &libiht_ops);
    if (proc_entry == NULL)
    {
        printk(KERN_INFO "LIBIHT-LKM: Create proc failed\n");
        return -1;
    }

    // Init hooks on context switches
    printk(KERN_INFO "LIBIHT-LKM: Initializing context switch hooks...\n");
    preempt_notifier_init(&notifier, &ops);

    // Enable LBR on each cpu (Not yet set the selection filter bit)
    printk(KERN_INFO "LIBIHT-LKM: Initializing LBR for all %d cpus\n", num_online_cpus());
    on_each_cpu(enable_lbr, NULL, 1);

    // Set the state list to NULL after module initialized
    lbr_state_list = NULL;

    printk(KERN_INFO "LIBIHT-LKM: Initialization complete\n");
    return 0;
}

static void __exit libiht_lkm_exit(void)
{
    // Free the state list
    struct lbr_state *tmp = lbr_state_list;
    while (tmp != NULL)
        kfree(tmp);

    // Remove the helper process if exist
    if (proc_entry)
        proc_remove(proc_entry);

    // Disable LBR on each cpu
    on_each_cpu(disable_lbr, NULL, 1);

    // TODO: Unregister hooks on context switches.
    // Problem: If the notifier not being registed, it will be an error
    // and crash the kernel. Need to identify the state
    // preempt_notifier_unregister(&notifier);

    printk(KERN_INFO "LIBIHT-LKM: Exiting\n");
}

module_init(libiht_lkm_init);
module_exit(libiht_lkm_exit);
