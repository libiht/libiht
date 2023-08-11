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
 * The prototype of the original NtCreateUserProcess function prototype
 */
typedef NTSTATUS(*NtCreateUserProcess)(
    OUT PHANDLE ProcessHandle,
    OUT PHANDLE ThreadHandle,
    ACCESS_MASK ProcessDesiredAccess,
    ACCESS_MASK ThreadDesiredAccess,
    POBJECT_ATTRIBUTES ProcessObjectAttributes OPTIONAL,
    POBJECT_ATTRIBUTES ThreadObjectAttributes OPTIONAL,
    ULONG ProcessFlags,
    ULONG ThreadFlags,
    PVOID ProcessParameters OPTIONAL,
    PVOID CreateInfo,
    PVOID AttributeList OPTIONAL
);

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
KIPI_BROADCAST_WORKER enable_lbr_wrap;
KIPI_BROADCAST_WORKER disable_lbr_wrap;

VOID lde_init(void);
VOID lde_destroy(void);
ULONG get_full_patch_size(PUCHAR addr);

KIRQL wpage_offx64(void);
VOID wpage_onx64(KIRQL irql);
PVOID get_func_addr(PCWSTR func_name);
PVOID kernel_api_hook(PVOID api_addr, PVOID proxy_addr, OUT PVOID* ori_api_addr, OUT ULONG* patch_size);
VOID kernel_api_unhook(PVOID api_addr, PVOID ori_api_addr, ULONG patch_size);

NTSTATUS NtCreateUserProcess_hook(
    OUT PHANDLE ProcessHandle,
    OUT PHANDLE ThreadHandle,
    ACCESS_MASK ProcessDesiredAccess,
    ACCESS_MASK ThreadDesiredAccess,
    POBJECT_ATTRIBUTES ProcessObjectAttributes OPTIONAL,
    POBJECT_ATTRIBUTES ThreadObjectAttributes OPTIONAL,
    ULONG ProcessFlags,
    ULONG ThreadFlags,
    PVOID ProcessParameters OPTIONAL,
    PVOID CreateInfo,
    PVOID AttributeList OPTIONAL
);
NTSTATUS NCUP_hook_create(void);
NTSTATUS NCUP_hook_remove(void);

NTSTATUS device_create(PDRIVER_OBJECT driver_obj);
NTSTATUS device_remove(PDRIVER_OBJECT driver_obj);
NTSTATUS device_ioctl(PDEVICE_OBJECT device_obj, PIRP Irp);
NTSTATUS device_default(PDEVICE_OBJECT device_obj, PIRP Irp);

NTSTATUS DriverEntry(PDRIVER_OBJECT driverObject, PUNICODE_STRING regPath);
NTSTATUS DriverExit(PDRIVER_OBJECT driverObject);
