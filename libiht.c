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


struct lbr_t lbr_cache[LBR_CACHE_SIZE];

/*
 * LBR helper functions
 */
static void flush_lbr(int enable)
{
    wrmsrl(MSR_LBR_TOS, 0);
}


static int device_open(struct inode *inode, struct file *filp)
{
    printk(KERN_ALERT "Device opened.\n");
    return 0;
}

static int device_release(struct inode *inode, struct file *filp)
{
    printk(KERN_ALERT "Device closed.\n");
    return 0;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
    printk(KERN_ALERT "Device read.\n");
    // TODO: read / dump out lbr info based on the mode set from ioctl
	return 0;
}

static ssize_t device_write(struct file *filp, const char *buf, size_t len, loff_t *off)
{
    printk(KERN_ALERT "LIBIHT doesn't support write.\n");
    return -EINVAL;
}

static long device_ioctl(struct file *filp, unsigned int ioctl_num, unsigned long ioctl_param)
{
    printk(KERN_ALERT "Got ioctl argument %#x!", ioctl_num);
    // TODO: control LBR_SELECT bits
    // TDOO: control read
    return 0;
}

static void lbr_init(void)
{
    /* Enable Last Branch Recording */
    wrmsrl(MSR_IA32_DEBUGCTLMSR, DEBUGCTLMSR_LBR);
    // TODO: initialize lbr
    wrmsrl(MSR_LBR_SELECT, LBR_SELECT);
}

static void lbr_exit(void)
{
    /* Disable Last Branch Recording */
    wrmsrl(MSR_IA32_DEBUGCTLMSR, 0);
}

static int __init libiht_init(void)
{
    printk(KERN_INFO "LIBIHT: Initializing\n");
    lbr_init();
    proc_entry = proc_create("libiht-info", 0666, NULL, &libiht_ops);
    return 0;
}

static void __exit libiht_exit(void)
{
    lbr_exit();
    if (proc_entry) proc_remove(proc_entry);
    printk(KERN_INFO "LIBIHT: Exiting\n");
}

module_init(libiht_init);
module_exit(libiht_exit);
