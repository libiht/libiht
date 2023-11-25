#include "../../commons/xplat.h"
#include "../include/headers_kmd.h"

// TODO: add comments/documentation

/* Cross platform globals (used for lock, allocation, etc.) */
const unsigned long g_tag = 'XPLT';

void* xmalloc(u64 size)
{
	return ExAllocatePool2(POOL_FLAG_NON_PAGED, size, g_tag);
}

void xfree(void *ptr)
{
	ExFreePool(ptr);
}

void xmemset(void* ptr, s32 c, u64 cnt)
{
	memset(ptr, c, cnt);
}

void xlock_core(void *old_irql)
{
	KeRaiseIrql(DISPATCH_LEVEL, (PKIRQL)old_irql);
}

void xrelease_core(void *new_irql)
{
	KeLowerIrql(*(PKIRQL)new_irql);
}

void xwrmsr(u32 msr, u64 val)
{
	__writemsr(msr, val);
}

void xrdmsr(u32 msr, u64 *val)
{
	*val = __readmsr(msr);
}

void xinit_lock(void *lock)
{
	KeInitializeSpinLock((PKSPIN_LOCK)lock);
}

void xacquire_lock(void *lock, void *old_irql)
{
	KeAcquireSpinLock((PKSPIN_LOCK)lock, (PKIRQL)old_irql);
}

void xrelease_lock(void *lock, void *new_irql)
{
	KeReleaseSpinLock((PKSPIN_LOCK)lock, *(PKIRQL)new_irql);
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

void xon_each_cpu(void (*func)(void))
{
	KeIpiGenericCall((PKIPI_BROADCAST_WORKER)func, 0);
}

void xprintdbg(const char *format, ...)
{
	va_list args;
	_crt_va_start(args, format);
	vDbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, format, args);
}

