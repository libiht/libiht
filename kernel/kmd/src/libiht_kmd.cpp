////////////////////////////////////////////////////////////////////////////////
//
//  File           : kernel/kmd/src/libiht_kmd.cpp
//  Description    : This is the main implementation for the kernel mode driver
//                   of libiht. This driver is used to capture the last branch
//                   records of a given process. This file is very platform
//                   specific and thanks to lyshark for sharing the book
//                   "Windows Kernel Programming" and the code on github.
//
//                   Reference: https://github.com/lyshark/WindowsKernelBook
//
//   Author        : Thomason Zhao
//   Last Modified : July 10, 2024
//

// Include Files
#pragma warning(disable : 4200 4201 4819 4311 4302 4996)
#include "../infinity_hook/hook.hpp"
#include "../include/libiht_kmd.h"

// TODO: Determine if this function are necessary, skip make description for now
/*
 * Bypass check sign
 */
BOOLEAN bypass_check_sign(PDRIVER_OBJECT driver_obj)
{
#ifdef _WIN64
    typedef struct _KLDR_DATA_TABLE_ENTRY
    {
        LIST_ENTRY listEntry;
        ULONG64 __Undefined1;
        ULONG64 __Undefined2;
        ULONG64 __Undefined3;
        ULONG64 NonPagedDebugInfo;
        ULONG64 DllBase;
        ULONG64 EntryPoint;
        ULONG SizeOfImage;
        UNICODE_STRING path;
        UNICODE_STRING name;
        ULONG Flags;
        USHORT LoadCount;
        USHORT __Undefined5;
        ULONG64 __Undefined6;
        ULONG CheckSum;
        ULONG __padding1;
        ULONG TimeDateStamp;
        ULONG __padding2;
    } KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY;
#else // _WIN32
    typedef struct _KLDR_DATA_TABLE_ENTRY
    {
        LIST_ENTRY listEntry;
        ULONG unknown1;
        ULONG unknown2;
        ULONG unknown3;
        ULONG unknown4;
        ULONG unknown5;
        ULONG unknown6;
        ULONG unknown7;
        UNICODE_STRING path;
        UNICODE_STRING name;
        ULONG Flags;
    } KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY;
#endif
    PKLDR_DATA_TABLE_ENTRY pLdrData = (PKLDR_DATA_TABLE_ENTRY)driver_obj -> DriverSection;
    pLdrData->Flags = pLdrData->Flags | 0x20;
    return TRUE;
}

//
// Infinity Hook related context manipulation functions

////////////////////////////////////////////////////////////////////////////////
//
// Function     : create_proc_notify
// Description  : This function is used to create a process notifier. It will
//                be called when a process is created or terminated. If the
//                process is created by a parent in the `lbr_state_list`, the
//                child will also be added to the `lbr_state_list`. If the
//                process is terminated, it will be removed from the LBR
//                monitor list.
//
// Inputs       : proc - the process object
//                proc_id - the process id
//                create_info - the created process info
// Outputs      : void

