#ifndef _LIBIHT_KMD_H
#define _LIBIHT_KMD_H

////////////////////////////////////////////////////////////////////////////////
//
//  File           : kmd/include/libiht_kmd.h
//  Description    : This is the header file for the kernel mode driver of 
//                   libiht. 
//
//   Author        : Thomason Zhao
//   Last Modified : May 15, 2023
//

// Includes Files
#include "../../commons/lbr.h"
#include "../../commons/bts.h"
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
#define LIBIHT_KMD_IOCTL_TYPE       0x8888
#define LIBIHT_KMD_IOCTL_FUNC       0x888
#define LIBIHT_KMD_IOCTL_BASE       CTL_CODE(LIBIHT_KMD_IOCTL_TYPE, LIBIHT_KMD_IOCTL_FUNC + 0, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// Type definitions

// Nothing here

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

