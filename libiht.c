#include <linux/cpumask.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <asm/msr.h>

#include "libiht.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Thomason Zhao");
MODULE_DESCRIPTION("Intel Hardware Trace Library");

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

static struct lbr_t lbr_cache;
static spinlock_t lbr_cache_lock;
static struct proc_dir_entry *proc_entry;
static pid_t target_proc;

/************************************************
 * LBR helper functions
 *
 * Help to manage LBR stack/registers and kernel LBR datastructure
 ************************************************/

/*
 * Flush the LBR registers. Caller should ensure this function run on
 * single cpu (by wrapping get_cpu() and put_cpu())
 */
static void flush_lbr(bool enable)
{
    int i;

    wrmsrl(MSR_LBR_TOS, 0);
    for (i = 0; i < LBR_ENTRIES; i++)
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

    rdmsrl(MSR_IA32_DEBUGCTLMSR, lbr_cache.debug);
    rdmsrl(MSR_LBR_SELECT, lbr_cache.select);
    rdmsrl(MSR_LBR_TOS, lbr_cache.tos);

    for (i = 0; i < LBR_ENTRIES; i++)
    {
        rdmsrl(MSR_LBR_NHM_FROM + i, lbr_cache.from[i]);
        rdmsrl(MSR_LBR_NHM_TO + i, lbr_cache.to[i]);
    }
}

/*
 * Write the LBR registers from kernel maintained datastructure
 */
static void put_lbr(void)
{
    int i;

    wrmsrl(MSR_IA32_DEBUGCTLMSR, lbr_cache.debug);
    wrmsrl(MSR_LBR_SELECT, lbr_cache.select);
    wrmsrl(MSR_LBR_TOS, lbr_cache.tos);

    for (i = 0; i < LBR_ENTRIES; i++)
    {
        wrmsrl(MSR_LBR_NHM_FROM + i, lbr_cache.from[i]);
        wrmsrl(MSR_LBR_NHM_TO + i, lbr_cache.to[i]);
    }
}

/*
 * Dump out the LBR registers to kernel message
 */
static void dump_lbr(void)
{
    int i;

    printk(KERN_ALERT "MSR_IA32_DEBUGCTLMSR: 0x%llx\n", lbr_cache.debug);
    printk(KERN_ALERT "MSR_LBR_SELECT:       0x%llx\n", lbr_cache.select);
    printk(KERN_ALERT "MSR_LBR_TOS:          %lld\n", lbr_cache.tos);

    for (i = 0; i < LBR_ENTRIES; i++)
    {
        printk(KERN_ALERT "MSR_LBR_NHM_FROM[%2d]: 0x%llx\n", i, lbr_cache.from[i]);
        printk(KERN_ALERT "MSR_LBR_NHM_TO  [%2d]: 0x%llx\n", i, lbr_cache.to[i]);
    }
}

/*
 * Enable the LBR feature for the current CPU. *info may be NULL (it is required
 * by on_each_cpu()).
 */
static void enable_lbr(void *info)
{

    get_cpu();

    printk(KERN_ALERT "Enable LBR on cpu core: %d\n", smp_processor_id());

    /* Apply the filter (what kind of branches do we want to track?) */
    wrmsrl(MSR_LBR_SELECT, LBR_SELECT);

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

    printk(KERN_ALERT "Disable LBR on cpu core: %d\n", smp_processor_id());

    /* Apply the filter (what kind of branches do we want to track?) */
    wrmsrl(MSR_LBR_SELECT, 0);

    /* Flush the LBR and disable it */
    flush_lbr(false);

    put_cpu();
}

/************************************************
 * Context switch hook functions
 *
 * Save and Restore the LBR state during context switches. Should be done as fast
 * as possiable to minimize the overhead of context switches.
 ************************************************/

/*
 * Save LBR state
 */
static void save_lbr(void)
{
    unsigned long lbr_cache_flags;

    // Only save for target process
    if (target_proc == current->pid)
    {
        spin_lock_irqsave(&lbr_cache_lock, lbr_cache_flags);
        get_lbr();
        wrmsrl(MSR_IA32_DEBUGCTLMSR, 0);
        spin_unlock_irqrestore(&lbr_cache_lock, lbr_cache_flags);
    }
}

/*
 * Restore LBR state
 */
static void restore_lbr(void)
{
    unsigned long lbr_cache_flags;

    // Only restore for target process
    if (target_proc == current->pid)
    {
        spin_lock_irqsave(&lbr_cache_lock, lbr_cache_flags);
        put_lbr();
        wrmsrl(MSR_IA32_DEBUGCTLMSR, DEBUGCTLMSR_LBR);
        spin_unlock_irqrestore(&lbr_cache_lock, lbr_cache_flags);
    }
}

/************************************************
 * Device hook functions
 *
 * Maintain functionality of the libiht-info helper process
 ************************************************/

/*
 * Device related functions
 */
static int device_open(struct inode *inode, struct file *filp)
{
    printk(KERN_ALERT "LIBIHT: Device opened.\n");
    return 0;
}

static int device_release(struct inode *inode, struct file *filp)
{
    printk(KERN_ALERT "LIBIHT: Device closed.\n");
    return 0;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
    printk(KERN_ALERT "LIBIHT: Device read.\n");
    // TODO: read / dump out lbr info based on the mode set from ioctl
    get_cpu();
    get_lbr();
    dump_lbr();
    printk(KERN_INFO "LIBIHT: LBR info for cpuid: %d\n", smp_processor_id());
    put_cpu();
    return 0;
}

static ssize_t device_write(struct file *filp, const char *buf, size_t len, loff_t *off)
{
    printk(KERN_ALERT "LIBIHT: doesn't support write.\n");
    return -EINVAL;
}

static long device_ioctl(struct file *filp, unsigned int ioctl_num, unsigned long ioctl_param)
{
    printk(KERN_ALERT "LIBIHT: Got ioctl argument %#x!", ioctl_num);
    // TODO: control LBR_SELECT bits
    // TDOO: control read
    return 0;
}

/************************************************
 * Kernel module functions
 ************************************************/

static int __init libiht_init(void)
{
    printk(KERN_INFO "LIBIHT: Initialize for all %d cpus\n", num_online_cpus());

    // Enable LBR on each cpu
    on_each_cpu(enable_lbr, NULL, 1);

    // Create user interactive helper process
    proc_entry = proc_create("libiht-info", 0666, NULL, &libiht_ops);
    if (proc_entry == NULL)
    {
        printk(KERN_ALERT "LIBIHT: Create proc failed\n");
        return -1;
    }

    printk(KERN_INFO "LIBIHT: Initialization complete\n");
    return 0;
}

static void __exit libiht_exit(void)
{
    if (proc_entry)
        proc_remove(proc_entry);

    // Disable LBR on each cpu
    on_each_cpu(disable_lbr, NULL, 1);

    printk(KERN_INFO "LIBIHT: Exiting\n");
}

module_init(libiht_init);
module_exit(libiht_exit);
