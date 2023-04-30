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

static struct lbr_t lbr_cache;
static struct proc_dir_entry *proc_entry;
static struct task_struct *kth_arr;
static int num_cpus;

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

static int init_lbr_thread(void *cpuid)
{
    int i;

    printk(KERN_ALERT "LIBIHT: Sleep for a while\n");
    msleep(1000);

    /* Enable lbr */
    // TODO: Fix BUG: unable to handle page fault for address: ffffffffc153628d
    wrmsrl(MSR_IA32_DEBUGCTLMSR, DEBUGCTLMSR_LBR);

    /* Apply filter */
    wrmsrl(MSR_LBR_SELECT, LBR_SELECT);

    /* Flush lbr entries */
    wrmsrl(MSR_LBR_TOS, 0);
    for (i = 0; i < LBR_ENTRIES; i++)
    {
        wrmsrl(MSR_LBR_NHM_FROM + i, 0);
        wrmsrl(MSR_LBR_NHM_TO + i, 0);
    }

    printk(KERN_ALERT "Enable LBR on CPU: %d", *(int *)cpuid);
    do_exit(0);
}

static int init_lbr(struct task_struct *kth, int cpuid)
{
    // TODO: need to init lbr link to one process or globally to trace all branches
    char kth_name[20];

    sprintf(kth_name, "init_lbr_%d", cpuid);
    kth = kthread_create(init_lbr_thread, &cpuid, kth_name);
    if (kth == NULL)
        return -1;

    kthread_bind(kth, cpuid);
    wake_up_process(kth);
    
    return 0;
}

static int exit_lbr_thread(void *cpuid)
{
    printk(KERN_ALERT "LIBIHT: Sleep for a while\n");
    msleep(1000);
    /* Disable lbr */
    wrmsrl(MSR_IA32_DEBUGCTLMSR, 0);

    printk(KERN_ALERT "Exit LBR on CPU: %d", *(int *)cpuid);
    do_exit(0);
}

static int exit_lbr(struct task_struct *kth, int cpuid)
{
    char kth_name[20];

    sprintf(kth_name, "exit_lbr_%d", cpuid);
    kth = kthread_create(exit_lbr_thread, &cpuid, kth_name);

    if (kth == NULL)
        return -1;

    kthread_bind(kth, cpuid);
    wake_up_process(kth);
    
    return 0;
}

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

static int __init libiht_init(void)
{
    int i;

    num_cpus = num_online_cpus();
    printk(KERN_INFO "LIBIHT: Initialize for all %d cpus\n", num_cpus);

    kth_arr = kmalloc(sizeof(struct task_struct) * num_cpus, GFP_KERNEL);
    if (kth_arr == NULL)
        return -1;
    
    for (i = 0; i < num_cpus; i++)
    {
        if (init_lbr(&kth_arr[i], i) < 0)
            return -1;
    }
    

    proc_entry = proc_create("libiht-info", 0666, NULL, &libiht_ops);
    if (proc_entry)
        return -1;
    
    return 0;
}

static void __exit libiht_exit(void)
{
    int i;
    
    if (proc_entry)
        proc_remove(proc_entry);

    for (i = 0; i < num_cpus; i++)
        exit_lbr(&kth_arr[i], i);
    kfree(kth_arr);

    msleep(5000);
    printk(KERN_INFO "LIBIHT: Exiting\n");
}

module_init(libiht_init);
module_exit(libiht_exit);
