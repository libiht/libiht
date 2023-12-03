////////////////////////////////////////////////////////////////////////////////
//
//  File           : commons/lbr.c
//  Description    : This is the implementation of the LBR feature for the
//                   libiht library. See associated documentation for more
//                   information.
//
//   Author        : Thomason Zhao
//   Last Modified : Nov 25, 2023
//

// Include Files
#include "lbr.h"
#include "xplat.h"

//
// Low level LBR stack and registers access

////////////////////////////////////////////////////////////////////////////////
//
// Function     : flush_lbr
// Description  : Flush the LBR stack and registers. Caller should ensure this
//                function is called with interrupts disabled (either on single
//                core or with interrupts disabled for that core).
//
// Inputs       : enable - TRUE to enable LBR, FALSE to disable LBR
// Outputs      : void

void flush_lbr(u8 enable)
{
    int i;

    xwrmsr(MSR_LBR_SELECT, 0);
    xwrmsr(MSR_LBR_TOS, 0);

    for (i = 0; i < lbr_capacity; i++)
    {
        xwrmsr(MSR_LBR_NHM_FROM + i, 0);
        xwrmsr(MSR_LBR_NHM_TO + i, 0);
    }

    if (enable)
        xwrmsr(MSR_IA32_DEBUGCTLMSR, DEBUGCTLMSR_LBR);
    else
        xwrmsr(MSR_IA32_DEBUGCTLMSR, 0);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : get_lbr
// Description  : Read the LBR registers into kernel maintained datastructure.
//
// Inputs       : pid - the process id
// Outputs      : void

void get_lbr(u32 pid)
{
    int i;
    char irql_flag[MAX_IRQL_LEN];

    xacquire_lock(lbr_state_lock, (void *)irql_flag);

    struct lbr_state *state = find_lbr_state_worker(pid);
    if (state == NULL)
        return;

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
// Inputs       : pid - the process id
// Outputs      : void

void put_lbr(u32 pid)
{
    int i;
    char irql_flag[MAX_IRQL_LEN];

    xacquire_lock(lbr_state_lock, (void *)irql_flag);

    struct lbr_state* state = find_lbr_state_worker(pid);
    if (state == NULL)
        return;

    xwrmsr(MSR_LBR_SELECT, state->lbr_select);
    xwrmsr(MSR_LBR_TOS, state->lbr_tos);

    for (i = 0; i < lbr_capacity; i++)
    {
        xwrmsr(MSR_LBR_NHM_FROM + i, state->entries[i].from);
        xwrmsr(MSR_LBR_NHM_TO + i, state->entries[i].to);
    }

    xrelease_lock(lbr_state_lock, (void *)irql_flag);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : dump_lbr
// Description  : Dump the LBR registers for the given process id
//
// Inputs       : pid - the process id
// Outputs      : void

void dump_lbr(u32 pid)
{
    int i;
    struct lbr_state* state;
    char irql_flag[MAX_IRQL_LEN];

    xacquire_lock(lbr_state_lock, (void *)irql_flag);

    state = find_lbr_state_worker(pid);
    if (state == NULL)
    {
        xprintdbg("LIBIHT-COM: find lbr_state failed\n");
        return;
    }

    // Dump the LBR state
    xprintdbg("PROC_PID:             %d\n", state->pid);
    xprintdbg("MSR_LBR_SELECT:       0x%llx\n", state->lbr_select);
    xprintdbg("MSR_LBR_TOS:          %lld\n", state->lbr_tos);

    for (i = 0; i < lbr_capacity; i++)
    {
        xprintdbg("MSR_LBR_NHM_FROM[%2d]: 0x%llx\n", i, state->entries[i].from);
        xprintdbg("MSR_LBR_NHM_TO  [%2d]: 0x%llx\n", i, state->entries[i].to);
    }

    xprintdbg("LIBIHT-COM: LBR info for cpuid: %d\n", xcoreid());

    xrelease_lock(lbr_state_lock, (void *)irql_flag);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : enable_lbr
// Description  : Enable the LBR feature for the current CPU core. This function
//                should be called on each CPU core by `xon_each_cpu()`
//				  function dispatch.
//
// Inputs       : void
// Outputs      : void

void enable_lbr(void)
{
    char irql_flag[MAX_IRQL_LEN];

    xacquire_lock(lbr_state_lock, (void *)irql_flag);

    xprintdbg("LIBIHT-COM: Enable LBR on cpu core: %d...\n", xcoreid());

    // Flush the LBR and enable it
    flush_lbr(TRUE);

    xrelease_lock(lbr_state_lock, (void *)irql_flag);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : disable_lbr
// Description  : Disable the LBR feature for the current CPU core. This
//				  function should be called on each CPU core by `xon_each_cpu()`
//				  function dispatch.
//
// Inputs       : void
// Outputs      : void

void disable_lbr(void)
{
    char irql_flag[MAX_IRQL_LEN];

    xacquire_lock(lbr_state_lock, (void *)irql_flag);

    xprintdbg("LIBIHT-COM: Disable LBR on cpu core: %d...\n", xcoreid());

    // Remove the selection mask
    __writemsr(MSR_LBR_SELECT, 0);

    // Flush the LBR and disable it
    flush_lbr(FALSE);

    xrelease_lock(lbr_state_lock, (void *)irql_flag);
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

    u64 state_size = sizeof(struct lbr_state) +
                        lbr_capacity * sizeof(struct lbr_stack_entry);

    state = xmalloc(state_size);
    if (state == NULL)
        return NULL;

    xmemset(state, 0, state_size);

    return state;
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
    struct lbr_state* head;
    char irql_flag[MAX_IRQL_LEN];

    if (new_state == NULL)
    {
        xprintdbg("LIBIHT-COM: Insert new state param is NULL\n");
        return;
    }

    xacquire_lock(lbr_state_lock, (void *)irql_flag);

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

    xprintdbg("LIBIHT-COM: Insert LBR state for pid %d\n", new_state->pid);

    xrelease_lock(lbr_state_lock, (void *)irql_flag);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : remove_lbr_state_worker
// Description  : Remove the LBR state from the list. This worker function
//				  is called with the `lbr_state_lock` acquired.
//
// Inputs       : old_state - the old LBR state
// Outputs      : void

void remove_lbr_state_worker(struct lbr_state* old_state)
{

    struct lbr_state* head;
    struct lbr_state* tmp;

    if (old_state == NULL)
    {
        xprintdbg("LIBIHT-COM: Remove old state param is NULL\n");
        return;
    }

    head = lbr_state_list;
    if (head == NULL)
    {
        xprintdbg("LIBIHT-COM: Remove old state list head is NULL\n");
        return;
    }

    if (head == old_state)
    {
        // Check if only one state in the list
        lbr_state_list = head->next == head ? NULL : head->next;
    }

    // Unlink from linked list
    old_state->prev->next = old_state->next;
    old_state->next->prev = old_state->prev;

    // Free all its child
    if (lbr_state_list != NULL)
    {
        tmp = lbr_state_list;
        do
        {
            if (tmp->parent == old_state)
                remove_lbr_state_worker(tmp);
            tmp = tmp->prev;
        } while (tmp != lbr_state_list);
    }

    xprintdbg("LIBIHT-COM: Remove LBR state for pid %d\n", old_state->pid);

    xfree(old_state);
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

    xacquire_lock(lbr_state_lock, (void *)irql_flag);

    remove_lbr_state_worker(old_state);

    xrelease_lock(lbr_state_lock, (void *)irql_flag);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : find_lbr_state_worker
// Description  : Find the LBR state for the given process id. This worker
//				  function is called with the `lbr_state_lock` acquired.
//
// Inputs       : pid - the process id
// Outputs      : struct lbr_state* - the LBR state

struct lbr_state* find_lbr_state_worker(u32 pid)
{
    struct lbr_state* tmp;

    if (lbr_state_list != NULL)
    {
        // Perform a backward traversal to find the state
        tmp = lbr_state_list;
        do {
            if (tmp->pid == pid)
            {
                return tmp;
            }
            tmp = tmp->prev;
        } while (tmp != lbr_state_list);
    }

    return NULL;
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
    struct lbr_state* state;
    char irql_flag[MAX_IRQL_LEN];

    xacquire_lock(lbr_state_lock, (void *)irql_flag);

    state = find_lbr_state_worker(pid);

    xrelease_lock(lbr_state_lock, (void *)irql_flag);

    return state;
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
    // Initialize the LBR state lock
    xinit_lock(lbr_state_lock);

    // Enable LBR on each cpu (Not yet set the selection filter bit)
    xprintdbg("LIBIHT-COM: Enabling LBR for all cpus...\n");
    xon_each_cpu(enable_lbr);

    // Set the state list to NULL after module initialized
    lbr_state_list = NULL;
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
    struct lbr_state* curr, * prev;

    // Free all LBR state
    xprintdbg("LIBIHT-COM: Freeing LBR state list...\n");
    if (lbr_state_list != NULL)
    {
        curr = lbr_state_list;
        do
        {
            prev = curr->prev;
            xfree(curr);
            curr = prev;
        } while (curr != lbr_state_list);
    }

    // Disable LBR on each cpu
    xprintdbg("LIBIHT-COM: Disabling LBR for all cpus...\n");
    xon_each_cpu(disable_lbr);
    return 0;
}
