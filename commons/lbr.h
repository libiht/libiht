#ifndef _COMMONS_LBR_H
#define _COMMONS_LBR_H

////////////////////////////////////////////////////////////////////////////////
//
//  File           : commons/lbr.h
//  Description    : This is the header file for the LBR (Last Branch Record)
//                   module, which is used to capture the last branch records
//                   of a given process.
//
//   Author        : Thomason Zhao
//   Last Modified : Nov 25, 2023
//

// Include Files
#include "types.h"
#include "xplat.h"

// cpp cross compile handler
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//
// Library constants

// MSR related constants
#ifndef MSR_IA32_DEBUGCTLMSR
#define MSR_IA32_DEBUGCTLMSR    0x000001d9
#endif

#ifndef MSR_LBR_SELECT
#define MSR_LBR_SELECT          0x000001c8
#endif

#ifndef MSR_LBR_TOS
#define MSR_LBR_TOS		        0x000001c9
#endif

#ifndef MSR_LBR_NHM_FROM
#define MSR_LBR_NHM_FROM	    0x00000680
#endif

#ifndef MSR_LBR_NHM_TO
#define MSR_LBR_NHM_TO		    0x000006c0
#endif

#ifndef DEBUGCTLMSR_LBR
#define DEBUGCTLMSR_LBR			0x00000001 
#endif

/* Bit Field    Bit Offset  Access  Description
 *
 * CPL_EQ_0         0   R/W     When set, do not capture branches ending in ring 0
 * CPL_NEQ_0        1   R/W     When set, do not capture branches ending in ring >0
 * JCC              2   R/W     When set, do not capture conditional branches
 * NEAR_REL_CALL    3   R/W     When set, do not capture near relative calls
 * NEAR_IND_CALL    4   R/W     When set, do not capture near indirect calls
 * NEAR_RET         5   R/W     When set, do not capture near returns
 * NEAR_IND_JMP     6   R/W     When set, do not capture near indirect jumps
 * NEAR_REL_JMP     7   R/W     When set, do not capture near relative jumps
 * FAR_BRANCH       8   R/W     When set, do not capture far branches
 * Reserved         63:9        Must be zero
 *
 * Default selection bit set to:
 * 0x1 = 00000001   --> capture branches occuring in ring >0
 */
#define LBR_SELECT              0x00000001

//
// Type definitions

struct lbr_stack_entry
{
    u64 from;   // MSR_LBR_NHM_FROM + offset
    u64 to;     // MSR_LBR_NHM_TO + offset
};

// Define LBR state
struct lbr_state
{
    u64 lbr_select;                 // MSR_LBR_SELECT
    u64 lbr_tos;                    // MSR_LBR_TOS
    u32 pid;                        // process id
    struct lbr_state *prev;         // previous state
    struct lbr_state *next;         // next state
    struct lbr_state *parent;       // parent state
    struct lbr_stack_entry entries[]; // flexible array member
};

//
// Global variables

struct lbr_state *lbr_state_list;
// The head of the lbr_state_list.

u64 lbr_capacity;
// The capacity of the LBR.

char lbr_state_lock[MAX_LOCK_LEN];
// The lock for lbr_state_list.

//
// Functional Prototypes

void flush_lbr(u8 enable);
// Flush the LBR.

void get_lbr(u32 pid);
// Get the LBR of a given process.

void put_lbr(u32 pid);
// Put the LBR of a given process.

void dump_lbr(u32 pid);
// Dump the LBR of a given process.

void enable_lbr(void);
// Enable the LBR.

void disable_lbr(void);
// Disable the LBR.

struct lbr_state *create_lbr_state(void);
// Create a new lbr_state.

void insert_lbr_state(struct lbr_state *new_state);
// Insert a new lbr_state to the lbr_state_list.

void remove_lbr_state_worker(struct lbr_state *old_state);
// Remove a lbr_state from the lbr_state_list.

void remove_lbr_state(struct lbr_state *old_state);
// Remove a lbr_state from the lbr_state_list.

struct lbr_state *find_lbr_state_worker(u32 pid);
// Find a lbr_state from the lbr_state_list.

struct lbr_state *find_lbr_state(u32 pid);
// Find a lbr_state from the lbr_state_list.

s32 lbr_init(void);
// Initialize the LBR.

s32 lbr_exit(void);
// Exit the LBR.

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* _COMMONS_LBR_H */