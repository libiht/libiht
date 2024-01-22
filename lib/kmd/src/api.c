#include "../../commons/api.h"
#include <iostream>
#include <Windows.h>
#include <winioctl.h>

#define DEVICE_NAME                L"\\Device\\libiht-info"
#define SYM_DEVICE_NAME            L"\\DosDevices\\libiht-info"

#define KMD_IOCTL_TYPE 0x8888
#define KMD_IOCTL_FUNC 0x888

HANDLE hDevice;
struct xioctl_request input,output;
DWORD ref_len = 0;

struct lbr_ioctl_request enalbe_lbr(){
    struct lbr_ioctl_request usr_request;
    usr_request.pid = GetCurrentProcessId();
    usr_request.lbr_config.lbr_select = 0;
    usr_request.buffer->lbr_tos = 0;
    usr_request.buffer->entries = 0;
    
    hDevice = CreatFileA("\\\\.\\libiht-info", GENERIC_READ |
        GENERIC_WRITE, 0,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NROMAL, NULL);
    if(hDevice == INVALID_HANDLE_VALUE){
        xprintdbg("Falied to open device!\n");
        CloseHandle(hDevice);
        return usr_request;
    }

    input.cmd = LIBIHT_IOCTL_ENABLE_LBR;
    input.body.lbr = usr_request;
    // DeviceIoControl(hDevice, LIBIHT_KMD_IOC_ENABLE_TRACE, &input, sizeof(input), &output, sizeof(output), &ref_len, 0)
    return usr_request;
}

void disable_lbr(struct lbr_ioctl_reqeust usr_request){
    input.cmd = LIBIHT_IOCTL_ENABLE_LBR;
    input.body.lbr = usr_request;
    // DeviceIoControl(hDevice, LIBIHT_KMD_IOC_DISABLE_TRACE, &input, sizeof(input), &output, sizeof(output), &ref_len, 0);
    CliseHandle(hDevice);
}

void dump_lbr(struct lbr_ioctl_request usr_request){
    input.cmd = LIBIHT_IOCTL_DUMP_LBR;
    input.body.lbr = usr_request;
    // DevieIoControl(hDevice, LIBIHT_KMD_IOC_DUMP_LBR, &input, sizeof(input), &output, sizeof(output), &ref_len, 0);
}

void select_lbr(struct lbr_ioctl_request usr_request){
    input.cmd = LIBIHT_IOCTL_SELECT_LBR;
    input.body.lbr = usr_request;
    // DeviceIoControl(hDevice, LIBIHT_KMD_IOC_SELECT_LBR, &input, sizeof(input), &output, sizeof(output), &ref_len, 0);
}