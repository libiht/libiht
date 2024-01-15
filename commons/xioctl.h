#ifndef _COMMONS_XIOCTL_H
#define _COMMONS_XIOCTL_H

////////////////////////////////////////////////////////////////////////////////
//
//  File           : commons/xioclt.h
//  Description    : This is the header file for the cross platform and cross 
//                   feature IOCTL definitions for the library.
//
//   Author        : Thomason Zhao
//   Last Modified : Jan 10, 2023
//

//
// Include Files
#include "types.h"

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
    u64 from;   // Retrieve from MSR_LBR_NHM_FROM + offset
    u64 to;     // Retrieve from MSR_LBR_NHM_TO + offset
};

// Define LBR configuration
struct lbr_config
{
    u32 pid;                          // Process ID
    u64 lbr_select;                   // MSR_LBR_SELECT
};

// Define LBR data
struct lbr_data
{
    u64 lbr_tos;                      // MSR_LBR_TOS
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
    u64 from;   // branch from
    u64 to;     // branch to
    u64 misc;   // misc information
};

// Define BTS configuration
struct bts_config
{
    u32 pid;                        // Process ID
    u64 bts_config;                 // MSR_IA32_DEBUGCTLMSR
    u64 bts_buffer_size;            // BTS buffer size
};

// Define BTS data
// TODO: pay attention when using this struct in dump bts
struct bts_data
{
    struct bts_record *bts_buffer_base; // BTS buffer base
    u64 bts_index;                      // BTS current index
    u64 bts_absolute_maximum;           // BTS absolute maximum
    u64 bts_interrupt_threshold;        // BTS interrupt threshold
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

#endif // _COMMONS_XIOCTL_H