////////////////////////////////////////////////////////////////////////////////
//
//  File           : commons/ioclt.h
//  Description    : This is the header file for the IOCTL definitions for the
//                   library.
//
//   Author        : Thomason Zhao
//   Last Modified : Dec 25, 2023
//

//
// Include Files
#include "types.h"

//
// Library constants
enum IOCTL {
    LIBIHT_IOCTL,               // Placeholder
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
struct ioctl_request{
    enum IOCTL cmd; // Command
    u32 pid;        // Process ID
    u64 buf;        // Buffer pointer (defined in headers of trace features)
};
