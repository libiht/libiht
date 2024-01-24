#include <unistd.h>

#define MAX_LIST_LEN    0x20

enum IOCTL{
    LIBIHT_IOCTL_BASE,

    LIBIHT_IOCTL_ENABLE_LBR,
    LIBIHT_IOCTL_DISABLE_LBR,
    LIBIHT_IOCTL_DUMP_LBR,
    LIBIHT_IOCTL_SELECT_LBR,
    LIBIHT_IOCTL_LBR_END,

    LIBIHT_IOCTL_ENABLE_BTS,
    LIBIHT_IOCTL_DISABLE_BTS,
    LIBIHT_IOCTL_DUMP_BTS,
    LIBIHT_IOCTL_CONFIG_BTS,
    LIBIHT_IOCTL_BTS_END,
};

struct lbr_stack_entry{
    unsigned long long from;
    unsigned long long to;
};

struct lbr_config{
    unsigned int pid;
    unsigned long long lbr_select;
};

struct lbr_data{
    unsigned long long lbr_tos;
    struct lbr_stack_entry *entries;
};

struct lbr_ioctl_request{
    struct lbr_config lbr_config;
    struct lbr_data *buffer;
};

struct bts_config{
    unsigned int pid;
    unsigned long long bts_config;
    unsigned long long bts_buffer_size;
};

struct bts_record{
    unsigned long long from;
    unsigned long long to;
    unsigned long long misc;
};

struct bts_ioctl_request{
    struct bts_config bts_config;
    struct bts_data *buffer;
};

struct xioctl_request{
    enum IOCTL cmd;
    union{
        struct lbr_ioctl_request lbr;
        struct bts_ioctl_request bts;
    }body;
};

struct lbr_ioctl_request enable_lbr();
void disable_lbr(struct lbr_ioctl_request usr_request);
void dump_lbr(struct lbr_ioctl_request usr_request);
void select_lbr(struct lbr_ioctl_request usr_request);