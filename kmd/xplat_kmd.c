#include "../commons/xplat.h"
#include "infinity_hook/imports.hpp"

/* Cross platform globals (used for lock, irql) */
KIRQL g_irql;
const unsigned long g_tag = 'XPLT';

void* xmalloc(u64 size)
{
	return ExAllocatePool2(POOL_FLAG_NON_PAGED, size, g_tag);
}

void xfree(void* ptr)
{
	ExFreePool(ptr);
}

void xlock_core(void)
{
	KeRaiseIrql(DISPATCH_LEVEL, &g_irql);
}

void xrelease_core(void)
{
	KeLowerIrql(g_irql);
}

void xwrmsr(u32 msr, u64 val)
{
	__writemsr(msr, val);
}

u64 xrdmsr(u32 msr)
{
	return __readmsr(msr);
}

void xinit_lock(void* lock)
{
	KeInitializeSpinLock((PKSPIN_LOCK)lock);
}

void xacquire_lock(void* lock)
{
	KeAcquireSpinLock((PKSPIN_LOCK)lock, &g_irql);
}

void xrelease_lock(void* lock)
{
	KeReleaseSpinLock((PKSPIN_LOCK)lock, g_irql);
}

u32 xcoreid(void)
{
	return KeGetCurrentProcessorNumberEx(0);
}

void xcpuid(u32 func_id, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx)
{
	s32 regs[4];
	__cpuid(regs, func_id);
	*eax = regs[0];
	*ebx = regs[1];
	*ecx = regs[2];
	*edx = regs[3];
}

u32 xplat_load(void)
{
	return 0;
}
