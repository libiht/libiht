#include <ntifs.h>
#include <ntddk.h>
#include <wdm.h>
#include <windef.h>
#include <intrin.h>

#include "libiht_kmd.h"

#pragma intrinsic(_disable)
#pragma intrinsic(_enable)

static void print_dbg(const char* format, ...)
#ifdef DEBUG_MSG
{
	va_list args;
	_crt_va_start(args, format);
	vDbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, format, args);
}
#else
{
	return;
}
#endif // DEBUG_MSG

/************************************************
 * Global variables
 ************************************************/
ULONG KSC_patch_size = 0;
PUCHAR KSC_head_n_byte = NULL;
KiSwapContext KSC_ori = NULL;


/************************************************
 * Cross platform LBR & LBR state related functions
 ************************************************/


/************************************************
 * LBR helper functions
 *
 * Help to manage LBR stack/registers
 ************************************************/

 /*
  * Flush the LBR registers. Caller should ensure this function run on
  * single cpu (by wrapping KeRaiseIrql() and KeLowerIrql())
  */
static void flush_lbr(u8 enable)
{
	int i;

	__writemsr(MSR_LBR_SELECT, 0);
	__writemsr(MSR_LBR_TOS, 0);

	for (i = 0; i < lbr_capacity; i++)
	{
		__writemsr(MSR_LBR_NHM_FROM + i, 0);
		__writemsr(MSR_LBR_NHM_TO + i, 0);
	}

	if (enable)
		__writemsr(MSR_IA32_DEBUGCTLMSR, DEBUGCTLMSR_LBR);
	else
		__writemsr(MSR_IA32_DEBUGCTLMSR, 0);
}

/*
 * Store the LBR registers to kernel maintained datastructure
 */
static void get_lbr(u32 pid)
{
	int i;

	struct lbr_state* state = find_lbr_state(pid);
	if (state == NULL)
		return;

	state->lbr_select = __readmsr(MSR_LBR_SELECT);
	state->lbr_tos = __readmsr(MSR_LBR_TOS);

	for (i = 0; i < lbr_capacity; i++)
	{
		state->entries[i].from = __readmsr(MSR_LBR_NHM_FROM + i);
		state->entries[i].to = __readmsr(MSR_LBR_NHM_TO + i);
	}
}

/*
 * Write the LBR registers from kernel maintained datastructure
 */
static void put_lbr(u32 pid)
{
	int i;

	struct lbr_state* state = find_lbr_state(pid);
	if (state == NULL)
		return;

	__writemsr(MSR_LBR_SELECT, state->lbr_select);
	__writemsr(MSR_LBR_TOS, state->lbr_tos);

	for (i = 0; i < lbr_capacity; i++)
	{
		__writemsr(MSR_LBR_NHM_FROM + i, state->entries[i].from);
		__writemsr(MSR_LBR_NHM_TO + i, state->entries[i].to);
	}
}

/*
 * Dump out the LBR registers to kernel message
 */
static void dump_lbr(u32 pid)
{
	int i;
	struct lbr_state* state;

	KIRQL oldIrql;
	KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);
	state = find_lbr_state(pid);
	if (state == NULL)
	{
		print_dbg("LIBIHT-KMD: find lbr_state failed\n");
		return;
	}

	get_lbr(pid);

	print_dbg("PROC_PID:             %d\n", state->pid);
	print_dbg("MSR_LBR_SELECT:       0x%llx\n", state->lbr_select);
	print_dbg("MSR_LBR_TOS:          %lld\n", state->lbr_tos);

	for (i = 0; i < lbr_capacity; i++)
	{
		print_dbg("MSR_LBR_NHM_FROM[%2d]: 0x%llx\n", i, state->entries[i].from);
		print_dbg("MSR_LBR_NHM_TO  [%2d]: 0x%llx\n", i, state->entries[i].to);
	}

	print_dbg("LIBIHT-KMD: LBR info for cpuid: %d\n", KeGetCurrentProcessorNumberEx(NULL));

	KeLowerIrql(oldIrql);
}

/*
 * Enable the LBR feature for the current CPU.
 */
static void enable_lbr()
{
	KIRQL oldIrql;
	KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);

	print_dbg("LIBIHT-KMD: Enable LBR on cpu core: %d...\n", KeGetCurrentProcessorNumberEx(NULL));

	/* Flush the LBR and enable it */
	flush_lbr(TRUE);

	KeLowerIrql(oldIrql);
}

