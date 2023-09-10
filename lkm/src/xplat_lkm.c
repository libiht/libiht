#include "xplat.h"

void* xmalloc(u64 size)
{
	return 0;
}

void xfree(void* ptr)
{
}

void xlock_core(void)
{
}

void xrelease_core(void)
{
}

void xwrmsr(u32 msr, u64 val)
{
}

u64 xrdmsr(u32 msr)
{
	return 0;
}

void xacquire_lock(void* lock)
{
}

void xrelease_lock(void* lock)
{
}

u32 xcoreid(void)
{
	return 0;
}

void xcpuid(u32 cpuinfo[4], u32 func_id)
{
}

u32 xplat_load(void)
{
	return u32();
}
