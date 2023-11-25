#include "lbr.h"
#include "xplat.h"

/************************************************
 * LBR helper functions
 *
 * Help to manage LBR stack/registers
 ************************************************/

 /*
  * Flush the LBR registers. Caller should ensure this function run on
  * single cpu (by wrapping KeRaiseIrql() and KeLowerIrql())
  */
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

/*
 * Store the LBR registers to kernel maintained datastructure
 */
void get_lbr(u32 pid)
{
	int i;

	xlock_core();
	xacquire_lock(lbr_state_lock);

	struct lbr_state *state = find_lbr_state_worker(pid);
	if (state == NULL)
		return;

	// TODO: Directly read from hardware may contaminated by other process
	//state->lbr_select = __readmsr(MSR_LBR_SELECT);
	state->lbr_tos = xrdmsr(MSR_LBR_TOS);

	for (i = 0; i < lbr_capacity; i++)
	{
		state->entries[i].from = xrdmsr(MSR_LBR_NHM_FROM + i);
		state->entries[i].to = xrdmsr(MSR_LBR_NHM_TO + i);
	}

	xrelease_lock(lbr_state_lock);
	xrelease_core();
}

/*
 * Write the LBR registers from kernel maintained datastructure
 */
void put_lbr(u32 pid)
{
	int i;

	xlock_core();
	xacquire_lock(lbr_state_lock);

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

	xrelease_lock(lbr_state_lock);
	xrelease_core();
}

/*
 * Dump out the LBR registers to kernel message
 */
void dump_lbr(u32 pid)
{
	int i;
	struct lbr_state* state;

	xlock_core();
	xacquire_lock(lbr_state_lock);

	state = find_lbr_state_worker(pid);
	if (state == NULL)
	{
		xprintdbg("LIBIHT-COM: find lbr_state failed\n");
		return;
	}

	// TODO: Depend on situation, fetch or not
	//get_lbr(pid);

	xprintdbg("PROC_PID:             %d\n", state->pid);
	xprintdbg("MSR_LBR_SELECT:       0x%llx\n", state->lbr_select);
	xprintdbg("MSR_LBR_TOS:          %lld\n", state->lbr_tos);

	for (i = 0; i < lbr_capacity; i++)
	{
		xprintdbg("MSR_LBR_NHM_FROM[%2d]: 0x%llx\n", i, state->entries[i].from);
		xprintdbg("MSR_LBR_NHM_TO  [%2d]: 0x%llx\n", i, state->entries[i].to);
	}

	xprintdbg("LIBIHT-COM: LBR info for cpuid: %d\n", xcoreid());

	xrelease_lock(lbr_state_lock);
	xrelease_core();
}

/*
 * Enable the LBR feature for the current CPU.
 */
void enable_lbr(void)
{
	xlock_core();
	xacquire_lock(lbr_state_lock);

	xprintdbg("LIBIHT-COM: Enable LBR on cpu core: %d...\n", xcoreid());

	/* Flush the LBR and enable it */
	flush_lbr(TRUE);

	xrelease_lock(lbr_state_lock);
	xrelease_core();
}

/*
 * Disable the LBR feature for the current CPU.
 */
void disable_lbr(void)
{
	xlock_core();
	xacquire_lock(lbr_state_lock);

	xprintdbg("LIBIHT-COM: Disable LBR on cpu core: %d...\n", xcoreid());

	/* Remove the filter */
	__writemsr(MSR_LBR_SELECT, 0);

	/* Flush the LBR and disable it */
	flush_lbr(FALSE);

	xrelease_lock(lbr_state_lock);
	xrelease_core();
}

/************************************************
 * LBR state helper functions
 *
 * Help to manage kernel LBR state datastructure
 ************************************************/

 /*
  * Create a new empty LBR state
  */
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

/*
 * Insert new LBR state into the back of the list
 */
void insert_lbr_state(struct lbr_state* new_state)
{
	struct lbr_state* head;

	if (new_state == NULL)
	{
		xprintdbg("LIBIHT-COM: Insert new state param is NULL\n");
		return;
	}

	xlock_core();
	xacquire_lock(lbr_state_lock);

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

	xrelease_lock(lbr_state_lock);
	xrelease_core();
}

/*
 * Remove old LBR state from the list
 */
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
				remove_lbr_state_worker(tmp);
			tmp = tmp->prev;
		} while (tmp != lbr_state_list);
	}

	xprintdbg("LIBIHT-COM: Remove LBR state for pid %d\n", old_state->pid);

	xfree(old_state);
}

void remove_lbr_state(struct lbr_state* old_state)
{
	xlock_core();
	xacquire_lock(lbr_state_lock);

	remove_lbr_state_worker(old_state);

	xrelease_lock(lbr_state_lock);
	xrelease_core();
}

/*
 * Find the LBR state by given the pid. (Try to do as fast as possiable)
 * 
 */
// TODO: High frequency function, try to optimize for best performance
struct lbr_state* find_lbr_state_worker(u32 pid)
{
	// Perform a backward traversal (typically, newly created processes are
	// more likely to be find)
	struct lbr_state* tmp;

	if (lbr_state_list != NULL)
	{
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
struct lbr_state* find_lbr_state(u32 pid)
{
	struct lbr_state* state;

	xlock_core();
	xacquire_lock(lbr_state_lock);

	state = find_lbr_state_worker(pid);

	xrelease_lock(lbr_state_lock);
	xrelease_core();

	return state;
}

/************************************************
 * Save and Restore the LBR during context switches.
 *
 * Should be done as fast as possiable to minimize the overhead of context switches.
 ************************************************/

 /*
  * Save LBR
  */
void save_lbr(u32 pid)
{
	get_lbr(pid);
}

/*
 * Restore LBR
 */
void restore_lbr(u32 pid)
{
	put_lbr(pid);
}

s32 lbr_init(void)
{
	// Initialize the LBR state lock
	lbr_state_lock = xmalloc(0x100);
	if (lbr_state_lock == NULL)
		return -1;
	xinit_lock(lbr_state_lock);

	// Enable LBR on each cpu (Not yet set the selection filter bit)
	xprintdbg("LIBIHT-COM: Enabling LBR for all cpus...\n");
	xon_each_cpu(enable_lbr);

	// Set the state list to NULL after module initialized
	lbr_state_list = NULL;
	return 0;
}

s32 lbr_exit(void)
{
	struct lbr_state* curr, * prev;

	// Free all LBR state
	xprintdbg("LIBIHT-KMD: Freeing LBR state list...\n");
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

	// Free the LBR state lock
	xfree(lbr_state_lock);
	lbr_state_lock = NULL;

	// Disable LBR on each cpu
	xprintdbg("LIBIHT-KMD: Disabling LBR for all cpus...\n");
	xon_each_cpu(disable_lbr);
	return 0;
}

