#pragma once
#include <cstdint>

extern "C" {
	extern __declspec(dllexport) uint64_t	CmaPerfMappedMemory;
	extern __declspec(dllexport) uint64_t	CmaPerfCallVirtualAlloc;
	extern __declspec(dllexport) uint64_t	CmaPerfCallVirtualAllocLP;
	extern __declspec(dllexport) uint64_t	CmaPerfCallVirtualFree;

	extern __declspec(dllexport) uint64_t	CmaPerfCallMemTotalComitted;
	extern __declspec(dllexport) uint64_t	CmaPerfCallMemTotalReserved;
	extern __declspec(dllexport) uint64_t	CmaPerfCallMemFlushCache;
	extern __declspec(dllexport) uint64_t	CmaPerfCallMemFlushCacheAll;
	extern __declspec(dllexport) uint64_t	CmaPerfCallMemSize;
	extern __declspec(dllexport) uint64_t	CmaPerfCallMemSizeA;
	extern __declspec(dllexport) uint64_t	CmaPerfCallMemAlloc;
	extern __declspec(dllexport) uint64_t	CmaPerfCallMemAllocA;
	extern __declspec(dllexport) uint64_t	CmaPerfCallMemFree;
	extern __declspec(dllexport) uint64_t	CmaPerfCallMemFreeA;


#define EXT0_MAKE_PERF(perf) { (&perf), (uint64_t*)(L#perf) }
#define EXT0_MAKE_PERFA(perf) { (&perf), (uint64_t*)(#perf) }

	static uint64_t	*CmaPerf[][2] = {
		EXT0_MAKE_PERF(CmaPerfMappedMemory),
		EXT0_MAKE_PERF(CmaPerfCallVirtualAlloc),
		EXT0_MAKE_PERF(CmaPerfCallVirtualAllocLP),
		EXT0_MAKE_PERF(CmaPerfCallVirtualFree),

		EXT0_MAKE_PERF(CmaPerfCallMemTotalComitted),
		EXT0_MAKE_PERF(CmaPerfCallMemTotalReserved),
		EXT0_MAKE_PERF(CmaPerfCallMemFlushCache),
		EXT0_MAKE_PERF(CmaPerfCallMemFlushCacheAll),
		EXT0_MAKE_PERF(CmaPerfCallMemSize),
		EXT0_MAKE_PERF(CmaPerfCallMemSizeA),
		EXT0_MAKE_PERF(CmaPerfCallMemAlloc),
		EXT0_MAKE_PERF(CmaPerfCallMemAllocA),
		EXT0_MAKE_PERF(CmaPerfCallMemFree),
		EXT0_MAKE_PERF(CmaPerfCallMemFreeA)
	};

	static uint64_t	*CmaPerfA[][2] = {
		EXT0_MAKE_PERF(CmaPerfMappedMemory),
		EXT0_MAKE_PERF(CmaPerfCallVirtualAlloc),
		EXT0_MAKE_PERF(CmaPerfCallVirtualAllocLP),
		EXT0_MAKE_PERF(CmaPerfCallVirtualFree),

		EXT0_MAKE_PERF(CmaPerfCallMemTotalComitted),
		EXT0_MAKE_PERF(CmaPerfCallMemTotalReserved),
		EXT0_MAKE_PERF(CmaPerfCallMemFlushCache),
		EXT0_MAKE_PERF(CmaPerfCallMemFlushCacheAll),
		EXT0_MAKE_PERF(CmaPerfCallMemSize),
		EXT0_MAKE_PERF(CmaPerfCallMemSizeA),
		EXT0_MAKE_PERF(CmaPerfCallMemAlloc),
		EXT0_MAKE_PERF(CmaPerfCallMemAllocA),
		EXT0_MAKE_PERF(CmaPerfCallMemFree),
		EXT0_MAKE_PERF(CmaPerfCallMemFreeA)
	};
}