#ifndef _COMMONS_CPU_H
#define _COMMONS_CPU_H

////////////////////////////////////////////////////////////////////////////////
//
//  File           : commons/cpu.h
//  Description    : This is the header file for the CPU module, which is used
//                   to identify the CPU model and its LBR capacity.
//
//   Author        : Thomason Zhao
//   Last Modified : Nov 25, 2023
//

// Include Files
#include "types.h"

// cpp cross compile handler
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//
// Type definitions

// CPU - LBR map
struct cpu_to_lbr
{
    u32 model;          // CPU model
    u32 lbr_capacity;   // LBR capacity
};

//
// Global variables

// CPU - LBR map table
static const struct cpu_to_lbr cpu_lbr_maps[] = {
    {0x5c, 32}, {0x5f, 32}, {0x4e, 32}, {0x5e, 32}, {0x8e, 32}, {0x9e, 32}, 
    {0x55, 32}, {0x66, 32}, {0x7a, 32}, {0x67, 32}, {0x6a, 32}, {0x6c, 32}, 
    {0x7d, 32}, {0x7e, 32}, {0x8c, 32}, {0x8d, 32}, {0xa5, 32}, {0xa6, 32}, 
    {0xa7, 32}, {0xa8, 32}, {0x86, 32}, {0x8a, 32}, {0x96, 32}, {0x9c, 32}, 
    {0x3d, 16}, {0x47, 16}, {0x4f, 16}, {0x56, 16}, {0x3c, 16}, {0x45, 16}, 
    {0x46, 16}, {0x3f, 16}, {0x2a, 16}, {0x2d, 16}, {0x3a, 16}, {0x3e, 16}, 
    {0x1a, 16}, {0x1e, 16}, {0x1f, 16}, {0x2e, 16}, {0x25, 16}, {0x2c, 16}, 
    {0x2f, 16}, {0x17,  4}, {0x1d,  4}, {0x0f,  4}, {0x37,  8}, {0x4a,  8},
    {0x4c,  8}, {0x4d,  8}, {0x5a,  8}, {0x5d,  8}, {0x1c,  8}, {0x26,  8},
    {0x27,  8}, {0x35,  8}, {0x36,  8}};

//
// Function prototypes

s32 identify_cpu(void);
// Identify CPU model and its LBR capacity

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* _COMMONS_CPU_H */