#include "../../commons/xioctl.h"
#include <linux/uaccess.h>

u32 xcopy_from_user(void *to, const void *from, u32 n){
    return copy_from_user(to, (const void __user *) from, n);
}

u32 xcopy_to_user(void *to, const void *from, u32 n){
    return copy_to_user((void __user *) to, from, n);
}
