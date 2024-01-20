////////////////////////////////////////////////////////////////////////////////
//
//  File           : demo/lkm-demo/lkm-demo.c
//  Description    : This is the main program for the lkm-demo program. It will
//                   open the helper process and send the ioctl request to the
//                   helper processs. The kernel module will trace the process 
//                   and dump the LBR to show the call stack information of the
//                   cross recursive function call.
//
//   Author        : Thomason Zhao
//   Last Modified : Jan 15, 2023
//

// Include Files
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

// Device name
#define DEVICE_NAME "libiht-info"

// I/O control macros
// TODO: Is IOCTL macros really needed?
#define LIBIHT_LKM_IOCTL_MAGIC 'l'
#define LIBIHT_LKM_IOCTL_BASE       _IO(LIBIHT_LKM_IOCTL_MAGIC, 0)

//
// Library constants
enum IOCTL {
    LIBIHT_IOCTL_BASE,          // Placeholder

    // LBR
    LIBIHT_IOCTL_ENABLE_LBR,
    LIBIHT_IOCTL_DISABLE_LBR,
    LIBIHT_IOCTL_DUMP_LBR,
    LIBIHT_IOCTL_SELECT_LBR,
    LIBIHT_IOCTL_LBR_END,       // End of LBR

    // BTS
    LIBIHT_IOCTL_ENABLE_BTS,
    LIBIHT_IOCTL_DISABLE_BTS,
    LIBIHT_IOCTL_DUMP_BTS,
    LIBIHT_IOCTL_CONFIG_BTS,
    LIBIHT_IOCTL_BTS_END,       // End of BTS
};

//
// LBR Type definitions

// Define LBR stack entry
struct lbr_stack_entry
{
    unsigned long long from;   // Retrieve from MSR_LBR_NHM_FROM + offset
    unsigned long long to;     // Retrieve from MSR_LBR_NHM_TO + offset
};

// Define LBR configuration
struct lbr_config
{
    unsigned int pid;                          // Process ID
    unsigned long long lbr_select;                   // MSR_LBR_SELECT
};

// Define LBR data
struct lbr_data
{
    unsigned long long lbr_tos;                      // MSR_LBR_TOS
    struct lbr_stack_entry *entries;  // LBR stack entries
};

// Define the lbr IOCTL structure
struct lbr_ioctl_request{
    struct lbr_config lbr_config;
    struct lbr_data *buffer;
};

//
// BTS Type definitions

// Define BTS record
struct bts_record
{
    unsigned long long from;   // branch from
    unsigned long long to;     // branch to
    unsigned long long misc;   // misc information
};

// Define BTS configuration
struct bts_config
{
    unsigned int pid;                        // Process ID
    unsigned long long bts_config;                 // MSR_IA32_DEBUGCTLMSR
    unsigned long long bts_buffer_size;            // BTS buffer size
};

// Define BTS data
// TODO: pay attention when using this struct in dump bts
struct bts_data
{
    struct bts_record *bts_buffer_base; // BTS buffer base
    unsigned long long bts_index;                      // BTS current index
    unsigned long long bts_absolute_maximum;           // BTS absolute maximum
    unsigned long long bts_interrupt_threshold;        // BTS interrupt threshold
};

// Define the bts IOCTL structure
struct bts_ioctl_request{
    struct bts_config bts_config;
    struct bts_data *buffer;
};

//
// xIOCTL Type definitions

// Define the xIOCTL structure
struct xioctl_request{
    enum IOCTL cmd;
    union {
        struct lbr_ioctl_request lbr;
        struct bts_ioctl_request bts;
    } body;
};

int cnt = 10;

void func1(void);
void func2(void);

void func1()
{
    //printf("func1: %d\n", cnt);
    if (cnt != 0)
    {
        cnt--;
        func2();
    }
}

void func2()
{
    //printf("func2: %d\n", cnt);
    if (cnt != 0)
    {
        cnt--;
        func1();
    }
}

void print_usage()
{
    printf("Usage: lkm-demo [pid] [count]\n");
    printf("pid: the pid of the process want to trace, trace it self if it is 0\n");
    printf("count: the number of recursive function call\n");
    printf("Example: lkm-demo 0 10\n");
    fflush(stdout);
    exit(-1);
}

int main(int argc, char* argv[])
{
    if (argc != 3)
        print_usage();

    int pid = atoi(argv[1]);
    if (pid == 0)
        pid = getpid();
    cnt = atoi(argv[2]);
    printf("pid: %d, count: %d\n", pid, cnt);
    printf("func1's ptr: %p\nfunc2's ptr: %p\n", &func1, &func2);
    fflush(stdout);
    sleep(1);

    int fd = open("/proc/" DEVICE_NAME, O_RDWR);

    if (fd < 0)
    {
        printf("Failed to open device!\n");
        return 0;
    }

    struct xioctl_request input;
    // Enable LBR
    input.body.lbr.lbr_config.lbr_select = 0;
    input.body.lbr.lbr_config.pid = pid;

    input.cmd = LIBIHT_IOCTL_ENABLE_LBR;
    ioctl(fd, LIBIHT_LKM_IOCTL_BASE, &input);
    sleep(1);

    // Simulate critical logic
    func1();

    // Dump LBR
    input.cmd = LIBIHT_IOCTL_DUMP_LBR;
    ioctl(fd, LIBIHT_LKM_IOCTL_BASE, &input);
    sleep(1);

    // Disable LBR
    input.cmd = LIBIHT_IOCTL_DISABLE_LBR;
    ioctl(fd, LIBIHT_LKM_IOCTL_BASE, &input);
    sleep(1);

    printf("Finished!\n");
    close(fd);
    return 0;
}
