#include "cma_api.h"
#include "cma_stats.h"
#include "cma_util.h"
#include "tbb\scalable_allocator.h"
#include <Windows.h>
#include <Psapi.h>
#include <strsafe.h>
#pragma comment(lib, "Psapi.lib")


void __stdcall CmaDumpStats(
	const wchar_t	*Context
)
{
	SYSTEMTIME	time;
	GetLocalTime(&time);

	PROCESS_MEMORY_COUNTERS	pmc;
	pmc.cb = sizeof(pmc);
	GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));

	WCHAR	mm[64];
	WCHAR	pwss[64];
	WCHAR	wss[64];
	WCHAR	qpppu[64];
	WCHAR	qppu[64];
	WCHAR	qpnppu[64];
	WCHAR	qnppu[64];
	WCHAR	pfu[64];
	WCHAR	ppfu[64];

	CmaFormatToUnit(pmc.PeakWorkingSetSize, pwss);
	CmaFormatToUnit(pmc.WorkingSetSize, wss);
	CmaFormatToUnit(pmc.QuotaPeakPagedPoolUsage, qpppu);
	CmaFormatToUnit(pmc.QuotaPagedPoolUsage, qppu);
	CmaFormatToUnit(pmc.QuotaPeakNonPagedPoolUsage, qpnppu);
	CmaFormatToUnit(pmc.QuotaNonPagedPoolUsage, qnppu);
	CmaFormatToUnit(pmc.PagefileUsage, pfu);
	CmaFormatToUnit(pmc.PeakPagefileUsage, ppfu);

	CmaLogWrite(true, 0, L"");

	CmaLogWrite(
		false, 0,
		L"Dumping stats (%s):\r\n"
		L"\tPageFaultCount: %lu\r\n"
		L"\tPeakWorkingSetSize: %s\r\n"
		L"\tWorkingSetSize: %s\r\n"
		L"\tQuotaPeakPagedPoolUsage: %s\r\n"
		L"\tQuotaPagedPoolUsage: %s\r\n"
		L"\tQuotaPeakNonPagedPoolUsage: %s\r\n"
		L"\tQuotaNonPagedPoolUsage: %s\r\n"
		L"\tPagefileUsage: %s\r\n"
		L"\tPeakPagefileUsage: %s",
		wcsnlen(Context, 32) < 32 ? Context : L"[Context too long]", pmc.PageFaultCount, pwss, wss, qpppu, qppu, qpnppu, qnppu, pfu, ppfu
	);

	for (DWORD i = 0; i < ARRAYSIZE(CmaPerf); i++) {
		PULONG64 v = CmaPerf[i][0];
		LPCWSTR	s = (LPCWSTR)CmaPerf[i][1];

		CmaLogWrite(false, 0, L"%s: %llu", s, *v);
	}

	CmaLogWrite(false, 0, L"");
}

size_t __stdcall MemTotalCommitted(
	void
)
{
	InterlockedIncrement64((LONG64*)&CmaPerfCallMemTotalComitted);

	return CmaPerfMappedMemory;
}

size_t __stdcall MemTotalReserved(
	void
)
{
	InterlockedIncrement64((LONG64*)&CmaPerfCallMemTotalReserved);

	return CmaPerfMappedMemory;
}

size_t __stdcall MemFlushCache(
	size_t	Size
)
{
	InterlockedIncrement64((LONG64*)&CmaPerfCallMemFlushCache);

	return Size;
}

void __stdcall MemFlushCacheAll(
	void
)
{
	InterlockedIncrement64((LONG64*)&CmaPerfCallMemFlushCacheAll);

	scalable_allocation_command(TBBMALLOC_CLEAN_ALL_BUFFERS, nullptr);
}

size_t __stdcall MemSize(
	void	*Mem
)
{
	InterlockedIncrement64((LONG64*)&CmaPerfCallMemSize);

	return scalable_msize(Mem);
}

size_t __stdcall MemSizeA(
	void	*Mem,
	size_t	Align
)
{
	UNREFERENCED_PARAMETER(Align);

	InterlockedIncrement64((LONG64*)&CmaPerfCallMemSizeA);

	return scalable_msize(Mem);
}

void* __stdcall MemAlloc(
	size_t	Size
)
{
	InterlockedIncrement64((LONG64*)&CmaPerfCallMemAlloc);

	return scalable_malloc(Size);
}

void* __stdcall MemAllocA(
	size_t	Size,
	size_t	Align
)
{
	InterlockedIncrement64((LONG64*)&CmaPerfCallMemAllocA);

	return scalable_aligned_malloc(Size, Align);
}

void __stdcall MemFree(
	void	*Mem
)
{
	InterlockedIncrement64((LONG64*)&CmaPerfCallMemFree);

	scalable_free(Mem);
}

void __stdcall MemFreeA(
	void	*Mem
)
{
	InterlockedIncrement64((LONG64*)&CmaPerfCallMemFreeA);

	scalable_aligned_free(Mem);
}

/*__declspec(dllexport) void __stdcall RVExtension(
	char		*Buffer,
	int			Length,
	const char *Function
)
{
	if (strncmp(Function, "QueryPerf:", sizeof("QueryPerf:") - 1) == 0) {
		for (DWORD i = 0; i < ARRAYSIZE(CmaPerfA); i++) {
			if (strcmp(Function + sizeof("QueryPerf:") - 1, (LPCSTR)CmaPerfA[i][1]) == 0) {
				snprintf(Buffer, Length, "%llu", CmaPerfA[i][0]);
				return;
			}
		}
	}
	else if (strcmp(Function, "CmaDumpStats") == 0) {
		CmaDumpStats(L"API");
		return;
	}
	else if (strcmp(Function, "test") == 0) {
		strcpy(Buffer, "test");
		return;
	}
}	*/

