#include "cma_api.h"
#include "cma_stats.h"
#include "tbb\scalable_allocator.h"
#include <Windows.h>
#include <Psapi.h>
#include <strsafe.h>
#include "cma.h"
#pragma comment(lib, "Psapi.lib")


size_t __stdcall MemTotalCommitted(void)
{
	InterlockedIncrement64((LONG64*)&CmaPerfCallMemTotalComitted);
	return CmaPerfMappedMemory;
}

size_t __stdcall MemTotalReserved(void)
{
	InterlockedIncrement64((LONG64*)&CmaPerfCallMemTotalReserved);
	return CmaPerfMappedMemory;
}

size_t __stdcall MemFlushCache(size_t size)
{
	InterlockedIncrement64((LONG64*)&CmaPerfCallMemFlushCache);
	return size;
}

void __stdcall MemFlushCacheAll(void)
{
	InterlockedIncrement64((LONG64*)&CmaPerfCallMemFlushCacheAll);
	scalable_allocation_command(TBBMALLOC_CLEAN_ALL_BUFFERS, nullptr);
}

size_t __stdcall MemSize(void* mem)
{
	InterlockedIncrement64((LONG64*)&CmaPerfCallMemSize);
	return scalable_msize(mem);
}

size_t __stdcall MemSizeA(void* mem, size_t align)
{
	UNREFERENCED_PARAMETER(align);
	InterlockedIncrement64((LONG64*)&CmaPerfCallMemSizeA);
	return scalable_msize(mem);
}

void* __stdcall MemAlloc(size_t size)
{
	InterlockedIncrement64((LONG64*)&CmaPerfCallMemAlloc);
	return scalable_malloc(size);
}

void* __stdcall MemAllocA(size_t size, size_t align)
{
	InterlockedIncrement64((LONG64*)&CmaPerfCallMemAllocA);
	return scalable_aligned_malloc(size, align);
}

void __stdcall MemFree(void* mem)
{
	InterlockedIncrement64((LONG64*)&CmaPerfCallMemFree);
	scalable_free(mem);
}

void __stdcall MemFreeA(void* mem)
{
	InterlockedIncrement64((LONG64*)&CmaPerfCallMemFreeA);
	scalable_aligned_free(mem);
}

void __stdcall EnableHugePages(void)
{
	if (CmaAssignLockMemoryPrivileges())
		scalable_allocation_mode(USE_HUGE_PAGES, 1);
}
