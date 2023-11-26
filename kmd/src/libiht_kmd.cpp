////////////////////////////////////////////////////////////////////////////////
//
//  File           : kmd/src/main.cpp
//  Description    : This is the main implementation for the kernel mode driver 
//                   of libiht. This driver is used to capture the last branch
//                   records of a given process. This file is very platform
//                   specific and thanks to lyshark for sharing the book
//                   "Windows Kernel Programming" and the code on github.
//                   Reference: https://github.com/lyshark/WindowsKernelBook
//
//   Author        : Thomason Zhao
//   Last Modified : Nov 25, 2023
//

// Include Files
#pragma warning(disable : 4201 4819 4311 4302 4996)
#include "../infinity_hook/hook.hpp"
#include "../include/libiht_kmd.h"


//
// Function Prototypes

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT driverObject, PUNICODE_STRING regPath);
extern "C" NTSTATUS DriverExit(PDRIVER_OBJECT driverObject);

/************************************************
 * Global variables
 ************************************************/

/************************************************
 * Platform specific hooking & entry functions
 *
 * Heavily referenced from: https://github.com/lyshark/WindowsKernelBook
 ************************************************/

/************************************************
 * Wrapper functions
 ************************************************/

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

/************************************************
 * Create process notifier functions
 ************************************************/

/*
 * Create process notify routine implementation
 */
VOID create_proc_notify(PEPROCESS proc, HANDLE proc_id,
    PPS_CREATE_NOTIFY_INFO create_info)
{
    UNREFERENCED_PARAMETER(proc);
    struct lbr_state *parent_state, *child_state, *state;

    if (create_info != NULL)
    {
        // Process is being created
        xprintdbg("LIBIHT-KMD: Process %ld is being created, parent: %ld\n", proc_id, create_info->ParentProcessId);

        parent_state = find_lbr_state((u32)(UINT_PTR)create_info->ParentProcessId);
        if (parent_state == NULL)
            // Ignore process that is not being monitored
            return;

        child_state = find_lbr_state((u32)(UINT_PTR)proc_id);
        if (child_state != NULL)
        {
            // Set up trace for new process
            child_state->lbr_select = parent_state->lbr_select;
            child_state->pid = (u32)(UINT_PTR)proc_id;
            child_state->parent = parent_state;

            insert_lbr_state(child_state);
            xprintdbg("LIBIHT-KMD: New child_state is created & inserted to monitor list\n");
        }
        else
        {
            xprintdbg("LIBIHT-KMD: New child_state is NULL, create state failed\n");
        }
    }
    else
    {
        // Process is being terminated
        xprintdbg("LIBIHT-KMD: Process %ld is being terminated\n", proc_id);
        state = find_lbr_state((u32)(UINT_PTR)proc_id);
        if (state != NULL)
            remove_lbr_state(state);
    }
}

/************************************************
 * Context switch event hook functions
 ************************************************/
void __fastcall cswitch_call_back(u32 new_proc, u32 old_proc)
{
    //unsigned long long core_idx = KeGetCurrentProcessorNumberEx(NULL);
    //print_dbg("LIBIHT-KMD: Context switch event on %lld, new_proc: %ld, old_proc: %ld\n", core_idx, new_proc, old_proc);

    if (find_lbr_state(new_proc))
    {
        //DbgBreakPoint();
        put_lbr(new_proc);
    }

    if (find_lbr_state(old_proc))
    {
        //DbgBreakPoint();
        get_lbr(old_proc);
    }
}

NTSTATUS infinity_hook_create()
{
    // Initialize and hook
    return k_hook::initialize(cswitch_call_back) && k_hook::start() ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}

NTSTATUS infinity_hook_remove()
{
    k_hook::stop();

    // Here you need to make sure that the execution point of the system is no longer in the current driver
    // For example, the current driver is unloaded, but the MyNtCreateFile you hooked is still executing the for operation, of course, the blue screen
    // The sleep 10 second method here can be directly improved
    LARGE_INTEGER integer{ 0 };
    integer.QuadPart = -10000;
    integer.QuadPart *= 10000;
    KeDelayExecutionThread(KernelMode, FALSE, &integer);

    return STATUS_SUCCESS;
}

/************************************************
 * Device hook functions
 *
 * Maintain functionality of the libiht-info helper process
 ************************************************/

 /*
  * Helper function to create user interactive helper device
  */
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

/*
 * Helper function to remove user interactive helper device
 */
NTSTATUS device_remove(PDRIVER_OBJECT driver_obj)
{
    PDEVICE_OBJECT device_obj = driver_obj->DeviceObject;
    UNICODE_STRING sym_device_name;
    RtlInitUnicodeString(&sym_device_name, SYM_DEVICE_NAME);
    IoDeleteSymbolicLink(&sym_device_name);
    IoDeleteDevice(device_obj);
    return STATUS_SUCCESS;
}

/*
 * Hooks for I/O controling the device
 */
