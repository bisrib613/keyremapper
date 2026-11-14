#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shellapi.h>
#include <shlobj.h>
#include <dwmapi.h>
#include <uxtheme.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "uxtheme.lib")

constexpr ULONG_PTR KR_MAGIC = 0x4B524D50;
constexpr UINT WM_KR_CAPTURE = WM_APP + 1;
constexpr UINT WM_KR_TRAY = WM_APP + 2;
constexpr UINT WM_KR_REFRESH = WM_APP + 3;
constexpr UINT TRAY_ID = 1;
constexpr UINT ID_TRAY_OPEN = 5001;
constexpr UINT ID_TRAY_ENABLE = 5002;
constexpr UINT ID_TRAY_PREFS = 5003;
constexpr UINT ID_TRAY_STARTUP = 5004;
constexpr UINT ID_TRAY_RESET = 5005;
constexpr UINT ID_TRAY_EXIT = 5006;
constexpr int MOUSE_LEFT = 0x1001;
constexpr int MOUSE_RIGHT = 0x1002;
constexpr int MOUSE_MIDDLE = 0x1003;
constexpr int MOUSE_X1 = 0x1004;
constexpr int MOUSE_X2 = 0x1005;
constexpr int WHEEL_UP = 0x1101;
constexpr int WHEEL_DOWN = 0x1102;

enum { ID_SOURCE=100, ID_TARGET, ID_CAP_SRC, ID_CAP_DST, ID_BLOCK, ID_ADD, ID_MODIFY, ID_REMOVE, ID_LIST, ID_PAUSE,
 ID_STARTUP, ID_STATUS, ID_TITLE, ID_SUBTITLE, ID_SECTION, ID_EMPTY, ID_LIST_SOURCE, ID_LIST_TARGET,
 ID_SRC_SHIFT, ID_SRC_CTRL, ID_SRC_ALT, ID_SRC_WIN, ID_DST_SHIFT, ID_DST_CTRL, ID_DST_ALT, ID_DST_WIN };
enum ModMask : unsigned { KRM_SHIFT=1, KRM_CTRL=2, KRM_ALT=4, KRM_WIN=8 };

struct Item { int code; const wchar_t* name; };
static const Item ITEMS[] = {
 {VK_BACK,L"Backspace"},{VK_TAB,L"Tab"},{VK_RETURN,L"Enter"},{VK_PAUSE,L"Pause/Break"},
 {VK_CAPITAL,L"Caps Lock"},{VK_ESCAPE,L"Escape"},{VK_SPACE,L"Space"},{VK_PRIOR,L"Page Up"},
 {VK_NEXT,L"Page Down"},{VK_END,L"End"},{VK_HOME,L"Home"},{VK_LEFT,L"Left Arrow"},
 {VK_UP,L"Up Arrow"},{VK_RIGHT,L"Right Arrow"},{VK_DOWN,L"Down Arrow"},{VK_SNAPSHOT,L"Print Screen"},
 {VK_INSERT,L"Insert"},{VK_DELETE,L"Delete"},{L'0',L"0"},{L'1',L"1"},{L'2',L"2"},
 {L'3',L"3"},{L'4',L"4"},{L'5',L"5"},{L'6',L"6"},{L'7',L"7"},{L'8',L"8"},
 {L'9',L"9"},{L'A',L"A"},{L'B',L"B"},{L'C',L"C"},{L'D',L"D"},{L'E',L"E"},
 {L'F',L"F"},{L'G',L"G"},{L'H',L"H"},{L'I',L"I"},{L'J',L"J"},{L'K',L"K"},
 {L'L',L"L"},{L'M',L"M"},{L'N',L"N"},{L'O',L"O"},{L'P',L"P"},{L'Q',L"Q"},
 {L'R',L"R"},{L'S',L"S"},{L'T',L"T"},{L'U',L"U"},{L'V',L"V"},{L'W',L"W"},
 {L'X',L"X"},{L'Y',L"Y"},{L'Z',L"Z"},
 {VK_LWIN,L"Left Win"},{VK_RWIN,L"Right Win"},{VK_APPS,L"Context Menu"},
 {VK_NUMPAD0,L"Numpad 0"},{VK_NUMPAD1,L"Numpad 1"},{VK_NUMPAD2,L"Numpad 2"},
 {VK_NUMPAD3,L"Numpad 3"},{VK_NUMPAD4,L"Numpad 4"},{VK_NUMPAD5,L"Numpad 5"},
 {VK_NUMPAD6,L"Numpad 6"},{VK_NUMPAD7,L"Numpad 7"},{VK_NUMPAD8,L"Numpad 8"},
 {VK_NUMPAD9,L"Numpad 9"},{VK_MULTIPLY,L"Numpad *"},{VK_ADD,L"Numpad +"},
 {VK_SUBTRACT,L"Numpad -"},{VK_DECIMAL,L"Numpad ."},{VK_DIVIDE,L"Numpad /"},
 {VK_F1,L"F1"},{VK_F2,L"F2"},{VK_F3,L"F3"},{VK_F4,L"F4"},{VK_F5,L"F5"},
 {VK_F6,L"F6"},{VK_F7,L"F7"},{VK_F8,L"F8"},{VK_F9,L"F9"},{VK_F10,L"F10"},
 {VK_F11,L"F11"},{VK_F12,L"F12"},{VK_F13,L"F13"},{VK_F14,L"F14"},{VK_F15,L"F15"},
 {VK_F16,L"F16"},{VK_F17,L"F17"},{VK_F18,L"F18"},{VK_F19,L"F19"},{VK_F20,L"F20"},
 {VK_F21,L"F21"},{VK_F22,L"F22"},{VK_F23,L"F23"},{VK_F24,L"F24"},
 {VK_NUMLOCK,L"Num Lock"},{VK_SCROLL,L"Scroll Lock"},
 {VK_LSHIFT,L"Left Shift"},{VK_RSHIFT,L"Right Shift"},{VK_LCONTROL,L"Left Ctrl"},
 {VK_RCONTROL,L"Right Ctrl"},{VK_LMENU,L"Left Alt"},{VK_RMENU,L"Right Alt"},
 {VK_OEM_1,L";"},{VK_OEM_PLUS,L"="},{VK_OEM_COMMA,L","},
 {VK_OEM_MINUS,L"-"},{VK_OEM_PERIOD,L"."},{VK_OEM_2,L"/"},
 {VK_OEM_3,L"`"},{VK_OEM_4,L"["},{VK_OEM_5,L"\\"},
 {VK_OEM_6,L"]"},{VK_OEM_7,L"' / \""},{VK_OEM_102,L"OEM 102"},
 {MOUSE_LEFT,L"Mouse Left"},{MOUSE_RIGHT,L"Mouse Right"},{MOUSE_MIDDLE,L"Mouse Middle"},
 {MOUSE_X1,L"Mouse X1"},{MOUSE_X2,L"Mouse X2"},{WHEEL_UP,L"Wheel Up"},{WHEEL_DOWN,L"Wheel Down"}
};

static const wchar_t SYMBOL_CHARS[] = L"!@#$%^&*()_+{}|:\"<>?~";
struct Mapping { int source; int target; bool block; unsigned srcMods=0; unsigned dstMods=0; };
struct Choice { int code=0; unsigned mods=0; };
static std::vector<Choice> gSourceChoices;
static std::vector<Choice> gTargetChoices;
static HINSTANCE gInst;
static HWND gWnd, gSource, gTarget, gBlock, gList, gPause, gStatus, gStartup;
static HWND gAdd, gModify, gRemove, gTitle, gSubtitle, gSection, gEmpty, gListSource, gListTarget, gIconControl;
static HWND gSourceLabel,gTargetLabel,gSrcModsLabel,gDstModsLabel,gMappingsLabel,gCaptureSrc,gCaptureDst;
static HWND gSrcShift,gSrcCtrl,gSrcAlt,gSrcWin,gDstShift,gDstCtrl,gDstAlt,gDstWin;
static int gEditingIndex=-1;

static constexpr COLORREF C_BG=RGB(247,247,250);
static constexpr COLORREF C_SURFACE=RGB(255,255,255);
static constexpr COLORREF C_CONTROL=RGB(246,247,250);
static constexpr COLORREF C_HOVER=RGB(236,239,244);
static constexpr COLORREF C_TEXT=RGB(24,27,32);
static constexpr COLORREF C_MUTED=RGB(96,103,115);
static constexpr COLORREF C_BORDER=RGB(221,225,233);
static constexpr COLORREF C_ACCENT=RGB(79,198,228);
static constexpr COLORREF C_ACCENT_HOVER=RGB(57,184,217);
static constexpr COLORREF C_LAVENDER=RGB(239,234,255);
static constexpr COLORREF C_LAVENDER_BORDER=RGB(216,207,248);
static constexpr COLORREF C_ACTIVE=RGB(21,164,105);
static constexpr COLORREF C_ACTIVE_BG=RGB(231,249,241);
static constexpr COLORREF C_SELECTED=RGB(232,247,252);
static HBRUSH gBrushBg=nullptr, gBrushSurface=nullptr, gBrushControl=nullptr;
static HFONT gFontBody=nullptr, gFontSmall=nullptr, gFontTitle=nullptr, gFontLabel=nullptr;
static HICON gIconLarge=nullptr, gIconSmall=nullptr;
static HANDLE gInstanceMutex=nullptr;
static NOTIFYICONDATAW gTray{};
static bool gTrayAdded=false;
static bool gExitRequested=false;
static bool gStartupMode=false;
static bool gRunAtStartup=false;
static bool gBlockSource=false;
static UINT gTaskbarCreatedMessage=0;
static UINT gDpi=96;
static unsigned gSourceUiMods=0;
static unsigned gTargetUiMods=0;

struct ComboWheelState { HWND combo=nullptr; int delta=0; };
static ComboWheelState gSourceWheel, gTargetWheel;
static HHOOK gKeyboard, gMouse;
static bool gPaused=false, gCaptureSource=false, gCaptureTarget=false;
static int gCaptureSwallow=0;
static int gCaptureModifierCandidate=0;
static unsigned gCaptureSuppressedMods=0;
static unsigned gPhysicalMods=0;
#ifdef KRM_TEST_BUILD
static bool gSelfTestMode=true;
static int gSelfCapturedCode=0;
static unsigned gSelfCapturedMods=0;
struct TestEvent { int vk; bool down; };
static std::vector<TestEvent> gSelfOutputEvents;
static bool gSelfObserveOutput=false;
constexpr ULONG_PTR TEST_MAGIC = 0x4B525453;
#endif
static std::vector<Mapping> gMaps;
struct ActiveEmission { int target=0; unsigned synthetic=0; unsigned suppressed=0; bool consumed=false; };
static std::vector<ActiveEmission> gActive(0x1200);
static DWORD gLastInjectionError=ERROR_SUCCESS;
static bool gConfigLoadFailed=false;
static bool gConfigSaveFailed=false;
static int S(int value) { return MulDiv(value,(int)gDpi,96); }