/*
 * Disable the LBR feature for the current CPU.
 */
static void disable_lbr(void)
{
	KIRQL oldIrql;
	KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);

	print_dbg("LIBIHT-KMD: Disable LBR on cpu core: %d...\n", KeGetCurrentProcessorNumberEx(NULL));

	/* Remove the filter */
	__writemsr(MSR_LBR_SELECT, 0);

	/* Flush the LBR and disable it */
	flush_lbr(FALSE);

	KeLowerIrql(oldIrql);
}

/************************************************
 * LBR state helper functions
 *
 * Help to manage kernel LBR state datastructure
 ************************************************/

 /*
  * Create a new empty LBR state
  */
static struct lbr_state* create_lbr_state(void)
{
	struct lbr_state* state;
	u64 state_size = sizeof(struct lbr_state) +
		lbr_capacity * sizeof(struct lbr_stack_entry);

	state = ExAllocatePool2(POOL_FLAG_NON_PAGED, state_size, LIBIHT_KMD_TAG);
	if (state == NULL)
		return NULL;

	memset(state, 0, state_size);

	return state;
}

/*
 * Insert new LBR state into the back of the list
 */
static void insert_lbr_state(struct lbr_state* new_state)
{
	struct lbr_state* head;

	if (new_state == NULL)
	{
		print_dbg("LIBIHT-KMD: Insert new state param is NULL\n");
		return;
	}

	head = lbr_state_list;
	if (head == NULL)
	{
		new_state->prev = new_state;
		new_state->next = new_state;
		lbr_state_list = new_state;
	}
	else
	{
		head->prev->next = new_state;
		new_state->prev = head->prev;
		head->prev = new_state;
		new_state->next = head;
	}
}

/*
 * Remove old LBR state from the list
 */
static void remove_lbr_state(struct lbr_state* old_state)
{
	struct lbr_state* head;
	struct lbr_state* tmp;

	if (old_state == NULL)
	{
		print_dbg("LIBIHT-KMD: Remove old state param is NULL\n");
		return;
	}

	head = lbr_state_list;
	if (head == NULL)
	{
		print_dbg("LIBIHT-KMD: Remove old state list head is NULL\n");
		return;
	}

	if (head == old_state)
	{
		// Check if only one state in the list
		lbr_state_list = head->next == head ? NULL : head->next;
	}

	// Unlink from linked list
	// if (old_state->prev != NULL)
	old_state->prev->next = old_state->next;

	// if (old_state->next != NULL)
	old_state->next->prev = old_state->prev;

	// Free all its child
	if (lbr_state_list != NULL)
	{
		tmp = lbr_state_list;
		do
		{
			if (tmp->parent == old_state)
				remove_lbr_state(tmp);
			tmp = tmp->prev;
		} while (tmp != lbr_state_list);
	}

	ExFreePoolWithTag(old_state, LIBIHT_KMD_TAG);
}

/*
 * Find the LBR state by given the pid. (Try to do as fast as possiable)
 */
static struct lbr_state* find_lbr_state(u32 pid)
{
	// Perform a backward traversal (typically, newly created processes are
	// more likely to be find)
	struct lbr_state* tmp;

	if (lbr_state_list != NULL)
	{
		tmp = lbr_state_list;
		do {
			if (tmp->pid == pid)
				return tmp;
			tmp = tmp->prev;
		} while (tmp != lbr_state_list);
	}

	return NULL;
}

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
#else
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
 * LDE engine helper functions
 ************************************************/

 /*
  * Initialize the LDE engine
  */
VOID lde_init()
{
	lde_disasm = ExAllocatePool2(POOL_FLAG_NON_PAGED_EXECUTE, 12800, LIBIHT_KMD_TAG);
	ASSERT(lde_disasm != NULL);
	memcpy(lde_disasm, sz_shellcode, 12800);
}

/*
 * Destroy the LDE engine
 */
VOID lde_destroy()
{
	ExFreePoolWithTag(lde_disasm, LIBIHT_KMD_TAG);
}

/*
 * Get the full patch size of the instruction in the given address
 */
ULONG get_full_patch_size(PUCHAR addr)
{
	ULONG len_cnt = 0, len = 0;

	// At least 14 bytes
	while (len_cnt <= 14)
	{
		len = lde_disasm(addr, 64);
		addr = addr + len;
		len_cnt = len_cnt + len;
	}
	return len_cnt;
}

