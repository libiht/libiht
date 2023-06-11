#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/ioctl.h>
#include <asm/fpu/types.h>

/*
 * Check Linux kernel version.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#define HAVE_PROC_OPS
#endif

/*
 * Total number of lbr trace records this kernel module maintains
 *
 * PS: might need change to variable for ioctl
 */
#define LBR_CACHE_SIZE 32

/* Bit Field        Bit Offset  Access  Description
 *
 * CPL_EQ_0         0           R/W     When set, do not capture branches ending in ring 0
 * CPL_NEQ_0        1           R/W     When set, do not capture branches ending in ring >0
 * JCC              2           R/W     When set, do not capture conditional branches
 * NEAR_REL_CALL    3           R/W     When set, do not capture near relative calls
 * NEAR_IND_CALL    4           R/W     When set, do not capture near indirect calls
 * NEAR_RET         5           R/W     When set, do not capture near returns
 * NEAR_IND_JMP     6           R/W     When set, do not capture near indirect jumps
 * NEAR_REL_JMP     7           R/W     When set, do not capture near relative jumps
 * FAR_BRANCH       8           R/W     When set, do not capture far branches
 * Reserved         63:9                Must be zero
 *
 * Default selection bit set to:
 * 0x1 = 00000001   --> capture branches occuring in ring >0
 */
#define LBR_SELECT 0x1

/*
 * I/O control table
 */
#define LIBIHT_LKM_IOC_MAGIC 'l'
#define LIBIHT_LKM_IOC_INIT_LBR     _IO(LIBIHT_LKM_IOC_MAGIC, 1)
#define LIBIHT_LKM_IOC_ENABLE_LBR   _IO(LIBIHT_LKM_IOC_MAGIC, 2)
#define LIBIHT_LKM_IOC_DISABLE_LBR  _IO(LIBIHT_LKM_IOC_MAGIC, 3)
#define LIBIHT_LKM_IOC_DUMP_LBR     _IO(LIBIHT_LKM_IOC_MAGIC, 4)
#define LIBIHT_LKM_IOC_SELECT_LBR   _IO(LIBIHT_LKM_IOC_MAGIC, 5)

/*
 * The struct used for I/O control communication
 */
struct ioctl_request{
    uint64_t lbr_select;
    pid_t pid;
};

/*
 * The struct to represent one lbr stack entry
 */
struct lbr_stack_entry
{
    uint64_t from;
    uint64_t to;
};

/*
 * The struct to represent one lbr trace record
 */
struct lbr_state
{
    uint64_t lbr_select; // contents of LBR_SELECT
    uint64_t lbr_tos;    // index to top of the stack (most recent entry)
    pid_t pid;           // target lbr trace process pid
    struct lbr_state *prev;
    struct lbr_state *next;
    struct lbr_stack_entry entries[];
};

/*
 * The struct represent the mapping between CPU model and its corrosponding
 * LBR entries (if exist)
 */
struct cpu_to_lbr {
    uint32_t model;
    uint32_t lbr_capacity;
};

/*
 * Static function prototypes
 */
static void flush_lbr(bool);
static void get_lbr(pid_t);
static void put_lbr(pid_t);
static void dump_lbr(pid_t);
static void enable_lbr(void *);
static void disable_lbr(void *);

static void save_lbr(void);
static void restore_lbr(void);

static struct lbr_state *create_lbr_state(void);
static void insert_lbr_state(struct lbr_state *);
static struct lbr_state *find_lbr_state(pid_t);

static void sched_in(struct preempt_notifier *, int);
static void sched_out(struct preempt_notifier *, struct task_struct *);

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static long device_ioctl(struct file *, unsigned int, unsigned long);

static int __init libiht_lkm_init(void);
static void __exit libiht_lkm_exit(void);