static std::wstring NameOf(int code) {
 for (auto &i:ITEMS) if(i.code==code) return i.name;
 wchar_t b[32]; wsprintfW(b,L"0x%X",code); return b;
}
static unsigned ModBitForVk(int vk) {
 switch(vk) {
  case VK_SHIFT: case VK_LSHIFT: case VK_RSHIFT: return KRM_SHIFT;
  case VK_CONTROL: case VK_LCONTROL: case VK_RCONTROL: return KRM_CTRL;
  case VK_MENU: case VK_LMENU: case VK_RMENU: return KRM_ALT;
  case VK_LWIN: case VK_RWIN: return KRM_WIN;
  default: return 0;
 }
}
static int VkForMod(unsigned bit) {
 if(bit==KRM_SHIFT) return VK_SHIFT;
 if(bit==KRM_CTRL) return VK_CONTROL;
 if(bit==KRM_ALT) return VK_MENU;
 if(bit==KRM_WIN) return VK_LWIN;
 return 0;
}
static bool ChoiceForSymbol(wchar_t ch,Choice& out) {
 SHORT encoded=VkKeyScanExW(ch,GetKeyboardLayout(0));
 if(encoded==-1) return false;
 int vk=LOBYTE(encoded);
 BYTE shiftState=HIBYTE(encoded);
 unsigned mods=0;
 if(shiftState&1) mods|=KRM_SHIFT;
 if(shiftState&2) mods|=KRM_CTRL;
 if(shiftState&4) mods|=KRM_ALT;
 if(shiftState&~7) return false;
 out={vk,mods};
 return vk!=0;
}
static wchar_t SymbolCharForKey(int code,unsigned mods) {
 for(const wchar_t* p=SYMBOL_CHARS;*p;++p) {
  Choice c{};
  if(ChoiceForSymbol(*p,c) && c.code==code && c.mods==mods) return *p;
 }
 return 0;
}
static std::wstring Describe(int code,unsigned mods) {
 if(wchar_t symbol=SymbolCharForKey(code,mods)) return std::wstring(1,symbol);
 std::wstring out;
 if(mods&KRM_CTRL) out+=L"Ctrl+";
 if(mods&KRM_ALT) out+=L"Alt+";
 if(mods&KRM_WIN) out+=L"Win+";
 if(mods&KRM_SHIFT) out+=L"Shift+";
 out+=NameOf(code);
 return out;
}
static bool Extended(int vk) {
 switch(vk) {
  case VK_RMENU:case VK_RCONTROL:case VK_INSERT:case VK_DELETE:case VK_HOME:case VK_END:
  case VK_PRIOR:case VK_NEXT:case VK_LEFT:case VK_RIGHT:case VK_UP:case VK_DOWN:
  case VK_NUMLOCK:case VK_SNAPSHOT:case VK_DIVIDE:case VK_LWIN:case VK_RWIN:case VK_APPS:return true;
 } return false;
}
static bool SendCode(int code,bool down) {
 INPUT in{}; in.type = code>=0x1000 ? INPUT_MOUSE : INPUT_KEYBOARD;
 if(code<0x1000) {
  in.ki.wVk=(WORD)code; in.ki.dwExtraInfo=KR_MAGIC;
  if(!down) in.ki.dwFlags|=KEYEVENTF_KEYUP;
  if(Extended(code)) in.ki.dwFlags|=KEYEVENTF_EXTENDEDKEY;
 } else {
  in.mi.dwExtraInfo=KR_MAGIC;
  switch(code) {
   case MOUSE_LEFT:in.mi.dwFlags=down?MOUSEEVENTF_LEFTDOWN:MOUSEEVENTF_LEFTUP;break;
   case MOUSE_RIGHT:in.mi.dwFlags=down?MOUSEEVENTF_RIGHTDOWN:MOUSEEVENTF_RIGHTUP;break;
   case MOUSE_MIDDLE:in.mi.dwFlags=down?MOUSEEVENTF_MIDDLEDOWN:MOUSEEVENTF_MIDDLEUP;break;
   case MOUSE_X1:in.mi.dwFlags=down?MOUSEEVENTF_XDOWN:MOUSEEVENTF_XUP;in.mi.mouseData=XBUTTON1;break;
   case MOUSE_X2:in.mi.dwFlags=down?MOUSEEVENTF_XDOWN:MOUSEEVENTF_XUP;in.mi.mouseData=XBUTTON2;break;
   case WHEEL_UP:if(!down)return true;in.mi.dwFlags=MOUSEEVENTF_WHEEL;in.mi.mouseData=WHEEL_DELTA;break;
   case WHEEL_DOWN:if(!down)return true;in.mi.dwFlags=MOUSEEVENTF_WHEEL;in.mi.mouseData=(DWORD)-WHEEL_DELTA;break;
   default:return false;
  }
 }
 if(SendInput(1,&in,sizeof(in))!=1) {
  gLastInjectionError=GetLastError();
  if(gWnd) PostMessageW(gWnd,WM_KR_REFRESH,0,0);
  return false;
 }
 return true;
}
#ifdef KRM_TEST_BUILD
static void SendTestKey(int vk,bool down) {
 INPUT in{};
 in.type=INPUT_KEYBOARD;
 in.ki.wVk=(WORD)vk;
 in.ki.dwExtraInfo=TEST_MAGIC;
 if(!down) in.ki.dwFlags|=KEYEVENTF_KEYUP;
 SendInput(1,&in,sizeof(in));
}
#endif
static ActiveEmission BeginTarget(const Mapping& m) {
 gLastInjectionError=ERROR_SUCCESS;
 ActiveEmission a{};
 a.consumed=true;
 a.target=m.target;
 unsigned all[]={KRM_SHIFT,KRM_CTRL,KRM_ALT,KRM_WIN};
 for(unsigned bit:all) {
  if((gPhysicalMods&bit) && !(m.dstMods&bit)) {
   if(SendCode(VkForMod(bit),false)) a.suppressed|=bit;
  }
 }
 for(unsigned bit:all) {
  if((m.dstMods&bit) && !(gPhysicalMods&bit)) {
   if(SendCode(VkForMod(bit),true)) a.synthetic|=bit;
  }
 }
 if(!SendCode(m.target,true)) {
  for(unsigned bit:{KRM_WIN,KRM_ALT,KRM_CTRL,KRM_SHIFT})
   if(a.synthetic&bit) SendCode(VkForMod(bit),false);
  for(unsigned bit:{KRM_WIN,KRM_ALT,KRM_CTRL,KRM_SHIFT})
   if((a.suppressed&bit)&&(gPhysicalMods&bit)) SendCode(VkForMod(bit),true);
  a.target=0;
  a.synthetic=0;
  a.suppressed=0;
 }
 return a;
}
static void EndTarget(const ActiveEmission& a) {
 if(a.target) SendCode(a.target,false);
 unsigned all[]={KRM_WIN,KRM_ALT,KRM_CTRL,KRM_SHIFT};
 for(unsigned bit:all) if(a.synthetic&bit) SendCode(VkForMod(bit),false);
 for(unsigned bit:all) if((a.suppressed&bit)&&(gPhysicalMods&bit)) SendCode(VkForMod(bit),true);
}
static Mapping* FindMap(int src,unsigned sourceMods) {
 if(gPaused) return nullptr;
 for(auto &m:gMaps) if(m.source==src && m.srcMods==sourceMods) return &m;
 return nullptr;
}
static bool Handle(int src,bool down,bool pulse=false,unsigned sourceMods=0) {
 unsigned modBit=ModBitForVk(src);
 if(gCaptureSource||gCaptureTarget) {
  if(modBit) {
   if(down) {
    gCaptureModifierCandidate=src;
    gCaptureSuppressedMods|=modBit;
    return true;
   }
   if(gCaptureModifierCandidate==src) {
    bool sourceCapture=gCaptureSource;
    gCaptureSource=gCaptureTarget=false;
    gCaptureModifierCandidate=0;
    gCaptureSuppressedMods&=~modBit;
    unsigned mods=sourceMods&~modBit;
    LPARAM packed=(LPARAM)((mods<<1)|(sourceCapture?0:1));
    PostMessageW(gWnd,WM_KR_CAPTURE,src,packed);
    return true;
   }
   return true;
  }
  if(down||pulse) {
   bool sourceCapture=gCaptureSource;
   gCaptureSource=gCaptureTarget=false;
   gCaptureModifierCandidate=0;
   gCaptureSwallow=pulse?0:src;
   LPARAM packed=(LPARAM)((sourceMods<<1)|(sourceCapture?0:1));
   PostMessageW(gWnd,WM_KR_CAPTURE,src,packed);
   return true;
  }
  return false;
 }
 if(modBit && (gCaptureSuppressedMods&modBit)) {
  if(!down) gCaptureSuppressedMods&=~modBit;
  return true;
 }
 if(gCaptureSwallow==src) { if(!down)gCaptureSwallow=0; return true; }

 if(!pulse && src>=0 && src<(int)gActive.size() && !down) {
  if(gActive[src].consumed) {
   EndTarget(gActive[src]);
   gActive[src]=ActiveEmission{};
   return true;
  }
  return false;
 }

 Mapping* m=FindMap(src,sourceMods);
 if(!m) return false;
 if(m->block) {
  if(!pulse && src>=0 && src<(int)gActive.size()) gActive[src].consumed=true;
  return true;
 }
 if(pulse) {
  ActiveEmission a=BeginTarget(*m);
  EndTarget(a);
  return true;
 }
 if(src>=0 && src<(int)gActive.size()) {
  if(down) {
   if(!gActive[src].consumed) gActive[src]=BeginTarget(*m);
   else if(gActive[src].target<0x1000 && gActive[src].target) SendCode(gActive[src].target,true);
  }
 } else if(down) {
  ActiveEmission a=BeginTarget(*m);
  EndTarget(a);
 }
 return true;
}
static LRESULT CALLBACK KeyHook(int n,WPARAM w,LPARAM l) {
 if(n<0)return CallNextHookEx(gKeyboard,n,w,l);
 auto*k=(KBDLLHOOKSTRUCT*)l;
 if(k->dwExtraInfo==KR_MAGIC) {
#ifdef KRM_TEST_BUILD
  if(gSelfObserveOutput) {
   bool od=w==WM_KEYDOWN||w==WM_SYSKEYDOWN;
   bool ou=w==WM_KEYUP||w==WM_SYSKEYUP;
   if(od||ou) gSelfOutputEvents.push_back({(int)k->vkCode,od});
  }
#endif
  return CallNextHookEx(gKeyboard,n,w,l);
 }
#ifndef KRM_TEST_BUILD
 if(k->flags&LLKHF_INJECTED) return CallNextHookEx(gKeyboard,n,w,l);
#endif
 bool d=w==WM_KEYDOWN||w==WM_SYSKEYDOWN,u=w==WM_KEYUP||w==WM_SYSKEYUP;
 if(!d&&!u) return CallNextHookEx(gKeyboard,n,w,l);

 int vk=(int)k->vkCode;
 unsigned bit=ModBitForVk(vk);
 unsigned modsForMatch=gPhysicalMods;
 bool consumed=Handle(vk,d,false,modsForMatch);

 if(bit) {
  if(d) gPhysicalMods|=bit;
  else gPhysicalMods&=~bit;
 }
 if(consumed)return 1;
 return CallNextHookEx(gKeyboard,n,w,l);
}
static LRESULT CALLBACK MouseHook(int n,WPARAM w,LPARAM l) {
 if(n<0)return CallNextHookEx(gMouse,n,w,l);
 auto*m=(MSLLHOOKSTRUCT*)l;
 if((m->flags&LLMHF_INJECTED)||m->dwExtraInfo==KR_MAGIC)return CallNextHookEx(gMouse,n,w,l);
 int c=0;bool d=false,p=false;
 switch(w) {
  case WM_LBUTTONDOWN:c=MOUSE_LEFT;d=true;break;case WM_LBUTTONUP:c=MOUSE_LEFT;break;
  case WM_RBUTTONDOWN:c=MOUSE_RIGHT;d=true;break;case WM_RBUTTONUP:c=MOUSE_RIGHT;break;
  case WM_MBUTTONDOWN:c=MOUSE_MIDDLE;d=true;break;case WM_MBUTTONUP:c=MOUSE_MIDDLE;break;
  case WM_XBUTTONDOWN:c=HIWORD(m->mouseData)==XBUTTON1?MOUSE_X1:MOUSE_X2;d=true;break;
  case WM_XBUTTONUP:c=HIWORD(m->mouseData)==XBUTTON1?MOUSE_X1:MOUSE_X2;break;
  case WM_MOUSEWHEEL:c=(short)HIWORD(m->mouseData)>0?WHEEL_UP:WHEEL_DOWN;d=true;p=true;break;
 }
 if(c&&Handle(c,d,p,gPhysicalMods))return 1;
 return CallNextHookEx(gMouse,n,w,l);
}
static bool InstallHooks() {
 if(gKeyboard||gMouse) return gKeyboard&&gMouse;
 HHOOK keyboard=SetWindowsHookExW(WH_KEYBOARD_LL,KeyHook,gInst,0);
 if(!keyboard) return false;
 HHOOK mouse=SetWindowsHookExW(WH_MOUSE_LL,MouseHook,gInst,0);
 if(!mouse) {
  UnhookWindowsHookEx(keyboard);
  return false;
 }
 gKeyboard=keyboard;
 gMouse=mouse;
 return true;
}
static void ReleaseActiveOutputs() {
 for(size_t i=0;i<gActive.size();++i) {
  if(gActive[i].target) EndTarget(gActive[i]);
  gActive[i]=ActiveEmission{};
 }
}
static void SetRemappingEnabled(bool enabled) {
 if(!enabled) ReleaseActiveOutputs();
 else gLastInjectionError=ERROR_SUCCESS;
 gPaused=!enabled;
}
static void Unhook() {
 ReleaseActiveOutputs();
 if(gKeyboard)UnhookWindowsHookEx(gKeyboard);
 if(gMouse)UnhookWindowsHookEx(gMouse);
 gKeyboard=gMouse=nullptr;
}
enum class ConfigReadResult { Missing, Valid, Invalid };

