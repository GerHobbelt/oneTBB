#include "cma.h"
#include "cma_api.h"
#include <Windows.h>
#include "Shlobj.h"

bool CmaAssignLockMemoryPrivileges(void)
{
	TOKEN_PRIVILEGES	tp = { NULL };
	LUID				luid;
	HANDLE				token;

	if (!LookupPrivilegeValueW(nullptr, L"SeLockMemoryPrivilege", &luid))
		return false;

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	tp.Privileges[0].Luid = luid;
		
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token))
		return false;

	if (!AdjustTokenPrivileges(
		token, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), nullptr, nullptr)) {
		CloseHandle(token);
		return false;
	}

	CloseHandle(token);

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
		return false;

	return true;
}
