# Kernel Module/Driver API Usage

User space applications can interact with the kernel space components through the provided IOCTL APIs. The specific IOCTL payloads and their corresponding operations are defined in the [`kernel/commons/xioctl.h`](../../kernel/commons/xioctl.h) header file. The following sections provide an overview of the IOCTL operations and their usage.

Kernel module/driver wrap the IOCTL request for different hardware trace capabilities into a single generic IOCTL request. 

## IOCTL Request Command Code

The IOCTL request command code is defined as follows:

```c
enum IOCTL {
    LIBIHT_IOCTL_BASE,          // Placeholder

    // LBR
    LIBIHT_IOCTL_ENABLE_LBR,
    LIBIHT_IOCTL_DISABLE_LBR,
    LIBIHT_IOCTL_DUMP_LBR,
    LIBIHT_IOCTL_SELECT_LBR,
    LIBIHT_IOCTL_LBR_END,       // End of LBR

    // BTS
    LIBIHT_IOCTL_ENABLE_BTS,
    LIBIHT_IOCTL_DISABLE_BTS,
    LIBIHT_IOCTL_DUMP_BTS,
    LIBIHT_IOCTL_CONFIG_BTS,
    LIBIHT_IOCTL_BTS_END,       // End of BTS
};
```

- `LIBIHT_IOCTL_BASE`: Placeholder for the base IOCTL request code.
- `LIBIHT_IOCTL_ENABLE_LBR`: Enable the Last Branch Record (LBR) hardware trace capability
- `LIBIHT_IOCTL_DISABLE_LBR`: Disable the Last Branch Record (LBR) hardware trace capability
- `LIBIHT_IOCTL_DUMP_LBR`: Dump the Last Branch Record (LBR) hardware trace information
- `LIBIHT_IOCTL_SELECT_LBR`: Select the Last Branch Record (LBR) hardware trace information
- `LIBIHT_IOCTL_LBR_END`: End of Last Branch Record (LBR) hardware trace commands
- `LIBIHT_IOCTL_ENABLE_BTS`: Enable the Branch Trace Store (BTS) hardware trace capability
- `LIBIHT_IOCTL_DISABLE_BTS`: Disable the Branch Trace Store (BTS) hardware trace capability
- `LIBIHT_IOCTL_DUMP_BTS`: Dump the Branch Trace Store (BTS) hardware trace information
- `LIBIHT_IOCTL_CONFIG_BTS`: Configure the Branch Trace Store (BTS) hardware trace capability
- `LIBIHT_IOCTL_BTS_END`: End of Branch Trace Store (BTS) hardware trace commands

## Generic IOCTL Request Format

The generic IOCTL request format is defined as follows:

```c
struct xioctl_request{
    enum IOCTL cmd;
    union {
        struct lbr_ioctl_request lbr;
        struct bts_ioctl_request bts;
    } body;
};
```

- `cmd`: The IOCTL command code.
- `body`: The body of the IOCTL request, which contains the specific hardware trace capability request.

### LBR IOCTL Request

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

### BTS IOCTL Request

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
