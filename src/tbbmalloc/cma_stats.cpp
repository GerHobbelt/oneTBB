#include "cma_stats.h"

extern "C" {
	uint64_t	CmaPerfMappedMemory = 0;
	uint64_t	CmaPerfCallVirtualAlloc = 0;
	uint64_t	CmaPerfCallVirtualAllocLP = 0;
	uint64_t	CmaPerfCallVirtualFree = 0;

	uint64_t	CmaPerfCallMemTotalComitted = 0;
	uint64_t	CmaPerfCallMemTotalReserved = 0;
	uint64_t	CmaPerfCallMemFlushCache = 0;
	uint64_t	CmaPerfCallMemFlushCacheAll = 0;
	uint64_t	CmaPerfCallMemSize = 0;
	uint64_t	CmaPerfCallMemSizeA = 0;
	uint64_t	CmaPerfCallMemAlloc = 0;
	uint64_t	CmaPerfCallMemAllocA = 0;
	uint64_t	CmaPerfCallMemFree = 0;
	uint64_t	CmaPerfCallMemFreeA = 0;
}