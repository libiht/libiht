#ifndef _COMMONS_BTS_H_
#define _COMMONS_BTS_H_

////////////////////////////////////////////////////////////////////////////////
//
//  File           : commons/bts.h
//  Description    : This is the header file for the BTS (Branch Trace Store)
//                   module, which is used to capture the branch trace records
//                   of a given process.
//
//   Author        : Thomason Zhao
//   Last Modified : Dec 15, 2023
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

#ifndef MSR_IA32_DS_AREA
#define MSR_IA32_DS_AREA        0x00000600
#endif

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

//
// Type definitions
// TODO

// Define BTS record
struct bts_record
{
    u64 from;   // branch from
    u64 to;     // branch to
    u64 misc;   // misc information
};

// Define Debug Store buffer management area
struct ds_area
{
    u64 bts_buffer_base;            // BTS buffer base
    u64 bts_index;                  // BTS current index
    u64 bts_absolute_maximum;       // BTS absolute maximum
    u64 bts_interrupt_threshold;    // BTS interrupt threshold
    u64 pebs_buffer_base;
    u64 pebs_index;
    u64 pebs_absolute_maximum;
    u64 pebs_interrupt_threshold;
};


//
// Global Variables
// TODO: should be extern

//
// Function Prototypes

s32 bts_init(void);
// Initialize the BTS

s32 bts_exit(void);
// Exit the BTS


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _COMMONS_BTS_H_
