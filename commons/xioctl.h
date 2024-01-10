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
    // TODO: adapt lbr to this new structure
    LIBIHT_IOCTL_ENABLE_LBR,
    LIBIHT_IOCTL_DISABLE_LBR,
    LIBIHT_IOCTL_DUMP_LBR,
    LIBIHT_IOCTL_SELECT_LBR,

    // BTS
    LIBIHT_IOCTL_ENABLE_BTS,
    LIBIHT_IOCTL_DISABLE_BTS,
    LIBIHT_IOCTL_DUMP_BTS,
    LIBIHT_IOCTL_CONFIG_BTS,
};

//
// Type definitions

// Define the lbr IOCTL structure
struct lbr_ioctl_request{
    u32 pid;
    u64 lbr_select;
};

// Define the bts IOCTL structure
struct bts_ioctl_request{
    u32 pid;
    u64 bts_config;
    u64 bts_buffer_size;
};

// Define the xIOCTL structure
struct xioctl_request{
    enum IOCTL cmd;
    union {
        struct lbr_ioctl_request lbr;
        struct bts_ioctl_request bts;
    } data;
};

#endif // _COMMONS_XIOCTL_H