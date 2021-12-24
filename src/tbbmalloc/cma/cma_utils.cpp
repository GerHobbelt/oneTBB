#include "cma_utils.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

bool lockMemoryPrivilegesHasRun = false;
bool lockMemoryPrivilegesActivated = false;

bool CmaAcquireLockMemoryPrivileges(void)
{
	if (lockMemoryPrivilegesHasRun)
		return lockMemoryPrivilegesActivated;

	LUID luid;
	if (!LookupPrivilegeValueW(nullptr, L"SeLockMemoryPrivilege", &luid))
	{
		lockMemoryPrivilegesHasRun = true;
		return false;
	}

	TOKEN_PRIVILEGES tp;
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	tp.Privileges[0].Luid = luid;
		
	HANDLE token;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token))
	{
		lockMemoryPrivilegesHasRun = true;
		return false;
	}

	if (!AdjustTokenPrivileges(token, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), nullptr, nullptr))
	{
		CloseHandle(token);
		lockMemoryPrivilegesHasRun = true;
		return false;
	}

	CloseHandle(token);

	if (ERROR_NOT_ALL_ASSIGNED == GetLastError())
	{
		lockMemoryPrivilegesHasRun = true;
		return false;
	}

	lockMemoryPrivilegesHasRun = true;
	lockMemoryPrivilegesActivated = true;

	return true;
}
