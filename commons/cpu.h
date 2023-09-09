#ifndef _COMMONS_CPU_H
#define _COMMONS_CPU_H

#include "types.h"

/* cpp cross compile handler */
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*
 * The struct represent the mapping between CPU model and its corrosponding
 * LBR entries (if exist)
 */
struct cpu_to_lbr
{
    u32 model;
    u32 lbr_capacity;
};

/*
 * Constant CPU - LBR map, if the model not listed, it does not
 * support the LBR feature.
 */
static const struct cpu_to_lbr cpu_lbr_maps[] = {
    {0x5c, 32}, {0x5f, 32}, {0x4e, 32}, {0x5e, 32}, {0x8e, 32}, {0x9e, 32}, 
    {0x55, 32}, {0x66, 32}, {0x7a, 32}, {0x67, 32}, {0x6a, 32}, {0x6c, 32}, 
    {0x7d, 32}, {0x7e, 32}, {0x8c, 32}, {0x8d, 32}, {0xa5, 32}, {0xa6, 32}, 
    {0xa7, 32}, {0xa8, 32}, {0x86, 32}, {0x8a, 32}, {0x96, 32}, {0x9c, 32}, 
    {0x3d, 16}, {0x47, 16}, {0x4f, 16}, {0x56, 16}, {0x3c, 16}, {0x45, 16}, 
    {0x46, 16}, {0x3f, 16}, {0x2a, 16}, {0x2d, 16}, {0x3a, 16}, {0x3e, 16}, 
    {0x1a, 16}, {0x1e, 16}, {0x1f, 16}, {0x2e, 16}, {0x25, 16}, {0x2c, 16}, 
    {0x2f, 16}, {0x17, 4}, {0x1d, 4}, {0x0f, 4}, {0x37, 8}, {0x4a, 8}, {0x4c, 8}, 
    {0x4d, 8}, {0x5a, 8}, {0x5d, 8}, {0x1c, 8}, {0x26, 8}, {0x27, 8}, {0x35, 8}, 
    {0x36, 8}};

/* CPU related function prototypes */
static s32 identify_cpu(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* _COMMONS_CPU_H */