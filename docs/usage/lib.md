# User Library Usage

User space applications can interact with the kernel space components through the provided user space APIs. The specific user space APIs and their corresponding operations are defined in the [`api.h`](../../lib/commons/api.h) header file. The following sections provide an overview of the user space APIs and their usage.

Currently, the user space APIs are just wrappers of the kernel space IOCTLs. This allows user space applications have a more easier wayy to interact with the kernel space components.

You may also refer to the kernel document for details. We will be providing more high-level APIs in the future.

## Installation

To use the LIB on Linux, please include the `lib/include/api.h` and `lib/lkm/include/lkm.h` header files in your code, and compile your code together with `liblbr_api.so`.

On Windows, copy the `x64/Release` directory to your code directory. In your code, include the `lib/include/api.h` and `lib/kmd/kmd.h` header files. Copy the files from `lib/kmd/x64/Release` to your code directory and link against `kmd.dll`.

## Usage

To obtain a `struct lbr_ioctl_request` or `struct bts_ioctl_request` variable for enabling LBR or BTS, you can follow the steps below:

1. Declare a variable of type `struct lbr_ioctl_request` or `struct bts_ioctl_request`. For example:

   ```c
   struct lbr_ioctl_request lbr_request;
   struct bts_ioctl_request bts_request;
   ```

2. Set the necessary parameters in the variable. These parameters may include the buffer size, sampling period, or other configuration options specific to LBR or BTS. Refer to the documentation or header files for the specific structure fields and their meanings. For example:

   ```c
   lbr_request = enable_lbr();
   bts_reqeust = enable_bts();
   ```

3. Call the corresponding functions to perform the desired action, such as `dump_lbr`, `config_lbr`, `disable_lbr`, `dump_bts`, `config_bts`, or `disable_bts`. Pass the created variable as an argument to these functions. For example:

   ```c
   dump_lbr(lbr_request);
   config_lbr(lbr_request);
   disable_lbr(lbr_request);
   ```

   ```c
   dump_bts(&bts_request);
   config_bts(&bts_request);
   disable_bts(&bts_request);
   ```

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

### IOCTL Requests

#### LBR IOCTL Request

The LBR IOCTL request is defined as follows:

```c
struct lbr_ioctl_request{
    struct lbr_config lbr_config;
    struct lbr_data *buffer;
};
```

- `lbr_config`: The LBR configuration structure.
- `buffer`: The buffer for storing the LBR trace information.

The LBR configuration structure is defined as follows:

```c
struct lbr_config
{
    u32 pid;                          // Process ID
    u64 lbr_select;                   // MSR_LBR_SELECT
};
```

- `pid`: The process ID for filtering the LBR trace information.
- `lbr_select`: The value of the `MSR_LBR_SELECT` register.

The LBR data structure is defined as follows:

```c
struct lbr_data
{
    u64 lbr_tos;                      // MSR_LBR_TOS
    struct lbr_stack_entry *entries;  // LBR stack entries
};
```

- `lbr_tos`: The value of the `MSR_LBR_TOS` register.
- `entries`: The LBR stack entries.

The LBR stack entry structure is defined as follows:

```c
struct lbr_stack_entry
{
    u64 from;   // Retrieve from MSR_LBR_NHM_FROM + offset
    u64 to;     // Retrieve from MSR_LBR_NHM_TO + offset
};
```

- `from`: The value of the `MSR_LBR_NHM_FROM` register.

- `to`: The value of the `MSR_LBR_NHM_TO` register.

#### BTS IOCTL Request

The BTS IOCTL request is defined as follows:

```c
struct bts_ioctl_request{
    struct bts_config bts_config;
    struct bts_data *buffer;
};
```

- `bts_config`: The BTS configuration structure.
- `buffer`: The buffer for storing the BTS trace information.

The BTS configuration structure is defined as follows:

```c
struct bts_config
{
    u32 pid;                        // Process ID
    u64 bts_config;                 // MSR_IA32_DEBUGCTLMSR
    u64 bts_buffer_size;            // BTS buffer size
};
```

- `pid`: The process ID for filtering the BTS trace information.
- `bts_config`: The value of the `MSR_IA32_DEBUGCTLMSR` register.
- `bts_buffer_size`: The size of the BTS buffer.

The BTS data structure is defined as follows:

```c
struct bts_data
{
    struct bts_record *bts_buffer_base; // BTS buffer base
    struct bts_record *bts_index;       // BTS current index
    u64 bts_interrupt_threshold;        // BTS interrupt threshold
};
```

- `bts_buffer_base`: The base address of the BTS buffer.
- `bts_index`: The current index of the BTS buffer.
- `bts_interrupt_threshold`: The interrupt threshold of the BTS buffer.

The BTS record structure is defined as follows:

```c
struct bts_record
{
    u64 from;   // branch from
    u64 to;     // branch to
    u64 misc;   // misc information
};
```

- `from`: The source address of the branch.
- `to`: The destination address of the branch.
- `misc`: The miscellaneous information of the branch.
