////////////////////////////////////////////////////////////////////////////////
//
//  File           : commons/bts.c
//  Description    : This is the implementation of the BTS feature for the
//                   libiht library. See associated documentation for more
//                   information.
//
//   Author        : Thomason Zhao
//   Last Modified : Jan 15, 2023
//

// Include Files
#include "bts.h"

//
// Global Variables
char bts_state_lock[MAX_LOCK_LEN];
// Lock for bts state list

char bts_state_head[MAX_LIST_LEN];
// Head of bts state list

////////////////////////////////////////////////////////////////////////////////
//
// Function     : get_bts
// Description  : Get the BTS records out from the BTS buffer. Pause the BTS
//                tracing.
//
// Inputs       : state - the BTS state
// Outputs      : 0 if successful, -1 if failure

void get_bts(struct bts_state *state)
{
    u64 dbgctlmsr;

    // Disable BTS
    xrdmsr(MSR_IA32_DEBUGCTLMSR, &dbgctlmsr);
    dbgctlmsr &= ~state->config.bts_config;
    xwrmsr(MSR_IA32_DEBUGCTLMSR, dbgctlmsr);

    // Reset BTS debug store buffer pointer
    xwrmsr(MSR_IA32_DS_AREA, NULL);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : put_bts
// Description  : Put the BTS records into the BTS buffer. Resume the BTS
//                tracing.
//
// Inputs       : state - the BTS state
// Outputs      : 0 if successful, -1 if failure

void put_bts(struct bts_state *state)
{
    u64 dbgctlmsr;

    // Setup BTS debug store buffer pointer
    xwrmsr(MSR_IA32_DS_AREA, (u64)state->ds_area);

    // Enable BTS
    xrdmsr(MSR_IA32_DEBUGCTLMSR, &dbgctlmsr);
    dbgctlmsr |= state->config.bts_config;
    xwrmsr(MSR_IA32_DEBUGCTLMSR, dbgctlmsr);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : flush_bts
// Description  : Flush the BTS buffer. Caller should ensure this function is
//                called with interrupts disabled (either on single core or
//                with interrupts disabled for that core).
//
// Inputs       : void
// Outputs      : void

void flush_bts(void)
{
    u64 dbgctlmsr;
    u64 bts_bits;
    char irql_flag[MAX_IRQL_LEN];

    bts_bits = DEBUGCTLMSR_TR |
                DEBUGCTLMSR_BTS |
                DEBUGCTLMSR_BTINT |
                DEBUGCTLMSR_BTS_OFF_OS |
                DEBUGCTLMSR_BTS_OFF_USR;

    xlock_core(irql_flag);

    // Disable BTS
    xprintdbg("LIBIHT-COM: Flush BTS on cpu core: %d...\n", xcoreid());
    xrdmsr(MSR_IA32_DEBUGCTLMSR, &dbgctlmsr);
    dbgctlmsr &= ~bts_bits;
    xwrmsr(MSR_IA32_DEBUGCTLMSR, dbgctlmsr);

    // Reset BTS debug store buffer pointer
    xwrmsr(MSR_IA32_DS_AREA, NULL);

    xrelease_core(irql_flag);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : enable_bts
// Description  : Enable the BTS.
//
// Inputs       : request - the BTS ioctl request
// Outputs      : 0 if successful, -1 if failure

s32 enable_bts(struct bts_ioctl_request *request)
{
    struct bts_state *state;

    state = find_bts_state(request->bts_config.pid);
    if (state)
    {
        xprintdbg("LIBIHT-COM: BTS already enabled for pid %d.\n",
                    request->bts_config.pid);
        return -1;
    }

    state = create_bts_state();
    if (state == NULL)
    {
        xprintdbg("LIBIHT-COM: Create BTS state failed.\n");
        return -1;
    }

    // Setup fields for BTS state
    state->parent = NULL;
    state->config.pid = request->bts_config.pid ?
                request->bts_config.pid : xgetcurrent_pid();
    state->config.bts_config = request->bts_config.bts_config ?
                request->bts_config.bts_config : DEFAULT_BTS_CONFIG;
    state->config.bts_buffer_size = request->bts_config.bts_buffer_size ?
                request->bts_config.bts_buffer_size : DEFAULT_BTS_BUFFER_SIZE;

    // Setup fields for BTS debug store area
    state->ds_area->bts_buffer_base = (u64)xmalloc(state->config.bts_buffer_size);
    state->ds_area->bts_index = state->ds_area->bts_buffer_base;
    state->ds_area->bts_absolute_maximum =
            state->ds_area->bts_buffer_base +
            state->config.bts_buffer_size + 1;
    // Not yet support state->ds_area->bts_interrupt_threshold

    // Print BTS debug store area info
    xprintdbg("LIBIHT-COM: BTS ds_area pointer: %llx, bts_buffer_base: %llx, "
                "bts_index: %llx, bts_absolute_maximum: %llx.\n",
                (u64)state->ds_area, state->ds_area->bts_buffer_base,
                state->ds_area->bts_index,
                state->ds_area->bts_absolute_maximum);


    insert_bts_state(state);
    // If the requesting process is the current process, trace it right away
    if (state->config.pid == xgetcurrent_pid())
        put_bts(state);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : disable_bts
// Description  : Disable the BTS tracing for a given process in request.
//
// Inputs       : request - the BTS ioctl request
// Outputs      : 0 if successful, -1 if failure

s32 disable_bts(struct bts_ioctl_request *request)
{
    struct bts_state *state;

    state = find_bts_state(request->bts_config.pid);
    if (state == NULL)
    {
        xprintdbg("LIBIHT-COM: BTS not enabled for pid %d.\n",
                    request->bts_config.pid);
        return -1;
    }

    if (state->config.pid == xgetcurrent_pid())
        get_bts(state);
    remove_bts_state(state);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : dump_bts
// Description  : Dump the BTS records for a given process in request.
//
// Inputs       : request - the BTS ioctl request
// Outputs      : 0 if successful, -1 if failure

s32 dump_bts(struct bts_ioctl_request *request)
{
    struct bts_state *state;
    struct bts_record *record;
    u64 i;
    char irql_flag[MAX_IRQL_LEN];

    state = find_bts_state(request->bts_config.pid);
    if (state == NULL)
    {
        xprintdbg("LIBIHT-COM: BTS not enabled for pid %d.\n",
                    request->bts_config.pid);
        return -1;
    }

    // Dump some BTS buffer records
    // TODO: fix this 32 later
    xacquire_lock(bts_state_lock, irql_flag);

    for (i = 0; i < 32; i++)
    {
        record = (struct bts_record*)state->ds_area->bts_buffer_base + i;
        xprintdbg("LIBIHT-COM: BTS record ptr: 0x%llx.\n", (u64)record);
        xprintdbg("LIBIHT-COM: BTS record %d: from %llx to %llx.\n",
                    i, record->from, record->to);
    }

    xrelease_lock(bts_state_lock, irql_flag);

    // TODO: Dump data to user request pointer

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : config_bts
// Description  : Configure the BTS trace bits and BTS buffer size for a given
//                process in request.
//
// Inputs       : request - the BTS ioctl request
// Outputs      : 0 if successful, -1 if failure

s32 config_bts(struct bts_ioctl_request *request)
{
    struct bts_state *state;

    state = find_bts_state(request->bts_config.pid);
    if (state == NULL)
    {
        xprintdbg("LIBIHT-COM: BTS not enabled for pid %d.\n",
                    request->bts_config.pid);
        return -1;
    }

    state->config.bts_config = request->bts_config.bts_config;
    // If the current process is the target process, we need to
    // disable and re-enable BTS to apply the new configuration
    if (xgetcurrent_pid() == request->bts_config.pid)
    {
        // TODO: check if it works
        get_bts(state);

        if (request->bts_config.bts_buffer_size != state->config.bts_buffer_size &&
            request->bts_config.bts_buffer_size != 0)
        {
            state->config.bts_buffer_size = request->bts_config.bts_buffer_size;

            // Reconfigure BTS debug store area
            xfree((void *)state->ds_area->bts_buffer_base);
            state->ds_area->bts_buffer_base = (u64)xmalloc(request->bts_config.bts_buffer_size);
            state->ds_area->bts_index = 0;
            state->ds_area->bts_absolute_maximum =
                    state->ds_area->bts_buffer_base +
                    request->bts_config.bts_buffer_size + 1;
        }

        put_bts(state);
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : create_bts_state
// Description  : Create a new BTS state.
//
// Inputs       : void
// Outputs      : The new BTS state

struct bts_state *create_bts_state(void)
{
    struct bts_state *state;

    state = xmalloc(sizeof(struct bts_state));
    if (state == NULL)
        return NULL;
    // TODO: check if this satisfy the page alignment requirement
    state->ds_area = xmalloc(sizeof(struct ds_area));
    if (state->ds_area == NULL)
    {
        xfree(state);
        return NULL;
    }

    return state;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : find_bts_state
// Description  : Find a BTS state by pid.
//
// Inputs       : pid - the pid of the target process
// Outputs      : The BTS state

struct bts_state *find_bts_state(u32 pid)
{
    char irql_flag[MAX_IRQL_LEN];
    struct bts_state *curr_state, *ret_state = NULL;
    void *curr_list;
    u64 offset;

    xacquire_lock(bts_state_lock, irql_flag);

    // offsetof(st, m) macro implementation of stddef.h
    offset = (u64)(&((struct bts_state *)0)->list);
    curr_list = xlist_next(bts_state_head);
    while (curr_list != bts_state_head)
    {
        curr_state = (struct bts_state *)((u64)curr_list - offset);
        curr_list = xlist_next(curr_list);
        if (curr_state->config.pid == pid)
        {
            ret_state = curr_state;
            break;
        }
    }

    xrelease_lock(bts_state_lock, irql_flag);

    return ret_state;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : insert_bts_state
// Description  : Insert a new BTS state into the list.
//
// Inputs       : new_state - the new BTS state
// Outputs      : void

void insert_bts_state(struct bts_state *new_state)
{
    char irql_flag[MAX_IRQL_LEN];

    if (new_state == NULL)
        return;

    xacquire_lock(bts_state_lock, irql_flag);
    xprintdbg("LIBIHT-COM: Insert BTS state for pid %d.\n",
                new_state->config.pid);
    xlist_add(new_state->list, bts_state_head);
    xrelease_lock(bts_state_lock, irql_flag);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : remove_bts_state
// Description  : Remove a BTS state from the list.
//
// Inputs       : old_state - the old BTS state
// Outputs      : void

void remove_bts_state(struct bts_state *old_state)
{
    char irql_flag[MAX_IRQL_LEN];

    if (old_state == NULL)
        return;

    xacquire_lock(bts_state_lock, irql_flag);
    xprintdbg("LIBIHT-COM: Remove BTS state for pid %d.\n",
                old_state->config.pid);
    xlist_del(&old_state->list);
    xfree((void *)old_state->ds_area->bts_buffer_base);
    xfree(old_state->ds_area);
    xfree(old_state);
    xrelease_lock(bts_state_lock, irql_flag);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : free_bts_state_list
// Description  : Free the BTS state list.
//
// Inputs       : void
// Outputs      : void

void free_bts_state_list(void)
{
    char irql_flag[MAX_IRQL_LEN];
    struct bts_state *curr_state;
    void *curr_list;
    u64 offset;

    xacquire_lock(bts_state_lock, irql_flag);

    // offsetof(st, m) macro implementation of stddef.h
    offset = (u64)(&((struct bts_state *)0)->list);
    curr_list = xlist_next(bts_state_head);
    while (curr_list != bts_state_head)
    {
        curr_state = (struct bts_state *)((u64)curr_list - offset);
        curr_list = xlist_next(curr_list);
        xprintdbg("LIBIHT-COM: Free BTS state for pid %d.\n",
                    curr_state->config.pid);

        xlist_del(curr_state->list);
        xfree((void *)curr_state->ds_area->bts_buffer_base);
        xfree(curr_state->ds_area);
        xfree(curr_state);
    }

    xrelease_lock(bts_state_lock, irql_flag);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : bts_ioctl_handler
// Description  : The ioctl handler for the BTS.
//
// Inputs       : request - the cross platform ioctl request
// Outputs      : 0 if successful, -1 if failure

s32 bts_ioctl_handler(struct xioctl_request *request)
{
    s32 ret = 0;

    xprintdbg("LIBIHT-COM: BTS ioctl command %d.\n", request->cmd);
    switch (request->cmd)
    {
    case LIBIHT_IOCTL_ENABLE_BTS:
        xprintdbg("LIBIHT-COM: Enable BTS for pid %d.\n",
                    request->body.bts.bts_config.pid);
        ret = enable_bts(&request->body.bts);
        break;

    case LIBIHT_IOCTL_DISABLE_BTS:
        xprintdbg("LIBIHT-COM: Disable BTS for pid %d.\n",
                    request->body.bts.bts_config.pid);
        ret = disable_bts(&request->body.bts);
        break;

    case LIBIHT_IOCTL_DUMP_BTS:
        xprintdbg("LIBIHT-COM: Dump BTS for pid %d.\n",
                    request->body.bts.bts_config.pid);
        ret = dump_bts(&request->body.bts);
        break;

    case LIBIHT_IOCTL_CONFIG_BTS:
        xprintdbg("LIBIHT-COM: Config BTS for pid %d.\n",
                    request->body.bts.bts_config.pid);
        ret = config_bts(&request->body.bts);
        break;

    default:
        xprintdbg("LIBIHT-COM: Invalid BTS ioctl command.\n");
        ret = -1;
        break;
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : bts_cswitch_handler
// Description  : The context switch handler for the BTS.
//
// Inputs       : prev_pid - the pid of the previous process
//                next_pid - the pid of the next process
// Outputs      : void

void bts_cswitch_handler(u32 prev_pid, u32 next_pid)
{
    struct bts_state *prev_state, *next_state;

    prev_state = find_bts_state(prev_pid);
    next_state = find_bts_state(next_pid);

    if (prev_state)
    {
        xprintdbg("LIBIHT-COM: BTS context switch from pid %d\n",
            prev_state->config.pid);
        get_bts(prev_state);
    }

    if (next_state)
    {
        xprintdbg("LIBIHT-COM: BTS context switch to pid %d\n",
                next_state->config.pid);
        put_bts(next_state);
    }
}

void bts_newproc_handler(u32 parent_pid, u32 child_pid)
{
    struct bts_state *parent_state, *child_state;
    // char irql_flag[MAX_IRQL_LEN];

    parent_state = find_bts_state(parent_pid);
    if (parent_state == NULL)
        return;

    xprintdbg("LIBIHT-COM: BTS new process %d parent pid %d\n",
            child_pid, parent_pid);
    child_state = create_bts_state();
    if (child_state == NULL)
        return;

    child_state->parent = parent_state;
    child_state->config.pid = child_pid;
    child_state->config.bts_config = parent_state->config.bts_config;
    child_state->config.bts_buffer_size = parent_state->config.bts_buffer_size;
    // TODO: memcpy or not? overhead? If yes, acquire lock for this operation
    insert_bts_state(child_state);

    // If the child process is the current process, trace it right away
    if (child_pid == xgetcurrent_pid())
        put_bts(child_state);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : bts_check
// Description  : Check if the BTS is available.
//
// Inputs       : void
// Outputs      : 0 if successful, -1 if failure

s32 bts_check(void)
{
    u32 cpuinfo[4] = { 0 };
    u64 misc_msr;

    xcpuid(1, &cpuinfo[0], &cpuinfo[1], &cpuinfo[2], &cpuinfo[3]);

    // Check if BTS is supported
    if (!(cpuinfo[3] & (1 << X64_FEATURE_DS)))
        return -1;

    // Check if BTS is available
    xrdmsr(MSR_IA32_MISC_ENABLE, &misc_msr);
    if (misc_msr & MSR_IA32_MISC_ENABLE_BTS_UNAVAIL)
        return -1;

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : bts_init
// Description  : Initialize the BTS.
//
// Inputs       : void
// Outputs      : 0 if successful, -1 if failure

s32 bts_init(void)
{
    // Check if BTS is supported and available
    if (bts_check())
    {
        xprintdbg("LIBIHT-COM: BTS is not supported or available.\n");
        return -1;
    }

    xprintdbg("LIBIHT-COM: Init BTS related structs.\n");
    xinit_lock(bts_state_lock);
    xinit_list_head(bts_state_head);

    // Flush BTS on each cpu
    xprintdbg("LIBIHT-COM: Flushing BTS for all cpus...\n");
    xon_each_cpu(flush_bts);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : bts_exit
// Description  : Exit the BTS.
//
// Inputs       : void
// Outputs      : 0 if successful, -1 if failure

s32 bts_exit(void)
{
    // Flush BTS on each cpu
    xprintdbg("LIBIHT-COM: Flushing BTS for all cpus...\n");
    xon_each_cpu(flush_bts);

    // Free bts_state_list
    xprintdbg("LIBIHT-COM: Freeing BTS state list.\n");
    free_bts_state_list();

    return 0;
}
