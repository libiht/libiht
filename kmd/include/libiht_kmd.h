#ifndef _LIBIHT_KMD_H
#define _LIBIHT_KMD_H

#include "../../commons/lbr.h"
#include "../../commons/cpu.h"
#include "../../commons/types.h"
#include "../../commons/debug.h"
#include "../infinity_hook/imports.hpp"

/* cpp cross compile handler */
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define LIBIHT_KMD_TAG 'THIL'

/*
 * I/O Device name
 */
#define DEVICE_NAME			L"\\Device\\libiht-info"
#define SYM_DEVICE_NAME		L"\\DosDevices\\libiht-info"

/*
 * I/O control table
 */
#define KMD_IOCTL_TYPE 0x8888
#define KMD_IOCTL_FUNC 0x888

#define LIBIHT_KMD_IOC_ENABLE_TRACE		CTL_CODE(KMD_IOCTL_TYPE, KMD_IOCTL_FUNC + 1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define LIBIHT_KMD_IOC_DISABLE_TRACE    CTL_CODE(KMD_IOCTL_TYPE, KMD_IOCTL_FUNC + 2, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define LIBIHT_KMD_IOC_DUMP_LBR			CTL_CODE(KMD_IOCTL_TYPE, KMD_IOCTL_FUNC + 3, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define LIBIHT_KMD_IOC_SELECT_LBR		CTL_CODE(KMD_IOCTL_TYPE, KMD_IOCTL_FUNC + 4, METHOD_BUFFERED, FILE_ANY_ACCESS)

/*
 * The struct used for I/O control communication
 */
struct ioctl_request
{
	u64 lbr_select;
	u32 pid;
};

KSPIN_LOCK lbr_cache_lock;

/*
 * Function prototypes
 */
ULONG_PTR enable_lbr_wrap(ULONG_PTR info);
ULONG_PTR disable_lbr_wrap(ULONG_PTR info);

BOOLEAN bypass_check_sign(PDRIVER_OBJECT driver_obj);

VOID create_proc_notify(PEPROCESS proc, HANDLE proc_id,
    PPS_CREATE_NOTIFY_INFO create_info);

NTSTATUS device_create(PDRIVER_OBJECT driver_obj);
NTSTATUS device_remove(PDRIVER_OBJECT driver_obj);
NTSTATUS device_ioctl(PDEVICE_OBJECT device_obj, PIRP Irp);
NTSTATUS device_default(PDEVICE_OBJECT device_obj, PIRP Irp);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _LIBIHT_KMD_H

