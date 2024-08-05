////////////////////////////////////////////////////////////////////////////////
//
//  File           : lib/kmd/kmd/kmd.cpp
//  Description    : This is the source file for the kernel module driver (KMD)
//                   APIs. It contains the implementation of the APIs.
//
//   Author        : Di Wu, Thomason Zhao
//   Last Modified : July 21, 2024
//

#include "pch.h" // use stdafx.h in Visual Studio 2017 and earlier
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winioctl.h>
#include "kmd.h"

#define DEVICE_NAME                L"\\Device\\libiht-info"
#define SYM_DEVICE_NAME            L"\\DosDevices\\libiht-info"

#define KMD_IOCTL_TYPE 0x8888
#define KMD_IOCTL_FUNC 0x888

#define LIBIHT_KMD_IOCTL_BASE       CTL_CODE(KMD_IOCTL_TYPE, KMD_IOCTL_FUNC + 0, METHOD_BUFFERED, FILE_ANY_ACCESS)

HANDLE lbr_hDevice;

struct xioctl_request lbr_send_request;

////////////////////////////////////////////////////////////////////////////////
//
// Function     : enable_lbr
// Description  : Enable the LBR feature for the requested process id.
//
// Inputs       : pid - the process identifier
// Outputs      : struct lbr_ioctl_request - the LBR configuration request 
struct lbr_ioctl_request enable_lbr(unsigned int pid) {
    struct lbr_ioctl_request usr_request;
    if (pid == 0) {
        usr_request.lbr_config.pid = GetCurrentProcessId();
    }
    else {
        usr_request.lbr_config.pid = pid;
    }
    usr_request.lbr_config.lbr_select = 0;

    fprintf(stderr, "LIBIHT-API: starting enable LBR on pid : %u\n", usr_request.lbr_config.pid);

    usr_request.buffer = (struct lbr_data*)malloc(sizeof(struct lbr_data));
    usr_request.buffer->lbr_tos = 0;
    usr_request.buffer->entries = (struct lbr_stack_entry*)malloc(sizeof(struct lbr_stack_entry) * MAX_LBR_LIST_LEN);

    lbr_hDevice = CreateFileA("\\\\.\\libiht-info", GENERIC_READ |
        GENERIC_WRITE, 0,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (lbr_hDevice == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "LIBIHT-API: failed to open device\n");
        return usr_request;
    }

    lbr_send_request.cmd = LIBIHT_IOCTL_ENABLE_LBR;
    lbr_send_request.body.lbr = usr_request;
    int res = DeviceIoControl(lbr_hDevice, LIBIHT_KMD_IOCTL_BASE, &lbr_send_request, sizeof(lbr_send_request), NULL, 0, NULL, NULL);

    if (res == 0) {
        fprintf(stderr, "LIBIHT-API: enable LBR for pid : %d\n", usr_request.lbr_config.pid);
    }
    else {
        fprintf(stderr, "LIBIHT-API: failed to enable LBR for pid : %d\n", usr_request.lbr_config.pid);
    }

