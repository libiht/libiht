#ifndef _COMMONS_BTS_H_
#define _COMMONS_BTS_H_

////////////////////////////////////////////////////////////////////////////////
//
//  File           : commons/bts.h
//  Description    : This is the header file for the BTS (Branch Trace Store)
//                   module, which is used to capture the branch trace records
//                   of a given process. Some of the macros and constants are
//                   adapted from the Linux kernel source code.
//
//   Author        : Thomason Zhao
//   Last Modified : Jan 10, 2023
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

// Intel-defined CPU features, CPUID level 0x00000001 (EDX), word 0
#ifndef X64_FEATURE_DS
#define X64_FEATURE_DS          (0*32+2)
#endif

// MSR related constants
#ifndef MSR_IA32_DEBUGCTLMSR
#define MSR_IA32_DEBUGCTLMSR    0x000001d9
#endif

#ifndef MSR_IA32_DS_AREA
#define MSR_IA32_DS_AREA        0x00000600
#endif

#ifndef MSR_IA32_MISC_ENABLE
#define MSR_IA32_MISC_ENABLE    0x000001a0
#endif

// MSR bit shifts
#ifndef DEBUGCTLMSR_TR
#define DEBUGCTLMSR_TR          (1UL <<  6)
#endif

#ifndef DEBUGCTLMSR_BTS
#define DEBUGCTLMSR_BTS         (1UL <<  7)
#endif

#ifndef DEBUGCTLMSR_BTINT
#define DEBUGCTLMSR_BTINT       (1UL <<  8)
#endif

#ifndef DEBUGCTLMSR_BTS_OFF_OS
#define DEBUGCTLMSR_BTS_OFF_OS  (1UL <<  9)
#endif

#ifndef DEBUGCTLMSR_BTS_OFF_USR
#define DEBUGCTLMSR_BTS_OFF_USR (1UL << 10)
#endif

#ifndef MSR_IA32_MISC_ENABLE_BTS_UNAVAIL_BIT
#define MSR_IA32_MISC_ENABLE_BTS_UNAVAIL_BIT    11
#endif

#ifndef MSR_IA32_MISC_ENABLE_BTS_UNAVAIL
#define MSR_IA32_MISC_ENABLE_BTS_UNAVAIL    (1ULL << MSR_IA32_MISC_ENABLE_BTS_UNAVAIL_BIT)
#endif

/* CPL-Qualified Branch Trace Store Encodings (Table 18-6 from Intel SDM)
 *
 * TR  BTS  BTS_OFF_OS  BTS_OFF_USR  BTINT  Description
 * 0    X        X           X         X    Branch trace messages (BTMs) off
 * 1    0        X           X         X    Generates BTMs but do not store BTMs
 * 1    1        0           0         0    Store all BTMs in the BTS buffer, 
 *                                          used here as a circular buffer
 * 1    1        1           0         0    Store BTMs with CPL > 0 in the BTS 
 *                                          buffer
 * 1    1        0           1         0    Store BTMs with CPL = 0 in the BTS 
 *                                          buffer
 * 1    1        1           1         X    Generate BTMs but do not store BTMs
 * 1    1        0           0         1    Store all BTMs in the BTS buffer; 
 *                                          generate an interrupt when the 
 *                                          buffer is nearly full
 * 1    1        1           0         1    Store BTMs with CPL > 0 in the BTS 
 *                                          buffer; generate an interrupt when 
 *                                          the buffer is nearly full
 * 1    1        0           1         1    Store BTMs with CPL = 0 in the BTS 
 *                                          buffer; generate an interrupt when 
 *                                          the buffer is nearly full
 */
#define DEFAULT_BTS_CONFIG     (DEBUGCTLMSR_TR | DEBUGCTLMSR_BTS | DEBUGCTLMSR_BTS_OFF_OS)

// BTS buffer size 0x200 * 2 = 0x400 = 1024 records
#define DEFAULT_BTS_BUFFER_SIZE        (0x3000 << 1) 

//
// Type definitions

// Define BTS record
struct bts_record
{
    u64 from;   // branch from
    u64 to;     // branch to
    u64 misc;   // misc information
};

// Define Debug Store buffer management area (Assuming 64-bit)
struct ds_area
{
    u64 bts_buffer_base;            // BTS buffer base
    u64 bts_index;                  // BTS current index
    u64 bts_absolute_maximum;       // BTS absolute maximum
    u64 bts_interrupt_threshold;    // BTS interrupt threshold
    u64 pebs_buffer_base;           // PEBS placeholder
    u64 pebs_index;                 // PEBS placeholder
    u64 pebs_absolute_maximum;      // PEBS placeholder
    u64 pebs_interrupt_threshold;   // PEBS placeholder
};

// Define BTS state
struct bts_state
{
    char list[MAX_LIST_LEN];        // Kernel linked list
    struct bts_state *parent;       // Parent bts_state
    struct bts_ioctl_request bts_request;
    struct ds_area *ds_area;        // Debug Store area pointer
};

//
// Global Variables

extern char bts_state_lock[MAX_LOCK_LEN];
// The lock for bts_state_list.

extern char bts_state_head[MAX_LIST_LEN];
// The head of the bts_state_list.

//
// Function Prototypes

void get_bts(struct bts_state *state);
// Get the BTS records from the BTS buffer.

void put_bts(struct bts_state *state);
// Put the BTS records into the BTS buffer.

void flush_bts(void);
// Flush the BTS buffer.

s32 enable_bts(struct bts_ioctl_request *request);
// Enable the BTS.

s32 disable_bts(struct bts_ioctl_request *request);
// Disable the BTS.

s32 dump_bts(struct bts_ioctl_request *request);
// Dump the BTS records.

s32 config_bts(struct bts_ioctl_request *request);
// Configure the BTS trace bits

struct bts_state *create_bts_state(void);
// Create a new BTS state

struct bts_state *find_bts_state(u32 pid);
// Find the BTS state by pid

void insert_bts_state(struct bts_state *new_state);
// Insert the BTS state into the list

void remove_bts_state(struct bts_state *old_state);
// Remove the BTS state from the list

void free_bts_state_list(void);
// Free the BTS state list

s32 bts_ioctl_handler(struct xioctl_request *request);
// The ioctl handler for the BTS

void bts_cswitch_handler(u32 prev_pid, u32 next_pid);
// The context switch handler for the BTS

void bts_newproc_handler(u32 parent_pid, u32 child_pid);
// The new process handler for the BTS

s32 bts_check(void);
// Check if the BTS is available

s32 bts_init(void);
// Initialize the BTS

s32 bts_exit(void);
// Exit the BTS


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _COMMONS_BTS_H_
