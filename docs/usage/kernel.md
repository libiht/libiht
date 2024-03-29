# Kernel Module/Driver Usage

This document provides an overview of the IOCTL operations and their usage in the kernel module/driver. The kernel module/driver is responsible for retrieving raw hardware trace information from Intel processors. Therefore, it only provides very low level operations to users and hides part of the complexity of hardware-assisted tracing. 

To interact with the kernel module/driver, it requires the user to have a basic understanding of the hardware trace capabilities of Intel processors. For more detailed information about Intel LBR or BTS, please refer to the respective chapter in Volume 3 of the Intel Software Developer's Manual at http://www.intel.com/sdm.

## Introduction

The libiht kernel module/driver provides a set of simple IOCTL operations to interact with the hardware trace capabilities of Intel processors. The IOCTL operations are used to enable/disable, configure, and retrieve the raw hardware trace information from the Last Branch Record (LBR) and Branch Trace Store (BTS) features. The IOCTL operations are organized as follows:

- *Enable trace capabilities* \- Enable the hardware trace capabilities with a specified config (if none, use the default config) for specified process ID and its future children.
- *Disable trace capabilities* \- Disable the hardware trace capabilities for the specified process ID.
- *Config trace information* \- Configure the hardware trace preference (e.g., trace filter, buffer size, etc.) for the specified process ID.
- *Dump trace information* \- Dump the most recent raw hardware trace information for the specified process ID.

For cross-platform compatibility, the IOCTL operations are defined in a generic format and implemented in the kernel module/driver. The user can use one IOCTL request structure to interact all the hardware trace capabilities across different platforms.

## Enable Trace Capabilities

By default, after loading the kernel module/driver, the hardware trace capabilities are disabled and all trace related hardware registers are flushed. The kernel module/driver will expose a process or character device interface to user space applications. To enable the hardware trace capabilities, the user needs to send an IOCTL request with the command code `LIBIHT_IOCTL_ENABLE_LBR` or `LIBIHT_IOCTL_ENABLE_BTS` to the kernel module/driver. The kernel module/driver will enable the hardware trace capabilities with specified configuration for the specified process ID and its future children.

It gives the user the flexibility to start tracing the target process only when needed. They can enable the trace as shown below:

```c
struct xioctl_request request;
int fd, pid;

// Open the process or character device
fd = open(<proc_or_char_dev>, O_RDWR);
pid = <target_pid>;

// Setup the buffers for storing the trace information
memset(&request, 0, sizeof(request));
request.body.<feature>.buffer  = malloc(<size_may_varies>);
// May require multiple malloc setup for the buffer

// Setup the target process ID and the trace configuration
request.body.<feature>.<feature_config>.<specific_config> = <specified_config>;
request.body.<feature>.<feature_config>.pid = pid;

// Enable the trace capabilities
request.cmd = <feature_code>;
ioctl(fd, <feature_code_base>, &request);

// Follow with the crucial operations that we want to trace
...
```

