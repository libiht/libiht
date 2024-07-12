#ifndef LIBIHT_API_H
#define LIBIHT_API_H

////////////////////////////////////////////////////////////////////////////////
//
//  File           : lib/commons/api.h
//  Description    : This is the header file for user-space library APIs. It
//                   contains all the necessary header files for the library.
//
//   Author        : Di Wu, Thomason Zhao
//   Last Modified : July 10, 2024
//

//
// Global variables

unsigned int MAX_LBR_LIST_LEN = 0x20;
// The default maximum number of LBR entries is 32
// (may vary in different hardware)

unsigned int MAX_BTS_LIST_LEN = 0x400;
// The default maximum number of BTS entries is 1024
// (may vary by the user request)

//
// Library constants (copied from kernel/commons/xioctl.h)

enum IOCTL {
    LIBIHT_IOCTL_BASE,

    LIBIHT_IOCTL_ENABLE_LBR,
    LIBIHT_IOCTL_DISABLE_LBR,
    LIBIHT_IOCTL_DUMP_LBR,
    LIBIHT_IOCTL_CONFIG_LBR,
    LIBIHT_IOCTL_LBR_END,

    LIBIHT_IOCTL_ENABLE_BTS,
    LIBIHT_IOCTL_DISABLE_BTS,
    LIBIHT_IOCTL_DUMP_BTS,
    LIBIHT_IOCTL_CONFIG_BTS,
    LIBIHT_IOCTL_BTS_END,
};

struct lbr_stack_entry {
    unsigned long long from;
    unsigned long long to;
};

struct lbr_config {
    unsigned int pid;
    unsigned long long lbr_select;
};

struct lbr_data {
    unsigned long long lbr_tos;
    struct lbr_stack_entry* entries;
};

struct lbr_ioctl_request {
    struct lbr_config lbr_config;
    struct lbr_data* buffer;
};

struct bts_config {
    unsigned int pid;
    unsigned long long bts_config;
    unsigned long long bts_buffer_size;
};

struct bts_record {
    unsigned long long from;
    unsigned long long to;
    unsigned long long misc;
};

struct bts_data {
    struct bts_record* bts_buffer_base;
    struct bts_record* bts_index;
    unsigned long long bts_interrupt_threshold;
};

struct bts_ioctl_request {
    struct bts_config bts_config;
    struct bts_data* buffer;
};

struct xioctl_request {
    enum IOCTL cmd;
    union {
        struct lbr_ioctl_request lbr;
        struct bts_ioctl_request bts;
    }body;
};

// The above definitions are same as those in "types.h"
#endif // LIBIHT_API_H
