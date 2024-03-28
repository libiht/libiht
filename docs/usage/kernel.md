# Kernel Module/Driver Usage

This document provides an overview of the IOCTL operations and their usage in the kernel module/driver.

For detailed information about Intel LBR or BTS, please refer to the respective chapter in Volume 3 of the Intel Software Developer's Manual at http://www.intel.com/sdm.

## Introduction

The libiht kernel module/driver provides a set of simple IOCTL operations to interact with the hardware trace capabilities of Intel processors. The IOCTL operations are used to enable/disable, configure, and retrieve the raw hardware trace information from the Last Branch Record (LBR) and Branch Trace Store (BTS) features. The IOCTL operations are organized as follows:

- *Enable trace capabilities* \- Enable the hardware trace capabilities with a specified config (if none, use the default config) for specified process ID and its future children.
- *Disable trace capabilities* \- Disable the hardware trace capabilities for the specified process ID.
- *Config trace information* \- Configure the hardware trace preference (e.g., trace filter, buffer size, etc.) for the specified process ID.
- *Dump trace information* \- Dump the most recent raw hardware trace information for the specified process ID.

For cross-platform compatibility, the IOCTL operations are defined in a generic format and implemented in the kernel module/driver. The user can use one IOCTL request structure to interact all the hardware trace capabilities across different platforms.

## Enable Trace Capabilities

By default, after loading the kernel module/driver, the hardware trace capabilities are disabled and all trace related registers are flushed. The kernel module/driver will expose a process or character device interface to user space applications. To enable the hardware trace capabilities, the user needs to send an IOCTL request with the command code `LIBIHT_IOCTL_ENABLE_LBR` or `LIBIHT_IOCTL_ENABLE_BTS` to the kernel module/driver. The kernel module/driver will enable the hardware trace capabilities with default configuration for the specified process ID and its future children.

It gives the user the flexibility to start tracing the target process only when needed. They can enable the trace as shown below:

```c
struct xioctl_request request;
int fd;

// Open the process or character device
fd = open("/proc/libiht-info", O_RDWR);

// Setup the buffers for storing the trace information
memset(&request, 0, sizeof(request));
request.body.<feature>.buffer  = malloc(<size may varies>);

// Enable the trace capabilities
request.cmd = <feature code>;
```

TODO

## Appendix

### IOCTL Request Command Code

The IOCTL request command code is defined as follows:

```c
enum IOCTL {
    LIBIHT_IOCTL_BASE,          // Placeholder

    // LBR
    LIBIHT_IOCTL_ENABLE_LBR,
    LIBIHT_IOCTL_DISABLE_LBR,
    LIBIHT_IOCTL_DUMP_LBR,
    LIBIHT_IOCTL_CONFIG_LBR,
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
- `LIBIHT_IOCTL_CONFIG_LBR`: Config the Last Branch Record (LBR) hardware trace information
- `LIBIHT_IOCTL_LBR_END`: End of Last Branch Record (LBR) hardware trace commands
- `LIBIHT_IOCTL_ENABLE_BTS`: Enable the Branch Trace Store (BTS) hardware trace capability
- `LIBIHT_IOCTL_DISABLE_BTS`: Disable the Branch Trace Store (BTS) hardware trace capability
- `LIBIHT_IOCTL_DUMP_BTS`: Dump the Branch Trace Store (BTS) hardware trace information
- `LIBIHT_IOCTL_CONFIG_BTS`: Configure the Branch Trace Store (BTS) hardware trace capability
- `LIBIHT_IOCTL_BTS_END`: End of Branch Trace Store (BTS) hardware trace commands

### Generic IOCTL Request Format

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