static std::wstring ConfigDir() {
#ifdef KRM_TEST_BUILD
 wchar_t p[MAX_PATH]{};
 GetTempPathW(MAX_PATH,p);
 wchar_t suffix[64]{};
 wsprintfW(suffix,L"KeyRemapperTests-%lu",(unsigned long)GetCurrentProcessId());
 std::wstring d=std::wstring(p)+suffix;
#else
 wchar_t p[MAX_PATH]{};
 SHGetFolderPathW(nullptr,CSIDL_APPDATA,nullptr,SHGFP_TYPE_CURRENT,p);
 std::wstring d=std::wstring(p)+L"\\KeyRemapperClean";
#endif
 CreateDirectoryW(d.c_str(),nullptr);
 return d;
}
static std::wstring ConfigPath() { return ConfigDir()+L"\\mappings.txt"; }
static std::wstring ConfigBackupPath() { return ConfigDir()+L"\\mappings.bak"; }
static std::wstring ConfigTempPath() { return ConfigDir()+L"\\mappings.tmp"; }
static const wchar_t* STARTUP_RUN_KEY=L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static const wchar_t* StartupValueName() {
#ifdef KRM_TEST_BUILD
 return L"KeyRemapper.Test";
#else
 return L"KeyRemapper";
#endif
}

