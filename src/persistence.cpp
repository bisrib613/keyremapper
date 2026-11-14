#define NOMINMAX
#include "persistence.h"
#include <windows.h>
#include <shlobj.h>
#include <fstream>
#include <sstream>
#include <utility>

#pragma comment(lib, "shell32.lib")

static bool ValidMapping(const Mapping& m) {
  if(m.source<=0 || m.source>=KRM_CODE_LIMIT) return false;
  if(!m.block && (m.target<=0 || m.target>=KRM_CODE_LIMIT)) return false;
  if((m.srcMods|m.dstMods)&~(KRM_SHIFT|KRM_CTRL|KRM_ALT|KRM_WIN)) return false;
  return true;
}

std::wstring ConfigDir() {
#ifdef KRM_TEST_BUILD
  wchar_t p[MAX_PATH]{};
  GetTempPathW(MAX_PATH,p);
  wchar_t suffix[64]{};
  wsprintfW(suffix,L"KeyRemapperTests-%lu",(unsigned long)GetCurrentProcessId());
  std::wstring d=std::wstring(p)+suffix;
#else
  wchar_t p[MAX_PATH]{};
  if(FAILED(SHGetFolderPathW(nullptr,CSIDL_APPDATA,nullptr,SHGFP_TYPE_CURRENT,p))) return L"";
  std::wstring d=std::wstring(p)+L"\\KeyRemapperClean";
#endif
  if(!d.empty() && !CreateDirectoryW(d.c_str(),nullptr) && GetLastError()!=ERROR_ALREADY_EXISTS) return L"";
  return d;
}
std::wstring ConfigPath() { auto d=ConfigDir(); return d.empty()?L"":d+L"\\mappings.txt"; }
std::wstring ConfigBackupPath() { auto d=ConfigDir(); return d.empty()?L"":d+L"\\mappings.bak"; }
std::wstring ConfigTempPath() { auto d=ConfigDir(); return d.empty()?L"":d+L"\\mappings.tmp"; }

ConfigReadResult ReadMappingsFile(const std::wstring& path,std::vector<Mapping>& out) {
  out.clear();
  if(path.empty()) return ConfigReadResult::Missing;
  std::wifstream f(path);
  if(!f) return ConfigReadResult::Missing;

  std::wstring first;
  if(!std::getline(f,first)) return ConfigReadResult::Valid;

  if(first.rfind(L"KRM_CONFIG ",0)==0) {
    std::wistringstream header(first);
    std::wstring magic; int version=0;
    if(!(header>>magic>>version) || magic!=L"KRM_CONFIG" || version!=2) return ConfigReadResult::Invalid;

    std::wstring countLine;
    if(!std::getline(f,countLine)) return ConfigReadResult::Invalid;
    std::wistringstream countIn(countLine);
    std::wstring key; size_t expected=0;
    if(!(countIn>>key>>expected) || key!=L"COUNT" || expected>4096) return ConfigReadResult::Invalid;

    std::vector<Mapping> parsed;
    parsed.reserve(expected);
    std::wstring line;
    while(std::getline(f,line)) {
      if(line.empty()) continue;
      std::wistringstream in(line);
      int src=0,dst=0,block=0; unsigned sm=0,dm=0;
      if(!(in>>src>>dst>>block>>sm>>dm)) return ConfigReadResult::Invalid;
      if(block!=0 && block!=1) return ConfigReadResult::Invalid;
      Mapping m{src,dst,block!=0,sm,dm};
      if(!ValidMapping(m)) return ConfigReadResult::Invalid;
      parsed.push_back(m);
    }
    if(parsed.size()!=expected) return ConfigReadResult::Invalid;
    out=std::move(parsed);
    return ConfigReadResult::Valid;
  }

  std::vector<Mapping> parsed;
  auto parseLegacy=[&](const std::wstring& line)->bool {
    if(line.empty()) return true;
    std::wistringstream in(line);
    int src=0,dst=0,block=0; unsigned sm=0,dm=0;
    if(!(in>>src>>dst>>block)) return false;
    if(!(in>>sm>>dm)) { sm=0; dm=0; }
    if(block!=0 && block!=1) return false;
    Mapping m{src,dst,block!=0,sm,dm};
    if(!ValidMapping(m)) return false;
    parsed.push_back(m);
    return true;
  };
  if(!parseLegacy(first)) return ConfigReadResult::Invalid;
  std::wstring line;
  while(std::getline(f,line)) if(!parseLegacy(line)) return ConfigReadResult::Invalid;
  out=std::move(parsed);
  return ConfigReadResult::Valid;
}