NTSTATUS device_ioctl(PDEVICE_OBJECT device_obj, PIRP Irp)
{
    PIO_STACK_LOCATION irp_stack;
    ULONG ioctl_cmd;
    struct lbr_state* state;
    struct ioctl_request* request;
    u64 request_size;
    NTSTATUS status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(device_obj);

    irp_stack = IoGetCurrentIrpStackLocation(Irp);

    ioctl_cmd = irp_stack->Parameters.DeviceIoControl.IoControlCode;
    request_size = irp_stack->Parameters.DeviceIoControl.InputBufferLength;
    request = (struct ioctl_request*)Irp->AssociatedIrp.SystemBuffer; // Input buffer

    if (request_size != sizeof(ioctl_request))
    {
        xprintdbg("LIBIHT-KMD: Wrong request size of %ld, expect: %ld\n", request_size, sizeof(ioctl_request));
        status = STATUS_INVALID_DEVICE_REQUEST;
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return status;
    }

    xprintdbg("LIBIHT-KMD: Got ioctl argument %#x!\n", ioctl_cmd);
    xprintdbg("LIBIHT-KMD: request select bits: %lld\n", request->lbr_select);
    xprintdbg("LIBIHT-KMD: request pid: %d\n", request->pid);

    switch (ioctl_cmd)
    {
    case(LIBIHT_KMD_IOC_ENABLE_TRACE):
        xprintdbg("LIBIHT-KMD: ENABLE_TRACE\n");
        // Enable trace for assigned process
        state = find_lbr_state(request->pid);
        if (state)
        {
            xprintdbg("LIBIHT-KMD: Process %d already enabled\n", request->pid);
            status = STATUS_UNSUCCESSFUL;
            break;
        }
        state = create_lbr_state();
        if (state == NULL)
        {
            xprintdbg("LIBIHT-KMD: create lbr_state failed\n");
            status = STATUS_UNSUCCESSFUL;
            break;
        }

        // Set the field
        state->lbr_select = request->lbr_select ? request->lbr_select : LBR_SELECT;
        state->pid = request->pid ? request->pid : (u32)(ULONG_PTR)PsGetCurrentProcessId();
        state->parent = NULL;

        insert_lbr_state(state);
        break;
    case(LIBIHT_KMD_IOC_DISABLE_TRACE):
        xprintdbg("LIBIHT-KMD: DISABLE_TRACE\n");
        // Disable trace for assigned process (and its children)
        state = find_lbr_state(request->pid);
        if (state == NULL)
        {
            xprintdbg("LIBIHT-KMD: find lbr_state failed\n");
            status = STATUS_UNSUCCESSFUL;
            break;
        }

        remove_lbr_state(state);
        break;
    case(LIBIHT_KMD_IOC_DUMP_LBR):
        xprintdbg("LIBIHT-KMD: DUMP_LBR\n");
        // Dump LBR info for assigned process
        dump_lbr(request->pid);
        break;
    case(LIBIHT_KMD_IOC_SELECT_LBR):
        xprintdbg("LIBIHT-KMD: SELECT_LBR\n");
        // Update the select bits for assigned process
        state = create_lbr_state();
        if (state == NULL)
        {
            xprintdbg("LIBIHT-KMD: create lbr_state failed\n");
            status = STATUS_UNSUCCESSFUL;
            break;
        }

        state->lbr_select = request->lbr_select;
        break;
    default:
        // Error command code
        xprintdbg("LIBIHT-KMD: Error IOCTL command \n");
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    // Complete the request
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}

/*
 * Device dispatch function
 */
NTSTATUS device_default(PDEVICE_OBJECT device_obj, PIRP Irp)
{
    UNREFERENCED_PARAMETER(device_obj);
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}


/************************************************
 * Kernel mode driver functions
 ************************************************/

NTSTATUS DriverEntry(PDRIVER_OBJECT driver_obj, PUNICODE_STRING reg_path)
{
    NTSTATUS status;
    UNREFERENCED_PARAMETER(reg_path);
    driver_obj->DriverUnload = (PDRIVER_UNLOAD)DriverExit;

    xprintdbg("LIBIHT-KMD: Initializing...\n");

    // Bypass check sign
    // LINKER_FLAGS=/INTEGRITYCHECK
    bypass_check_sign(driver_obj);

    // Init global lock for LBR cache
    KeInitializeSpinLock(&lbr_cache_lock);

    // Check availability of the cpu
    if (identify_cpu() < 0)
    {
        xprintdbg("LIBIHT-KMD: Identify CPU failed\n");
        return STATUS_UNSUCCESSFUL;
    }

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

    xprintdbg("LIBIHT-KMD: Initialized\n");
    return STATUS_SUCCESS;
}

NTSTATUS DriverExit(PDRIVER_OBJECT driver_obj)
{
    NTSTATUS status;
    UNREFERENCED_PARAMETER(driver_obj);
    //struct lbr_state* curr, * prev;

    xprintdbg("LIBIHT-KMD: Exiting...\n");

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