VOID create_proc_notify(PEPROCESS proc, HANDLE proc_id,
    PPS_CREATE_NOTIFY_INFO create_info)
{
    UNREFERENCED_PARAMETER(proc);
    if (create_info != NULL)
    {
        // Process is being created
        lbr_newproc_handler((u32)(UINT_PTR)create_info->ParentProcessId, (u32)proc_id);
        bts_newproc_handler((u32)(UINT_PTR)create_info->ParentProcessId, (u32)proc_id);
    }
    else
    {
        // Process is being terminated
        // TODO: remove the process? But this feature not implemented on lkm
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : cswitch_call_back
// Description  : This function is used to hook the context switch. It will be
//                called when a context switch happens. If the new process is
//                in the `lbr_state_list`, it will be set up to monitor the LBR
//                of the new process. If the old process is in the
//                `lbr_state_list`, it will be removed from the LBR monitor
//                list.
//
// Inputs       : new_proc - the new process id to be switched to
//                old_proc - the old process id to be switched from
// Outputs      : void

void __fastcall cswitch_call_back(u32 new_proc, u32 old_proc)
{
    lbr_cswitch_handler(old_proc, new_proc);
    bts_cswitch_handler(old_proc, new_proc);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : infinity_hook_create
// Description  : This function is used to initialize and start the context
//                switch hook.
//
// Inputs       : void
// Outputs      : NTSTATUS - the status of the hook

NTSTATUS infinity_hook_create()
{
    // Initialize and hook
    return k_hook::initialize(cswitch_call_back) && k_hook::start() ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : infinity_hook_remove
// Description  : This function is used to stop and remove the context switch
//                hook.
//
// Inputs       : void
// Outputs      : NTSTATUS - the status of the hook

NTSTATUS infinity_hook_remove()
{
    k_hook::stop();

    // Here you need to make sure that the execution point of the system is no
    // longer in the current driver
    // For example, the current driver is unloaded, but the MyNtCreateFile you
    // hooked is still executing the for operation, of course, the blue screen
    // The sleep 10 second method here can be directly improved
    LARGE_INTEGER integer{ 0 };
    integer.QuadPart = -10000;
    integer.QuadPart *= 10000;
    KeDelayExecutionThread(KernelMode, FALSE, &integer);

    return STATUS_SUCCESS;
}

//
// Device related manipulation functions

////////////////////////////////////////////////////////////////////////////////
//
// Function     : device_create
// Description  : This function is used to create a device object and a
//                symbolic link for user interactive helper.
//
// Inputs       : driver_obj - the driver object
// Outputs      : NTSTATUS - the status of the device creation

NTSTATUS device_create(PDRIVER_OBJECT driver_obj)
{
    NTSTATUS status;
    PDEVICE_OBJECT device_obj;
    UNICODE_STRING device_name, sym_device_name;

    // Set dispatch routines
    for (ULONG i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
        driver_obj->MajorFunction[i] = device_default;
    }
    driver_obj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = device_ioctl;

    // Create device object
    RtlInitUnicodeString(&device_name, DEVICE_NAME);
    status = IoCreateDevice(driver_obj, 0, &device_name, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &device_obj);
    if (!NT_SUCCESS(status)) {
        xprintdbg("Failed to create device object\n");
        return status;
    }

    // Create symbolic link
    RtlInitUnicodeString(&sym_device_name, SYM_DEVICE_NAME);
    status = IoCreateSymbolicLink(&sym_device_name, &device_name);
    if (!NT_SUCCESS(status)) {
        KdPrint(("Failed to create symbolic link\n"));
        IoDeleteDevice(device_obj);
        return status;
    }

    return STATUS_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : device_remove
// Description  : This function is used to remove the device object and the
//                symbolic link for user interactive helper.
//
// Inputs       : driver_obj - the driver object
// Outputs      : NTSTATUS - the status of the device removal

NTSTATUS device_remove(PDRIVER_OBJECT driver_obj)
{
    PDEVICE_OBJECT device_obj = driver_obj->DeviceObject;
    UNICODE_STRING sym_device_name;
    RtlInitUnicodeString(&sym_device_name, SYM_DEVICE_NAME);
    IoDeleteSymbolicLink(&sym_device_name);
    IoDeleteDevice(device_obj);
    return STATUS_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : device_ioctl
// Description  : This function is used to handle the ioctl request from user
//                interactive helper.
//
// Inputs       : device_obj - the device object
//                Irp - the I/O request packet
// Outputs      : NTSTATUS - the status of the ioctl request

NTSTATUS device_ioctl(PDEVICE_OBJECT device_obj, PIRP Irp)
{
    PIO_STACK_LOCATION irp_stack;
    ULONG ioctl_cmd;
    struct xioctl_request* request;
    u64 request_size;
    NTSTATUS status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(device_obj);

    // Copy user request
    irp_stack = IoGetCurrentIrpStackLocation(Irp);
    ioctl_cmd = irp_stack->Parameters.DeviceIoControl.IoControlCode;
    request_size = irp_stack->Parameters.DeviceIoControl.InputBufferLength;
    request = (struct xioctl_request*)Irp->AssociatedIrp.SystemBuffer; // Input buffer

    if (request_size != sizeof(xioctl_request))
    {
        xprintdbg("LIBIHT-KMD: Wrong request size of %ld, expect: %ld\n", request_size, sizeof(xioctl_request));
        status = STATUS_INVALID_DEVICE_REQUEST;
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return status;
    }

    // Process request
    if (request->cmd <= LIBIHT_IOCTL_LBR_END)
    {
        // LBR request
        xprintdbg("LIBIHT-KMD: LBR request\n");
        if (lbr_ioctl_handler(request) != 0)
            status = STATUS_UNSUCCESSFUL;
    }
	else if (request->cmd <= LIBIHT_IOCTL_BTS_END)
	{
		// BTS request
		xprintdbg("LIBIHT-KMD: BTS request\n");
		if (bts_ioctl_handler(request) != 0)
			status = STATUS_UNSUCCESSFUL;
	}
	else
	{
		// Unknown request
		xprintdbg("LIBIHT-KMD: Unknown request\n");
		status = STATUS_INVALID_DEVICE_REQUEST;
	}

    // Complete the request
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : device_default
// Description  : This function is used to handle the default request from user
//                interactive helper.
//
// Inputs       : device_obj - the device object
//                Irp - the I/O request packet
// Outputs      : NTSTATUS - the status of the default request

NTSTATUS device_default(PDEVICE_OBJECT device_obj, PIRP Irp)
{
    UNREFERENCED_PARAMETER(device_obj);
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

//
// Driver manipulation functions

////////////////////////////////////////////////////////////////////////////////
//
// Function     : DriverEntry
// Description  : This function is the main entry point for the driver. It will
//                initialize the driver and register all the required services.
//
// Inputs       : driver_obj - the driver object
//                reg_path - the registry path
// Outputs      : NTSTATUS - the status of the driver initialization

NTSTATUS DriverEntry(PDRIVER_OBJECT driver_obj, PUNICODE_STRING reg_path)
{
    NTSTATUS status;
    UNREFERENCED_PARAMETER(reg_path);
    driver_obj->DriverUnload = (PDRIVER_UNLOAD)DriverExit;

    xprintdbg("LIBIHT-KMD: Initializing...\n");

    // Bypass check sign
    // LINKER_FLAGS=/INTEGRITYCHECK
    bypass_check_sign(driver_obj);

    // Create user interactive helper device
    xprintdbg("LIBIHT-KMD: Creating helper device...\n");
    status = device_create(driver_obj);
    if (!NT_SUCCESS(status))
        return status;

    // Register create process notifier
    xprintdbg("LIBIHT-KMD: Registering create proc notifier...\n");
    status = PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)create_proc_notify, FALSE);
    if (!NT_SUCCESS(status))
        return status;

    // Init & Register hooks on context switches
    xprintdbg("LIBIHT-KMD: Initializing & Registering context switch hooks...\n");
    status = infinity_hook_create();
    if (!NT_SUCCESS(status))
        return status;

    // Init LBR
    lbr_init();

    // Init BTS
    bts_init();

    xprintdbg("LIBIHT-KMD: Initialized\n");
    return STATUS_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : DriverExit
// Description  : This function is the main exit point for the driver. It will
//                unregister all the registered services and clean up the
//                driver.
//
// Inputs       : driver_obj - the driver object
// Outputs      : NTSTATUS - the status of the driver exit

NTSTATUS DriverExit(PDRIVER_OBJECT driver_obj)
{
    NTSTATUS status;
    UNREFERENCED_PARAMETER(driver_obj);

    xprintdbg("LIBIHT-KMD: Exiting...\n");

    // Exit BTS
    bts_exit();

    // Exit LBR
    lbr_exit();

    // Unregister hooks on context switches.
    xprintdbg("LIBIHT-KMD: Unregistering context switch hooks (may take around 10s)...\n");
    status = infinity_hook_remove();
    if (!NT_SUCCESS(status))
        return status;

    // Unregister create process notifier
    xprintdbg("LIBIHT-KMD: Unregistering create proc notifier...\n");
    status = PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)create_proc_notify, TRUE);
    if (!NT_SUCCESS(status))
        return status;

    // Remove the helper device if exist
    xprintdbg("LIBIHT-KMD: Removing helper device...\n");
    status = device_remove(driver_obj);
    if (!NT_SUCCESS(status))
        return status;

    xprintdbg("LIBIHT-KMD: Exit complete\n");
    return STATUS_SUCCESS;
}
