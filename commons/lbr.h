#ifndef _COMMONS_LBR_H
#define _COMMONS_LBR_H

#include "types.h"

/* cpp cross compile handler */
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

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

/* LBR related extern globals */

struct lbr_state *lbr_state_list;
u64 lbr_capacity;

/* LBR related function prototypes */

void flush_lbr(u8 enable);
void get_lbr(u32 pid);
void put_lbr(u32 pid);
void dump_lbr(u32 pid);
void enable_lbr(void);
void disable_lbr(void);

struct lbr_state *create_lbr_state(void);
void insert_lbr_state(struct lbr_state *new_state);
void remove_lbr_state_worker(struct lbr_state *old_state);
void remove_lbr_state(struct lbr_state *old_state);
struct lbr_state *find_lbr_state_worker(u32 pid);
struct lbr_state *find_lbr_state(u32 pid);

void save_lbr(u32 pid);
void restore_lbr(u32 pid);

u32 lbr_init(void);
u32 lbr_exit(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* _COMMONS_LBR_H */