////////////////////////////////////////////////////////////////////////////////
//
//  File           : commons/lbr.c
//  Description    : This is the implementation of the LBR feature for the
//                   libiht library. See associated documentation for more
//                   information. The implementation is largely based on the
//                   LBR manipulation of PathArmor project.
//
//                   Reference: 
//                   https://github.com/vusec/patharmor/blob/master/lkm/lbr.c
//
//   Author        : Thomason Zhao
//   Last Modified : Jan 9, 2023
//

// Include Files
#include "lbr.h"

//
// Global Variables

u64 lbr_capacity;
// The capacity of the LBR.

char lbr_state_lock[MAX_LOCK_LEN];
// The lock for lbr_state_list.

char lbr_state_head[MAX_LIST_LEN];
// The head of the lbr_state_list.

static const struct cpu_to_lbr cpu_lbr_maps[] = {
    {0x5c, 32}, {0x5f, 32}, {0x4e, 32}, {0x5e, 32}, {0x8e, 32}, {0x9e, 32}, 
    {0x55, 32}, {0x66, 32}, {0x7a, 32}, {0x67, 32}, {0x6a, 32}, {0x6c, 32}, 
    {0x7d, 32}, {0x7e, 32}, {0x8c, 32}, {0x8d, 32}, {0xa5, 32}, {0xa6, 32}, 
    {0xa7, 32}, {0xa8, 32}, {0x86, 32}, {0x8a, 32}, {0x96, 32}, {0x9c, 32}, 
    {0x3d, 16}, {0x47, 16}, {0x4f, 16}, {0x56, 16}, {0x3c, 16}, {0x45, 16}, 
    {0x46, 16}, {0x3f, 16}, {0x2a, 16}, {0x2d, 16}, {0x3a, 16}, {0x3e, 16}, 
    {0x1a, 16}, {0x1e, 16}, {0x1f, 16}, {0x2e, 16}, {0x25, 16}, {0x2c, 16}, 
    {0x2f, 16}, {0x17,  4}, {0x1d,  4}, {0x0f,  4}, {0x37,  8}, {0x4a,  8},
    {0x4c,  8}, {0x4d,  8}, {0x5a,  8}, {0x5d,  8}, {0x1c,  8}, {0x26,  8},
    {0x27,  8}, {0x35,  8}, {0x36,  8}};
// CPU - LBR map table

// TODO: revise function calls to use lbr_state as parameter to avoid unnecessary find_lbr_state calls

//
// Low level LBR stack and registers access

////////////////////////////////////////////////////////////////////////////////
//
// Function     : get_lbr
// Description  : Read the LBR registers into kernel maintained datastructure.
//
// Inputs       : state - the LBR state
// Outputs      : void

