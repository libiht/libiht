// Redefine/Copy the structs for IOCTL

// Device name
#define DEVICE_NAME "libiht-info"

// I/O control macros
// TODO: Is IOCTL macros really needed? seems xioctl_request handles it
#define LIBIHT_LKM_IOCTL_MAGIC 'l'
#define LIBIHT_LKM_IOCTL_BASE       _IO(LIBIHT_LKM_IOCTL_MAGIC, 0)

//
// Library constants
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

//
// LBR Type definitions

// Define LBR stack entry
struct lbr_stack_entry
{
    unsigned long long from;   // Retrieve from MSR_LBR_NHM_FROM + offset
    unsigned long long to;     // Retrieve from MSR_LBR_NHM_TO + offset
};

// Define LBR configuration
struct lbr_config
{
    unsigned int pid;                          // Process ID
    unsigned long long lbr_select;                   // MSR_LBR_SELECT
};

// Define LBR data
struct lbr_data
{
    unsigned long long lbr_tos;                      // MSR_LBR_TOS
    struct lbr_stack_entry *entries;  // LBR stack entries
};

// Define the lbr IOCTL structure
struct lbr_ioctl_request{
    struct lbr_config lbr_config;
    struct lbr_data *buffer;
};

//
// BTS Type definitions

// Define BTS record
struct bts_record
{
    unsigned long long from;   // branch from
    unsigned long long to;     // branch to
    unsigned long long misc;   // misc information
};

// Define BTS configuration
struct bts_config
{
    unsigned int pid;                        // Process ID
    unsigned long long bts_config;                 // MSR_IA32_DEBUGCTLMSR
    unsigned long long bts_buffer_size;            // BTS buffer size
};

// Define BTS data
// TODO: pay attention when using this struct in dump bts
struct bts_data
{
    struct bts_record *bts_buffer_base; // BTS buffer base
    unsigned long long bts_index;                      // BTS current index
    unsigned long long bts_absolute_maximum;           // BTS absolute maximum
    unsigned long long bts_interrupt_threshold;        // BTS interrupt threshold
};

// Define the bts IOCTL structure
struct bts_ioctl_request{
    struct bts_config bts_config;
    struct bts_data *buffer;
};

//
// xIOCTL Type definitions

// Define the xIOCTL structure
struct xioctl_request{
    enum IOCTL cmd;
    union {
        struct lbr_ioctl_request lbr;
        struct bts_ioctl_request bts;
    } body;
};