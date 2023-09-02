#include <linux/version.h>
#include <linux/sched.h>
#include <linux/ioctl.h>
#include <asm/fpu/types.h>

#include "../../commons/types.h"
#include "../../commons/lbr.h"
#include "../../commons/cpu.h"
#include "../../commons/debug.h"

/*
 * Check Linux kernel version.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#define HAVE_PROC_OPS
#endif

/*
 * I/O control table
 */
#define LIBIHT_LKM_IOC_MAGIC 'l'
#define LIBIHT_LKM_IOC_ENABLE_TRACE     _IO(LIBIHT_LKM_IOC_MAGIC, 1)
#define LIBIHT_LKM_IOC_DISABLE_TRACE    _IO(LIBIHT_LKM_IOC_MAGIC, 2)
#define LIBIHT_LKM_IOC_DUMP_LBR         _IO(LIBIHT_LKM_IOC_MAGIC, 3)
#define LIBIHT_LKM_IOC_SELECT_LBR       _IO(LIBIHT_LKM_IOC_MAGIC, 4)

/*
 * The struct used for I/O control communication
 */
struct ioctl_request{
    uint64_t lbr_select;
    pid_t pid;
};

/*
 * Static function prototypes
 */
static void sched_in(struct preempt_notifier *, int);
static void sched_out(struct preempt_notifier *, struct task_struct *);

static int __kprobes pre_fork_handler(struct kprobe *, struct pt_regs *);
static void __kprobes post_fork_handler(struct kprobe *, struct pt_regs *, unsigned long);

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static long device_ioctl(struct file *, unsigned int, unsigned long);

static int __init libiht_lkm_init(void);
static void __exit libiht_lkm_exit(void);