/************************************************
 * Kernel hook helper functions
 ************************************************/

 /*
  * Helper function to disable memory protection
  */
KIRQL wpage_offx64()
{
	KIRQL irql = KeRaiseIrqlToDpcLevel();
	UINT64 cr0 = __readcr0();
	cr0 &= 0xfffffffffffeffff;
	__writecr0(cr0);
	_disable();
	return irql;
}

/*
 * Helper function to disable memory protection
 */
VOID wpage_onx64(KIRQL irql)
{
	UINT64 cr0 = __readcr0();
	cr0 |= 0x10000;
	_enable();
	__writecr0(cr0);
	KeLowerIrql(irql);
}

/*
 * Dynamicly get the address of the function
 */
PVOID get_func_addr(PCWSTR func_name)
{
	UNICODE_STRING unicode_func_name;
	PVOID addr;
	RtlInitUnicodeString(&unicode_func_name, func_name);
	addr = MmGetSystemRoutineAddress(&unicode_func_name);
	ASSERT(addr != NULL);
	return addr;
}

/*
 * Inline hook kernel api function
 */
PVOID kernel_api_hook(PVOID api_addr, PVOID proxy_addr, OUT PVOID* patch_addr, OUT ULONG* patch_size)
{
	KIRQL irql;
	UINT64 tmpv;
	PVOID ori_bytes, patch_buf;

	// Save return instruction: JMP QWORD PTR [After this instruction]
	UCHAR jmp_code[] =
		"\xFF\x25\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF";
	// Save original instruction.
	UCHAR jmp_code_orifunc[] =
		"\xFF\x25\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF";

	// Get function address instruction size
	*patch_size = get_full_patch_size((PUCHAR)api_addr);

	// Allocate memory for the original instruction of the api function address
	ori_bytes = ExAllocatePool2(POOL_FLAG_NON_PAGED, *patch_size, LIBIHT_KMD_TAG);
	ASSERT(ori_bytes != NULL);

	irql = wpage_offx64();
	RtlCopyMemory(ori_bytes, api_addr, *patch_size);
	wpage_onx64(irql);

	// Construct the jmp instruction to the proxy function

	// 1. Original machine code + jump machine code
	patch_buf = ExAllocatePool2(POOL_FLAG_NON_PAGED_EXECUTE, *patch_size + 14, LIBIHT_KMD_TAG);
	ASSERT(patch_buf != NULL);
	RtlFillMemory(patch_buf, *patch_size + 14, 0x90);

	// 2. Jump to unpatched byte
	tmpv = (ULONG64)api_addr + *patch_size;
	RtlCopyMemory(jmp_code_orifunc + 6, &tmpv, 8);
	RtlCopyMemory((PUCHAR)patch_buf, ori_bytes, *patch_size);
	RtlCopyMemory((PUCHAR)patch_buf + *patch_size, jmp_code_orifunc, 14);
	*patch_addr = patch_buf;

	// 3. Get proxy address
	tmpv = (ULONG64)proxy_addr;
	RtlCopyMemory(jmp_code + 6, &tmpv, 8);

	// 4. Patch the api function address
	irql = wpage_offx64();
	RtlFillMemory(api_addr, *patch_size, 0x90);
	RtlCopyMemory(api_addr, jmp_code, 14);
	wpage_onx64(irql);

	return ori_bytes;
}

/*
 * Unhook kernel api function
 */
VOID kernel_api_unhook(PVOID api_addr, PVOID ori_bytes, PVOID patch_buf, ULONG patch_size)
{
	KIRQL irql;

	irql = wpage_offx64();
	RtlCopyMemory(api_addr, ori_bytes, patch_size);
	wpage_onx64(irql);

	ExFreePoolWithTag(ori_bytes, LIBIHT_KMD_TAG);
	ExFreePoolWithTag(patch_buf, LIBIHT_KMD_TAG);
}

/************************************************
 * Swap context hook functions
 ************************************************/

VOID KSC_proxy()
{
	// TODO: Add proxy code here
	// print_dbg("LIBIHT_KMD: KSC_proxy called\n");
	KSC_ori();
}

/*
 * Helper function to create KiSwapContext hook
 */
