////////////////////////////////////////////////////////////////////////////////
//
//  File           : commons/bts.c
//  Description    : This is the implementation of the BTS feature for the
//                   libiht library. See associated documentation for more
//                   information.
//
//   Author        : Thomason Zhao
//   Last Modified : Dec 25, 2023
//

// Include Files
#include "bts.h"

//
// Global Variables
char bts_state_lock[MAX_LOCK_LEN];
// Lock for bts state list

char bts_state_head[MAX_LIST_LEN];
// Head of bts state list

void get_bts(struct bts_state *state)
{
    u64 dbgctlmsr;

    // Disable BTS
    xrdmsr(MSR_IA32_DEBUGCTLMSR, &dbgctlmsr);
    dbgctlmsr &= ~state->bts_request.bts_config;
    xwrmsr(MSR_IA32_DEBUGCTLMSR, dbgctlmsr);

    // Reset BTS debug store buffer pointer
    xwrmsr(MSR_IA32_DS_AREA, NULL);
}

void put_bts(struct bts_state *state)
{
    u64 dbgctlmsr;

    // Setup BTS debug store buffer pointer
    xwrmsr(MSR_IA32_DS_AREA, (u64)state->ds_area);

    // Enable BTS
    xrdmsr(MSR_IA32_DEBUGCTLMSR, &dbgctlmsr);
    dbgctlmsr |= state->bts_request.bts_config;
    xwrmsr(MSR_IA32_DEBUGCTLMSR, dbgctlmsr);
}

void flush_bts()
{
    u64 dbgctlmsr;
    u64 bts_bits;

    bts_bits = DEBUGCTLMSR_TR |
                DEBUGCTLMSR_BTS |
                DEBUGCTLMSR_BTINT |
                DEBUGCTLMSR_BTS_OFF_OS |
                DEBUGCTLMSR_BTS_OFF_USR;

    // Disable BTS
    xprintdbg("LIBIHT-COM: Disable BTS on cpu core: %d...\n", xcoreid());
    xrdmsr(MSR_IA32_DEBUGCTLMSR, &dbgctlmsr);
    dbgctlmsr &= ~bts_bits;
    xwrmsr(MSR_IA32_DEBUGCTLMSR, dbgctlmsr);

    // Reset BTS debug store buffer pointer
    xwrmsr(MSR_IA32_DS_AREA, NULL);
}

s32 enable_bts(struct bts_ioctl_request *request)
{
    struct bts_state *state;

    state = find_bts_state(request->pid);
    if (state != NULL)
    {
        xprintdbg("LIBIHT-COM: BTS already enabled for pid %d.\n",
                    request->pid);
        return -1;
    }

    state = create_bts_state();
    if (state == NULL)
    {
        xprintdbg("LIBIHT-COM: Create BTS state failed.\n");
        return -1;
    }

    // Setup fields for BTS state
    state->bts_request.pid = request->pid ?
                request->pid : xgetcurrent_pid();
    state->bts_request.bts_config = request->bts_config ? 
                request->bts_config : DEFAULT_BTS_CONFIG;
    state->bts_request.bts_buffer_size = request->bts_buffer_size ?
                request->bts_buffer_size : DEFAULT_BTS_BUFFER_SIZE;
    
    // Setup fields for BTS debug store area
    state->ds_area->bts_buffer_base = (u64)xmalloc(state->bts_request.bts_buffer_size);
    state->ds_area->bts_index = 0;
    state->ds_area->bts_absolute_maximum = 
            state->ds_area->bts_buffer_base +
            state->bts_request.bts_buffer_size + 1;
    // Not yet support state->ds_area->bts_interrupt_threshold

    insert_bts_state(state);
    return 0;
}

s32 disable_bts(struct bts_ioctl_request *request)
{
    struct bts_state *state;

    state = find_bts_state(request->pid);
    if (state == NULL)
    {
        xprintdbg("LIBIHT-COM: BTS not enabled for pid %d.\n",
                    request->pid);
        return -1;
    }

    remove_bts_state(state);
    xfree((void *)state->ds_area->bts_buffer_base);
    xfree(state->ds_area);
    xfree(state);
    
    return 0;
}

s32 dump_bts(struct bts_ioctl_request *request)
{
    struct bts_state *state;
    struct bts_record *record;
    u32 i, start, end;

    state = find_bts_state(request->pid);
    if (state == NULL)
    {
        xprintdbg("LIBIHT-COM: BTS not enabled for pid %d.\n",
                    request->pid);
        return -1;
    }

    // Dump some BTS buffer records
    // TODO: fix this 32 later
    start = state->ds_area->bts_index - 32;
    end = state->ds_area->bts_index;
    for (i = start; i < end; i++)
    {
        record = (struct bts_record*)state->ds_area->bts_buffer_base + i;
        xprintdbg("LIBIHT-COM: BTS record %d: from %llx to %llx.\n",
                    i, record->from, record->to);
    }

    return 0;
}