void get_lbr(struct lbr_state *state)
{
    int i;
    char irql_flag[MAX_IRQL_LEN];
    u64 dbgctlmsr;

    // Disable LBR
    xrdmsr(MSR_IA32_DEBUGCTLMSR, &dbgctlmsr);
    dbgctlmsr &= ~DEBUGCTLMSR_LBR;
    xwrmsr(MSR_IA32_DEBUGCTLMSR, dbgctlmsr);

    // Read out LBR registers
    xacquire_lock(lbr_state_lock, (void *)irql_flag);

    xrdmsr(MSR_LBR_SELECT, &state->lbr_request.lbr_select);
    xrdmsr(MSR_LBR_TOS, &state->lbr_tos);

    for (i = 0; i < lbr_capacity; i++)
    {
        xrdmsr(MSR_LBR_NHM_FROM + i, &state->entries[i].from);
        xrdmsr(MSR_LBR_NHM_TO + i, &state->entries[i].to);
    }

    xrelease_lock(lbr_state_lock, (void *)irql_flag);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : put_lbr
// Description  : Write the LBR registers from kernel maintained datastructure.
//
// Inputs       : state - the LBR state
// Outputs      : void

void put_lbr(struct lbr_state *state)
{
    int i;
    char irql_flag[MAX_IRQL_LEN];
    u64 dbgctlmsr;

    // Write in LBR registers
    xacquire_lock(lbr_state_lock, (void *)irql_flag);

    xwrmsr(MSR_LBR_SELECT, state->lbr_request.lbr_select);
    xwrmsr(MSR_LBR_TOS, state->lbr_tos);

    for (i = 0; i < lbr_capacity; i++)
    {
        xwrmsr(MSR_LBR_NHM_FROM + i, state->entries[i].from);
        xwrmsr(MSR_LBR_NHM_TO + i, state->entries[i].to);
    }

    xrelease_lock(lbr_state_lock, (void *)irql_flag);

    // Enable LBR
    xrdmsr(MSR_IA32_DEBUGCTLMSR, &dbgctlmsr);
    dbgctlmsr |= DEBUGCTLMSR_LBR;
    xwrmsr(MSR_IA32_DEBUGCTLMSR, dbgctlmsr);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : flush_lbr
// Description  : Flush the LBR stack and registers. Caller should ensure this
//                function is called with interrupts disabled (either on single
//                core or with interrupts disabled for that core).
//
// Inputs       : enable - TRUE to enable LBR, FALSE to disable LBR
// Outputs      : void

void flush_lbr(void)
{
    int i;
    u64 dbgctlmsr;

    // Disable LBR
    xprintdbg("LIBIHT-COM: Flush LBR on cpu core: %d\n", xcoreid());
    xrdmsr(MSR_IA32_DEBUGCTLMSR, &dbgctlmsr);
    dbgctlmsr &= ~DEBUGCTLMSR_LBR;
    xwrmsr(MSR_IA32_DEBUGCTLMSR, dbgctlmsr);

    xwrmsr(MSR_LBR_SELECT, 0);
    xwrmsr(MSR_LBR_TOS, 0);

    for (i = 0; i < lbr_capacity; i++)
    {
        xwrmsr(MSR_LBR_NHM_FROM + i, 0);
        xwrmsr(MSR_LBR_NHM_TO + i, 0);
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : enable_lbr
// Description  : Enable the LBR feature for the current CPU core. This function
//                should be called on each CPU core by `xon_each_cpu()`
//                dispatch.
//
// Inputs       : request - the LBR ioctl request
// Outputs      : s32 - 0 on success, -1 on failure

s32 enable_lbr(struct lbr_ioctl_request *request)
{
    struct lbr_state *state;

    state = find_lbr_state(request->pid);
    if (state == NULL)
    {
        xprintdbg("LIBIHT-COM: LBR already enabled for pid %d\n", request->pid);
        return -1;
    }

    state = create_lbr_state();
    if (state == NULL)
    {
        xprintdbg("LIBIHT-COM: Create LBR state failed\n");
        return -1;
    }

    // Setup fields for LBR state
    state->parent = NULL;
    state->lbr_request.pid = request->pid ? request->pid : xgetcurrent_pid();
    state->lbr_request.lbr_select = request->lbr_select ?
                                    request->lbr_select : LBR_SELECT;

    insert_lbr_state(state);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : disable_lbr
// Description  : Disable the LBR feature for the current CPU core. This
//                function should be called on each CPU core by `xon_each_cpu()`
//                function dispatch.
//
// Inputs       : request - the LBR ioctl request
// Outputs      : s32 - 0 on success, -1 on failure


s32 disable_lbr(struct lbr_ioctl_request *request)
{
    struct lbr_state *state;

    state = find_lbr_state(request->pid);
    if (state == NULL)
    {
        xprintdbg("LIBIHT-COM: LBR not enabled for pid %d\n", request->pid);
        return -1;
    }

    remove_lbr_state(state);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : dump_lbr
// Description  : Dump the LBR registers for the given process id

s32 dump_lbr(struct lbr_ioctl_request *request)
{
    int i;
    struct lbr_state* state;
    char irql_flag[MAX_IRQL_LEN];

    state = find_lbr_state(request->pid);
    if (state == NULL)
    {
        xprintdbg("LIBIHT-COM: LBR not enabled for pid %d\n", request->pid);
        return -1;
    }

    // Examine if the current process is the owner of the LBR state
    if (state->lbr_request.pid == xgetcurrent_pid()) 
    {
        xprintdbg("LIBIHT-COM: Dump LBR for current process\n");
        // Get fresh LBR info
        get_lbr(state);
        put_lbr(state);
    }

    xacquire_lock(lbr_state_lock, (void *)irql_flag);

    // Dump the LBR state
    xprintdbg("PROC_PID:             %d\n", state->lbr_request.pid);
    xprintdbg("MSR_LBR_SELECT:       0x%llx\n", state->lbr_request.lbr_select);
    xprintdbg("MSR_LBR_TOS:          %lld\n", state->lbr_tos);

    for (i = 0; i < lbr_capacity; i++)
    {
        xprintdbg("MSR_LBR_NHM_FROM[%2d]: 0x%llx\n", i, state->entries[i].from);
        xprintdbg("MSR_LBR_NHM_TO  [%2d]: 0x%llx\n", i, state->entries[i].to);
    }

    xprintdbg("LIBIHT-COM: LBR info for cpuid: %d\n", xcoreid());

    xrelease_lock(lbr_state_lock, (void *)irql_flag);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : config_lbr
// Description  : Configure the LBR selection bit for the given process in the
//                request.
//
// Inputs       : request - the LBR ioctl request
// Outputs      : s32 - 0 on success, -1 on failure

s32 config_lbr(struct lbr_ioctl_request *request)
{
    struct lbr_state* state;

    state = find_lbr_state(request->pid);
    if (state == NULL)
    {
        xprintdbg("LIBIHT-COM: LBR not enabled for pid %d\n", request->pid);
        return -1;
    }

    get_lbr(state);
    state->lbr_request.lbr_select = request->lbr_select;
    put_lbr(state);

    return 0;
}

//
// LBR state (kernel maintained datastructure) helper functions

////////////////////////////////////////////////////////////////////////////////
//
// Function     : create_lbr_state
// Description  : Create a new LBR state for the given process id.
//
// Inputs       : pid - the process id
// Outputs      : struct lbr_state* - the newly created LBR state

struct lbr_state* create_lbr_state(void)
{
    struct lbr_state* state;
    u64 state_size;

    state_size = sizeof(struct lbr_state) +
                        lbr_capacity * sizeof(struct lbr_stack_entry);

    state = xmalloc(state_size);
    if (state == NULL)
        return NULL;

    xmemset(state, 0, state_size);

    return state;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : find_lbr_state
// Description  : Find the LBR state for the given process id.
//
// Inputs       : pid - the process id
// Outputs      : struct lbr_state* - the LBR state

struct lbr_state* find_lbr_state(u32 pid)
{
    char irql_flag[MAX_IRQL_LEN];
    struct lbr_state *curr_state, *ret_state = NULL;
    void *curr_list;
    u64 offset;

    xacquire_lock(lbr_state_lock, irql_flag);

    // offsetof(st, m) macro implementation of stddef.h
    offset = (u64)(&((struct lbr_state *)0)->list);
    curr_list = xlist_next(lbr_state_head);
    while (curr_list != lbr_state_head)
    {
        curr_list = xlist_next(curr_list);
        curr_state = (struct lbr_state *)((u64)curr_list - offset);
        if (curr_state->lbr_request.pid == pid)
        {
            ret_state = curr_state;
            break;
        }
    }

    xrelease_lock(lbr_state_lock, irql_flag);

    return ret_state;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : insert_lbr_state
// Description  : Insert new LBR state into the list.
//
// Inputs       : new_state - the new LBR state
// Outputs      : void

void insert_lbr_state(struct lbr_state* new_state)
{
    char irql_flag[MAX_IRQL_LEN];

    if (new_state == NULL)
        return;

    xacquire_lock(lbr_state_lock, irql_flag);
    xprintdbg("LIBIHT-COM: Insert LBR state for pid %d\n",
                new_state->lbr_request.pid);
    xlist_add(lbr_state_head, new_state->list);
    xrelease_lock(lbr_state_lock, irql_flag);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : remove_lbr_state
// Description  : Remove the LBR state from the list.
//
// Inputs       : old_state - the old LBR state
// Outputs      : void

void remove_lbr_state(struct lbr_state* old_state)
{
    char irql_flag[MAX_IRQL_LEN];

    if (old_state == NULL)
        return;
    
    xacquire_lock(lbr_state_lock, irql_flag);
    xprintdbg("LIBIHT-COM: Remove LBR state for pid %d\n",
                old_state->lbr_request.pid);
    xlist_del(old_state->list);
    xfree(old_state);
    xrelease_lock(lbr_state_lock, irql_flag);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : free_lbr_state_list
// Description  : Free the LBR state list.
//
// Inputs       : void
// Outputs      : void

void free_lbr_state_list(void)
{
    char irql_flag[MAX_IRQL_LEN];
    struct lbr_state *curr_state;
    void *curr_list;
    u64 offset;

    xacquire_lock(lbr_state_lock, irql_flag);

    // offsetof(st, m) macro implementation of stddef.h
    offset = (u64)(&((struct lbr_state *)0)->list);
    curr_list = xlist_next(lbr_state_head);
    while (curr_list != lbr_state_head)
    {
        curr_state = (struct lbr_state *)((u64)curr_list - offset);
        curr_list = xlist_next(curr_list);

        xlist_del(curr_state->list);
        xfree(curr_state);
    }

    xrelease_lock(lbr_state_lock, irql_flag);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : lbr_ioctl
// Description  : The ioctl handler for the LBR feature.
//
// Inputs       : request - the LBR ioctl request
// Outputs      : s32 - 0 on success, -1 on failure

s32 lbr_ioctl(struct xioctl_request *request)
{
    s32 ret = 0;

    xprintdbg("LIBIHT-COM: LBR ioctl command %d.\n", request->cmd);
    switch (request->cmd)
    {
        case LIBIHT_IOCTL_ENABLE_LBR:
            xprintdbg("LIBIHT-COM: Enable LBR for pid %d\n",
                        request->data.lbr.pid);
            ret = enable_lbr(&request->data.lbr);
            break;
        case LIBIHT_IOCTL_DISABLE_LBR:
            xprintdbg("LIBIHT-COM: Disable LBR for pid %d\n",
                        request->data.lbr.pid);
            ret = disable_lbr(&request->data.lbr);
            break;
        case LIBIHT_IOCTL_DUMP_LBR:
            xprintdbg("LIBIHT-COM: Dump LBR for pid %d\n",
                        request->data.lbr.pid);
            ret = dump_lbr(&request->data.lbr);
            break;
        default:
            xprintdbg("LIBIHT-COM: Invalid LBR ioctl command\n");
            ret = -1;
            break;
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : lbr_check
// Description  : Check if the LBR feature is available on the current CPU.
//                And set the global variable `lbr_capacity`.
//
// Inputs       : void
// Outputs      : s32 - 0 on success, -1 on failure

s32 lbr_check(void)
{
    u32 cpuinfo[4] = { 0 };
    u32 family, model;
    int i;

    xcpuid(1, &cpuinfo[0], &cpuinfo[1], &cpuinfo[2], &cpuinfo[3]);
    
    family = ((cpuinfo[0] >> 8) & 0xF) + ((cpuinfo[0] >> 20) & 0xFF);
    model = ((cpuinfo[0] >> 4) & 0xF) | ((cpuinfo[0] >> 12) & 0xF0);

    // Identify CPU model
    for (i = 0; i < sizeof(cpu_lbr_maps) / sizeof(cpu_lbr_maps[0]); ++i)
    {
        if (model == cpu_lbr_maps[i].model)
        {
            lbr_capacity = cpu_lbr_maps[i].lbr_capacity;
            break;
        }
    }

    xprintdbg("LIBIHT-COM: DisplayFamily_DisplayModel - %x_%xH\n",
                    family, model);
    xprintdbg("LIBIHT-COM: LBR capacity - %ld\n", lbr_capacity);

    if (lbr_capacity == 0)
    {
        // Model name not found
        xprintdbg("LIBIHT-COM: CPU model not found\n");
        return -1;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : lbr_init
// Description  : Initialize the LBR feature and its related datastructures.
//
// Inputs       : void
// Outputs      : s32 - 0 on success, -1 on failure

s32 lbr_init(void)
{
    if (lbr_check())
    {
        xprintdbg("LIBIHT-COM: LBR not available\n");
        return -1;
    }

    xprintdbg("LIBIHT-COM: Init LBR related structs.\n");
    xinit_lock(lbr_state_lock);
    xinit_list_head(lbr_state_head);

    // Flush LBR on each cpu
    xprintdbg("LIBIHT-COM: Flushing LBR for all cpus...\n");
    xon_each_cpu(flush_lbr);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : lbr_exit
// Description  : Free the LBR feature and its related datastructures.
//
// Inputs       : void
// Outputs      : s32 - 0 on success, -1 on failure

s32 lbr_exit(void)
{
    // Flush LBR on each cpu
    xprintdbg("LIBIHT-COM: Flushing LBR for all cpus...\n");
    xon_each_cpu(flush_lbr);

    // Free all LBR state
    xprintdbg("LIBIHT-COM: Freeing LBR state list...\n");
    free_lbr_state_list();

    return 0;
}
