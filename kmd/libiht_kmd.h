#include "../commons/lbr.h"
#include "../commons/cpu.h"
#include "../commons/types.h"
#include "../commons/debug.h"
#include "libiht_kmd_lde64.h"

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
 * The prototype of the original KiSwapContext function prototype
 */
typedef VOID(*KiSwapContext)(void);

 /*
  * The struct used for I/O control communication
  */
struct ioctl_request
{
	u64 lbr_select;
	u32 pid;
};

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

NTSTATUS DriverEntry(PDRIVER_OBJECT driverObject, PUNICODE_STRING regPath);
NTSTATUS DriverExit(PDRIVER_OBJECT driverObject);
