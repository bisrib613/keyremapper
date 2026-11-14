#include "startup.h"
#include <windows.h>
#include <vector>

static const wchar_t* STARTUP_RUN_KEY=L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";

static const wchar_t* StartupValueName() {
#ifdef KRM_TEST_BUILD
  return L"KeyRemapper.Test";
#else
  return L"KeyRemapper";
#endif
}

std::wstring StartupCommandValue() {
  HKEY key=nullptr;
  if(RegOpenKeyExW(HKEY_CURRENT_USER,STARTUP_RUN_KEY,0,KEY_QUERY_VALUE,&key)!=ERROR_SUCCESS) return L"";
  DWORD type=0,size=0;
  if(RegQueryValueExW(key,StartupValueName(),nullptr,&type,nullptr,&size)!=ERROR_SUCCESS || type!=REG_SZ || size<sizeof(wchar_t)) {
    RegCloseKey(key); return L"";
  }
  std::vector<wchar_t> buf(size/sizeof(wchar_t)+1,0);
  if(RegQueryValueExW(key,StartupValueName(),nullptr,&type,reinterpret_cast<BYTE*>(buf.data()),&size)!=ERROR_SUCCESS) {
    RegCloseKey(key); return L"";
  }
  RegCloseKey(key);
  return std::wstring(buf.data());
}

static std::wstring ExpectedStartupCommand() {
  wchar_t exe[MAX_PATH]{};
  if(!GetModuleFileNameW(nullptr,exe,MAX_PATH)) return L"";
  return L"\""+std::wstring(exe)+L"\" --startup";
}

bool StartupEnabled() {
  std::wstring actual=StartupCommandValue();
  std::wstring expected=ExpectedStartupCommand();
  return !actual.empty() && !expected.empty() && _wcsicmp(actual.c_str(),expected.c_str())==0;
}

bool SetStartupEnabled(bool enabled) {
  HKEY key=nullptr;
  if(RegCreateKeyExW(HKEY_CURRENT_USER,STARTUP_RUN_KEY,0,nullptr,0,KEY_SET_VALUE,nullptr,&key,nullptr)!=ERROR_SUCCESS) return false;
  LONG result=ERROR_SUCCESS;
  if(enabled) {
    std::wstring command=ExpectedStartupCommand();
    if(command.empty()) { RegCloseKey(key); return false; }
    result=RegSetValueExW(key,StartupValueName(),0,REG_SZ,
      reinterpret_cast<const BYTE*>(command.c_str()),(DWORD)((command.size()+1)*sizeof(wchar_t)));
  } else {
    result=RegDeleteValueW(key,StartupValueName());
    if(result==ERROR_FILE_NOT_FOUND) result=ERROR_SUCCESS;
  }
  RegCloseKey(key);
  return result==ERROR_SUCCESS;
}

#ifdef KRM_TEST_BUILD
void CleanupTestStartup() {
  SetStartupEnabled(false);
}
#endif
