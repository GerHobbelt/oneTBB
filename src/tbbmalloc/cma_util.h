#pragma once
#include <cstdint>

wchar_t *CmaFormatToUnit(
	uint64_t	Value,
	wchar_t		*Buffer
);

wchar_t	*CmaFormatToUnit(
	uint64_t	Value
);

void CmaLogInit(
	void
);

void CmaLogWrite(
	bool			Timestamp,
	uint32_t		Tabs,
	const wchar_t	*String,
	...
);

void CmaLogClose(
	void
);