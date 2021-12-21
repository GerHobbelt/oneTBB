#pragma once

extern "C" {
	__declspec(dllexport)  void __stdcall CmaDumpStats(
		const wchar_t	*Context
	);

	__declspec(dllexport) size_t __stdcall MemTotalCommitted(
		void
	);

	__declspec(dllexport) size_t __stdcall MemTotalReserved(
		void
	);

	__declspec(dllexport) size_t __stdcall MemFlushCache(
		size_t	Size
	);

	__declspec(dllexport) void __stdcall MemFlushCacheAll(
		void
	);

	__declspec(dllexport) size_t __stdcall MemSize(
		void	*Mem
	);

	__declspec(dllexport) size_t __stdcall MemSizeA(
		void	*Mem,
		size_t	Align
	);

	__declspec(dllexport) void* __stdcall MemAlloc(
		size_t	Size
	);

	__declspec(dllexport) void* __stdcall MemAllocA(
		size_t	Size,
		size_t	Align
	);

	__declspec(dllexport) void __stdcall MemFree(
		void	*Mem
	);

	__declspec(dllexport) void __stdcall MemFreeA(
		void	*Mem
	);
}