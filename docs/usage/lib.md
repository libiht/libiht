# User Library Usage TODO Revise all

User space applications can interact with the kernel space components through the provided user space APIs. The specific user space APIs and their corresponding operations are defined in the [`api.h`](../../lib/commons/api.h) header file. The following sections provide an overview of the user space APIs and their usage.

TODO: Currently, the user space APIs are just wrappers of the kernel space IOCTLs. You may refer to the kernel document for details. We will provide more high-level APIs in the future.

## Appendix

### User Space API Functions

The user space API functions are defined as follows:

```c
struct lbr_ioctl_request enable_lbr();
void disable_lbr(struct lbr_ioctl_request usr_request);
void dump_lbr(struct lbr_ioctl_request usr_request);
void select_lbr(struct lbr_ioctl_request usr_request);
struct bts_ioctl_request enable_bts();
void disable_bts(struct bts_ioctl_request usr_request);
void dump_bts(struct bts_ioctl_request usr_request);
void config_bts(struct bts_ioctl_request usr_request);
```

- `enable_lbr()`: Enable the Last Branch Record (LBR) hardware trace capability.
- `disable_lbr()`: Disable the Last Branch Record (LBR) hardware trace capability.
- `dump_lbr()`: Dump the Last Branch Record (LBR) hardware trace information.
- `select_lbr()`: Select the Last Branch Record (LBR) hardware trace information.
- `enable_bts()`: Enable the Branch Trace Store (BTS) hardware trace capability.
- `disable_bts()`: Disable the Branch Trace Store (BTS) hardware trace capability.
- `dump_bts()`: Dump the Branch Trace Store (BTS) hardware trace information.
- `config_bts()`: Configure the Branch Trace Store (BTS) hardware trace capability.

## LIBIHT (User-Space Library)
LIBIHT is a user-space library for easier usage of the kernel module in this project. By compiling libiht_api.so (for Linux systems) or kmd.dll (for Windows systems) along with your code, you can conveniently utilize the functionalities provided by this project.

## Installation
After installing the kernel part of this project, simply compile libiht_api.so (for Linux) or kmd.dll (for Windows) together with your code. Make sure your build environment is properly configured and can link to LIBIHT.

## Usage
1.  call enable_lbr(pid) or enable_bts(pid) in your code, where pid is the identifier of the process to operate on. When pid is 0, it represents initializing the current process. enable_lbr will return an lbr_ioctl_request, while enable_bts will return a bts_ioctl_request. You can use these request objects for subsequent configuration and querying operations.
1. Use config_lbr(lbr_ioctl_request) or config_bts(bts_ioctl_request) to modify the query settings. Pass the previously obtained lbr_ioctl_request or bts_ioctl_request to these functions for configuration.
1. Call dump_lbr(lbr_ioctl_request) or dump_bts(bts_ioctl_request) to retrieve the current LBR (Last Branch Record) content or BTS (Branch Trace Store) content.
1. After completing the usage, use disable_br(lbr_ioctl_request) or disable_bts(bts_ioctl_request) to disable the LBR or BTS functionality.
