#pragma once
#include <string>

std::wstring StartupCommandValue();
bool StartupEnabled();
bool SetStartupEnabled(bool enabled);

#ifdef KRM_TEST_BUILD
void CleanupTestStartup();
#endif