NTSTATUS KSC_hook_create()
{
	// TODO: get the address
	PVOID KSC_addr = (PVOID)0xfffff80361c3a6e0;

	lde_init();

	KSC_head_n_byte = kernel_api_hook(KSC_addr, KSC_proxy, &(PVOID)KSC_ori, &KSC_patch_size);
	return STATUS_SUCCESS;
}

/*
 * Helper function to remove KiSwapContext hook
 */
NTSTATUS KSC_hook_remove()
{
	// TODO: get the address
	PVOID KSC_addr = (PVOID)0xfffff80361c3a6e0;

	lde_destroy();
	kernel_api_unhook(KSC_addr, KSC_head_n_byte, KSC_ori, KSC_patch_size);
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
	KIRQL oldIrql;
	struct lbr_state* state;
	struct ioctl_request* request;
	u64 request_size;

	UNREFERENCED_PARAMETER(device_obj);

	irp_stack = IoGetCurrentIrpStackLocation(Irp);

	ioctl_cmd = irp_stack->Parameters.DeviceIoControl.IoControlCode;
	request_size = irp_stack->Parameters.DeviceIoControl.InputBufferLength;
	request = Irp->AssociatedIrp.SystemBuffer; // Input buffer

	if (request_size != sizeof(request))
	{
		print_dbg("LIBIHT-KMD: Wrong request size of %ld, expect: %ld\n", request_size, sizeof(request));
		return STATUS_INVALID_DEVICE_REQUEST;
	}

	print_dbg("LIBIHT-KMD: Got ioctl argument %#x!", ioctl_cmd);
	print_dbg("LIBIHT-KMD: request select bits: %lld", request->lbr_select);
	print_dbg("LIBIHT-KMD: request pid: %d", request->pid);

	switch (ioctl_cmd)
	{
	case(LIBIHT_KMD_IOC_ENABLE_TRACE):
		print_dbg("LIBIHT-KMD: ENABLE_TRACE\n");
		// Enable trace for assigned process
		state = create_lbr_state();
		if (state == NULL)
		{
			print_dbg("LIBIHT-KMD: create lbr_state failed\n");
			return STATUS_UNSUCCESSFUL;
		}

		// Set the field
		state->lbr_select = request->lbr_select ? request->lbr_select : LBR_SELECT;
		state->pid = request->pid ? request->pid : (u32)(ULONG_PTR)PsGetCurrentProcessId();
		state->parent = NULL;

		insert_lbr_state(state);
		KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);
		put_lbr(request->pid);
		KeLowerIrql(oldIrql);
		break;
	case(LIBIHT_KMD_IOC_DISABLE_TRACE):
		print_dbg("LIBIHT-KMD: DISABLE_TRACE\n");
		// Disable trace for assigned process (and its children)
		state = find_lbr_state(request->pid);
		if (state == NULL)
		{
			print_dbg("LIBIHT-KMD: find lbr_state failed\n");
			return STATUS_UNSUCCESSFUL;
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
			return STATUS_UNSUCCESSFUL;
		}

		state->lbr_select = request->lbr_select;
		KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);
		put_lbr(request->pid);
		KeLowerIrql(oldIrql);
		break;
	default:
		// Error command code
		print_dbg("LIBIHT-KMD: Error IOCTL command \n");
		return STATUS_INVALID_DEVICE_REQUEST;
	}

	// Complete the request
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
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

	if (lbr_capacity == -1)
	{
		// Model name not found
		return -1;
	}

	print_dbg("LIBIHT-KMD: DisplayFamily_DisplayModel - %x_%xH\n", family, model);

	return 0;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT driver_obj, PUNICODE_STRING reg_path)
{
	NTSTATUS status;
	UNREFERENCED_PARAMETER(reg_path);
	driver_obj->DriverUnload = DriverExit;

	print_dbg("LIBIHT-KMD: Initializing...\n");

	// Bypass check sign
	// LINKER_FLAGS=/INTEGRITYCHECK
	bypass_check_sign(driver_obj);

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

	// TODO: Init & Register hooks on context switches
	print_dbg("LIBIHT-KMD: Initializing & Registering context switch hooks...\n");
	status = KSC_hook_create();
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

	// TODO: Unregister hooks on context switches.
	print_dbg("LIBIHT-KMD: Unregistering context switch hooks...\n");
	status = KSC_hook_remove();
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
