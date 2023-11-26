#ifndef _LIBIHT_KMD_H
#define _LIBIHT_KMD_H

////////////////////////////////////////////////////////////////////////////////
//
//  File           : kmd/include/libiht_kmd.h
//  Description    : This is the header file for the kernel mode driver of 
//                   libiht. 
//
//   Author        : Thomason Zhao
//   Last Modified : Nov 26, 2023
//

// Includes Files
#include "../../commons/lbr.h"
#include "../../commons/cpu.h"
#include "../../commons/types.h"
#include "../../commons/debug.h"
#include "../infinity_hook/imports.hpp"

// cpp cross compile handler
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//
// Library constants

// Device specification
#define DEVICE_NAME         L"\\Device\\libiht-info"
#define SYM_DEVICE_NAME     L"\\DosDevices\\libiht-info"

// I/O control codes
#define KMD_IOCTL_TYPE 0x8888
#define KMD_IOCTL_FUNC 0x888

#define LIBIHT_KMD_IOC_ENABLE_TRACE     CTL_CODE(KMD_IOCTL_TYPE, KMD_IOCTL_FUNC + 1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define LIBIHT_KMD_IOC_DISABLE_TRACE    CTL_CODE(KMD_IOCTL_TYPE, KMD_IOCTL_FUNC + 2, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define LIBIHT_KMD_IOC_DUMP_LBR         CTL_CODE(KMD_IOCTL_TYPE, KMD_IOCTL_FUNC + 3, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define LIBIHT_KMD_IOC_SELECT_LBR       CTL_CODE(KMD_IOCTL_TYPE, KMD_IOCTL_FUNC + 4, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// Type definitions

// IOCTL request structure
struct ioctl_request
{
    u64 lbr_select; // LBR selection bit
    u32 pid;        // Process ID
};

KSPIN_LOCK lbr_cache_lock;

//
// Function Prototypes

// TODO: Determine if this function are necessary
BOOLEAN bypass_check_sign(PDRIVER_OBJECT driver_obj);

VOID create_proc_notify(PEPROCESS proc, HANDLE proc_id,
                        PPS_CREATE_NOTIFY_INFO create_info);
// This function is used to create a process notifier

NTSTATUS device_create(PDRIVER_OBJECT driver_obj);
// This function is used to create a device object

NTSTATUS device_remove(PDRIVER_OBJECT driver_obj);
// This function is used to remove a device object

NTSTATUS device_ioctl(PDEVICE_OBJECT device_obj, PIRP Irp);
// This function is used to handle IOCTL requests

NTSTATUS device_default(PDEVICE_OBJECT device_obj, PIRP Irp);
// This function is used to handle default requests

NTSTATUS DriverEntry(PDRIVER_OBJECT driverObject, PUNICODE_STRING regPath);
// This function is used to initialize the driver

NTSTATUS DriverExit(PDRIVER_OBJECT driverObject);
// This function is used to exit the driver

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _LIBIHT_KMD_H

