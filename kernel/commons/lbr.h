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
//   Last Modified : Jan 15, 2023
//

// Include Files
#include "types.h"
#include "xplat.h"
#include "xioctl.h"

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
#define MSR_LBR_TOS             0x000001c9
#endif

#ifndef MSR_LBR_NHM_FROM
#define MSR_LBR_NHM_FROM        0x00000680
#endif

#ifndef MSR_LBR_NHM_TO
#define MSR_LBR_NHM_TO          0x000006c0
#endif

#ifndef DEBUGCTLMSR_LBR
#define DEBUGCTLMSR_LBR         (1UL <<  0)
#endif

/* Bit Field  Bit Offset  Access  Description
 *
 * CPL_EQ_0      0   R/W     When set, do not capture branches ending in ring 0
 * CPL_NEQ_0     1   R/W     When set, do not capture branches ending in ring >0
 * JCC           2   R/W     When set, do not capture conditional branches
 * NEAR_REL_CALL 3   R/W     When set, do not capture near relative calls
 * NEAR_IND_CALL 4   R/W     When set, do not capture near indirect calls
 * NEAR_RET      5   R/W     When set, do not capture near returns
 * NEAR_IND_JMP  6   R/W     When set, do not capture near indirect jumps
 * NEAR_REL_JMP  7   R/W     When set, do not capture near relative jumps
 * FAR_BRANCH    8   R/W     When set, do not capture far branches
 * Reserved      63:9        Must be zero
 *
 * Default selection bit set to:
 * 0x1 = 00000001   --> capture branches occuring in ring >0
 */
#define LBR_SELECT              (1UL <<  0)

//
// Type definitions

// Define LBR state
struct lbr_state
{
    char list[MAX_LIST_LEN];          // Kernel linked list
    struct lbr_state *parent;         // Parent lbr_state
    struct lbr_config config;         // LBR configuration
    struct lbr_data *data;            // LBR data
};

// CPU - LBR map
struct cpu_to_lbr
{
    u32 model;          // CPU model
    u32 lbr_capacity;   // LBR capacity
};

//
// Global variables

extern u64 lbr_capacity;
// The capacity of the LBR.

extern char lbr_state_lock[MAX_LOCK_LEN];
// The lock for lbr_state_list.

extern char lbr_state_head[MAX_LIST_LEN];
// The head of the lbr_state_list.

//
// Function Prototypes

void get_lbr(struct lbr_state *state);
// Get the LBR of a given process.

void put_lbr(struct lbr_state *state);
// Put the LBR of a given process.

void flush_lbr(void);
// Flush the LBR.

s32 enable_lbr(struct lbr_ioctl_request *request);
// Enable the LBR.

s32 disable_lbr(struct lbr_ioctl_request *request);
// Disable the LBR.

s32 dump_lbr(struct lbr_ioctl_request *request);
// Dump the LBR of a given process.

s32 config_lbr(struct lbr_ioctl_request *request);
// Configure the LBR.

struct lbr_state *create_lbr_state(void);
// Create a new lbr_state.

struct lbr_state *find_lbr_state(u32 pid);
// Find a lbr_state from the lbr_state_list.

void insert_lbr_state(struct lbr_state *new_state);
// Insert a new lbr_state to the lbr_state_list.

void remove_lbr_state(struct lbr_state *old_state);
// Remove a lbr_state from the lbr_state_list.

void free_lbr_state_list(void);
// Free the lbr_state_list.

s32 lbr_ioctl_handler(struct xioctl_request *request);
// The ioctl handler for the LBR.

void lbr_cswitch_handler(u32 prev_pid, u32 next_pid);
// The context switch handler for the LBR.

void lbr_newproc_handler(u32 parent_pid, u32 child_pid);
// The new process handler for the LBR.

s32 lbr_check(void);
// Check if the LBR is available.

s32 lbr_init(void);
// Initialize the LBR.

s32 lbr_exit(void);
// Exit the LBR.

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _COMMONS_LBR_H