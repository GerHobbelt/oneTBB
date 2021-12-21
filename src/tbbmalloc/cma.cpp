#include "cma.h"
#include "cma_util.h"
#include "cma_config.h"
#include "cma_api.h"
#include <Windows.h>
#include "Shlobj.h"

uint32_t CmaSetPrivileges(
	void
)
{
	TOKEN_PRIVILEGES	tp;
	LUID				luid;
	HANDLE				token;

	if (!LookupPrivilegeValueW(nullptr, L"SeLockMemoryPrivilege", &luid))
		return __LINE__;

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	tp.Privileges[0].Luid = luid;
		
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token))
		return __LINE__;

	if (!AdjustTokenPrivileges(
		token, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), nullptr, nullptr)) {
		CloseHandle(token);
		return __LINE__;
	}

	CloseHandle(token);

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
		return __LINE__;

	return 0;
}

bool CmaInit(
	void
)
{
	WCHAR	file[4096];

	CmaLogWrite(true, 0, L"Init");

	GetCurrentDirectoryW(sizeof(file) / 2, file);
	wcscat(file, L"\\cma.ini");

	if (GetPrivateProfileIntW(L"Default", L"DebugBreak", 0, file)) {
		while (!IsDebuggerPresent())
			Sleep(0);
		DebugBreak();
	}
	
	CmaCfgUseLargePages = GetPrivateProfileIntW(L"Default", L"UseLargePages", 0, file) ? true : false;

	if (CmaCfgUseLargePages) {
		BOOL		isAdmin;
		uint32_t	status;

		isAdmin = IsUserAnAdmin();
		status = CmaSetPrivileges();

		if (!isAdmin || status)
			CmaCfgUseLargePages = false;

		CmaLogWrite(false, 0,
					L"Config:\r\n"
					L"User is admin: %s\r\n"
					L"tSeLockMemoryPrivilege: %s\r\n"
					L"%s",
					isAdmin ? L"yes" : L"no",
					status ? L"failed" : L"ok",
					CmaCfgUseLargePages ? L"Using large pages" : L"Using normal pages"
		);
	}
	else {
		CmaLogWrite(false, 0,
					L"Config:\r\n"
					L"%s",
					CmaCfgUseLargePages ? L"Using large pages" : L"Using normal pages"
		);
	}

	CmaLogWrite(false, 0, L"Init done\r\n");

	return CmaCfgUseLargePages;
}

void CmaExit(
	void
)
{
	CmaDumpStats(L"Exit");
	CmaLogWrite(false, 0, L"Exit");
	CmaLogClose();
}
