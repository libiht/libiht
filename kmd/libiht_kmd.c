#include <ntddk.h>
#include <intrin.h>

#include "libiht_kmd.h"

/************************************************
 * Global variables
 ************************************************/

 /*
  * Constant CPU - LBR map, if the model not listed, it does not
  * support the LBR feature.
  */
static const struct cpu_to_lbr cpu_lbr_maps[] = {
    {0x5c, 32}, {0x5f, 32}, {0x4e, 32}, {0x5e, 32}, {0x8e, 32}, {0x9e, 32},
    {0x55, 32}, {0x66, 32}, {0x7a, 32}, {0x67, 32}, {0x6a, 32}, {0x6c, 32},
    {0x7d, 32}, {0x7e, 32}, {0x8c, 32}, {0x8d, 32}, {0xa5, 32}, {0xa6, 32},
    {0xa7, 32}, {0xa8, 32}, {0x86, 32}, {0x8a, 32}, {0x96, 32}, {0x9c, 32},
    {0x3d, 16}, {0x47, 16}, {0x4f, 16}, {0x56, 16}, {0x3c, 16}, {0x45, 16},
    {0x46, 16}, {0x3f, 16}, {0x2a, 16}, {0x2d, 16}, {0x3a, 16}, {0x3e, 16},
    {0x1a, 16}, {0x1e, 16}, {0x1f, 16}, {0x2e, 16}, {0x25, 16}, {0x2c, 16},
    {0x2f, 16}, {0x17, 4}, {0x1d, 4}, {0x0f, 4}, {0x37, 8}, {0x4a, 8}, {0x4c, 8},
    {0x4d, 8}, {0x5a, 8}, {0x5d, 8}, {0x1c, 8}, {0x26, 8}, {0x27, 8}, {0x35, 8},
    {0x36, 8} };

static struct lbr_state* lbr_state_list;
static u64 lbr_capacity;

static void print_dbg(const char* format, ...)
#ifdef DEBUG_MSG
{
    va_list args;
    _crt_va_start(args, format);
    DbgPrint(format, args);
}
#else
{
    return;
}
#endif // DEBUG_MSG



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

    put_cpu();
}

/*
 * Enable the LBR feature for the current CPU. *info may be NULL (it is required
 * by on_each_cpu()).
 */
static void enable_lbr(void* info)
{
    UNREFERENCED_PARAMETER(info);
    get_cpu();

    print_dbg("LIBIHT-KMD: Enable LBR on cpu core: %d...\n", KeGetCurrentProcessorNumberEx(NULL));

    /* Flush the LBR and enable it */
    flush_lbr(TRUE);

    put_cpu();
}

/*
 * Disable the LBR feature for the current CPU. *info may be NULL (it is required
 * by on_each_cpu()).
 */
static void disable_lbr(void* info)
{
    UNREFERENCED_PARAMETER(info);
    get_cpu();

    print_dbg("LIBIHT-KMD: Disable LBR on cpu core: %d...\n", KeGetCurrentProcessorNumberEx(NULL));

    /* Remove the filter */
    __writemsr(MSR_LBR_SELECT, 0);

    /* Flush the LBR and disable it */
    flush_lbr(FALSE);

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

    // TODO: fix malloc
    // state = kmalloc(state_size, GFP_KERNEL);
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

    // TODO: fix it
    // kfree(old_state);
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

VOID DriverExit(IN PDRIVER_OBJECT driverObject)
{
	UNREFERENCED_PARAMETER(driverObject);
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Unloading driver\n"));
}

NTSTATUS DriverEntry(IN PDRIVER_OBJECT driverObject, IN PUNICODE_STRING regPath)
{
	UNREFERENCED_PARAMETER(regPath);
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "========== Entry Point ==========\n"));
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "========== Hello World ==========\n"));

	driverObject->DriverUnload = DriverExit;
	return STATUS_SUCCESS;
}
