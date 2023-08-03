#include <ntddk.h>
#include <wdm.h>
#include <intrin.h>

#include "libiht_kmd.h"

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


/************************************************
 * LBR helper functions
 *
 * Help to manage LBR stack/registers
 ************************************************/

 /*
  * Flush the LBR registers. Caller should ensure this function run on
  * single cpu (by wrapping get_cpu() and put_cpu())
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

    // rdmsrl(MSR_IA32_DEBUGCTLMSR, state.lbr_ctl);
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

    // TODO: fix this
    get_cpu();
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

    // TODO: fix this
    put_cpu();
}

/*
 * Enable the LBR feature for the current CPU. *info may be NULL (it is required
 * by on_each_cpu()).
 */
ULONG_PTR enable_lbr_wrap(ULONG_PTR info)
{
    UNREFERENCED_PARAMETER(info);
	enable_lbr();
	return 0;
}
static void enable_lbr()
{
    // TODO: fix this
    get_cpu();

    print_dbg("LIBIHT-KMD: Enable LBR on cpu core: %d...\n", KeGetCurrentProcessorNumberEx(NULL));

    /* Flush the LBR and enable it */
    flush_lbr(TRUE);

    // TODO: fix this
    put_cpu();
}

/*
 * Disable the LBR feature for the current CPU. *info may be NULL (it is required
 * by on_each_cpu()).
 */
ULONG_PTR disable_lbr_wrap(ULONG_PTR info)
{
	UNREFERENCED_PARAMETER(info);
	disable_lbr();
	return 0;
}
static void disable_lbr(void)
{
    // TODO: fix this
    get_cpu();

    print_dbg("LIBIHT-KMD: Disable LBR on cpu core: %d...\n", KeGetCurrentProcessorNumberEx(NULL));

    /* Remove the filter */
    __writemsr(MSR_LBR_SELECT, 0);

    /* Flush the LBR and disable it */
    flush_lbr(FALSE);

    // TODO: fix this
    put_cpu();
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

    state = ExAllocatePool2(POOL_FLAG_NON_PAGED, state_size, LBR_STATE_TAG);
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

    ExFreePoolWithTag(old_state, LBR_STATE_TAG);
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
 * Kernel module functions
 ************************************************/

static int identify_cpu(void)
{
    u32 eax, ebx, ecx, edx;
	u32 family, model;
	int i;

	cpuid(0x1, &eax, &ebx, &ecx, &edx);

	family = (eax >> 8) & 0xf;
	model = (eax >> 4) & 0xf;

    // Identify CPU model
    lbr_capacity = (u64) - 1;
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

    print_dbg("LIBIHT-LKM: DisplayFamily_DisplayModel - %x_%xH\n", family, model);

    return 0;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT driverObject, PUNICODE_STRING regPath)
{
	UNREFERENCED_PARAMETER(regPath);
    driverObject->DriverUnload = DriverExit;

    print_dbg("LIBIHT-KMD: Initializing...\n");

    // Check availability of the cpu
    if (identify_cpu() < 0)
	{
		print_dbg("LIBIHT-KMD: Identify CPU failed\n");
		return STATUS_UNSUCCESSFUL;
	}

    // TODO: Create user interactive helper process
    print_dbg("LIBIHT-KMD: Creating helper process...\n");

    // TODO: Register kprobe hooks on fork system call
    print_dbg("LIBIHT-KMD: Registering system call hooks...\n");

    // TODO: Init & Register hooks on context switches
    print_dbg("LIBIHT-KMD: Initializing & Registering context switch hooks...\n");

    // Enable LBR on each cpu (Not yet set the selection filter bit)
    print_dbg("LIBIHT-KMD: Enabling LBR for all %d cpus...\n", KeQueryActiveProcessorCount(NULL));
    KeIpiGenericCall(enable_lbr_wrap, 0);

    // Set the state list to NULL after module initialized
    lbr_state_list = NULL;

	print_dbg("LIBIHT-KMD: Initialized\n");
	return STATUS_SUCCESS;
}

void DriverExit(PDRIVER_OBJECT driverObject)
{
    UNREFERENCED_PARAMETER(driverObject);
    struct lbr_state* tmp;

    print_dbg("LIBIHT-KMD: Exiting...\n");

    // Free the LBR state list
    print_dbg("LIBIHT-KMD: Freeing LBR state list...\n");
    if (lbr_state_list != NULL)
    {
        tmp = lbr_state_list;
        do
        {
            ExFreePoolWithTag(tmp, LBR_STATE_TAG);
            tmp = tmp->prev;
        } while (tmp != lbr_state_list);
    }

    // Disable LBR on each cpu
    print_dbg("LIBIHT-KMD: Disabling LBR for all %d cpus...\n", KeQueryActiveProcessorCount(NULL));
    KeIpiGenericCall(disable_lbr_wrap, 0);

    // TODO: Unregister hooks on context switches.
    print_dbg("LIBIHT-KMD: Unregistering context switch hooks...\n");

    // TODO: Unregister hooks on fork system call
    print_dbg("LIBIHT-KMD: Unregistering system call hooks...\n");

    // TODO: Remove the helper process if exist
    print_dbg("LIBIHT-KMD: Removing helper process...\n");

    print_dbg("LIBIHT-KMD: Exit complete\n");
}