    return usr_request;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : disable_lbr
// Description  : Disable the LBR feature for the specified process.
//
// Inputs       : usr_request - the LBR configuration request structure
// Outputs      : None
void disable_lbr(struct lbr_ioctl_request usr_request) {
    lbr_send_request.cmd = LIBIHT_IOCTL_DISABLE_LBR;
    lbr_send_request.body.lbr = usr_request;
    fprintf(stderr, "LIBIHT-API: disable LBR for pid : %d\n", usr_request.lbr_config.pid);
    DeviceIoControl(lbr_hDevice, LIBIHT_KMD_IOCTL_BASE, &lbr_send_request, sizeof(lbr_send_request), NULL, 0, NULL, NULL);
    CloseHandle(lbr_hDevice);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : dump_lbr
// Description  : Dump the Last Branch Recording (LBR) for the specified process.
//
// Inputs       : usr_request - the LBR configuration request structure
// Outputs      : None
void dump_lbr(struct lbr_ioctl_request usr_request) {
    lbr_send_request.cmd = LIBIHT_IOCTL_DUMP_LBR;
    lbr_send_request.body.lbr = usr_request;
    fprintf(stderr, "LIBIHT-API: dump LBR for pid : %d\n", usr_request.lbr_config.pid);
    DeviceIoControl(lbr_hDevice, LIBIHT_KMD_IOCTL_BASE, &lbr_send_request, sizeof(lbr_send_request), NULL, 0, NULL, NULL);
}


////////////////////////////////////////////////////////////////////////////////
//
// Function     : config_lbr
// Description  : Configure the LBR for the specified process.
//
// Inputs       : usr_request - the LBR configuration request structure
// Outputs      : None
void config_lbr(struct lbr_ioctl_request usr_request) {
    lbr_send_request.cmd = LIBIHT_IOCTL_CONFIG_LBR;
    lbr_send_request.body.lbr = usr_request;
    fprintf(stderr, "LIBIHT-API: select LBR for pid : %d\n", usr_request.lbr_config.pid);
    DeviceIoControl(lbr_hDevice, LIBIHT_KMD_IOCTL_BASE, &lbr_send_request, sizeof(lbr_send_request), NULL, 0, NULL, NULL);
}

HANDLE bts_hDevice;
struct xioctl_request bts_send_request;


////////////////////////////////////////////////////////////////////////////////
//
// Function     : enable_bts
// Description  : Enable the Branch Trace Store (BTS) for the specified process.
//
// Inputs       : pid - the process identifier
// Outputs      : struct bts_ioctl_request - the BTS configuration request structure
struct bts_ioctl_request enable_bts(unsigned int pid) {
    struct bts_ioctl_request usr_request;
    if (pid == 0) {
        usr_request.bts_config.pid = GetCurrentProcessId();
    }
    else {
        usr_request.bts_config.pid = pid;
    }

    fprintf(stderr, "LIBIHT-API: starting enable BTS on pid : %u\n", usr_request.bts_config.pid);

    usr_request.bts_config.bts_config = 0;
    usr_request.bts_config.bts_buffer_size = 0;
    usr_request.buffer = (struct bts_data*)malloc(sizeof(struct bts_data));
    usr_request.buffer->bts_buffer_base = (struct bts_record*)malloc(sizeof(struct bts_record) * MAX_BTS_LIST_LEN);
    usr_request.buffer->bts_index = (struct bts_record*)malloc(sizeof(struct bts_record) * MAX_BTS_LIST_LEN);

    bts_hDevice = CreateFileA("\\\\.\\libiht-info", GENERIC_READ |
        GENERIC_WRITE, 0,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (bts_hDevice == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "LIBIHT-API: failed to open device\n");
        return usr_request;
    }

    bts_send_request.body.bts = usr_request;
    bts_send_request.cmd = LIBIHT_IOCTL_ENABLE_BTS;
    int res = DeviceIoControl(bts_hDevice, LIBIHT_KMD_IOCTL_BASE, &bts_send_request, sizeof(bts_send_request), NULL, 0, NULL, NULL);

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
// Description  : Disable the Branch Trace Store (BTS) for the specified process.
//
// Inputs       : usr_request - the BTS configuration request structure
// Outputs      : None
void disable_bts(struct bts_ioctl_request usr_request) {
    bts_send_request.cmd = LIBIHT_IOCTL_DISABLE_BTS;
    bts_send_request.body.bts = usr_request;
    fprintf(stderr, "LIBIHT-API: disable BTS for pid : %u\n", usr_request.bts_config.pid);
    DeviceIoControl(bts_hDevice, LIBIHT_KMD_IOCTL_BASE, &bts_send_request, sizeof(bts_send_request), NULL, 0, NULL, NULL);
    CloseHandle(bts_hDevice);
}


////////////////////////////////////////////////////////////////////////////////
//
// Function     : dump_bts
// Description  : Dump the Branch Trace Store (BTS) for the specified process.
//
// Inputs       : usr_request - the BTS configuration request structure
// Outputs      : None
void dump_bts(struct bts_ioctl_request usr_request) {
    bts_send_request.cmd = LIBIHT_IOCTL_DUMP_BTS;
    bts_send_request.body.bts = usr_request;
    fprintf(stderr, "LIBIHT-API: dump BTS for pid : %u\n", usr_request.bts_config.pid);
    DeviceIoControl(bts_hDevice, LIBIHT_KMD_IOCTL_BASE, &bts_send_request, sizeof(bts_send_request), NULL, 0, NULL, NULL);
}


////////////////////////////////////////////////////////////////////////////////
//
// Function     : config_bts
// Description  : Configure the Branch Trace Store (BTS) for the specified process.
//
// Inputs       : usr_request - the BTS configuration request structure
// Outputs      : None
void config_bts(struct bts_ioctl_request usr_request) {
    bts_send_request.cmd = LIBIHT_IOCTL_CONFIG_BTS;
    bts_send_request.body.bts = usr_request;
    fprintf(stderr, "LIBIHT-API: config BTS for pid : %u\n", usr_request.bts_config.pid);
    DeviceIoControl(bts_hDevice, LIBIHT_KMD_IOCTL_BASE, &bts_send_request, sizeof(bts_send_request), NULL, 0, NULL, NULL);
}