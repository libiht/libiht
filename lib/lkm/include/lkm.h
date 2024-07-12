#ifndef LIBIHT_LKM_H
#define LIBIHT_LKM_H

////////////////////////////////////////////////////////////////////////////////
//
//  File           : lib/lkm/include/lkm.h
//  Description    : This is the header file for the Linux kernel module (LKM)
//                   APIs. It contains all the necessary header files for LKM.
//
//   Author        : Di Wu, Thomason Zhao
//   Last Modified : July 10, 2024
//

#include "../../commons/api.h"

//
// Function prototypes

// TODO: Redefine the functions, current design is bad architected :(

// For LBR

struct lbr_ioctl_request enable_lbr(unsigned int pid);
// Enable LBR for a given process ID

void disable_lbr(struct lbr_ioctl_request usr_request);
// Disable LBR for a user request

void dump_lbr(struct lbr_ioctl_request usr_request);
// Dump LBR for a user request

void config_lbr(struct lbr_ioctl_request usr_request);
// Configure LBR for a user request

// For BTS

struct bts_ioctl_request enable_bts(unsigned int pid);
// Enable BTS for a given process ID

void disable_bts(struct bts_ioctl_request usr_request);
// Disable BTS for a user request

void dump_bts(struct bts_ioctl_request usr_request);
// Dump BTS for a user request

void config_bts(struct bts_ioctl_request usr_request);
// Configure BTS for a user request

#endif // LIBIHT_LKM_H