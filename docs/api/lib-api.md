# User Library API Usage

User space applications can interact with the kernel space components through the provided user space APIs. The specific user space APIs and their corresponding operations are defined in the [`api.h`](../../lib/commons/api.h) header file. The following sections provide an overview of the user space APIs and their usage.

TODO: Currently, the user space APIs are just wrappers of the kernel space IOCTLs. We will provide more high-level APIs in the future.

## User Space API Functions

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