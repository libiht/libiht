#include <linux/cpumask.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <asm/msr.h>

#include "libiht.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Thomason Zhao");
MODULE_DESCRIPTION("Intel Hardware Trace Library");

struct lbr_t lbr_cache;
struct proc_dir_entry *proc_entry;

/************************************************
 * LBR helper functions
 ************************************************/
static void get_lbr()
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

static void dump_lbr()
{
    int i;

    printk(KERN_ALERT "MSR_IA32_DEBUGCTLMSR:    0x%llx\n", lbr_cache.debug);
    printk(KERN_ALERT "MSR_LBR_SELECT:          0x%llx\n", lbr_cache.select);
    printk(KERN_ALERT "MSR_LBR_TOS:             %lld\n", lbr_cache.tos);

    for (i = 0; i < LBR_ENTRIES; i++)
    {
        printk(KERN_ALERT "MSR_LBR_NHM_FROM[%d]: 0x%llx\n", i, lbr_cache.from[i]);
        printk(KERN_ALERT "MSR_LBR_NHM_TO  [%d]: 0x%llx\n", i, lbr_cache.to[i]);
    }
}

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

static void lbr_init(void)
{
    int i;

    /* Apply filter */
    wrmsrl(MSR_LBR_SELECT, LBR_SELECT);

    // TODO: need to init lbr link to one process or globally to trace all branches
    /* Flush lbr entries */
    wrmsrl(MSR_LBR_TOS, 0);
    for (i = 0; i < LBR_ENTRIES; i++)
    {
        wrmsrl(MSR_LBR_NHM_FROM + i, 0);
        wrmsrl(MSR_LBR_NHM_TO + i, 0);
    }

    /* Enable lbr */
    wrmsrl(MSR_IA32_DEBUGCTLMSR, DEBUGCTLMSR_LBR);
}

static void lbr_exit(void)
{
    /* Disable lbr */
    wrmsrl(MSR_IA32_DEBUGCTLMSR, 0);
}

static int __init libiht_init(void)
{
    printk(KERN_INFO "LIBIHT: Initialize for all %d cpus\n", num_online_cpus());
    // TODO: do lbr_init in all cores by using kthread and kthread_bind
    // lbr_init();
    proc_entry = proc_create("libiht-info", 0666, NULL, &libiht_ops);
    return 0;
}

static void __exit libiht_exit(void)
{
    // lbr_exit();
    if (proc_entry)
        proc_remove(proc_entry);
    printk(KERN_INFO "LIBIHT: Exiting\n");
}

module_init(libiht_init);
module_exit(libiht_exit);