bool WriteMappingsFile(const std::wstring& path,const std::vector<Mapping>& maps) {
  if(path.empty()) return false;
  std::wofstream f(path,std::ios::trunc);
  if(!f) return false;
  f<<L"KRM_CONFIG 2\n";
  f<<L"COUNT "<<maps.size()<<L"\n";
  for(const auto&m:maps)
    f<<m.source<<L' '<<m.target<<L' '<<(m.block?1:0)<<L' '<<m.srcMods<<L' '<<m.dstMods<<L'\n';
  f.flush();
  return (bool)f;
}

bool SaveMappings(const std::vector<Mapping>& maps) {
  const std::wstring path=ConfigPath();
  const std::wstring temp=ConfigTempPath();
  const std::wstring backup=ConfigBackupPath();
  if(path.empty()||temp.empty()||backup.empty()) return false;

  if(!WriteMappingsFile(temp,maps)) return false;

  std::vector<Mapping> existing;
  if(ReadMappingsFile(path,existing)==ConfigReadResult::Valid) {
    if(!CopyFileW(path.c_str(),backup.c_str(),FALSE)) {
      DeleteFileW(temp.c_str());
      return false;
    }
  }

  if(!MoveFileExW(temp.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)) {
    DeleteFileW(temp.c_str());
    return false;
  }

  std::vector<Mapping> verify;
  if(ReadMappingsFile(path,verify)!=ConfigReadResult::Valid) return false;

  if(GetFileAttributesW(backup.c_str())==INVALID_FILE_ATTRIBUTES)
    if(!CopyFileW(path.c_str(),backup.c_str(),FALSE)) return false;

  return true;
}

bool LoadMappings(std::vector<Mapping>& maps,ConfigLoadStatus* status) {
  auto setStatus=[&](ConfigLoadStatus value){ if(status) *status=value; };
  maps.clear();

  const std::wstring primary=ConfigPath();
  const std::wstring backup=ConfigBackupPath();
  if(primary.empty()||backup.empty()) {
    setStatus(ConfigLoadStatus::Invalid);
    return false;
  }

  std::vector<Mapping> parsed;
  ConfigReadResult primaryResult=ReadMappingsFile(primary,parsed);
  if(primaryResult==ConfigReadResult::Valid) {
    maps=std::move(parsed);

    bool versioned=false;
    {
      // Keep the probe handle scoped here. SaveMappings() must be able to
      // replace mappings.txt immediately afterwards on Windows.
      std::wifstream check(primary);
      std::wstring first;
      if(std::getline(check,first))
        versioned=first.rfind(L"KRM_CONFIG ",0)==0;
    }

    if(!versioned) {
      if(!SaveMappings(maps)) {
        setStatus(ConfigLoadStatus::MigrationWriteFailed);
        return true; // mappings are usable in memory even if migration could not be persisted
      }
      setStatus(ConfigLoadStatus::Migrated);
      return true;
    }

    setStatus(ConfigLoadStatus::Ready);
    return true;
  }

  std::vector<Mapping> recovered;
  if(ReadMappingsFile(backup,recovered)==ConfigReadResult::Valid) {
    maps=std::move(recovered);
    if(!WriteMappingsFile(primary,maps)) {
      setStatus(ConfigLoadStatus::RecoveryWriteFailed);
      return true; // recovered mappings remain usable for this process
    }
    setStatus(ConfigLoadStatus::Recovered);
    return true;
  }

  if(primaryResult==ConfigReadResult::Missing) {
    setStatus(ConfigLoadStatus::Missing);
    return true;
  }

  setStatus(ConfigLoadStatus::Invalid);
  return false;
}

#ifdef KRM_TEST_BUILD
void CleanupTestConfig() {
  DeleteFileW(ConfigTempPath().c_str());
  DeleteFileW(ConfigPath().c_str());
  DeleteFileW(ConfigBackupPath().c_str());
  std::wstring dir=ConfigDir();
  if(!dir.empty()) RemoveDirectoryW(dir.c_str());
}
#endif
