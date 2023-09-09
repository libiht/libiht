#pragma warning(disable : 4201 4819 4311 4302 4996)
#include "infinity_hook/hook.hpp"
#include "infinity_hook/imports.hpp"
#include "libiht_kmd.h"


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
 * enable_lbr wrapper worker function
 */
ULONG_PTR enable_lbr_wrap(ULONG_PTR info)
{
	UNREFERENCED_PARAMETER(info);
	enable_lbr();
	return 0;
}

/*
 * disable_lbr wrapper worker function
 */
ULONG_PTR disable_lbr_wrap(ULONG_PTR info)
{
	UNREFERENCED_PARAMETER(info);
	disable_lbr();
	return 0;
}

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
		print_dbg("LIBIHT-KMD: Process %ld is being created, parent: %ld\n", proc_id, create_info->ParentProcessId);

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
			print_dbg("LIBIHT-KMD: New child_state is created & inserted to monitor list\n");
		}
		else
		{
			print_dbg("LIBIHT-KMD: New child_state is NULL, create state failed\n");
		}
	}
	else
	{
		// Process is being terminated
		print_dbg("LIBIHT-KMD: Process %ld is being terminated\n", proc_id);
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
		restore_lbr(new_proc);
	}

	if (find_lbr_state(old_proc))
	{
		//DbgBreakPoint();
		save_lbr(old_proc);
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
		print_dbg("Failed to create device object\n");
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
		print_dbg("LIBIHT-KMD: Wrong request size of %ld, expect: %ld\n", request_size, sizeof(ioctl_request));
		status = STATUS_INVALID_DEVICE_REQUEST;
		Irp->IoStatus.Status = status;
		Irp->IoStatus.Information = 0;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);

		return status;
	}

	print_dbg("LIBIHT-KMD: Got ioctl argument %#x!\n", ioctl_cmd);
	print_dbg("LIBIHT-KMD: request select bits: %lld\n", request->lbr_select);
	print_dbg("LIBIHT-KMD: request pid: %d\n", request->pid);

	switch (ioctl_cmd)
	{
	case(LIBIHT_KMD_IOC_ENABLE_TRACE):
		print_dbg("LIBIHT-KMD: ENABLE_TRACE\n");
		// Enable trace for assigned process
		state = find_lbr_state(request->pid);
		if (state)
		{
			print_dbg("LIBIHT-KMD: Process %d already enabled\n", request->pid);
			status = STATUS_UNSUCCESSFUL;
			break;
		}
		state = create_lbr_state();
		if (state == NULL)
		{
			print_dbg("LIBIHT-KMD: create lbr_state failed\n");
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
		print_dbg("LIBIHT-KMD: DISABLE_TRACE\n");
		// Disable trace for assigned process (and its children)
		state = find_lbr_state(request->pid);
		if (state == NULL)
		{
			print_dbg("LIBIHT-KMD: find lbr_state failed\n");
			status = STATUS_UNSUCCESSFUL;
			break;
		}

		remove_lbr_state(state);
		break;
	case(LIBIHT_KMD_IOC_DUMP_LBR):
		print_dbg("LIBIHT-KMD: DUMP_LBR\n");
		// Dump LBR info for assigned process
		dump_lbr(request->pid);
		break;
	case(LIBIHT_KMD_IOC_SELECT_LBR):
		print_dbg("LIBIHT-KMD: SELECT_LBR\n");
		// Update the select bits for assigned process
		state = create_lbr_state();
		if (state == NULL)
		{
			print_dbg("LIBIHT-KMD: create lbr_state failed\n");
			status = STATUS_UNSUCCESSFUL;
			break;
		}

		state->lbr_select = request->lbr_select;
		break;
	default:
		// Error command code
		print_dbg("LIBIHT-KMD: Error IOCTL command \n");
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

static int identify_cpu(void)
{
	s32 cpuinfo[4] = { 0 };
	u32 family, model;
	int i;

	__cpuid(cpuinfo, 1);

	family = ((cpuinfo[0] >> 8) & 0xF) + ((cpuinfo[0] >> 20) & 0xFF);
	model = ((cpuinfo[0] >> 4) & 0xF) | ((cpuinfo[0] >> 12) & 0xF0);

	// Identify CPU model
	lbr_capacity = (u64)-1;
	for (i = 0; i < sizeof(cpu_lbr_maps) / sizeof(cpu_lbr_maps[0]); ++i)
	{
		if (model == cpu_lbr_maps[i].model)
		{
			lbr_capacity = cpu_lbr_maps[i].lbr_capacity;
			break;
		}
	}

	print_dbg("LIBIHT-KMD: DisplayFamily_DisplayModel - %x_%xH\n", family, model);
	print_dbg("LIBIHT-KMD: LBR capacity - %ld\n", lbr_capacity);

	if (lbr_capacity == -1)
	{
		// Model name not found
		print_dbg("LIBIHT-KMD: CPU model not found\n");
		return -1;
	}

	return 0;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT driver_obj, PUNICODE_STRING reg_path)
{
	NTSTATUS status;
	UNREFERENCED_PARAMETER(reg_path);
	driver_obj->DriverUnload = (PDRIVER_UNLOAD)DriverExit;

	print_dbg("LIBIHT-KMD: Initializing...\n");

	// Bypass check sign
	// LINKER_FLAGS=/INTEGRITYCHECK
	bypass_check_sign(driver_obj);

	// Init global lock for LBR cache
	KeInitializeSpinLock(&lbr_cache_lock);

	// Check availability of the cpu
	if (identify_cpu() < 0)
	{
		print_dbg("LIBIHT-KMD: Identify CPU failed\n");
		return STATUS_UNSUCCESSFUL;
	}

	// Create user interactive helper device
	print_dbg("LIBIHT-KMD: Creating helper device...\n");
	status = device_create(driver_obj);
	if (!NT_SUCCESS(status))
		return status;

	// Register create process notifier
	print_dbg("LIBIHT-KMD: Registering create proc notifier...\n");
	status = PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)create_proc_notify, FALSE);
	if (!NT_SUCCESS(status))
		return status;

	// Init & Register hooks on context switches
	print_dbg("LIBIHT-KMD: Initializing & Registering context switch hooks...\n");
	status = infinity_hook_create();
	if (!NT_SUCCESS(status))
		return status;

	// Enable LBR on each cpu (Not yet set the selection filter bit)
	print_dbg("LIBIHT-KMD: Enabling LBR for all %d cpus...\n", KeQueryActiveProcessorCount(NULL));
	KeIpiGenericCall(enable_lbr_wrap, 0);

	// Set the state list to NULL after module initialized
	lbr_state_list = NULL;

	print_dbg("LIBIHT-KMD: Initialized\n");
	return STATUS_SUCCESS;
}

