#include "cma_util.h"
#include <Windows.h>
#include <cstdint>
#include <Strsafe.h>

HANDLE	mFile = INVALID_HANDLE_VALUE;

wchar_t *CmaFormatToUnit(
	uint64_t	Value,
	wchar_t		*Buffer
)
{
	if (Value > 1024 * 1024 * 1024) {
		swprintf(Buffer, L"%.3f gb", (double)Value / 1024.0 / 1024.0 / 1024.0);
		return Buffer;
	}
	else if (Value > 1024 * 1024) {
		swprintf(Buffer, L"%.3f mb", (double)Value / 1024.0 / 1024.0);
		return Buffer;
	}
	else if (Value > 1024) {
		swprintf(Buffer, L"%.3f kb", (double)Value / 1024.0);
		return Buffer;
	}
	else {
		swprintf(Buffer, L"%.3f b", (double)Value);
		return Buffer;
	}
}

wchar_t	*CmaFormatToUnit(
	uint64_t	Value
)
{
	static WCHAR	buf[1024];

	return CmaFormatToUnit(Value, buf);
}

void CmaLogInit(
	void
)
{
	if (mFile != INVALID_HANDLE_VALUE)
		return;

	mFile = CreateFileW(L"cma.txt", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
}

void CmaLogWrite(
	bool			Timestamp,
	uint32_t		Tabs,
	const wchar_t	*String,
	...
)
{
	SYSTEMTIME	time;
	WCHAR		buf[4096];
	va_list		va;
	DWORD		w;

	CmaLogInit();

	for (uint32_t i = 0; i < Tabs; i++) 
		buf[i] = L'\t';
	buf[Tabs] = 0;
	

	if (Timestamp) {
		GetLocalTime(&time);
		StringCbPrintfW(buf + Tabs, sizeof(buf) - (Tabs * 2), L"%02u:%02u:%02u: ", (ULONG)time.wHour, (ULONG)time.wMinute, (ULONG)time.wSecond);
	}

	va_start(va, String);
	StringCbVPrintfW(buf + wcslen(buf), sizeof(buf) - (wcslen(buf) * 2), String, va);
	va_end(va);

	wcscat(buf, L"\r\n");
	WriteFile(mFile, buf, wcslen(buf) * 2, &w, nullptr);
}

void CmaLogClose(
	void
)
{
	if (mFile != INVALID_HANDLE_VALUE)
		CloseHandle(mFile);

	mFile = INVALID_HANDLE_VALUE;
}

#ifdef EXT0_LOG_STATS
void Ext0LogStatInc(
	bool	Exp,
	ULONG64	*True,
	ULONG64	*False
)
{
	if (Exp)
		InterlockedIncrement64((LONG64*)True);
	else
		InterlockedIncrement64((LONG64*)False);
}
#endif