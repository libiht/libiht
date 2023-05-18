#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>

/*
 * Check Linux kernel version. 
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
#define HAVE_PROC_OPS
#endif

/*
 * The CPU hard-coded LBR stack capacity / entries
 * 
 * PS: might need change to variable to make it CPU dependent
 */
#define LBR_ENTRIES 32

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
 * Currently set to:
 * 0x1 = 00000001   --> capture branches occuring in ring >0
 */
 #define LBR_SELECT 0x1

/*
 * The struct to represent one lbr trace record
 */
struct lbr_t {
    uint64_t debug;   // contents of IA32_DEBUGCTL MSR
    uint64_t select;  // contents of LBR_SELECT
    uint64_t tos;     // index to most recent branch entry
    uint64_t from[LBR_ENTRIES];
    uint64_t   to[LBR_ENTRIES];
    // struct task_struct *task; // pointer to the task_struct this state belongs to
};

/*
 * Maintian lbr trace records
 * 
 * PS: might need change to variable for ioctl
 */
// struct lbr_t lbr_cache[LBR_CACHE_SIZE];

/*
 * Static function prototypes
 */
static void flush_lbr(bool);
static void get_lbr(void);
static void put_lbr(void);
static void dump_lbr(void);
static void enable_lbr(void *);
static void disable_lbr(void *);

static void save_lbr(void);
static void restore_lbr(void);

static void sched_in(struct preempt_notifier *, int );
static void sched_out(struct preempt_notifier *, struct task_struct *);

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static long device_ioctl(struct file *, unsigned int, unsigned long);

static int __init libiht_init(void);
static void __exit libiht_exit(void);