NTSTATUS DriverExit(PDRIVER_OBJECT driver_obj)
{
	NTSTATUS status;
	UNREFERENCED_PARAMETER(driver_obj);
	struct lbr_state* curr, * prev;

	print_dbg("LIBIHT-KMD: Exiting...\n");

	// Free the LBR state list
	print_dbg("LIBIHT-KMD: Freeing LBR state list...\n");
	if (lbr_state_list != NULL)
	{
		curr = lbr_state_list;
		do
		{
			prev = curr->prev;
			ExFreePoolWithTag(curr, LIBIHT_KMD_TAG);
			curr = prev;
		} while (curr != lbr_state_list);
	}

	// Disable LBR on each cpu
	print_dbg("LIBIHT-KMD: Disabling LBR for all %d cpus...\n", KeQueryActiveProcessorCount(NULL));
	KeIpiGenericCall(disable_lbr_wrap, 0);

	// Unregister hooks on context switches.
	print_dbg("LIBIHT-KMD: Unregistering context switch hooks (may take around 10s)...\n");
	status = infinity_hook_remove();
	if (!NT_SUCCESS(status))
		return status;

	// Unregister create process notifier
	print_dbg("LIBIHT-KMD: Unregistering create proc notifier...\n");
	status = PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)create_proc_notify, TRUE);
	if (!NT_SUCCESS(status))
		return status;

	// Remove the helper device if exist
	print_dbg("LIBIHT-KMD: Removing helper device...\n");
	status = device_remove(driver_obj);
	if (!NT_SUCCESS(status))
		return status;

	print_dbg("LIBIHT-KMD: Exit complete\n");
	return STATUS_SUCCESS;
}