s32 config_bts(struct bts_ioctl_request *request)
{
    struct bts_state *state;

    state = find_bts_state(request->pid);
    if (state == NULL)
    {
        xprintdbg("LIBIHT-COM: BTS not enabled for pid %d.\n",
                    request->pid);
        return -1;
    }

    state->bts_request.bts_config = request->bts_config;
    // If the current process is the target process, we need to
    // disable and re-enable BTS to apply the new configuration
    if (xgetcurrent_pid() == request->pid)
    {
        // TODO: check if it works
        get_bts(state);

        if (request->bts_buffer_size != state->bts_request.bts_buffer_size &&
            request->bts_buffer_size != 0)
        {
            state->bts_request.bts_buffer_size = request->bts_buffer_size;

            // Reconfigure BTS debug store area
            xfree((void *)state->ds_area->bts_buffer_base);
            state->ds_area->bts_buffer_base = (u64)xmalloc(request->bts_buffer_size);
            state->ds_area->bts_index = 0;
            state->ds_area->bts_absolute_maximum = 
                    state->ds_area->bts_buffer_base +
                    request->bts_buffer_size + 1;
        }

        put_bts(state);
    }

    return 0;
}

struct bts_state *create_bts_state()
{
    struct bts_state *state;
    
    state = xmalloc(sizeof(struct bts_state));
    // TODO: check if this satisfy the page alignment requirement
    state->ds_area = xmalloc(sizeof(struct ds_area));

    return state;
}

struct bts_state *find_bts_state(u32 pid)
{
    void *curr_list;
    struct bts_state *curr_state;
    u32 offset;

    // offsetof(st, m) macro implementation of stddef.h
    offset = (u64)(&((struct bts_state *)0)->list);
    curr_list = bts_state_head;
    do
    {
        curr_list = xlist_next(curr_list);
        if (curr_list == bts_state_head)
            return NULL;
        curr_state = (struct bts_state *)((u64)curr_list - offset);
    } while (curr_state->bts_request.pid != pid);

    return curr_state;
}

void insert_bts_state(struct bts_state *new_state)
{
    char irql_flag[MAX_IRQL_LEN];

    if (new_state == NULL)
        return;
    
    xacquire_lock(&bts_state_lock, irql_flag);
    xlist_add(&new_state->list, &bts_state_head);
    xprintdbg("LIBIHT-COM: Insert BTS state for pid %d.\n",
                new_state->bts_request.pid);
    xrelease_lock(&bts_state_lock, irql_flag);
}

void remove_bts_state(struct bts_state *old_state)
{
    char irql_flag[MAX_IRQL_LEN];

    if (old_state == NULL)
        return;
    
    xacquire_lock(&bts_state_lock, irql_flag);
    xlist_del(&old_state->list);
    xprintdbg("LIBIHT-COM: Remove BTS state for pid %d.\n",
                old_state->bts_request.pid);
    xrelease_lock(&bts_state_lock, irql_flag);
}

s32 bts_ioctl(struct xioctl_request *request)
{
    s32 ret;

    xprintdbg("LIBIHT-COM: BTS ioctl command %d.\n", request->cmd);
    switch (request->cmd)
    {
    case LIBIHT_IOCTL_ENABLE_BTS:
        xprintdbg("LIBIHT-COM: Enable BTS.\n");
        ret = enable_bts(&request->data.bts);
        break;
    
    case LIBIHT_IOCTL_DISABLE_BTS:
        xprintdbg("LIBIHT-COM: Disable BTS.\n");
        ret = disable_bts(&request->data.bts);
        break;

    case LIBIHT_IOCTL_DUMP_BTS:
        xprintdbg("LIBIHT-COM: Dump BTS.\n");
        ret = dump_bts(&request->data.bts);
        break;

    case LIBIHT_IOCTL_CONFIG_BTS:
        xprintdbg("LIBIHT-COM: Config BTS.\n");
        ret = config_bts(&request->data.bts);
        break;
    
    default:
        xprintdbg("LIBIHT-COM: Invalid BTS ioctl command.\n");
        break;
    }

    return ret;
}

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

s32 bts_init(void)
{
    // Check if BTS is supported and available
    if (bts_check())
    {
        xprintdbg("LIBIHT-COM: BTS is not supported or available.\n");
        return -1;
    }

    xprintdbg("LIBIHT-COM: Init BTS related structs.\n");
    xinit_lock(&bts_state_lock);
    xinit_list_head(&bts_state_head);

    return 0;
}

s32 bts_exit(void)
{
    // Disable BTS on each cpu
    xon_each_cpu(flush_bts);

    return 0;
}
