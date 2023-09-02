#include <ntifs.h>
#include <ntddk.h>
#include <wdm.h>
#include <windef.h>
#include <intrin.h>

#include "libiht_kmd.h"

#pragma intrinsic(_disable)
#pragma intrinsic(_enable)

void print_dbg(const char* format, ...)
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
void flush_lbr(u8 enable)
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
void get_lbr(u32 pid)
{
	int i;

	KIRQL old_irql;
	KeAcquireSpinLock(&lbr_cache_lock, &old_irql);

	struct lbr_state* state = find_lbr_state_worker(pid);
	if (state == NULL)
		return;

	// TODO: Directly read from hardware may contaminated by other process
	//state->lbr_select = __readmsr(MSR_LBR_SELECT);
	state->lbr_tos = __readmsr(MSR_LBR_TOS);

	for (i = 0; i < lbr_capacity; i++)
	{
		state->entries[i].from = __readmsr(MSR_LBR_NHM_FROM + i);
		state->entries[i].to = __readmsr(MSR_LBR_NHM_TO + i);
	}

	KeReleaseSpinLock(&lbr_cache_lock, old_irql);
}

/*
 * Write the LBR registers from kernel maintained datastructure
 */
void put_lbr(u32 pid)
{
	int i;

	KIRQL old_irql;
	KeAcquireSpinLock(&lbr_cache_lock, &old_irql);

	struct lbr_state* state = find_lbr_state_worker(pid);
	if (state == NULL)
		return;

	__writemsr(MSR_LBR_SELECT, state->lbr_select);
	__writemsr(MSR_LBR_TOS, state->lbr_tos);

	for (i = 0; i < lbr_capacity; i++)
	{
		__writemsr(MSR_LBR_NHM_FROM + i, state->entries[i].from);
		__writemsr(MSR_LBR_NHM_TO + i, state->entries[i].to);
	}

	KeReleaseSpinLock(&lbr_cache_lock, old_irql);
}

/*
 * Dump out the LBR registers to kernel message
 */
void dump_lbr(u32 pid)
{
	int i;
	struct lbr_state* state;

	KIRQL oldIrql;
	KeAcquireSpinLock(&lbr_cache_lock, &oldIrql);

	state = find_lbr_state_worker(pid);
	if (state == NULL)
	{
		print_dbg("LIBIHT-KMD: find lbr_state failed\n");
		return;
	}

	// TODO: Depend on situation, fetch or not
	//get_lbr(pid);

	print_dbg("PROC_PID:             %d\n", state->pid);
	print_dbg("MSR_LBR_SELECT:       0x%llx\n", state->lbr_select);
	print_dbg("MSR_LBR_TOS:          %lld\n", state->lbr_tos);

	for (i = 0; i < lbr_capacity; i++)
	{
		print_dbg("MSR_LBR_NHM_FROM[%2d]: 0x%llx\n", i, state->entries[i].from);
		print_dbg("MSR_LBR_NHM_TO  [%2d]: 0x%llx\n", i, state->entries[i].to);
	}

	print_dbg("LIBIHT-KMD: LBR info for cpuid: %d\n", KeGetCurrentProcessorNumberEx(NULL));

	KeReleaseSpinLock(&lbr_cache_lock, oldIrql);
}

/*
 * Enable the LBR feature for the current CPU.
 */
void enable_lbr()
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
void disable_lbr(void)
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
struct lbr_state* create_lbr_state(void)
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
void insert_lbr_state(struct lbr_state* new_state)
{
	struct lbr_state* head;

	if (new_state == NULL)
	{
		print_dbg("LIBIHT-KMD: Insert new state param is NULL\n");
		return;
	}

	KIRQL oldIrql;
	KeAcquireSpinLock(&lbr_cache_lock, &oldIrql);

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

	print_dbg("LIBIHT-KMD: Insert LBR state for pid %d\n", new_state->pid);

	KeReleaseSpinLock(&lbr_cache_lock, oldIrql);
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
				remove_lbr_state_worker(tmp);
			tmp = tmp->prev;
		} while (tmp != lbr_state_list);
	}

	print_dbg("LIBIHT-KMD: Remove LBR state for pid %d\n", old_state->pid);

	ExFreePoolWithTag(old_state, LIBIHT_KMD_TAG);
}

void remove_lbr_state(struct lbr_state* old_state)
{
	KIRQL oldIrql;
	KeAcquireSpinLock(&lbr_cache_lock, &oldIrql);

	remove_lbr_state_worker(old_state);

	KeReleaseSpinLock(&lbr_cache_lock, oldIrql);
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

	KIRQL oldIrql;
	KeAcquireSpinLock(&lbr_cache_lock, &oldIrql);

	state = find_lbr_state_worker(pid);

	KeReleaseSpinLock(&lbr_cache_lock, oldIrql);

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

