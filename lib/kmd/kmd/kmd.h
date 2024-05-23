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