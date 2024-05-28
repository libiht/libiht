#include "../../commons/api.h"
#include "../include/lkm.h"
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#define DEVICE_NAME "libiht-info"

#define LIBIHT_LKM_IOCTL_MAGIC 'l'
#define LIBIHT_LKM_IOCTL_BASE       _IO(LIBIHT_LKM_IOCTL_MAGIC, 0)

int lbr_fd;

struct xioctl_request lbr_send_request;

struct lbr_ioctl_request enable_lbr(unsigned int pid) {
    struct lbr_ioctl_request usr_request;
    if (pid == 0) {
        usr_request.lbr_config.pid = getpid();
    }
    else {
        usr_request.lbr_config.pid = pid;
    }

    fprintf(stderr, "LIBIHT-API: starting enable LBR on pid : %u\n", usr_request.lbr_config.pid);

    usr_request.lbr_config.lbr_select = 0;

    usr_request.buffer = NULL;

    usr_request.buffer = malloc(sizeof(struct lbr_data));
    usr_request.buffer->lbr_tos = 0;
    usr_request.buffer->entries = malloc(sizeof(struct lbr_stack_entry) * MAX_LBR_LIST_LEN);

    lbr_fd = open("/proc/" DEVICE_NAME, O_RDWR);

    lbr_send_request.cmd = LIBIHT_IOCTL_ENABLE_LBR;
    lbr_send_request.body.lbr = usr_request;
    int res = ioctl(lbr_fd, LIBIHT_LKM_IOCTL_BASE, &lbr_send_request);

    if (res == 0) {
        fprintf(stderr, "LIBIHT-API: enable LBR for pid %u\n", usr_request.lbr_config.pid);
    }
    else {
        fprintf(stderr, "LIBIHT-API: failed to enable LBR for pid %u\n", usr_request.lbr_config.pid);
    }

    return usr_request;
}

void disable_lbr(struct lbr_ioctl_request usr_request) {
    lbr_send_request.cmd = LIBIHT_IOCTL_DISABLE_LBR;
    lbr_send_request.body.lbr = usr_request;
    fprintf(stderr, "LIBIHT-API: disable LBR for pid %u\n", usr_request.lbr_config.pid);
    ioctl(lbr_fd, LIBIHT_LKM_IOCTL_BASE, &lbr_send_request);
    lbr_fd = 0;
}

void dump_lbr(struct lbr_ioctl_request usr_request) {
    lbr_send_request.cmd = LIBIHT_IOCTL_DUMP_LBR;
    lbr_send_request.body.lbr = usr_request;
    fprintf(stderr, "LIBIHT-API: dump LBR for pid %u\n", usr_request.lbr_config.pid);
    ioctl(lbr_fd, LIBIHT_LKM_IOCTL_BASE, &lbr_send_request);
}

void select_lbr(struct lbr_ioctl_request usr_request) {
    lbr_send_request.cmd = LIBIHT_IOCTL_SELECT_LBR;
    lbr_send_request.body.lbr = usr_request;
    fprintf(stderr, "LIBIHT-API: select LBR for pid %u\n", usr_request.lbr_config.pid);
    ioctl(lbr_fd, LIBIHT_LKM_IOCTL_BASE, &lbr_send_request);
}

int bts_fd;

struct xioctl_request bts_send_request;

struct bts_ioctl_request enable_bts(unsigned int pid) {
    struct bts_ioctl_request usr_request;
    if (pid == 0) {
        usr_request.bts_config.pid = getpid();
    }
    else {
        usr_request.bts_config.pid = pid;
    }

    fprintf(stderr, "LIBIHT-API: starting enable BTS on pid : %u\n", usr_request.bts_config.pid);

    usr_request.bts_config.bts_config = 0;
    usr_request.bts_config.bts_buffer_size = 0;
    usr_request.buffer = malloc(sizeof(struct bts_data));
    usr_request.buffer->bts_buffer_base = malloc(sizeof(struct bts_record) * MAX_BTS_LIST_LEN);
    usr_request.buffer->bts_index = malloc(sizeof(struct bts_record) * MAX_BTS_LIST_LEN);

    bts_fd = open("/proc/" DEVICE_NAME, O_RDWR);

    bts_send_request.body.bts = usr_request;
    bts_send_request.cmd = LIBIHT_IOCTL_ENABLE_BTS;
    int res = ioctl(bts_fd, LIBIHT_LKM_IOCTL_BASE, &bts_send_request);

    if (res == 0) {
        fprintf(stderr, "LIBIHT-API: enable BTS for pid %u\n", usr_request.bts_config.pid);
    }
    else {
        fprintf(stderr, "LIBIHT-API: failed to enable BTS for pid %u\n", usr_request.bts_config.pid);
    }

    return usr_request;
}

void disable_bts(struct bts_ioctl_request usr_request) {
    bts_send_request.cmd = LIBIHT_IOCTL_DISABLE_BTS;
    bts_send_request.body.bts = usr_request;
    fprintf(stderr, "LIBIHT-API: disable BTS for pid : %u\n", usr_request.bts_config.pid);
    ioctl(bts_fd, LIBIHT_LKM_IOCTL_BASE, &bts_send_request);
    bts_fd = 0;
}

void dump_bts(struct bts_ioctl_request usr_request) {
    bts_send_request.cmd = LIBIHT_IOCTL_DUMP_BTS;
    bts_send_request.body.bts = usr_request;
    fprintf(stderr, "LIBIHT-API: dump BTS for pid : %u\n", usr_request.bts_config.pid);
    ioctl(bts_fd, LIBIHT_LKM_IOCTL_BASE, &bts_send_request);
}

void config_bts(struct bts_ioctl_request usr_request) {
    bts_send_request.cmd = LIBIHT_IOCTL_CONFIG_BTS;
    bts_send_request.body.bts = usr_request;
    fprintf(stderr, "LIBIHT-API: config BTS for pid : %u\n", usr_request.bts_config.pid);
    ioctl(bts_fd, LIBIHT_LKM_IOCTL_BASE, &bts_send_request);
}
