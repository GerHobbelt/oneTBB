#pragma once

#include <atomic>

extern std::atomic<size_t> CmaMappedMemory;

bool CmaAcquireLockMemoryPrivileges(void);
