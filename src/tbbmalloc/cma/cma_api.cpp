#include "cma_api.h"

#include "cma_utils.h"

#include "tbb\scalable_allocator.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

size_t __stdcall MemTotalCommitted(void)
{
	return 0;
}

size_t __stdcall MemTotalReserved(void)
{
	return 0;
}

size_t __stdcall MemFlushCache(size_t size)
{
	return size;
}

void __stdcall MemFlushCacheAll(void)
{
	scalable_allocation_command(TBBMALLOC_CLEAN_ALL_BUFFERS, nullptr);
}

size_t __stdcall MemSize(void* mem)
{
	return scalable_msize(mem);
}

size_t __stdcall MemSizeA(void* mem, size_t /*align*/)
{
	return scalable_msize(mem);
}

void* __stdcall MemAlloc(size_t size)
{
	return scalable_malloc(size);
}

void* __stdcall MemAllocA(size_t size, size_t align)
{
	return scalable_aligned_malloc(size, align);
}

void __stdcall MemFree(void* mem)
{
	scalable_free(mem);
}

void __stdcall MemFreeA(void* mem)
{
	scalable_aligned_free(mem);
}

void __stdcall EnableHugePages(void)
{
	if (CmaAcquireLockMemoryPrivileges())
		scalable_allocation_mode(USE_HUGE_PAGES, 1);
}
