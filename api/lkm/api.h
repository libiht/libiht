#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

#include "../../commons/types.h"

// Device name
#define DEVICE_NAME "libiht-info"

// I/O control table
#define LIBIHT_LKM_IOC_MAGIC 'l'
#define LIBIHT_LKM_IOC_ENABLE_TRACE     _IO(LIBIHT_LKM_IOC_MAGIC, 1)
#define LIBIHT_LKM_IOC_DISABLE_TRACE    _IO(LIBIHT_LKM_IOC_MAGIC, 2)
#define LIBIHT_LKM_IOC_DUMP_LBR         _IO(LIBIHT_LKM_IOC_MAGIC, 3)
#define LIBIHT_LKM_IOC_SELECT_LBR       _IO(LIBIHT_LKM_IOC_MAGIC, 4)
#define LIBIHT_LKM_IOC_COPY_LBR         _IO(LIBIHT_LKM_IOC_MAGIC, 5)

struct lbr_stack_entry
{
    u64 from;   // MSR_LBR_NHM_FROM + offset
    u64 to;     // MSR_LBR_NHM_TO + offset
};

// Define LBR state
struct lbr_state
{
    u64 lbr_select;                   // MSR_LBR_SELECT
    u64 lbr_tos;                      // MSR_LBR_TOS
    u32 pid;                          // process id
    struct lbr_state *prev;           // previous state
    struct lbr_state *next;           // next state
    struct lbr_state *parent;         // parent state
    struct lbr_stack_entry entries[]; // flexible array member
};

// Define LBR request
struct lbr_request
{
    unsigned long long lbr_select;
    unsigned int pid;
    struct lbr_state *content; // the place where lbr stored
};

struct lbr_request enable_lbr();
void disable_lbr(struct lbr_request *user_req);
void dump_lbr(struct lbr_request *user_req);
void select_lbr(struct lbr_request *user_req);
void copy_lbr(struct lbr_request *user_req);
