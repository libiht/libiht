#include "../../commons/api.h"
#include <stdio.h>
#include <Windows.h>
#include <winioctl.h>

#define DEVICE_NAME                L"\\Device\\libiht-info"
#define SYM_DEVICE_NAME            L"\\DosDevices\\libiht-info"

#define KMD_IOCTL_TYPE 0x8888
#define KMD_IOCTL_FUNC 0x888

#define LIBIHT_KMD_IOC_GENERAL_INFO		CTL_CODE(KMD_IOCTL_TYPE, KMD_IOCTL_FUNC + 0, METHOD_BUFFERED, FILE_ANY_ACCESS)

HANDLE hDevice;
DWORD ref_len = 0;

struct xioctl_request send_request;

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
    usr_request.buffer->entries = (struct lbr_stack_entry*)malloc(sizeof(struct lbr_stack_entry) * MAX_LIST_LEN);

    hDevice = CreateFileA("\\\\.\\libiht-info", GENERIC_READ |
        GENERIC_WRITE, 0,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hDevice == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "LIBIHT-API: failed to open device\n");
        CloseHandle(hDevice);
        return usr_request;
    }

    send_request.cmd = LIBIHT_IOCTL_ENABLE_LBR;
    send_request.body.lbr = usr_request;
    int res = DeviceIoControl(hDevice, LIBIHT_KMD_IOC_GENERAL_INFO, &send_request, sizeof(send_request), NULL, 0, NULL, NULL);

    if (res == 0) {
        fprintf(stderr, "LIBIHT-API: enable LBR for pid : %d\n", usr_request.lbr_config.pid);
    }
    else {
        fprintf(stderr, "LIBIHT-API: failed to enable LBR for pid : %d\n", usr_request.lbr_config.pid);
    }

    return usr_request;
}

void disable_lbr(struct lbr_ioctl_request usr_request) {
    send_request.cmd = LIBIHT_IOCTL_DISABLE_LBR;
    send_request.body.lbr = usr_request;
    fprintf(stderr, "LIBIHT-API: disable LBR for pid : %d\n", usr_request.lbr_config.pid);
    DeviceIoControl(hDevice, LIBIHT_KMD_IOC_GENERAL_INFO, &send_request, sizeof(send_request), NULL, 0, NULL, NULL);
    CloseHandle(hDevice);
}

void dump_lbr(struct lbr_ioctl_request usr_request) {
    send_request.cmd = LIBIHT_IOCTL_DUMP_LBR;
    send_request.body.lbr = usr_request;
    fprintf(stderr, "LIBIHT-API: dump LBR for pid : %d\n", usr_request.lbr_config.pid);
    // DeviceIoControl(hDevice, LIBIHT_KMD_IOC_GENERAL_INFO, &send_request, sizeof(send_request), NULL, 0, NULL, NULL);
}

void select_lbr(struct lbr_ioctl_request usr_request) {
    send_request.cmd = LIBIHT_IOCTL_SELECT_LBR;
    send_request.body.lbr = usr_request;
    fprintf(stderr, "LIBIHT-API: select LBR for pid : %d\n", usr_request.lbr_config.pid);
    DeviceIoControl(hDevice, LIBIHT_KMD_IOC_GENERAL_INFO, &send_request, sizeof(send_request), NULL, 0, NULL, NULL);
}
