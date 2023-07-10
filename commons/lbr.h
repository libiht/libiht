#ifndef _COMMONS_LBR_H
#define _COMMONS_LBR_H

#include "types.h"

/* LBR MSR register and config definitions */

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

/* Bit Field        Bit Offset  Access  Description
 *
 * CPL_EQ_0         0           R/W     When set, do not capture branches ending in ring 0
 * CPL_NEQ_0        1           R/W     When set, do not capture branches ending in ring >0
 * JCC              2           R/W     When set, do not capture conditional branches
 * NEAR_REL_CALL    3           R/W     When set, do not capture near relative calls
 * NEAR_IND_CALL    4           R/W     When set, do not capture near indirect calls
 * NEAR_RET         5           R/W     When set, do not capture near returns
 * NEAR_IND_JMP     6           R/W     When set, do not capture near indirect jumps
 * NEAR_REL_JMP     7           R/W     When set, do not capture near relative jumps
 * FAR_BRANCH       8           R/W     When set, do not capture far branches
 * Reserved         63:9                Must be zero
 *
 * Default selection bit set to:
 * 0x1 = 00000001   --> capture branches occuring in ring >0
 */
#define LBR_SELECT              0x00000001

/* LBR related structs */

struct lbr_stack_entry
{
    u64 from;
    u64 to;
};

struct lbr_state
{
    u64 lbr_select;
    u64 lbr_tos;
    u32 pid;
    struct lbr_state *prev;
    struct lbr_state *next;
    struct lbr_state *parent;
    struct lbr_stack_entry entries[];
};

/*
 * The struct represent the mapping between CPU model and its corrosponding
 * LBR entries (if exist)
 */
struct cpu_to_lbr
{
    u32 model;
    u32 lbr_capacity;
};

/* LBR related function prototypes */

static void flush_lbr(u8);
static void get_lbr(u32);
static void put_lbr(u32);
static void dump_lbr(u32);
static void enable_lbr(void *);
static void disable_lbr(void *);

static struct lbr_state *create_lbr_state(void);
static void insert_lbr_state(struct lbr_state *);
static void remove_lbr_state(struct lbr_state *);
static struct lbr_state *find_lbr_state(u32);

static void save_lbr(void);
static void restore_lbr(void);

#endif /* _COMMONS_LBR_H */