#include <unistd.h>

#include "../../commons/xplat.h"
#include "../../commons/xioctl.h"

struct lbr_ioctl_request enable_lbr();
void disalbe_lbr(struct lbr_ioctl_request usr_request);
void dump_lbr(struct lbr_ioctl_request usr_request);
void select_lbr(struct lbr_ioctl_request usr_request);