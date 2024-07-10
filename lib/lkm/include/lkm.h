#pragma once
#include "../../commons/api.h"

struct lbr_ioctl_request enable_lbr(unsigned int pid);
void disable_lbr(struct lbr_ioctl_request usr_request);
void dump_lbr(struct lbr_ioctl_request usr_request);
void config_lbr(struct lbr_ioctl_request usr_request);

struct bts_ioctl_request enable_bts(unsigned int pid);
void disable_bts(struct bts_ioctl_request usr_request);
void dump_bts(struct bts_ioctl_request usr_request);
void config_bts(struct bts_ioctl_request usr_request);