For more details about the buffer setup and raw trace data structure, please check appendix [LBR IOCTL Request](#lbr-ioctl-request) and [BTS IOCTL Request](#bts-ioctl-request) for the specific hardware trace.

## Disable Trace Capabilities

To disable the hardware trace capabilities, the user needs to send an IOCTL request with the command code `LIBIHT_IOCTL_DISABLE_LBR` or `LIBIHT_IOCTL_DISABLE_BTS` to the kernel module/driver. The kernel module/driver will disable the hardware trace capabilities and their traced information for the specified process ID.

It gives the user the flexibility to stop tracing the target process when it is no longer needed. They can disable the trace as shown below:

```c
struct xioctl_request request;
int fd, pid;

// Finish the crucial operations that we want to trace
...

// Open the process or character device
fd = open(<proc_or_char_dev>, O_RDWR);
pid = <target_pid>;

// Setup the target process ID
request.body.<feature>.<feature_config>.pid = pid;

// Disable the trace capabilities
request.cmd = <feature_code>;
ioctl(fd, <feature_code_base>, &request);
```

## Config Trace Information

To configure the hardware trace information, the user needs to send an IOCTL request with the command code `LIBIHT_IOCTL_CONFIG_LBR` or `LIBIHT_IOCTL_CONFIG_BTS` to the kernel module/driver. The kernel module/driver will configure the hardware trace preference (e.g., trace filter, buffer size, etc.) for the specified process ID.

The configuration of the hardware trace information is crucial for the user to capture the desired trace information. It may vary based on the user's requirements and the hardware trace capabilities. (e.g., only want to trace function call jumps with LBR, or want to trace all branches with BTS). Users can configure the trace information as shown below:

```c
struct xioctl_request request;
int fd, pid;

// Open the process or character device
fd = open(<proc_or_char_dev>, O_RDWR);
pid = <target_pid>;

// Setup the target process ID and the trace configuration
request.body.<feature>.<feature_config>.<specific_config> = <specified_config>;
request.body.<feature>.<feature_config>.pid = pid;

// Config the trace information
request.cmd = <feature_code>;
ioctl(fd, <feature_code_base>, &request);
```

For more detailed explanation of the configuration, please check appendix [LBR Configuration](#lbr-configuration) and [BTS Configuration](#bts-configuration) for the specific hardware trace

## Dump Trace Information

To dump the hardware trace information, the user needs to send an IOCTL request with the command code `LIBIHT_IOCTL_DUMP_LBR` or `LIBIHT_IOCTL_DUMP_BTS` to the kernel module/driver. The kernel module/driver will dump the most recent raw hardware trace information for the specified process ID.

The dump operation is crucial for the users and our user space library component to analyze the trace information and understand the control flow behavior of the target program. Users can dump the trace information as shown below:

```c
struct xioctl_request request;
int fd, pid;

// Open the process or character device
fd = open(<proc_or_char_dev>, O_RDWR);
pid = <target_pid>;

// Setup the target process ID
request.body.<feature>.<feature_config>.pid = pid;

// Dump the trace information (Assume the buffer is already allocated)
request.cmd = <feature_code>;
ioctl(fd, <feature_code_base>, &request);
// All the trace information will be copied to the userspace buffer
```

For more details about the buffer setup and raw trace data structure, please check appendix [LBR IOCTL Request](#lbr-ioctl-request) and [BTS IOCTL Request](#bts-ioctl-request) for the specific hardware trace.

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

#### LBR Configuration

The LBR uses the `MSR_LBR_SELECT` register to configure the LBR trace information. The `MSR_LBR_SELECT` register is defined as follows:

```c
/* Bit Field  Bit Offset  Access  Description
 *
 * CPL_EQ_0      0   R/W     When set, do not capture branches ending in ring 0
 * CPL_NEQ_0     1   R/W     When set, do not capture branches ending in ring >0
 * JCC           2   R/W     When set, do not capture conditional branches
 * NEAR_REL_CALL 3   R/W     When set, do not capture near relative calls
 * NEAR_IND_CALL 4   R/W     When set, do not capture near indirect calls
 * NEAR_RET      5   R/W     When set, do not capture near returns
 * NEAR_IND_JMP  6   R/W     When set, do not capture near indirect jumps
 * NEAR_REL_JMP  7   R/W     When set, do not capture near relative jumps
 * FAR_BRANCH    8   R/W     When set, do not capture far branches
 * Reserved      63:9        Must be zero
 *
 * Default selection bit set to:
 * 0x1 = 00000001   --> capture branches occuring in ring >0
 */
#define LBR_SELECT              (1UL <<  0)
```

By default, the `MSR_LBR_SELECT` register is set to capture all branches occurring in ring >0. Users can configure the `MSR_LBR_SELECT` register to filter the LBR trace information based on their requirements.

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

#### BTS Configuration

The BTS uses the `MSR_IA32_DEBUGCTLMSR` register to configure the BTS trace information. The `MSR_IA32_DEBUGCTLMSR` register is defined as follows:

```c
/* CPL-Qualified Branch Trace Store Encodings (Table 18-6 from Intel SDM)
 *
 * TR  BTS  BTS_OFF_OS  BTS_OFF_USR  BTINT  Description
 * 0    X        X           X         X    Branch trace messages (BTMs) off
 * 1    0        X           X         X    Generates BTMs but do not store BTMs
 * 1    1        0           0         0    Store all BTMs in the BTS buffer, 
 *                                          used here as a circular buffer
 * 1    1        1           0         0    Store BTMs with CPL > 0 in the BTS 
 *                                          buffer
 * 1    1        0           1         0    Store BTMs with CPL = 0 in the BTS 
 *                                          buffer
 * 1    1        1           1         X    Generate BTMs but do not store BTMs
 * 1    1        0           0         1    Store all BTMs in the BTS buffer; 
 *                                          generate an interrupt when the 
 *                                          buffer is nearly full
 * 1    1        1           0         1    Store BTMs with CPL > 0 in the BTS 
 *                                          buffer; generate an interrupt when 
 *                                          the buffer is nearly full
 * 1    1        0           1         1    Store BTMs with CPL = 0 in the BTS 
 *                                          buffer; generate an interrupt when 
 *                                          the buffer is nearly full
 */
#define DEFAULT_BTS_CONFIG     (DEBUGCTLMSR_TR | DEBUGCTLMSR_BTS | DEBUGCTLMSR_BTS_OFF_OS)
```

By default, BTS is configured to store all traces in the BTS buffer as a circular buffer. Users can configure the buffer size to store the desired number of records. Base on the design of the processor, the buffer size must be the multiple of `0x18` bytes and the number of records (size / `0x18`) is preferred to be align with a page. The buffer size is defined as follows:

```c
// BTS buffer size 0x200 * 2 = 0x400 = 1024 records
#define DEFAULT_BTS_BUFFER_SIZE        (0x3000 << 1) 
```
