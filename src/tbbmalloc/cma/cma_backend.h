#pragma once

#include "cma_utils.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../tbbmalloc_internal.h"

void* MapMemory(size_t bytes, PageType pageType)
{
    PVOID mem = VirtualAlloc(NULL, bytes, MEM_RESERVE | MEM_COMMIT | (REGULAR == pageType ? NULL : MEM_LARGE_PAGES), PAGE_READWRITE);
    if (!mem)
        return nullptr;

    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(mem, &mbi, sizeof(mbi)) == sizeof(mbi))
        CmaMappedMemory += mbi.RegionSize;

    return mem;
}

int UnmapMemory(void* area, size_t /*bytes*/)
{
	MEMORY_BASIC_INFORMATION mbi;
	if (VirtualQuery(area, &mbi, sizeof(mbi)) == sizeof(mbi))
        CmaMappedMemory -= mbi.RegionSize;

	BOOL result = VirtualFree(area, 0, MEM_RELEASE);

	return !result;
}
