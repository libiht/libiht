////////////////////////////////////////////////////////////////////////////////
//
//  File           : lib/kmd/kmd/kmd.h
//  Description    : This is the header file for the Kernel Mode Driver (KMD)
//                   APIs. It contains all the necessary header files for KMD.
//                   Mainly the function prototypes for the APIs.
//
//   Author        : Di Wu, Thomason Zhao
//   Last Modified : July 10, 2024
//

#pragma once
#include "../../commons/api.h"
#ifdef KMD_EXPORTS
#define KMD_API __declspec(dllexport)
#else
#define KMD_API __declspec(dllimport)
#endif
#
extern "C" KMD_API struct lbr_ioctl_request enable_lbr(unsigned int pid);
extern "C" KMD_API void disable_lbr(struct lbr_ioctl_request usr_request);
extern "C" KMD_API void dump_lbr(struct lbr_ioctl_request usr_request);
extern "C" KMD_API void config_lbr(struct lbr_ioctl_request usr_request);

extern "C" KMD_API struct bts_ioctl_request enable_bts(unsigned int pid);
extern "C" KMD_API void disable_bts(struct bts_ioctl_request usr_request);
extern "C" KMD_API void dump_bts(struct bts_ioctl_request usr_request);
extern "C" KMD_API void config_bts(struct bts_ioctl_request usr_request);