////////////////////////////////////////////////////////////////////////////////
//  File           : commons/cpu.c
//  Description    : This is the implementation of the CPU related functions for
//                   the libiht library. See associated documentation for more
//                   information.
//
//   Author        : Thomason Zhao
//   Last Modified : Nov 25, 2023
//

// Include Files
#include "cpu.h"
#include "lbr.h"
#include "xplat.h"

////////////////////////////////////////////////////////////////////////////////
//
// Function     : identify_cpu
// Description  : This function identifies the CPU model and its LBR capacity.
//                And set the global variable `lbr_capacity`.
//
// Inputs       : None
// Outputs      : 0 if successful, -1 if failure

s32 identify_cpu(void)
{
    u32 cpuinfo[4] = { 0 };
    u32 family, model;
    int i;

    xcpuid(1, &cpuinfo[0], &cpuinfo[1], &cpuinfo[2], &cpuinfo[3]);
    
    family = ((cpuinfo[0] >> 8) & 0xF) + ((cpuinfo[0] >> 20) & 0xFF);
    model = ((cpuinfo[0] >> 4) & 0xF) | ((cpuinfo[0] >> 12) & 0xF0);

    // Identify CPU model
    lbr_capacity = (u64)-1;
    for (i = 0; i < sizeof(cpu_lbr_maps) / sizeof(cpu_lbr_maps[0]); ++i)
    {
        if (model == cpu_lbr_maps[i].model)
        {
            lbr_capacity = cpu_lbr_maps[i].lbr_capacity;
            break;
        }
    }

    xprintdbg("LIBIHT-COM: DisplayFamily_DisplayModel - %x_%xH\n",
                    family, model);
    xprintdbg("LIBIHT-COM: LBR capacity - %ld\n", lbr_capacity);

    if (lbr_capacity == -1)
    {
        // Model name not found
        xprintdbg("LIBIHT-COM: CPU model not found\n");
        return -1;
    }

    return 0;
}