static std::wstring StartupCommandValue() {
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
static bool StartupEnabled() {
 std::wstring actual=StartupCommandValue();
 std::wstring expected=ExpectedStartupCommand();
 return !actual.empty() && !expected.empty() && _wcsicmp(actual.c_str(),expected.c_str())==0;
}
static bool SetStartupEnabled(bool enabled) {
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

static bool ValidMapping(const Mapping& m) {
 if(m.source<=0 || m.source>=(int)gActive.size()) return false;
 if(!m.block && (m.target<=0 || m.target>=(int)gActive.size())) return false;
 if((m.srcMods|m.dstMods)&~(KRM_SHIFT|KRM_CTRL|KRM_ALT|KRM_WIN)) return false;
 return true;
}

static ConfigReadResult ReadMappingsFile(const std::wstring& path,std::vector<Mapping>& out) {
 std::wifstream f(path);
 if(!f) return ConfigReadResult::Missing;

 std::wstring first;
 if(!std::getline(f,first)) {
  // Legacy v1.x allowed an empty file to mean zero mappings.
  out.clear();
  return ConfigReadResult::Valid;
 }

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

 // Legacy format migration: every non-empty row must be valid.
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

static bool WriteMappingsFile(const std::wstring& path,const std::vector<Mapping>& maps) {
 std::wofstream f(path,std::ios::trunc);
 if(!f) return false;
 f<<L"KRM_CONFIG 2\n";
 f<<L"COUNT "<<maps.size()<<L"\n";
 for(const auto&m:maps)
  f<<m.source<<L' '<<m.target<<L' '<<(m.block?1:0)<<L' '<<m.srcMods<<L' '<<m.dstMods<<L'\n';
 f.flush();
 return (bool)f;
}

static bool Save() {
 const std::wstring path=ConfigPath();
 const std::wstring temp=ConfigTempPath();
 const std::wstring backup=ConfigBackupPath();

 if(!WriteMappingsFile(temp,gMaps)) return false;

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

 if(GetFileAttributesW(backup.c_str())==INVALID_FILE_ATTRIBUTES) {
  if(!CopyFileW(path.c_str(),backup.c_str(),FALSE)) return false;
 }
 return true;
}

static bool Load() {
 gMaps.clear();
 const std::wstring primary=ConfigPath();
 const std::wstring backup=ConfigBackupPath();

 std::vector<Mapping> parsed;
 ConfigReadResult primaryResult=ReadMappingsFile(primary,parsed);
 if(primaryResult==ConfigReadResult::Valid) {
  gMaps=std::move(parsed);
  // Migrate legacy content to the versioned format on first successful load.
  std::wifstream check(primary);
  std::wstring first; std::getline(check,first);
  if(first.rfind(L"KRM_CONFIG ",0)!=0 && !Save()) return false;
  return true;
 }

 std::vector<Mapping> recovered;
 if(ReadMappingsFile(backup,recovered)==ConfigReadResult::Valid) {
  gMaps=std::move(recovered);
  if(!WriteMappingsFile(primary,gMaps)) return false;
  return true;
 }
 return primaryResult==ConfigReadResult::Missing;
}

static LRESULT CALLBACK ComboWheelProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam,UINT_PTR,DWORD_PTR refData) {
 auto* state=reinterpret_cast<ComboWheelState*>(refData);
 if(msg==WM_MOUSEWHEEL && state && state->combo) {
  state->delta+=GET_WHEEL_DELTA_WPARAM(wParam);
  int count=ComboBox_GetCount(state->combo);
  int current=ComboBox_GetCurSel(state->combo);
  if(current==CB_ERR) current=0;
  bool changed=false;
  while(state->delta>=WHEEL_DELTA) {
   int next=std::max(0,current-1);
   changed|=(next!=current); current=next; state->delta-=WHEEL_DELTA;
  }
  while(state->delta<=-WHEEL_DELTA) {
   int next=std::min(std::max(0,count-1),current+1);
   changed|=(next!=current); current=next; state->delta+=WHEEL_DELTA;
  }
  if(changed) {
   ComboBox_SetCurSel(state->combo,current);
   COMBOBOXINFO info{sizeof(info)};
   if(GetComboBoxInfo(state->combo,&info) && info.hwndList) {
    SendMessageW(info.hwndList,LB_SETCURSEL,current,0);
    InvalidateRect(info.hwndList,nullptr,TRUE);
   }
   InvalidateRect(state->combo,nullptr,TRUE);
  }
  return 0;
 }
 return DefSubclassProc(hwnd,msg,wParam,lParam);
}
static void InstallStableComboWheel(HWND combo,ComboWheelState& state) {
 state.combo=combo; state.delta=0;
 SetWindowSubclass(combo,ComboWheelProc,1,reinterpret_cast<DWORD_PTR>(&state));
 COMBOBOXINFO info{sizeof(info)};
 if(GetComboBoxInfo(combo,&info) && info.hwndList)
  SetWindowSubclass(info.hwndList,ComboWheelProc,2,reinterpret_cast<DWORD_PTR>(&state));
}
static Choice SelectedChoice(HWND h,const std::vector<Choice>& choices) {
 int i=ComboBox_GetCurSel(h);
 if(i==CB_ERR || i<0 || (size_t)i>=choices.size()) return {};
 return choices[(size_t)i];
}
static int ComboCode(HWND h,const std::vector<Choice>& choices) {
 return SelectedChoice(h,choices).code;
}
static unsigned ComboImpliedMods(HWND h,const std::vector<Choice>& choices) {
 return SelectedChoice(h,choices).mods;
}
static void SelectChoice(HWND h,const std::vector<Choice>& choices,int code,unsigned mods) {
 for(size_t i=0;i<choices.size();++i) {
  if(choices[i].code==code && choices[i].mods==mods) {
   ComboBox_SetCurSel(h,(int)i); return;
  }
 }
 for(size_t i=0;i<choices.size();++i) {
  if(choices[i].code==code && choices[i].mods==0) {
   ComboBox_SetCurSel(h,(int)i); return;
  }
 }
}
static void FillCombo(HWND h,std::vector<Choice>& choices) {
 ComboBox_ResetContent(h);
 choices.clear();
 choices.reserve((sizeof(ITEMS)/sizeof(ITEMS[0]))+wcslen(SYMBOL_CHARS));
 for(auto&i:ITEMS) {
  ComboBox_AddString(h,i.name);
  choices.push_back({i.code,0});
 }
 for(const wchar_t* p=SYMBOL_CHARS;*p;++p) {
  Choice c{};
  if(!ChoiceForSymbol(*p,c)) continue;
  wchar_t label[2]{*p,0};
  ComboBox_AddString(h,label);
  choices.push_back(c);
 }
}
static unsigned CheckedMods(HWND sh,HWND,HWND,HWND) {
 if(sh==gSrcShift) return gSourceUiMods;
 if(sh==gDstShift) return gTargetUiMods;
 return 0;
}
static void InvalidateModifierGroup(HWND sh,HWND ctrl,HWND alt,HWND win) {
 for(HWND h:{sh,ctrl,alt,win}) if(h) InvalidateRect(h,nullptr,TRUE);
}
static void SetModChecks(unsigned m,HWND sh,HWND ctrl,HWND alt,HWND win) {
 if(sh==gSrcShift) gSourceUiMods=m;
 else if(sh==gDstShift) gTargetUiMods=m;
 InvalidateModifierGroup(sh,ctrl,alt,win);
}
static unsigned ModifierBitForControlId(int id) {
 switch(id) {
  case ID_SRC_SHIFT: case ID_DST_SHIFT: return KRM_SHIFT;
  case ID_SRC_CTRL: case ID_DST_CTRL: return KRM_CTRL;
  case ID_SRC_ALT: case ID_DST_ALT: return KRM_ALT;
  case ID_SRC_WIN: case ID_DST_WIN: return KRM_WIN;
  default: return 0;
 }
}
static void ToggleModifierControl(int id) {
 unsigned bit=ModifierBitForControlId(id);
 if(!bit) return;
 if(id>=ID_SRC_SHIFT && id<=ID_SRC_WIN) {
  gSourceUiMods^=bit;
  InvalidateModifierGroup(gSrcShift,gSrcCtrl,gSrcAlt,gSrcWin);
 } else {
  gTargetUiMods^=bit;
  InvalidateModifierGroup(gDstShift,gDstCtrl,gDstAlt,gDstWin);
 }
}
static void SetBlockSourceState(bool enabled) {
 gBlockSource=enabled;
 if(gBlock) InvalidateRect(gBlock,nullptr,TRUE);
 BOOL targetEnabled=enabled?FALSE:TRUE;
 if(gTarget) EnableWindow(gTarget,targetEnabled);
 if(gDstShift) EnableWindow(gDstShift,targetEnabled);
 if(gDstCtrl) EnableWindow(gDstCtrl,targetEnabled);
 if(gDstAlt) EnableWindow(gDstAlt,targetEnabled);
 if(gDstWin) EnableWindow(gDstWin,targetEnabled);
}
static unsigned PreferredImpliedMods(int code,unsigned mods) {
 return SymbolCharForKey(code,mods)?mods:0;
}
static int SelectedMappingIndex() {
 int row=ListView_GetNextItem(gList,-1,LVNI_SELECTED);
 if(row<0) return -1;
 LVITEMW item{}; item.mask=LVIF_PARAM; item.iItem=row;
 if(!ListView_GetItem(gList,&item)) return -1;
 Mapping* ptr=reinterpret_cast<Mapping*>(item.lParam);
 if(!ptr) return -1;
 for(size_t i=0;i<gMaps.size();++i) if(&gMaps[i]==ptr) return (int)i;
 return -1;
}
static void ResetEditorState() {
 gEditingIndex=-1;
 if(gAdd) SetWindowTextW(gAdd,L"Add mapping");
 if(gSection) SetWindowTextW(gSection,L"Create mapping");
}
static void LoadMappingIntoEditor(int index) {
 if(index<0 || index>=(int)gMaps.size()) return;
 const Mapping& m=gMaps[(size_t)index];
 unsigned srcImplicit=PreferredImpliedMods(m.source,m.srcMods);
 unsigned dstImplicit=PreferredImpliedMods(m.target,m.dstMods);
 SelectChoice(gSource,gSourceChoices,m.source,srcImplicit);
 SelectChoice(gTarget,gTargetChoices,m.target,dstImplicit);
 SetModChecks(m.srcMods&~srcImplicit,gSrcShift,gSrcCtrl,gSrcAlt,gSrcWin);
 SetModChecks(m.dstMods&~dstImplicit,gDstShift,gDstCtrl,gDstAlt,gDstWin);
 SetBlockSourceState(m.block);
 gEditingIndex=index;
 if(gAdd) SetWindowTextW(gAdd,L"Save changes");
 if(gSection) SetWindowTextW(gSection,L"Edit mapping");
 if(gStatus) SetWindowTextW(gStatus,L"Editing selected mapping");
}
static void ModifySelected() {
 int index=SelectedMappingIndex();
 if(index<0) {
  if(gStatus) SetWindowTextW(gStatus,L"Select a mapping to modify");
  return;
 }
 LoadMappingIntoEditor(index);
}
static void Refresh() {
 ListView_DeleteAllItems(gList);
 int row=0;
 for(auto&m:gMaps) {
  std::wstring a=Describe(m.source,m.srcMods);
  std::wstring b=m.block?L"Blocked":Describe(m.target,m.dstMods);
  LVITEMW it{};it.mask=LVIF_TEXT|LVIF_PARAM;it.iItem=row;it.pszText=a.data();it.lParam=(LPARAM)&m;
  ListView_InsertItem(gList,&it);
  ListView_SetItemText(gList,row,1,b.data());
  ++row;
 }
 if(gEmpty) ShowWindow(gEmpty,gMaps.empty()?SW_SHOW:SW_HIDE);
 wchar_t st[160]{};
 unsigned count=(unsigned)gMaps.size();
 const wchar_t* unit=count==1?L"mapping":L"mappings";
 if(gConfigLoadFailed)
  wsprintfW(st,L"Configuration recovery failed  |  %u %s",count,unit);
 else if(gConfigSaveFailed)
  wsprintfW(st,L"Could not save mappings  |  %u %s",count,unit);
 else if(!gKeyboard || !gMouse)
  wsprintfW(st,L"Input hooks unavailable  |  %u %s",count,unit);
 else if(gLastInjectionError!=ERROR_SUCCESS)
  wsprintfW(st,L"Input injection failed (%lu)  |  %u %s",gLastInjectionError,count,unit);
 else if(gPaused)
  wsprintfW(st,L"Paused  |  %u %s",count,unit);
 else
  wsprintfW(st,L"Active  |  %u %s",count,unit);
 if(gStatus) SetWindowTextW(gStatus,st);
 if(gPause) SetWindowTextW(gPause,gPaused?L"Resume":L"Pause");
}

static HWND Make(HWND p,const wchar_t*c,const wchar_t*t,DWORD s,int x,int y,int w,int h,int id,DWORD ex=0) {
 return CreateWindowExW(ex,c,t,s|WS_CHILD|WS_VISIBLE,x,y,w,h,p,(HMENU)(INT_PTR)id,gInst,nullptr);
}

static HFONT MakeUiFont(HWND h,int px,int weight) {
 UINT dpi=GetDpiForWindow(h);
 return CreateFontW(-MulDiv(px,(int)dpi,96),0,0,0,weight,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
  OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH|FF_DONTCARE,L"Segoe UI");
}

static void EnsureUiResources(HWND h) {
 if(!gBrushBg) gBrushBg=CreateSolidBrush(C_BG);
 if(!gBrushSurface) gBrushSurface=CreateSolidBrush(C_SURFACE);
 if(!gBrushControl) gBrushControl=CreateSolidBrush(C_CONTROL);
 if(!gFontBody) gFontBody=MakeUiFont(h,14,FW_NORMAL);
 if(!gFontSmall) gFontSmall=MakeUiFont(h,12,FW_NORMAL);
 if(!gFontLabel) gFontLabel=MakeUiFont(h,13,FW_SEMIBOLD);
 if(!gFontTitle) gFontTitle=MakeUiFont(h,21,FW_SEMIBOLD);
}

static void ApplyFont(HWND h,HFONT font) {
 if(h && font) SendMessageW(h,WM_SETFONT,(WPARAM)font,TRUE);
}

static bool IsModifierId(UINT id) {
 return id==ID_SRC_SHIFT||id==ID_SRC_CTRL||id==ID_SRC_ALT||id==ID_SRC_WIN||
        id==ID_DST_SHIFT||id==ID_DST_CTRL||id==ID_DST_ALT||id==ID_DST_WIN;
}

static bool IsSwitchId(UINT id) {
 return id==ID_BLOCK || id==ID_STARTUP;
}

static void DrawRoundedRect(HDC dc,const RECT& r,COLORREF fill,COLORREF border,int radius) {
 HBRUSH brush=CreateSolidBrush(fill);
 HPEN pen=CreatePen(PS_SOLID,1,border);
 HGDIOBJ oldBrush=SelectObject(dc,brush);
 HGDIOBJ oldPen=SelectObject(dc,pen);
 int scaledRadius=S(radius);
 RoundRect(dc,r.left,r.top,r.right,r.bottom,scaledRadius,scaledRadius);
 SelectObject(dc,oldBrush);
 SelectObject(dc,oldPen);
 DeleteObject(brush);
 DeleteObject(pen);
}

static void DrawToggleChip(const DRAWITEMSTRUCT* d) {
 RECT r=d->rcItem;
 unsigned bit=ModifierBitForControlId((int)d->CtlID);
 bool source=(d->CtlID>=ID_SRC_SHIFT && d->CtlID<=ID_SRC_WIN);
 bool checked=source?((gSourceUiMods&bit)!=0):((gTargetUiMods&bit)!=0);
 bool pressed=(d->itemState&ODS_SELECTED)!=0;
 bool disabled=(d->itemState&ODS_DISABLED)!=0;

 COLORREF fill=checked?C_LAVENDER:C_SURFACE;
 COLORREF border=checked?C_LAVENDER_BORDER:C_BORDER;
 COLORREF text=disabled?RGB(170,174,182):C_TEXT;
 if(pressed && !disabled) fill=checked?RGB(229,222,252):C_HOVER;

 DrawRoundedRect(d->hDC,r,fill,border,12);

 wchar_t label[64]{};
 GetWindowTextW(d->hwndItem,label,64);
 SetBkMode(d->hDC,TRANSPARENT);
 SetTextColor(d->hDC,text);
 HFONT old=(HFONT)SelectObject(d->hDC,gFontSmall?gFontSmall:GetStockObject(DEFAULT_GUI_FONT));
 DrawTextW(d->hDC,label,-1,&r,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
 SelectObject(d->hDC,old);

 if(d->itemState&ODS_FOCUS) {
  RECT focus=r;InflateRect(&focus,-3,-3);DrawFocusRect(d->hDC,&focus);
 }
}

static void DrawSwitchControl(const DRAWITEMSTRUCT* d) {
 RECT r=d->rcItem;
 bool checked=(d->CtlID==ID_BLOCK)?gBlockSource:gRunAtStartup;
 bool disabled=(d->itemState&ODS_DISABLED)!=0;
 int trackW=S(34),trackH=S(18);
 RECT track{r.left,r.top+(r.bottom-r.top-trackH)/2,r.left+trackW,r.top+(r.bottom-r.top+trackH)/2};
 COLORREF trackFill=checked?C_ACCENT:RGB(207,211,219);
 if(disabled) trackFill=RGB(225,227,232);
 DrawRoundedRect(d->hDC,track,trackFill,trackFill,trackH);

 int knob=S(14);
 int inset=S(2);
 int knobX=checked?track.right-knob-inset:track.left+inset;
 RECT k{knobX,track.top+inset,knobX+knob,track.top+inset+knob};
 HBRUSH kb=CreateSolidBrush(RGB(255,255,255));
 HPEN kp=CreatePen(PS_SOLID,1,RGB(255,255,255));
 HGDIOBJ ob=SelectObject(d->hDC,kb),op=SelectObject(d->hDC,kp);
 Ellipse(d->hDC,k.left,k.top,k.right,k.bottom);
 SelectObject(d->hDC,ob);SelectObject(d->hDC,op);DeleteObject(kb);DeleteObject(kp);

 wchar_t label[96]{};
 GetWindowTextW(d->hwndItem,label,96);
 RECT tr=r;tr.left=track.right+S(10);
 SetBkMode(d->hDC,TRANSPARENT);
 SetTextColor(d->hDC,disabled?RGB(160,165,174):C_TEXT);
 HFONT old=(HFONT)SelectObject(d->hDC,gFontSmall?gFontSmall:GetStockObject(DEFAULT_GUI_FONT));
 DrawTextW(d->hDC,label,-1,&tr,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
 SelectObject(d->hDC,old);
}

static void DrawOwnerButton(const DRAWITEMSTRUCT* d) {
 if(IsModifierId(d->CtlID)) { DrawToggleChip(d); return; }
 if(IsSwitchId(d->CtlID)) { DrawSwitchControl(d); return; }

 RECT r=d->rcItem;
 bool primary=d->CtlID==ID_ADD;
 bool capture=d->CtlID==ID_CAP_SRC||d->CtlID==ID_CAP_DST;
 bool pause=d->CtlID==ID_PAUSE;
 bool pressed=(d->itemState&ODS_SELECTED)!=0;
 bool disabled=(d->itemState&ODS_DISABLED)!=0;

 COLORREF fill=C_SURFACE,border=C_BORDER,text=C_TEXT;
 if(primary) { fill=C_ACCENT;border=C_ACCENT;text=RGB(8,42,51); }
 else if(capture) { fill=C_LAVENDER;border=C_LAVENDER_BORDER;text=C_TEXT; }
 else if(pause) { fill=RGB(28,30,35);border=RGB(28,30,35);text=RGB(255,255,255); }
 else { fill=C_CONTROL;border=C_BORDER;text=C_TEXT; }

 if(pressed && !disabled) {
  if(primary) fill=C_ACCENT_HOVER;
  else if(capture) fill=RGB(229,222,252);
  else if(pause) fill=RGB(48,51,58);
  else fill=C_HOVER;
 }
 if(disabled) { fill=RGB(243,244,247);border=C_BORDER;text=RGB(160,165,174); }

 DrawRoundedRect(d->hDC,r,fill,border,8);

 wchar_t label[128]{};
 GetWindowTextW(d->hwndItem,label,128);
 SetBkMode(d->hDC,TRANSPARENT);
 SetTextColor(d->hDC,text);
 HFONT old=(HFONT)SelectObject(d->hDC,gFontLabel?gFontLabel:GetStockObject(DEFAULT_GUI_FONT));
 DrawTextW(d->hDC,label,-1,&r,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
 SelectObject(d->hDC,old);

 if(d->itemState&ODS_FOCUS) {
  RECT focus=r;InflateRect(&focus,-4,-4);DrawFocusRect(d->hDC,&focus);
 }
}

static void DrawOwnerCombo(const DRAWITEMSTRUCT* d) {
 RECT r=d->rcItem;
 COLORREF fill=(d->itemState&ODS_SELECTED)?C_SELECTED:C_SURFACE;
 HBRUSH comboBrush=CreateSolidBrush(fill);
 FillRect(d->hDC,&r,comboBrush);
 DeleteObject(comboBrush);
 if(d->itemID==(UINT)-1) return;

 wchar_t buf[128]{};
 SendMessageW(d->hwndItem,CB_GETLBTEXT,d->itemID,(LPARAM)buf);
 r.left+=S(10);r.right-=S(8);
 SetBkMode(d->hDC,TRANSPARENT);
 SetTextColor(d->hDC,C_TEXT);
 HFONT old=(HFONT)SelectObject(d->hDC,gFontBody?gFontBody:GetStockObject(DEFAULT_GUI_FONT));
 DrawTextW(d->hDC,buf,-1,&r,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
 SelectObject(d->hDC,old);
}

static void ApplyAllFonts() {
 ApplyFont(gTitle,gFontTitle);
 ApplyFont(gSubtitle,gFontSmall);
 ApplyFont(gSection,gFontLabel);
 ApplyFont(gSourceLabel,gFontSmall);
 ApplyFont(gTargetLabel,gFontSmall);
 ApplyFont(gSrcModsLabel,gFontSmall);
 ApplyFont(gDstModsLabel,gFontSmall);
 ApplyFont(gMappingsLabel,gFontLabel);
 ApplyFont(gListSource,gFontLabel);
 ApplyFont(gListTarget,gFontLabel);
 ApplyFont(gEmpty,gFontSmall);
 ApplyFont(gStatus,gFontSmall);
 HWND controls[]={gSource,gCaptureSrc,gTarget,gCaptureDst,gBlock,gAdd,gModify,gRemove,gPause,gStartup,
  gSrcShift,gSrcCtrl,gSrcAlt,gSrcWin,gDstShift,gDstCtrl,gDstAlt,gDstWin};
 for(HWND z:controls) ApplyFont(z,gFontBody);
 ApplyFont(gList,gFontBody);
}

static void RecreateFonts(HWND h) {
 HFONT oldBody=gFontBody,oldSmall=gFontSmall,oldTitle=gFontTitle,oldLabel=gFontLabel;
 gFontBody=MakeUiFont(h,14,FW_NORMAL);
 gFontSmall=MakeUiFont(h,12,FW_NORMAL);
 gFontLabel=MakeUiFont(h,13,FW_SEMIBOLD);
 gFontTitle=MakeUiFont(h,21,FW_SEMIBOLD);
 ApplyAllFonts();
 if(oldBody) DeleteObject(oldBody);
 if(oldSmall) DeleteObject(oldSmall);
 if(oldTitle) DeleteObject(oldTitle);
 if(oldLabel) DeleteObject(oldLabel);
}

static void LayoutUi(int width,int height) {
 if(!gList) return;
 const int margin=S(20);
 const int cardLeft=S(20);
 const int listTop=S(336);
 const int footerY=height-S(44);
 const int listBottom=std::max(listTop+S(118),footerY-S(16));
 const int listWidth=std::max(S(610),width-margin*2);
 const int half=listWidth/2;

 MoveWindow(gIconControl,S(20),S(15),S(32),S(32),TRUE);
 MoveWindow(gTitle,S(62),S(13),S(300),S(28),TRUE);
 MoveWindow(gSubtitle,S(62),S(39),S(420),S(20),TRUE);
 MoveWindow(gSection,S(32),S(82),S(180),S(22),TRUE);

 MoveWindow(gSourceLabel,S(32),S(122),S(56),S(20),TRUE);
 MoveWindow(gSource,S(96),S(112),S(210),S(320),TRUE);
 MoveWindow(gCaptureSrc,S(316),S(112),S(80),S(32),TRUE);
 MoveWindow(gSrcModsLabel,S(414),S(122),S(62),S(20),TRUE);
 MoveWindow(gSrcShift,S(482),S(114),S(52),S(28),TRUE);
 MoveWindow(gSrcCtrl,S(540),S(114),S(48),S(28),TRUE);
 MoveWindow(gSrcAlt,S(594),S(114),S(46),S(28),TRUE);
 MoveWindow(gSrcWin,S(646),S(114),S(48),S(28),TRUE);

 MoveWindow(gTargetLabel,S(32),S(164),S(56),S(20),TRUE);
 MoveWindow(gTarget,S(96),S(154),S(210),S(320),TRUE);
 MoveWindow(gCaptureDst,S(316),S(154),S(80),S(32),TRUE);
 MoveWindow(gDstModsLabel,S(414),S(164),S(62),S(20),TRUE);
 MoveWindow(gDstShift,S(482),S(156),S(52),S(28),TRUE);
 MoveWindow(gDstCtrl,S(540),S(156),S(48),S(28),TRUE);
 MoveWindow(gDstAlt,S(594),S(156),S(46),S(28),TRUE);
 MoveWindow(gDstWin,S(646),S(156),S(48),S(28),TRUE);

 MoveWindow(gAdd,S(32),S(204),S(108),S(34),TRUE);
 MoveWindow(gModify,S(150),S(204),S(88),S(34),TRUE);
 MoveWindow(gRemove,S(248),S(204),S(88),S(34),TRUE);
 MoveWindow(gBlock,S(366),S(207),S(122),S(28),TRUE);

 MoveWindow(gMappingsLabel,S(32),S(276),S(100),S(22),TRUE);
 MoveWindow(gListSource,cardLeft+S(12),S(310),half-S(18),S(20),TRUE);
 MoveWindow(gListTarget,cardLeft+half+S(6),S(310),half-S(18),S(20),TRUE);
 MoveWindow(gList,cardLeft,listTop,listWidth,listBottom-listTop,TRUE);

 if(gEmpty) {
  int emptyW=std::min(S(380),listWidth-S(40));
  MoveWindow(gEmpty,cardLeft+(listWidth-emptyW)/2,listTop+S(38),emptyW,S(28),TRUE);
 }
 MoveWindow(gStartup,margin,footerY,S(156),S(28),TRUE);
 MoveWindow(gStatus,S(198),footerY+S(4),std::max(S(210),width-S(318)),S(20),TRUE);
 MoveWindow(gPause,width-S(106),footerY-S(2),S(86),S(32),TRUE);

 ListView_SetColumnWidth(gList,0,half);
 ListView_SetColumnWidth(gList,1,listWidth-half);
}

static void CreateUi(HWND h) {
 gDpi=GetDpiForWindow(h);
 EnsureUiResources(h);
 BOOL dark=FALSE;
 DwmSetWindowAttribute(h,20,&dark,sizeof(dark));

 gIconControl=Make(h,L"STATIC",L"",SS_ICON|SS_REALSIZECONTROL,20,15,32,32,0);
 if(gIconLarge) SendMessageW(gIconControl,STM_SETICON,(WPARAM)gIconLarge,0);

 gTitle=Make(h,L"STATIC",L"Key Remapper",SS_LEFT,62,13,300,28,ID_TITLE);
 gSubtitle=Make(h,L"STATIC",L"Remap keyboard and mouse input.",SS_LEFT,62,39,420,20,ID_SUBTITLE);

 gSection=Make(h,L"STATIC",L"Create mapping",SS_LEFT,32,82,180,22,ID_SECTION);

 gSourceLabel=Make(h,L"STATIC",L"Source",SS_LEFT,32,122,56,20,0);
 gSource=Make(h,WC_COMBOBOXW,L"",CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS|WS_VSCROLL,96,112,210,320,ID_SOURCE);
 gCaptureSrc=Make(h,L"BUTTON",L"Capture",BS_OWNERDRAW,316,112,80,32,ID_CAP_SRC);
 gSrcModsLabel=Make(h,L"STATIC",L"Modifiers",SS_LEFT,414,122,62,20,0);
 gSrcShift=Make(h,L"BUTTON",L"Shift",BS_OWNERDRAW,482,114,52,28,ID_SRC_SHIFT);
 gSrcCtrl=Make(h,L"BUTTON",L"Ctrl",BS_OWNERDRAW,540,114,48,28,ID_SRC_CTRL);
 gSrcAlt=Make(h,L"BUTTON",L"Alt",BS_OWNERDRAW,594,114,46,28,ID_SRC_ALT);
 gSrcWin=Make(h,L"BUTTON",L"Win",BS_OWNERDRAW,646,114,48,28,ID_SRC_WIN);

 gTargetLabel=Make(h,L"STATIC",L"Target",SS_LEFT,32,164,56,20,0);
 gTarget=Make(h,WC_COMBOBOXW,L"",CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS|WS_VSCROLL,96,154,210,320,ID_TARGET);
 gCaptureDst=Make(h,L"BUTTON",L"Capture",BS_OWNERDRAW,316,154,80,32,ID_CAP_DST);
 gDstModsLabel=Make(h,L"STATIC",L"Modifiers",SS_LEFT,414,164,62,20,0);
 gDstShift=Make(h,L"BUTTON",L"Shift",BS_OWNERDRAW,482,156,52,28,ID_DST_SHIFT);
 gDstCtrl=Make(h,L"BUTTON",L"Ctrl",BS_OWNERDRAW,540,156,48,28,ID_DST_CTRL);
 gDstAlt=Make(h,L"BUTTON",L"Alt",BS_OWNERDRAW,594,156,46,28,ID_DST_ALT);
 gDstWin=Make(h,L"BUTTON",L"Win",BS_OWNERDRAW,646,156,48,28,ID_DST_WIN);

 gAdd=Make(h,L"BUTTON",L"Add mapping",BS_OWNERDRAW,32,204,108,34,ID_ADD);
 gModify=Make(h,L"BUTTON",L"Modify",BS_OWNERDRAW,150,204,88,34,ID_MODIFY);
 gRemove=Make(h,L"BUTTON",L"Remove",BS_OWNERDRAW,248,204,88,34,ID_REMOVE);
 gBlock=Make(h,L"BUTTON",L"Block source",BS_OWNERDRAW,366,207,122,28,ID_BLOCK);

 gMappingsLabel=Make(h,L"STATIC",L"Mappings",SS_LEFT,32,276,100,22,0);
 gListSource=Make(h,L"STATIC",L"Source",SS_LEFT,32,310,220,20,ID_LIST_SOURCE);
 gListTarget=Make(h,L"STATIC",L"Target",SS_LEFT,430,310,220,20,ID_LIST_TARGET);

 gList=Make(h,WC_LISTVIEWW,L"",LVS_REPORT|LVS_SINGLESEL|LVS_SHOWSELALWAYS|LVS_NOCOLUMNHEADER,20,336,780,160,ID_LIST,0);
 ListView_SetExtendedListViewStyle(gList,LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER);
 ListView_SetBkColor(gList,C_SURFACE);
 ListView_SetTextBkColor(gList,C_SURFACE);
 ListView_SetTextColor(gList,C_TEXT);
 LVCOLUMNW col{};col.mask=LVCF_TEXT|LVCF_WIDTH;
 col.pszText=(LPWSTR)L"Source";col.cx=390;ListView_InsertColumn(gList,0,&col);
 col.pszText=(LPWSTR)L"Target";col.cx=390;ListView_InsertColumn(gList,1,&col);

 gEmpty=Make(h,L"STATIC",L"No mappings yet. Add one above.",SS_CENTER,210,382,380,28,ID_EMPTY);
 gStartup=Make(h,L"BUTTON",L"Run at startup",BS_OWNERDRAW,20,524,156,28,ID_STARTUP);
 gStatus=Make(h,L"STATIC",L"",SS_LEFT,198,528,420,20,ID_STATUS);
 gPause=Make(h,L"BUTTON",L"Pause",BS_OWNERDRAW,714,522,86,32,ID_PAUSE);

 ApplyAllFonts();

 SetWindowTheme(gSource,L"Explorer",nullptr);
 SetWindowTheme(gTarget,L"Explorer",nullptr);
 SetWindowTheme(gList,L"Explorer",nullptr);

 FillCombo(gSource,gSourceChoices);FillCombo(gTarget,gTargetChoices);
 SelectChoice(gSource,gSourceChoices,VK_CAPITAL,0);
 SelectChoice(gTarget,gTargetChoices,VK_ESCAPE,0);
 InstallStableComboWheel(gSource,gSourceWheel);
 InstallStableComboWheel(gTarget,gTargetWheel);
 gRunAtStartup=StartupEnabled();
 InvalidateRect(gStartup,nullptr,TRUE);

 RECT rc{};GetClientRect(h,&rc);LayoutUi(rc.right,rc.bottom);
 Refresh();
}
static void AddMap() {
 Mapping m;
 m.source=ComboCode(gSource,gSourceChoices); m.target=ComboCode(gTarget,gTargetChoices);
 m.block=gBlockSource;
 m.srcMods=CheckedMods(gSrcShift,gSrcCtrl,gSrcAlt,gSrcWin) | ComboImpliedMods(gSource,gSourceChoices);
 m.dstMods=CheckedMods(gDstShift,gDstCtrl,gDstAlt,gDstWin) | ComboImpliedMods(gTarget,gTargetChoices);
 if(!m.source||(!m.block&&!m.target)) return;

 int selectIndex=-1;
 if(gEditingIndex>=0 && gEditingIndex<(int)gMaps.size()) {
  gMaps[(size_t)gEditingIndex]=m;
  selectIndex=gEditingIndex;
 } else {
  for(size_t i=0;i<gMaps.size();++i) {
   if(gMaps[i].source==m.source && gMaps[i].srcMods==m.srcMods) {
    gMaps[i]=m;
    selectIndex=(int)i;
    break;
   }
  }
  if(selectIndex<0) {
   gMaps.push_back(m);
   selectIndex=(int)gMaps.size()-1;
  }
 }
 gConfigSaveFailed=!Save();
 ResetEditorState();
 Refresh();
 if(selectIndex>=0 && selectIndex<ListView_GetItemCount(gList)) {
  ListView_SetItemState(gList,selectIndex,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
  ListView_EnsureVisible(gList,selectIndex,FALSE);
 }
}
static void RemoveMap() {
 int index=SelectedMappingIndex();
 if(index<0) {
  if(gStatus) SetWindowTextW(gStatus,L"Select a mapping to remove");
  return;
 }
 gMaps.erase(gMaps.begin()+index);
 ResetEditorState();
 gConfigSaveFailed=!Save();
 Refresh();
}

static void UpdateTrayIconState() {
 if(!gTrayAdded) return;
 gTray.uFlags=NIF_TIP;
 const wchar_t* state=(!gKeyboard||!gMouse)?L"Hook unavailable":
                       (gLastInjectionError!=ERROR_SUCCESS?L"Input injection failed":
                       (gPaused?L"Remapping disabled":L"Remapping enabled"));
 std::wstring tip=L"Key Remapper - ";
 tip+=state;
 lstrcpynW(gTray.szTip,tip.c_str(),ARRAYSIZE(gTray.szTip));
 Shell_NotifyIconW(NIM_MODIFY,&gTray);
}

static bool AddTrayIcon(HWND owner) {
 if(gTrayAdded) return true;
 ZeroMemory(&gTray,sizeof(gTray));
 gTray.cbSize=sizeof(gTray);
 gTray.hWnd=owner;
 gTray.uID=TRAY_ID;
 gTray.uFlags=NIF_MESSAGE|NIF_ICON|NIF_TIP;
 gTray.uCallbackMessage=WM_KR_TRAY;
 gTray.hIcon=gIconSmall?gIconSmall:gIconLarge;
 lstrcpynW(gTray.szTip,L"Key Remapper - Remapping enabled",ARRAYSIZE(gTray.szTip));
 if(!Shell_NotifyIconW(NIM_ADD,&gTray)) return false;
 gTrayAdded=true;
 UpdateTrayIconState();
 return true;
}

static void RemoveTrayIcon() {
 if(!gTrayAdded) return;
 Shell_NotifyIconW(NIM_DELETE,&gTray);
 gTrayAdded=false;
 ZeroMemory(&gTray,sizeof(gTray));
}

static void ShowDashboard() {
 if(!gWnd) return;
 ShowWindow(gWnd,SW_SHOW);
 if(IsIconic(gWnd)) ShowWindow(gWnd,SW_RESTORE);
 SetForegroundWindow(gWnd);
}

static void HideDashboard() {
 if(gWnd) ShowWindow(gWnd,SW_HIDE);
}

static bool ApplyRunAtStartup(bool enabled) {
 if(!SetStartupEnabled(enabled)) return false;
 gRunAtStartup=enabled;
 if(gStartup) InvalidateRect(gStartup,nullptr,TRUE);
 return true;
}

static void ResetDefaults() {
 int answer=MessageBoxW(gWnd,
  L"Reset mappings and preferences to defaults?\n\nThis clears all mappings, enables remapping, and disables Run at startup.",
  L"Reset Defaults",MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2);
 if(answer!=IDYES) return;

 gMaps.clear();
 gEditingIndex=-1;
 gSourceUiMods=0;
 gTargetUiMods=0;
 SetBlockSourceState(false);
 SetRemappingEnabled(true);
 ApplyRunAtStartup(false);
 SelectChoice(gSource,gSourceChoices,VK_CAPITAL,0);
 SelectChoice(gTarget,gTargetChoices,VK_ESCAPE,0);
 gConfigSaveFailed=!Save();
 ResetEditorState();
 Refresh();
 UpdateTrayIconState();
 InvalidateRect(gWnd,nullptr,TRUE);
}
static void ExitApplication() {
 if(!gWnd) return;
 gExitRequested=true;
 DestroyWindow(gWnd);
}

static void ExecuteTrayCommand(UINT command) {
 switch(command) {
  case ID_TRAY_OPEN:
   ShowDashboard();
   break;
  case ID_TRAY_ENABLE:
   SetRemappingEnabled(gPaused);
   Refresh();
   InvalidateRect(gWnd,nullptr,TRUE);
   UpdateTrayIconState();
   break;
  case ID_TRAY_STARTUP: {
   bool next=!gRunAtStartup;
   if(!ApplyRunAtStartup(next) && gStatus)
    SetWindowTextW(gStatus,L"Could not update Run at startup");
   break;
  }
  case ID_TRAY_RESET:
   ShowDashboard();
   ResetDefaults();
   break;
  case ID_TRAY_EXIT:
   ExitApplication();
   break;
 }
}

static void ShowTrayMenu() {
 HMENU root=CreatePopupMenu();
 HMENU prefs=CreatePopupMenu();
 if(!root||!prefs) {
  if(prefs) DestroyMenu(prefs);
  if(root) DestroyMenu(root);
  return;
 }

 AppendMenuW(root,MF_STRING,ID_TRAY_OPEN,L"Open");
 AppendMenuW(root,MF_SEPARATOR,0,nullptr);
 AppendMenuW(root,MF_STRING|(!gPaused?MF_CHECKED:MF_UNCHECKED),ID_TRAY_ENABLE,L"Enable Remapping");
 AppendMenuW(root,MF_SEPARATOR,0,nullptr);

 AppendMenuW(prefs,MF_STRING|(gRunAtStartup?MF_CHECKED:MF_UNCHECKED),ID_TRAY_STARTUP,L"Run at startup");
 AppendMenuW(prefs,MF_STRING,ID_TRAY_RESET,L"Reset Defaults");
 AppendMenuW(root,MF_POPUP,(UINT_PTR)prefs,L"Preferences");

 AppendMenuW(root,MF_SEPARATOR,0,nullptr);
 AppendMenuW(root,MF_STRING,ID_TRAY_EXIT,L"Exit");
 SetMenuDefaultItem(root,ID_TRAY_OPEN,FALSE);

 POINT pt{};
 GetCursorPos(&pt);
 SetForegroundWindow(gWnd);
 UINT cmd=TrackPopupMenu(root,TPM_RETURNCMD|TPM_RIGHTBUTTON|TPM_NONOTIFY,
                        pt.x,pt.y,0,gWnd,nullptr);
 PostMessageW(gWnd,WM_NULL,0,0);
 DestroyMenu(root);
 if(cmd) ExecuteTrayCommand(cmd);
}
static LRESULT CALLBACK WndProc(HWND h,UINT m,WPARAM w,LPARAM l) {
 if(gTaskbarCreatedMessage && m==gTaskbarCreatedMessage) {
  gTrayAdded=false;
  if(!AddTrayIcon(h) && !IsWindowVisible(h)) ShowWindow(h,SW_SHOWMINIMIZED);
  return 0;
 }
 switch(m) {
  case WM_CREATE:
   CreateUi(h);
   InstallHooks();
   AddTrayIcon(h);
   Refresh();
   UpdateTrayIconState();
   return 0;

  case WM_SIZE:
   LayoutUi(LOWORD(l),HIWORD(l));
   InvalidateRect(h,nullptr,TRUE);
   return 0;

  case WM_INPUTLANGCHANGE: {
   Choice source=SelectedChoice(gSource,gSourceChoices);
   Choice target=SelectedChoice(gTarget,gTargetChoices);
   FillCombo(gSource,gSourceChoices);
   FillCombo(gTarget,gTargetChoices);
   SelectChoice(gSource,gSourceChoices,source.code,source.mods);
   SelectChoice(gTarget,gTargetChoices,target.code,target.mods);
   Refresh();
   return 0;
  }

  case WM_DPICHANGED: {
   gDpi=HIWORD(w);
   auto* suggested=reinterpret_cast<RECT*>(l);
   SetWindowPos(h,nullptr,suggested->left,suggested->top,
                suggested->right-suggested->left,suggested->bottom-suggested->top,
                SWP_NOZORDER|SWP_NOACTIVATE);
   RecreateFonts(h);
   RECT rc{}; GetClientRect(h,&rc);
   LayoutUi(rc.right,rc.bottom);
   InvalidateRect(h,nullptr,TRUE);
   return 0;
  }

  case WM_GETMINMAXINFO: {
   auto* info=reinterpret_cast<MINMAXINFO*>(l);
   info->ptMinTrackSize.x=S(760);
   info->ptMinTrackSize.y=S(560);
   return 0;
  }

  case WM_PAINT: {
   PAINTSTRUCT ps{};
   HDC dc=BeginPaint(h,&ps);
   RECT client{};GetClientRect(h,&client);

   RECT editor{S(20),S(70),client.right-S(20),S(252)};
   RECT mappings{S(20),S(264),client.right-S(20),client.bottom-S(58)};
   DrawRoundedRect(dc,editor,C_SURFACE,C_BORDER,12);
   DrawRoundedRect(dc,mappings,C_SURFACE,C_BORDER,12);

   int footerY=client.bottom-S(44);
   COLORREF statusColor=(!gKeyboard||!gMouse)?RGB(198,67,67):(gPaused?RGB(214,147,35):C_ACTIVE);
   HBRUSH dotBrush=CreateSolidBrush(statusColor);
   HGDIOBJ old=SelectObject(dc,dotBrush);
   Ellipse(dc,S(184),footerY+S(8),S(192),footerY+S(16));
   SelectObject(dc,old);
   DeleteObject(dotBrush);

   EndPaint(h,&ps);
   return 0;
  }

  case WM_MEASUREITEM: {
   auto* mi=reinterpret_cast<MEASUREITEMSTRUCT*>(l);
   if(mi->CtlID==ID_SOURCE || mi->CtlID==ID_TARGET) {
    mi->itemHeight=S(28);
    return TRUE;
   }
   break;
  }

  case WM_DRAWITEM: {
   auto* di=reinterpret_cast<DRAWITEMSTRUCT*>(l);
   if(di->CtlType==ODT_BUTTON) { DrawOwnerButton(di); return TRUE; }
   if(di->CtlType==ODT_COMBOBOX) { DrawOwnerCombo(di); return TRUE; }
   break;
  }

  case WM_CTLCOLORSTATIC: {
   HDC dc=(HDC)w; HWND control=(HWND)l;
   int id=GetDlgCtrlID(control);
   if(id==ID_STATUS) {
    SetBkMode(dc,OPAQUE);
    SetBkColor(dc,C_BG);
    SetTextColor(dc,C_MUTED);
    return (LRESULT)gBrushBg;
   }
   if(id==ID_SECTION || id==ID_EMPTY) {
    SetBkMode(dc,OPAQUE);
    SetBkColor(dc,C_SURFACE);
    SetTextColor(dc,id==ID_SECTION?C_TEXT:C_MUTED);
    return (LRESULT)gBrushSurface;
   }
   SetBkMode(dc,TRANSPARENT);
   if(id==ID_TITLE) SetTextColor(dc,C_TEXT);
   else SetTextColor(dc,C_MUTED);
   return (LRESULT)GetStockObject(HOLLOW_BRUSH);
  }

  case WM_CTLCOLORBTN: {
   HDC dc=(HDC)w;
   SetBkMode(dc,TRANSPARENT);
   SetTextColor(dc,C_TEXT);
   return (LRESULT)GetStockObject(HOLLOW_BRUSH);
  }

  case WM_CTLCOLORLISTBOX: {
   HDC dc=(HDC)w;
   SetBkColor(dc,C_SURFACE);
   SetTextColor(dc,C_TEXT);
   return (LRESULT)gBrushSurface;
  }

  case WM_NOTIFY: {
   auto* hdr=reinterpret_cast<NMHDR*>(l);
   if(hdr->hwndFrom==gList) {
    if(hdr->code==NM_DBLCLK) {
     ModifySelected();
     return 0;
    }
    if(hdr->code==NM_CUSTOMDRAW) {
     auto* cd=reinterpret_cast<LPNMLVCUSTOMDRAW>(l);
     if(cd->nmcd.dwDrawStage==CDDS_PREPAINT) return CDRF_NOTIFYITEMDRAW;
     if(cd->nmcd.dwDrawStage==CDDS_ITEMPREPAINT) {
      bool selected=(ListView_GetItemState(gList,(int)cd->nmcd.dwItemSpec,LVIS_SELECTED)&LVIS_SELECTED)!=0;
      cd->clrText=C_TEXT;
      cd->clrTextBk=selected?C_SELECTED:C_SURFACE;
      if(gFontBody) SelectObject(cd->nmcd.hdc,gFontBody);
      return CDRF_NEWFONT;
     }
    }
   }
   break;
  }

  case WM_COMMAND: {
   int id=LOWORD(w);
   int notification=HIWORD(w);

   if(IsModifierId(id) && notification==BN_CLICKED) {
    ToggleModifierControl(id);
    return 0;
   }

   switch(id) {
    case ID_CAP_SRC:
     gCaptureSource=true;gCaptureTarget=false;
     SetWindowTextW(gStatus,L"Press the source key, mouse button, or wheel action");
     break;
    case ID_CAP_DST:
     gCaptureTarget=true;gCaptureSource=false;
     SetWindowTextW(gStatus,L"Press the target key, mouse button, or wheel action");
     break;
    case ID_ADD:
     AddMap();
     break;
    case ID_MODIFY:
     ModifySelected();
     break;
    case ID_REMOVE:
     RemoveMap();
     break;
    case ID_PAUSE:
     SetRemappingEnabled(gPaused);
     Refresh();
     InvalidateRect(h,nullptr,TRUE);
     UpdateTrayIconState();
     break;
    case ID_BLOCK:
     if(notification==BN_CLICKED) SetBlockSourceState(!gBlockSource);
     break;
    case ID_STARTUP:
     if(notification==BN_CLICKED) {
      bool next=!gRunAtStartup;
      if(!ApplyRunAtStartup(next)) {
       SetWindowTextW(gStatus,L"Could not update Run at startup");
       RedrawWindow(gStatus,nullptr,nullptr,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW);
      } else {
       Refresh();
      }
     }
     break;
   }
   return 0;
  }

  case WM_KR_CAPTURE: {
   int code=(int)w;
   bool src=(l&1)==0;
   unsigned mods=(unsigned)((ULONG_PTR)l>>1);
#ifdef KRM_TEST_BUILD
   gSelfCapturedCode=code; gSelfCapturedMods=mods; return 0;
#endif
   if(src) {
    SelectChoice(gSource,gSourceChoices,code,mods&KRM_SHIFT);
    SetModChecks(mods & ~ComboImpliedMods(gSource,gSourceChoices),gSrcShift,gSrcCtrl,gSrcAlt,gSrcWin);
   } else {
    SelectChoice(gTarget,gTargetChoices,code,mods&KRM_SHIFT);
    SetModChecks(mods & ~ComboImpliedMods(gTarget,gTargetChoices),gDstShift,gDstCtrl,gDstAlt,gDstWin);
   }
   SetWindowTextW(gStatus,src?L"Source captured":L"Target captured");
   InvalidateRect(h,nullptr,FALSE);
   return 0;
  }

  case WM_KR_REFRESH:
   Refresh();
   UpdateTrayIconState();
   InvalidateRect(h,nullptr,FALSE);
   return 0;

  case WM_KR_TRAY:
   switch((UINT)l) {
    case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
     ShowDashboard();
     break;
    case WM_RBUTTONUP:
    case WM_CONTEXTMENU:
     ShowTrayMenu();
     break;
   }
   return 0;

  case WM_QUERYENDSESSION:
   return TRUE;

  case WM_ENDSESSION:
   if(w) {
    gExitRequested=true;
    DestroyWindow(h);
   }
   return 0;

  case WM_CLOSE:
   if(gExitRequested) {
    DestroyWindow(h);
   } else if(gTrayAdded) {
    HideDashboard();
   } else {
    ShowWindow(h,SW_MINIMIZE);
   }
   return 0;

  case WM_DESTROY:
   RemoveTrayIcon();
   Save();
   Unhook();
   if(gIconLarge) DestroyIcon(gIconLarge);
   if(gIconSmall) DestroyIcon(gIconSmall);
   gIconLarge=gIconSmall=nullptr;
   if(gFontBody) DeleteObject(gFontBody);
   if(gFontSmall) DeleteObject(gFontSmall);
   if(gFontTitle) DeleteObject(gFontTitle);
   if(gFontLabel) DeleteObject(gFontLabel);
   if(gBrushBg) DeleteObject(gBrushBg);
   if(gBrushSurface) DeleteObject(gBrushSurface);
   if(gBrushControl) DeleteObject(gBrushControl);
   if(gInstanceMutex) { ReleaseMutex(gInstanceMutex); CloseHandle(gInstanceMutex); gInstanceMutex=nullptr; }
   gFontBody=gFontSmall=gFontTitle=gFontLabel=nullptr;
   gBrushBg=gBrushSurface=gBrushControl=nullptr;
   PostQuitMessage(0);
   return 0;
 }
 return DefWindowProcW(h,m,w,l);
}
#ifdef KRM_TEST_BUILD
static LRESULT CALLBACK SelfTestWndProc(HWND h,UINT m,WPARAM w,LPARAM l) {
 if(m==WM_KR_CAPTURE) {
  gSelfCapturedCode=(int)w;
  gSelfCapturedMods=(unsigned)((ULONG_PTR)l>>1);
  return 0;
 }
 return DefWindowProcW(h,m,w,l);
}
static int SelfTest() {
 gSelfTestMode=true;
 WNDCLASSEXW wc{sizeof(wc)};
 wc.hInstance=gInst; wc.lpfnWndProc=SelfTestWndProc; wc.lpszClassName=L"KRNativeSelfTest";
 RegisterClassExW(&wc);
 gWnd=CreateWindowExW(0,wc.lpszClassName,L"",WS_OVERLAPPED,0,0,715,525,nullptr,nullptr,gInst,nullptr);
 if(!gWnd) return 20;

 // Tray window lifecycle helpers: hide must keep the window/process alive, show must restore it.
 CreateUi(gWnd);
 ShowWindow(gWnd,SW_SHOW);
 HideDashboard();
 bool hideOk=!IsWindowVisible(gWnd);
 ShowDashboard();
 bool showOk=IsWindowVisible(gWnd)!=FALSE;
 if(!(hideOk&&showOk)) { DestroyWindow(gWnd); gWnd=nullptr; gSelfTestMode=false; return 31; }

 // Explicit owner-draw toggle state must survive without BM_GETCHECK.
 SetModChecks(KRM_SHIFT,gSrcShift,gSrcCtrl,gSrcAlt,gSrcWin);
 bool modifierStateOk=(CheckedMods(gSrcShift,gSrcCtrl,gSrcAlt,gSrcWin)&KRM_SHIFT)!=0;
 SetModChecks(0,gSrcShift,gSrcCtrl,gSrcAlt,gSrcWin);
 if(!modifierStateOk) { DestroyWindow(gWnd); gWnd=nullptr; gSelfTestMode=false; return 32; }

 // UI-selection test: choose @ directly in the Target combo, then Add/Update.
 SelectChoice(gSource,gSourceChoices,VK_OEM_3,0);
 SelectChoice(gTarget,gTargetChoices,'2',KRM_SHIFT);
 SetModChecks(0,gSrcShift,gSrcCtrl,gSrcAlt,gSrcWin);
 SetModChecks(0,gDstShift,gDstCtrl,gDstAlt,gDstWin);
 SetBlockSourceState(false);
 gMaps.clear();
 AddMap();
 wchar_t listTarget[64]{};
 if(ListView_GetItemCount(gList)>0) ListView_GetItemText(gList,0,1,listTarget,64);
 bool uiSelectionOk=(gMaps.size()==1 && gMaps[0].target=='2' &&
                     (gMaps[0].dstMods&KRM_SHIFT)!=0 && wcscmp(listTarget,L"@")==0);
 if(!uiSelectionOk) { DestroyWindow(gWnd); gWnd=nullptr; gSelfTestMode=false; return 24; }

 // Regression for real UI case: an existing source mapping must be replaced by Add mapping.
 gMaps.clear();
 gMaps.push_back({VK_OEM_3,'2',false,0,0});
 Refresh();
 SelectChoice(gSource,gSourceChoices,VK_OEM_3,0);
 SelectChoice(gTarget,gTargetChoices,'2',KRM_SHIFT);
 SetModChecks(0,gSrcShift,gSrcCtrl,gSrcAlt,gSrcWin);
 SetModChecks(0,gDstShift,gDstCtrl,gDstAlt,gDstWin);
 SetBlockSourceState(false);
 AddMap();
 wchar_t upsertTarget[64]{};
 if(ListView_GetItemCount(gList)>0) ListView_GetItemText(gList,0,1,upsertTarget,64);
 bool upsertOk=(gMaps.size()==1 && gMaps[0].target=='2' &&
                (gMaps[0].dstMods&KRM_SHIFT)!=0 && wcscmp(upsertTarget,L"@")==0);
 if(!upsertOk) { DestroyWindow(gWnd); gWnd=nullptr; gSelfTestMode=false; return 28; }

 // Persistence regression: reload, recover corrupt primary, and preserve last-known-good backup.
 if(!Save()) { DestroyWindow(gWnd); gWnd=nullptr; gSelfTestMode=false; return 29; }
 gMaps.clear();
 if(!Load()) { DestroyWindow(gWnd); gWnd=nullptr; gSelfTestMode=false; return 29; }
 bool reloadOk=(gMaps.size()==1 && gMaps[0].target=='2' && (gMaps[0].dstMods&KRM_SHIFT)!=0);
 if(!reloadOk) { DestroyWindow(gWnd); gWnd=nullptr; gSelfTestMode=false; return 29; }

 CopyFileW(ConfigPath().c_str(),ConfigBackupPath().c_str(),FALSE);
 {
  std::wofstream bad(ConfigPath(),std::ios::trunc);
  bad<<L"this is corrupt";
 }
 gMaps.clear();
 bool recovered=Load();
 bool recoveryOk=recovered && gMaps.size()==1 && gMaps[0].target=='2' &&
                 (gMaps[0].dstMods&KRM_SHIFT)!=0;
 if(!recoveryOk) { DestroyWindow(gWnd); gWnd=nullptr; gSelfTestMode=false; return 30; }

 // A corrupt primary must never overwrite a healthy backup during the next save.
 {
  std::wofstream bad(ConfigPath(),std::ios::trunc);
  bad<<L"still corrupt";
 }
 gMaps.clear();
 gMaps.push_back({'C','D',false,0,0});
 if(!Save()) { DestroyWindow(gWnd); gWnd=nullptr; gSelfTestMode=false; return 34; }
 std::vector<Mapping> backupMaps;
 bool backupPreserved=ReadMappingsFile(ConfigBackupPath(),backupMaps)==ConfigReadResult::Valid &&
                      backupMaps.size()==1 && backupMaps[0].target=='2' &&
                      (backupMaps[0].dstMods&KRM_SHIFT)!=0;
 if(!backupPreserved) { DestroyWindow(gWnd); gWnd=nullptr; gSelfTestMode=false; return 34; }

 gMaps=backupMaps;
 WriteMappingsFile(ConfigPath(),gMaps);
 Refresh();

 // Modify regression: select the row, load it, change target to #, then save.
 ListView_SetItemState(gList,0,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
 ModifySelected();
 bool modifyLoaded=(gEditingIndex==0);
 SelectChoice(gTarget,gTargetChoices,'3',KRM_SHIFT);
 SetModChecks(0,gDstShift,gDstCtrl,gDstAlt,gDstWin);
 AddMap();
 wchar_t modifiedTarget[64]{};
 if(ListView_GetItemCount(gList)>0) ListView_GetItemText(gList,0,1,modifiedTarget,64);
 bool modifyOk=modifyLoaded && gMaps.size()==1 && gMaps[0].target=='3' &&
               (gMaps[0].dstMods&KRM_SHIFT)!=0 && wcscmp(modifiedTarget,L"#")==0;
 if(!modifyOk) { DestroyWindow(gWnd); gWnd=nullptr; gSelfTestMode=false; return 25; }

 // Wheel regression: one 120 detent = one item; two 60 deltas = one item total.
 ComboBox_SetCurSel(gTarget,0); gTargetWheel.delta=0;
 int wheelStart=ComboBox_GetCurSel(gTarget);
 SendMessageW(gTarget,WM_MOUSEWHEEL,MAKEWPARAM(0,(WORD)-WHEEL_DELTA),0);
 bool wheelFull=(ComboBox_GetCurSel(gTarget)==wheelStart+1);
 ComboBox_SetCurSel(gTarget,0); gTargetWheel.delta=0;
 SendMessageW(gTarget,WM_MOUSEWHEEL,MAKEWPARAM(0,(WORD)-60),0);
 bool wheelHalfFirst=(ComboBox_GetCurSel(gTarget)==0);
 SendMessageW(gTarget,WM_MOUSEWHEEL,MAKEWPARAM(0,(WORD)-60),0);
 bool wheelHalfSecond=(ComboBox_GetCurSel(gTarget)==1);
 if(!(wheelFull&&wheelHalfFirst&&wheelHalfSecond)) {
  DestroyWindow(gWnd); gWnd=nullptr; gSelfTestMode=false; return 26;
 }

 // Startup regression: toggle HKCU Run and restore the original state.
 bool startupInitial=StartupEnabled();
 bool startupOn=SetStartupEnabled(true) && StartupEnabled();
 bool startupCommandOk=StartupCommandValue().find(L"--startup")!=std::wstring::npos;
 bool startupOff=SetStartupEnabled(false) && !StartupEnabled();
 bool startupRestore=SetStartupEnabled(startupInitial);
 if(!(startupOn&&startupCommandOk&&startupOff&&startupRestore)) {
  DestroyWindow(gWnd); gWnd=nullptr; gSelfTestMode=false; return 27;
 }
 gRunAtStartup=startupInitial; if(gStartup) InvalidateRect(gStartup,nullptr,TRUE);

 if(!InstallHooks()) { DestroyWindow(gWnd); return 21; }

 gSelfCapturedCode=0; gSelfCapturedMods=0; gPhysicalMods=0; gCaptureSource=true; gCaptureTarget=false;
 SendTestKey(VK_SHIFT,true);
 SendTestKey('2',true);
 SendTestKey('2',false);
 SendTestKey(VK_SHIFT,false);

 DWORD until=GetTickCount()+1000;
 MSG msg;
 while(GetTickCount()<until && gSelfCapturedCode==0) {
  while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)) {
   TranslateMessage(&msg); DispatchMessageW(&msg);
  }
  Sleep(5);
 }
 auto isShiftVk=[](int vk){return vk==VK_SHIFT||vk==VK_LSHIFT||vk==VK_RSHIFT;};
  bool captureOk=(gSelfCapturedCode=='2' && (gSelfCapturedMods&KRM_SHIFT)!=0);
 if(!captureOk) {
  Unhook(); DestroyWindow(gWnd); gWnd=nullptr; gSelfTestMode=false; return 22;
 }

 // Modifier-only capture must complete on modifier release without leaking the key to Windows.
 gSelfCapturedCode=0; gSelfCapturedMods=0; gPhysicalMods=0;
 gCaptureSource=true; gCaptureTarget=false; gCaptureModifierCandidate=0; gCaptureSuppressedMods=0;
 SendTestKey(VK_LSHIFT,true);
 SendTestKey(VK_LSHIFT,false);
 until=GetTickCount()+1000;
 while(GetTickCount()<until && gSelfCapturedCode==0) {
  while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
  Sleep(5);
 }
 bool modifierCaptureOk=isShiftVk(gSelfCapturedCode) && gSelfCapturedMods==0;
 if(!modifierCaptureOk) {
  Unhook(); DestroyWindow(gWnd); gWnd=nullptr; gSelfTestMode=false; return 35;
 }

 // Escape is a valid capturable key, not an implicit cancel key.
 gSelfCapturedCode=0; gSelfCapturedMods=0; gPhysicalMods=0;
 gCaptureSource=false; gCaptureTarget=true; gCaptureSwallow=0;
 SendTestKey(VK_ESCAPE,true);
 SendTestKey(VK_ESCAPE,false);
 until=GetTickCount()+1000;
 while(GetTickCount()<until && gSelfCapturedCode==0) {
  while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
  Sleep(5);
 }
 if(gSelfCapturedCode!=VK_ESCAPE) {
  Unhook(); DestroyWindow(gWnd); gWnd=nullptr; gSelfTestMode=false; return 36;
 }

 // Runtime emission test: A must produce Shift+2, not bare 2.
 gCaptureSource=gCaptureTarget=false; gCaptureSwallow=0; gPhysicalMods=0; gPaused=false;
 gMaps.clear(); gMaps.push_back({'A','2',false,0,KRM_SHIFT});
 gSelfOutputEvents.clear(); gSelfObserveOutput=true;
 SendTestKey('A',true);
 SendTestKey('A',false);
 until=GetTickCount()+1000;
 while(GetTickCount()<until && gSelfOutputEvents.size()<4) {
  while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)) {
   TranslateMessage(&msg); DispatchMessageW(&msg);
  }
  Sleep(5);
 }
 gSelfObserveOutput=false;
 bool outputOk=false;
 for(size_t i=0;i+3<gSelfOutputEvents.size();++i) {
  if(isShiftVk(gSelfOutputEvents[i].vk) && gSelfOutputEvents[i].down &&
     gSelfOutputEvents[i+1].vk=='2' && gSelfOutputEvents[i+1].down &&
     gSelfOutputEvents[i+2].vk=='2' && !gSelfOutputEvents[i+2].down &&
     isShiftVk(gSelfOutputEvents[i+3].vk) && !gSelfOutputEvents[i+3].down) {
    outputOk=true; break;
  }
 }
 if(!outputOk) {
  Unhook(); DestroyWindow(gWnd); gWnd=nullptr; gSelfTestMode=false; return 23;
 }

 // Disabling remapping while a mapped key is held must release its synthetic output.
 gMaps.clear(); gMaps.push_back({'B','2',false,0,KRM_SHIFT});
 gSelfOutputEvents.clear(); gSelfObserveOutput=true; gPaused=false;
 SendTestKey('B',true);
 until=GetTickCount()+1000;
 while(GetTickCount()<until && gSelfOutputEvents.size()<2) {
  while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
  Sleep(5);
 }
 SetRemappingEnabled(false);
 SendTestKey('B',false);
 until=GetTickCount()+1000;
 while(GetTickCount()<until && gSelfOutputEvents.size()<4) {
  while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
  Sleep(5);
 }
 gSelfObserveOutput=false;
 bool releasedOk=gPaused && !gActive['B'].target;
 bool sawRelease=false;
 for(size_t i=0;i+3<gSelfOutputEvents.size();++i) {
  if(isShiftVk(gSelfOutputEvents[i].vk) && gSelfOutputEvents[i].down &&
     gSelfOutputEvents[i+1].vk=='2' && gSelfOutputEvents[i+1].down &&
     gSelfOutputEvents[i+2].vk=='2' && !gSelfOutputEvents[i+2].down &&
     isShiftVk(gSelfOutputEvents[i+3].vk) && !gSelfOutputEvents[i+3].down) {
    sawRelease=true; break;
  }
 }
 SetRemappingEnabled(true);
 if(!(releasedOk&&sawRelease)) {
  Unhook(); DestroyWindow(gWnd); gWnd=nullptr; gSelfTestMode=false; return 33;
 }

 // A modifier key can be a source without requiring itself in srcMods.
 gMaps.clear(); gMaps.push_back({VK_LSHIFT,'A',false,0,0});
 gPhysicalMods=0; gPaused=false; gSelfOutputEvents.clear(); gSelfObserveOutput=true;
 SendTestKey(VK_LSHIFT,true);
 SendTestKey(VK_LSHIFT,false);
 until=GetTickCount()+1000;
 while(GetTickCount()<until && gSelfOutputEvents.size()<2) {
  while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
  Sleep(5);
 }
 gSelfObserveOutput=false;
 bool modifierSourceOk=false;
 for(size_t i=0;i+1<gSelfOutputEvents.size();++i) {
  if(gSelfOutputEvents[i].vk=='A' && gSelfOutputEvents[i].down &&
     gSelfOutputEvents[i+1].vk=='A' && !gSelfOutputEvents[i+1].down) {
    modifierSourceOk=true; break;
  }
 }
 if(!modifierSourceOk) {
  Unhook(); DestroyWindow(gWnd); gWnd=nullptr; gSelfTestMode=false; return 37;
 }

 // Releasing a key that was pressed while remapping was disabled must not synthesize a target.
 gMaps.clear(); gMaps.push_back({'C','D',false,0,0});
 SetRemappingEnabled(false); gSelfOutputEvents.clear(); gSelfObserveOutput=true;
 SendTestKey('C',true);
 until=GetTickCount()+100;
 while(GetTickCount()<until) {
  while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
  Sleep(2);
 }
 SetRemappingEnabled(true);
 SendTestKey('C',false);
 until=GetTickCount()+250;
 while(GetTickCount()<until) {
  while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
  Sleep(2);
 }
 gSelfObserveOutput=false;
 bool unexpectedTarget=false;
 for(const auto&e:gSelfOutputEvents) if(e.vk=='D') unexpectedTarget=true;
 if(unexpectedTarget) {
  Unhook(); DestroyWindow(gWnd); gWnd=nullptr; gSelfTestMode=false; return 38;
 }

 Unhook();
 DestroyWindow(gWnd); gWnd=nullptr; gSelfTestMode=false;
 return 0;
}
#endif
int WINAPI wWinMain(HINSTANCE hi,HINSTANCE,PWSTR cmd,int show) {
 gInst=hi;
#ifdef KRM_TEST_BUILD
 return SelfTest();
#else
 gStartupMode=wcsstr(cmd,L"--startup")!=nullptr;
 gTaskbarCreatedMessage=RegisterWindowMessageW(L"TaskbarCreated");

 gInstanceMutex=CreateMutexW(nullptr,TRUE,L"Local\\KeyRemapper.SingleInstance");
 if(!gInstanceMutex) return 3;
 if(GetLastError()==ERROR_ALREADY_EXISTS) {
  if(!gStartupMode) {
   HWND existing=FindWindowW(L"KRNative",nullptr);
   if(existing) {
    ShowWindow(existing,SW_SHOW);
    if(IsIconic(existing)) ShowWindow(existing,SW_RESTORE);
    SetForegroundWindow(existing);
   }
  }
  CloseHandle(gInstanceMutex);gInstanceMutex=nullptr;
  return 0;
 }

 SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
 gDpi=GetDpiForSystem();
 INITCOMMONCONTROLSEX ic{sizeof(ic),ICC_LISTVIEW_CLASSES};InitCommonControlsEx(&ic);
 gConfigLoadFailed=!Load();

 gIconLarge=(HICON)LoadImageW(hi,MAKEINTRESOURCEW(101),IMAGE_ICON,32,32,LR_DEFAULTCOLOR);
 gIconSmall=(HICON)LoadImageW(hi,MAKEINTRESOURCEW(101),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);

 WNDCLASSEXW wc{sizeof(wc)};
 wc.hInstance=hi;wc.lpfnWndProc=WndProc;wc.lpszClassName=L"KRNative";wc.hCursor=LoadCursorW(nullptr,IDC_ARROW);
 wc.hIcon=gIconLarge?gIconLarge:LoadIconW(nullptr,IDI_APPLICATION);
 wc.hIconSm=gIconSmall?gIconSmall:LoadIconW(nullptr,IDI_APPLICATION);
 wc.hbrBackground=CreateSolidBrush(C_BG);
 RegisterClassExW(&wc);

 gWnd=CreateWindowExW(0,wc.lpszClassName,L"Key Remapper",WS_OVERLAPPEDWINDOW,
  CW_USEDEFAULT,CW_USEDEFAULT,S(840),S(600),nullptr,nullptr,hi,nullptr);
 if(!gWnd)return 2;
 if(gStartupMode && gTrayAdded) ShowWindow(gWnd,SW_HIDE);
 else ShowWindow(gWnd,show);
 UpdateWindow(gWnd);
 MSG msg;while(GetMessageW(&msg,nullptr,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}
 return (int)msg.wParam;
#endif
}