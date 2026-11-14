#pragma once
#include <string>
#include <vector>
#include "model.h"

enum class ConfigReadResult { Missing, Valid, Invalid };

std::wstring ConfigDir();
std::wstring ConfigPath();
std::wstring ConfigBackupPath();
std::wstring ConfigTempPath();

ConfigReadResult ReadMappingsFile(const std::wstring& path, std::vector<Mapping>& out);
bool WriteMappingsFile(const std::wstring& path, const std::vector<Mapping>& maps);
bool SaveMappings(const std::vector<Mapping>& maps);
bool LoadMappings(std::vector<Mapping>& maps);

#ifdef KRM_TEST_BUILD
void CleanupTestConfig();
#endif
