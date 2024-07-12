////////////////////////////////////////////////////////////////////////////////
//
//  File           : lib/lkm/src/api.c
//  Description    : This is the source code for the Linux kernel module (LKM)
//                   APIs. It contains the implementation of the APIs.
//
//   Author        : Di Wu, Thomason Zhao
//   Last Modified : July 10, 2024
//

// TODO: Refactor the code, current design is bad architected :(

#include "../../commons/api.h"
#include "../include/lkm.h"
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define DEVICE_NAME "libiht-info"

#define LIBIHT_LKM_IOCTL_MAGIC 'l'
#define LIBIHT_LKM_IOCTL_BASE       _IO(LIBIHT_LKM_IOCTL_MAGIC, 0)

//
// Global Variables

int lbr_fd;
// File descriptor for opened LBR

struct xioctl_request lbr_send_request;
// Request for sending to LBR

int bts_fd;
// File descriptor for opened BTS

struct xioctl_request bts_send_request;
// Request for sending to BTS


//
// LBR management functions

////////////////////////////////////////////////////////////////////////////////
//
// Function     : enable_lbr
// Description  : Enable LBR for a given process ID
//
// Inputs       : unsigned int pid : the process ID
// Outputs      : struct lbr_ioctl_request : the request for LBR

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

////////////////////////////////////////////////////////////////////////////////
//
// Function     : disable_lbr
// Description  : Disable LBR for a user request
//
// Inputs       : struct lbr_ioctl_request usr_request : the request for LBR
// Outputs      : void

void disable_lbr(struct lbr_ioctl_request usr_request) {
    lbr_send_request.cmd = LIBIHT_IOCTL_DISABLE_LBR;
    lbr_send_request.body.lbr = usr_request;
    ioctl(lbr_fd, LIBIHT_LKM_IOCTL_BASE, &lbr_send_request);
    fprintf(stderr, "LIBIHT-API: disable LBR for pid %u\n", usr_request.lbr_config.pid);
    lbr_fd = 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : dump_lbr
// Description  : Dump LBR for a user request
//
// Inputs       : struct lbr_ioctl_request usr_request : the request for LBR
// Outputs      : void

void dump_lbr(struct lbr_ioctl_request usr_request) {
    lbr_send_request.cmd = LIBIHT_IOCTL_DUMP_LBR;
    lbr_send_request.body.lbr = usr_request;
    ioctl(lbr_fd, LIBIHT_LKM_IOCTL_BASE, &lbr_send_request);
    fprintf(stderr, "LIBIHT-API: dump LBR for pid %u\n", usr_request.lbr_config.pid);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : config_lbr
// Description  : Configure LBR for a user request
//
// Inputs       : struct lbr_ioctl_request usr_request : the request for LBR
// Outputs      : void

void config_lbr(struct lbr_ioctl_request usr_request) {
    lbr_send_request.cmd = LIBIHT_IOCTL_CONFIG_LBR;
    lbr_send_request.body.lbr = usr_request;
    ioctl(lbr_fd, LIBIHT_LKM_IOCTL_BASE, &lbr_send_request);
    fprintf(stderr, "LIBIHT-API: config LBR for pid %u\n", usr_request.lbr_config.pid);
}

//
// BTS management functions

////////////////////////////////////////////////////////////////////////////////
//
// Function     : enable_bts
// Description  : Enable BTS for a given process ID
//
// Inputs       : unsigned int pid : the process ID
// Outputs      : struct bts_ioctl_request : the request for BTS

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

////////////////////////////////////////////////////////////////////////////////
//
// Function     : disable_bts
// Description  : Disable BTS for a user request
//
// Inputs       : struct bts_ioctl_request usr_request : the request for BTS
// Outputs      : void

void disable_bts(struct bts_ioctl_request usr_request) {
    bts_send_request.cmd = LIBIHT_IOCTL_DISABLE_BTS;
    bts_send_request.body.bts = usr_request;
    ioctl(bts_fd, LIBIHT_LKM_IOCTL_BASE, &bts_send_request);
    fprintf(stderr, "LIBIHT-API: disable BTS for pid : %u\n", usr_request.bts_config.pid);
    bts_fd = 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : dump_bts
// Description  : Dump BTS for a user request
//
// Inputs       : struct bts_ioctl_request usr_request : the request for BTS
// Outputs      : void

void dump_bts(struct bts_ioctl_request usr_request) {
    bts_send_request.cmd = LIBIHT_IOCTL_DUMP_BTS;
    bts_send_request.body.bts = usr_request;
    ioctl(bts_fd, LIBIHT_LKM_IOCTL_BASE, &bts_send_request);
    fprintf(stderr, "LIBIHT-API: dump BTS for pid : %u\n", usr_request.bts_config.pid);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : config_bts
// Description  : Configure BTS for a user request
//
// Inputs       : struct bts_ioctl_request usr_request : the request for BTS
// Outputs      : void

void config_bts(struct bts_ioctl_request usr_request) {
    bts_send_request.cmd = LIBIHT_IOCTL_CONFIG_BTS;
    bts_send_request.body.bts = usr_request;
    ioctl(bts_fd, LIBIHT_LKM_IOCTL_BASE, &bts_send_request);
    fprintf(stderr, "LIBIHT-API: config BTS for pid : %u\n", usr_request.bts_config.pid);
}
