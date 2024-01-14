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
#include "lbr.h"
#include "bts.h"

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
// Type definitions

// Define the lbr IOCTL structure
struct lbr_ioctl_request{
    struct lbr_config lbr_config;
    struct lbr_data *buffer;
};

// Define the bts IOCTL structure
struct bts_ioctl_request{
    struct bts_config bts_config;
    struct bts_data *buffer;
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