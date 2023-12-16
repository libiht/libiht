////////////////////////////////////////////////////////////////////////////////
//
//  File           : commons/bts.c
//  Description    : This is the implementation of the BTS feature for the
//                   libiht library. See associated documentation for more
//                   information.
//
//   Author        : Thomason Zhao
//   Last Modified : Dec 15, 2023
//

// Include Files
#include "bts.h"

//
// Global Variables

void *bts_buffer;
// BTS buffer



// TODO
s32 bts_init(void)
{
    u64 ds_area, debugctlmsr;

    // Check if BTS is supported
    xcpuid(0x7, NULL, NULL, NULL, &ds_area);
    if (!(ds_area & (1UL << 25)))
    {
        xprintdbg("LIBIHT-COM: BTS is not supported\n");
        return -1;
    }

    // Check if BTS is enabled
    xrdmsr(MSR_IA32_DEBUGCTLMSR, &debugctlmsr);
    if (!(debugctlmsr & DEBUGCTLMSR_BTS))
    {
        xprintdbg("LIBIHT-COM: BTS is not enabled\n");
        return -1;
    }

    // Initialize BTS
    xwrmsr(MSR_IA32_DS_AREA, (u64)bts_buffer);

    return 0;
}

// TODO
s32 bts_exit(void)
{
    u64 debugctlmsr;

    // Disable BTS
    xrdmsr(MSR_IA32_DEBUGCTLMSR, &debugctlmsr);
    debugctlmsr &= ~DEBUGCTLMSR_BTS;
    xwrmsr(MSR_IA32_DEBUGCTLMSR, debugctlmsr);

    xwrmsr(MSR_IA32_DS_AREA, 0);

    return 0;
}
