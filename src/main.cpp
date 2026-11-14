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

struct SymbolItem { int code; const wchar_t* name; };
static const SymbolItem SYMBOL_ITEMS[] = {
 {'1',L"!"},{'2',L"@"},{'3',L"#"},{'4',L"$"},{'5',L"%"},{'6',L"^"},
 {'7',L"&"},{'8',L"*"},{'9',L"("},{'0',L")"},
 {VK_OEM_MINUS,L"_"},{VK_OEM_PLUS,L"+"},{VK_OEM_4,L"{"},{VK_OEM_6,L"}"},
 {VK_OEM_5,L"|"},{VK_OEM_1,L":"},{VK_OEM_7,L"\""},{VK_OEM_COMMA,L"<"},
 {VK_OEM_PERIOD,L">"},{VK_OEM_2,L"?"},{VK_OEM_3,L"~"}
};
struct Mapping { int source; int target; bool block; unsigned srcMods=0; unsigned dstMods=0; };
struct Choice { int code=0; unsigned mods=0; };
static std::vector<Choice> gSourceChoices;
static std::vector<Choice> gTargetChoices;
static HINSTANCE gInst;
static HWND gWnd, gSource, gTarget, gBlock, gList, gPause, gStatus, gStartup;
static HWND gAdd, gModify, gRemove, gTitle, gSubtitle, gSection, gEmpty, gListSource, gListTarget, gIconControl;
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
static unsigned gSourceUiMods=0;
static unsigned gTargetUiMods=0;

struct ComboWheelState { HWND combo=nullptr; int delta=0; };
static ComboWheelState gSourceWheel, gTargetWheel;
static HHOOK gKeyboard, gMouse;
static bool gPaused=false, gCaptureSource=false, gCaptureTarget=false, gSelfTestMode=false;
static int gCaptureSwallow=0;
static unsigned gPhysicalMods=0;
static int gSelfCapturedCode=0;
static unsigned gSelfCapturedMods=0;
struct TestEvent { int vk; bool down; };
static std::vector<TestEvent> gSelfOutputEvents;
static bool gSelfObserveOutput=false;
constexpr ULONG_PTR TEST_MAGIC = 0x4B525453;
static std::vector<Mapping> gMaps;
struct ActiveEmission { int target=0; unsigned synthetic=0; unsigned suppressed=0; };
static std::vector<ActiveEmission> gActive(0x1200);

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
static const wchar_t* ShiftSymbol(int code) {
 switch(code) {
  case '1': return L"!"; case '2': return L"@"; case '3': return L"#"; case '4': return L"$";
  case '5': return L"%"; case '6': return L"^"; case '7': return L"&"; case '8': return L"*";
  case '9': return L"("; case '0': return L")"; case VK_OEM_1: return L":";
  case VK_OEM_PLUS: return L"+"; case VK_OEM_COMMA: return L"<"; case VK_OEM_MINUS: return L"_";
  case VK_OEM_PERIOD: return L">"; case VK_OEM_2: return L"?"; case VK_OEM_3: return L"~";
  case VK_OEM_4: return L"{"; case VK_OEM_5: return L"|"; case VK_OEM_6: return L"}";
  case VK_OEM_7: return L"\"";
  default: return nullptr;
 }
}
static std::wstring Describe(int code,unsigned mods) {
 std::wstring out;
 if(mods&KRM_CTRL) out+=L"Ctrl+";
 if(mods&KRM_ALT) out+=L"Alt+";
 if(mods&KRM_WIN) out+=L"Win+";
 const wchar_t* symbol=((mods&KRM_SHIFT)?ShiftSymbol(code):nullptr);
 if(symbol) out+=symbol;
 else {
  if(mods&KRM_SHIFT) out+=L"Shift+";
  out+=NameOf(code);
 }
 return out;
}
static bool Extended(int vk) {
 switch(vk) {
  case VK_RMENU:case VK_RCONTROL:case VK_INSERT:case VK_DELETE:case VK_HOME:case VK_END:
  case VK_PRIOR:case VK_NEXT:case VK_LEFT:case VK_RIGHT:case VK_UP:case VK_DOWN:
  case VK_NUMLOCK:case VK_SNAPSHOT:case VK_DIVIDE:case VK_LWIN:case VK_RWIN:case VK_APPS:return true;
 } return false;
}
static void SendCode(int code,bool down) {
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
   case WHEEL_UP:if(!down)return;in.mi.dwFlags=MOUSEEVENTF_WHEEL;in.mi.mouseData=WHEEL_DELTA;break;
   case WHEEL_DOWN:if(!down)return;in.mi.dwFlags=MOUSEEVENTF_WHEEL;in.mi.mouseData=(DWORD)-WHEEL_DELTA;break;
   default:return;
  }
 }
 SendInput(1,&in,sizeof(in));
}
static void SendTestKey(int vk,bool down) {
 INPUT in{};
 in.type=INPUT_KEYBOARD;
 in.ki.wVk=(WORD)vk;
 in.ki.dwExtraInfo=TEST_MAGIC;
 if(!down) in.ki.dwFlags|=KEYEVENTF_KEYUP;
 SendInput(1,&in,sizeof(in));
}
static ActiveEmission BeginTarget(const Mapping& m) {
 ActiveEmission a{}; a.target=m.target;
 unsigned all[]={KRM_SHIFT,KRM_CTRL,KRM_ALT,KRM_WIN};
 for(unsigned bit:all) {
  if((gPhysicalMods&bit) && !(m.dstMods&bit)) {
   SendCode(VkForMod(bit),false); a.suppressed|=bit;
  }
 }
 for(unsigned bit:all) {
  if((m.dstMods&bit) && !(gPhysicalMods&bit)) {
   SendCode(VkForMod(bit),true); a.synthetic|=bit;
  }
 }
 SendCode(m.target,true);
 return a;
}
static void EndTarget(const ActiveEmission& a) {
 if(!a.target)return;
 SendCode(a.target,false);
 unsigned all[]={KRM_WIN,KRM_ALT,KRM_CTRL,KRM_SHIFT};
 for(unsigned bit:all) if(a.synthetic&bit) SendCode(VkForMod(bit),false);
 for(unsigned bit:all) if((a.suppressed&bit)&&(gPhysicalMods&bit)) SendCode(VkForMod(bit),true);
}
static Mapping* FindMap(int src) {
 if(gPaused) return nullptr;
 for(auto &m:gMaps) if(m.source==src && m.srcMods==gPhysicalMods) return &m;
 return nullptr;
}
static bool Handle(int src,bool down,bool pulse=false) {
 unsigned modBit=ModBitForVk(src);
 if(gCaptureSource||gCaptureTarget) {
  if(modBit) return true;
  if(down||pulse) {
   bool sourceCapture=gCaptureSource; gCaptureSource=gCaptureTarget=false; gCaptureSwallow=pulse?0:src;
   LPARAM packed=(LPARAM)((gPhysicalMods<<1)|(sourceCapture?0:1));
   PostMessageW(gWnd,WM_KR_CAPTURE,src,packed); return true;
  }
  return false;
 }
 if(gCaptureSwallow==src) { if(!down)gCaptureSwallow=0; return true; }
 Mapping* m=FindMap(src); if(!m)return false; if(m->block)return true;
 if(pulse) {
  ActiveEmission a=BeginTarget(*m);
  EndTarget(a);
  return true;
 }
 if(src>=0 && src<(int)gActive.size()) {
  if(down) {
   if(!gActive[src].target) gActive[src]=BeginTarget(*m);
   else if(m->target<0x1000) SendCode(m->target,true);
  } else {
   if(gActive[src].target) { EndTarget(gActive[src]); gActive[src]=ActiveEmission{}; }
   else { ActiveEmission a=BeginTarget(*m); EndTarget(a); }
  }
 } else {
  if(down) { ActiveEmission a=BeginTarget(*m); gActive[0]=a; }
 }
 return true;
}
static LRESULT CALLBACK KeyHook(int n,WPARAM w,LPARAM l) {
 if(n<0)return CallNextHookEx(gKeyboard,n,w,l);
 auto*k=(KBDLLHOOKSTRUCT*)l;
 if(k->dwExtraInfo==KR_MAGIC) {
  if(gSelfTestMode && gSelfObserveOutput) {
   bool od=w==WM_KEYDOWN||w==WM_SYSKEYDOWN;
   bool ou=w==WM_KEYUP||w==WM_SYSKEYUP;
   if(od||ou) gSelfOutputEvents.push_back({(int)k->vkCode,od});
  }
  return CallNextHookEx(gKeyboard,n,w,l);
 }
 if((k->flags&LLKHF_INJECTED) && !gSelfTestMode)
  return CallNextHookEx(gKeyboard,n,w,l);
 bool d=w==WM_KEYDOWN||w==WM_SYSKEYDOWN,u=w==WM_KEYUP||w==WM_SYSKEYUP;
 if(!d&&!u) return CallNextHookEx(gKeyboard,n,w,l);
 unsigned bit=ModBitForVk((int)k->vkCode);
 if(bit) {
  if(d) gPhysicalMods|=bit; else gPhysicalMods&=~bit;
 }
 if(Handle((int)k->vkCode,d))return 1;
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
 if(c&&Handle(c,d,p))return 1;
 return CallNextHookEx(gMouse,n,w,l);
}
static bool InstallHooks() {
 gKeyboard=SetWindowsHookExW(WH_KEYBOARD_LL,KeyHook,gInst,0);
 gMouse=SetWindowsHookExW(WH_MOUSE_LL,MouseHook,gInst,0);
 return gKeyboard&&gMouse;
}
static void ReleaseActiveOutputs() {
 for(size_t i=0;i<gActive.size();++i) {
  if(gActive[i].target) EndTarget(gActive[i]);
  gActive[i]=ActiveEmission{};
 }
}
static void SetRemappingEnabled(bool enabled) {
 if(!enabled) ReleaseActiveOutputs();
 gPaused=!enabled;
}
static void Unhook() {
 ReleaseActiveOutputs();
 if(gKeyboard)UnhookWindowsHookEx(gKeyboard);
 if(gMouse)UnhookWindowsHookEx(gMouse);
 gKeyboard=gMouse=nullptr;
}
static std::wstring ConfigDir() {
 wchar_t p[MAX_PATH]{};
 SHGetFolderPathW(nullptr,CSIDL_APPDATA,nullptr,SHGFP_TYPE_CURRENT,p);
 std::wstring d=std::wstring(p)+L"\\KeyRemapperClean";
 CreateDirectoryW(d.c_str(),nullptr);
 return d;
}
static std::wstring ConfigPath() { return ConfigDir()+L"\\mappings.txt"; }
static std::wstring ConfigBackupPath() { return ConfigDir()+L"\\mappings.bak"; }
static std::wstring ConfigTempPath() { return ConfigDir()+L"\\mappings.tmp"; }
static const wchar_t* STARTUP_RUN_KEY=L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static const wchar_t* STARTUP_VALUE=L"KeyRemapper";

static bool StartupEnabled() {
 HKEY key=nullptr;
 if(RegOpenKeyExW(HKEY_CURRENT_USER,STARTUP_RUN_KEY,0,KEY_QUERY_VALUE,&key)!=ERROR_SUCCESS) return false;
 DWORD type=0,size=0;
 LONG result=RegQueryValueExW(key,STARTUP_VALUE,nullptr,&type,nullptr,&size);
 RegCloseKey(key);
 return result==ERROR_SUCCESS && type==REG_SZ && size>sizeof(wchar_t);
}
static bool SetStartupEnabled(bool enabled) {
 HKEY key=nullptr;
 if(RegCreateKeyExW(HKEY_CURRENT_USER,STARTUP_RUN_KEY,0,nullptr,0,KEY_SET_VALUE,nullptr,&key,nullptr)!=ERROR_SUCCESS) return false;
 LONG result=ERROR_SUCCESS;
 if(enabled) {
  wchar_t exe[MAX_PATH]{};
  if(!GetModuleFileNameW(nullptr,exe,MAX_PATH)) { RegCloseKey(key); return false; }
  std::wstring command=L"\""+std::wstring(exe)+L"\" --startup";
  result=RegSetValueExW(key,STARTUP_VALUE,0,REG_SZ,
    reinterpret_cast<const BYTE*>(command.c_str()),(DWORD)((command.size()+1)*sizeof(wchar_t)));
 } else {
  result=RegDeleteValueW(key,STARTUP_VALUE);
  if(result==ERROR_FILE_NOT_FOUND) result=ERROR_SUCCESS;
 }
 RegCloseKey(key);
 return result==ERROR_SUCCESS;
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
static bool ParseMappingsFile(const std::wstring& path,std::vector<Mapping>& out) {
 std::wifstream f(path);
 if(!f)return false;
 std::wstring line;
 while(std::getline(f,line)) {
  if(line.empty())continue;
  std::wistringstream in(line);
  int src=0,dst=0,block=0; unsigned sm=0,dm=0;
  if(!(in>>src>>dst>>block))continue;
  in>>sm>>dm;
  out.push_back({src,dst,block!=0,sm,dm});
 }
 return true;
}
static void Save() {
 const std::wstring path=ConfigPath();
 const std::wstring temp=ConfigTempPath();
 const std::wstring backup=ConfigBackupPath();

 {
  std::wofstream f(temp,std::ios::trunc);
  if(!f)return;
  for(auto&m:gMaps) f<<m.source<<L' '<<m.target<<L' '<<(m.block?1:0)<<L' '<<m.srcMods<<L' '<<m.dstMods<<L'\n';
  f.flush();
  if(!f)return;
 }

 if(GetFileAttributesW(path.c_str())!=INVALID_FILE_ATTRIBUTES)
  CopyFileW(path.c_str(),backup.c_str(),FALSE);

 if(!MoveFileExW(temp.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))
  DeleteFileW(temp.c_str());
}
static void Load() {
 gMaps.clear();
 const std::wstring primary=ConfigPath();
 const std::wstring backup=ConfigBackupPath();

 if(ParseMappingsFile(primary,gMaps)) return;

 std::vector<Mapping> recovered;
 if(ParseMappingsFile(backup,recovered)) {
  gMaps=std::move(recovered);
  CopyFileW(backup.c_str(),primary.c_str(),FALSE);
 }
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
 choices.reserve((sizeof(ITEMS)/sizeof(ITEMS[0]))+(sizeof(SYMBOL_ITEMS)/sizeof(SYMBOL_ITEMS[0])));
 for(auto&i:ITEMS) {
  ComboBox_AddString(h,i.name);
  choices.push_back({i.code,0});
 }
 for(auto&i:SYMBOL_ITEMS) {
  ComboBox_AddString(h,i.name);
  choices.push_back({i.code,KRM_SHIFT});
 }
}
static unsigned CheckedMods(HWND sh,HWND ctrl,HWND alt,HWND win) {
 unsigned m=0;
 if(Button_GetCheck(sh)==BST_CHECKED)m|=KRM_SHIFT;
 if(Button_GetCheck(ctrl)==BST_CHECKED)m|=KRM_CTRL;
 if(Button_GetCheck(alt)==BST_CHECKED)m|=KRM_ALT;
 if(Button_GetCheck(win)==BST_CHECKED)m|=KRM_WIN;
 return m;
}
static void SetModChecks(unsigned m,HWND sh,HWND ctrl,HWND alt,HWND win) {
 Button_SetCheck(sh,(m&KRM_SHIFT)?BST_CHECKED:BST_UNCHECKED);
 Button_SetCheck(ctrl,(m&KRM_CTRL)?BST_CHECKED:BST_UNCHECKED);
 Button_SetCheck(alt,(m&KRM_ALT)?BST_CHECKED:BST_UNCHECKED);
 Button_SetCheck(win,(m&KRM_WIN)?BST_CHECKED:BST_UNCHECKED);
}
static unsigned PreferredImpliedMods(int code,unsigned mods) {
 return ((mods&KRM_SHIFT) && ShiftSymbol(code)) ? KRM_SHIFT : 0;
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
 Button_SetCheck(gBlock,m.block?BST_CHECKED:BST_UNCHECKED);
 EnableWindow(gTarget,!m.block);
 EnableWindow(gDstShift,!m.block);
 EnableWindow(gDstCtrl,!m.block);
 EnableWindow(gDstAlt,!m.block);
 EnableWindow(gDstWin,!m.block);
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
 if(!gKeyboard || !gMouse)
  wsprintfW(st,L"Input hooks unavailable  |  %u %s",count,unit);
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
 RoundRect(dc,r.left,r.top,r.right,r.bottom,radius,radius);
 SelectObject(dc,oldBrush);
 SelectObject(dc,oldPen);
 DeleteObject(brush);
 DeleteObject(pen);
}

static void DrawToggleChip(const DRAWITEMSTRUCT* d) {
 RECT r=d->rcItem;
 bool checked=Button_GetCheck(d->hwndItem)==BST_CHECKED;
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
 bool checked=Button_GetCheck(d->hwndItem)==BST_CHECKED;
 bool disabled=(d->itemState&ODS_DISABLED)!=0;
 int trackW=34,trackH=18;
 RECT track{r.left,r.top+(r.bottom-r.top-trackH)/2,r.left+trackW,r.top+(r.bottom-r.top+trackH)/2};
 COLORREF trackFill=checked?C_ACCENT:RGB(207,211,219);
 if(disabled) trackFill=RGB(225,227,232);
 DrawRoundedRect(d->hDC,track,trackFill,trackFill,trackH);

 int knob=14;
 int knobX=checked?track.right-knob-2:track.left+2;
 RECT k{knobX,track.top+2,knobX+knob,track.top+2+knob};
 HBRUSH kb=CreateSolidBrush(RGB(255,255,255));
 HPEN kp=CreatePen(PS_SOLID,1,RGB(255,255,255));
 HGDIOBJ ob=SelectObject(d->hDC,kb),op=SelectObject(d->hDC,kp);
 Ellipse(d->hDC,k.left,k.top,k.right,k.bottom);
 SelectObject(d->hDC,ob);SelectObject(d->hDC,op);DeleteObject(kb);DeleteObject(kp);

 wchar_t label[96]{};
 GetWindowTextW(d->hwndItem,label,96);
 RECT tr=r;tr.left=track.right+10;
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
 r.left+=10;r.right-=8;
 SetBkMode(d->hDC,TRANSPARENT);
 SetTextColor(d->hDC,C_TEXT);
 HFONT old=(HFONT)SelectObject(d->hDC,gFontBody?gFontBody:GetStockObject(DEFAULT_GUI_FONT));
 DrawTextW(d->hDC,buf,-1,&r,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
 SelectObject(d->hDC,old);
}

static void LayoutUi(int width,int height) {
 if(!gList) return;
 const int margin=20;
 const int cardLeft=20;
 const int listTop=336;
 const int footerY=height-44;
 const int listBottom=std::max(listTop+118,footerY-16);
 const int listWidth=std::max(610,width-margin*2);
 const int half=listWidth/2;

 if(gListSource) MoveWindow(gListSource,cardLeft+12,310,half-18,20,TRUE);
 if(gListTarget) MoveWindow(gListTarget,cardLeft+half+6,310,half-18,20,TRUE);
 MoveWindow(gList,cardLeft,listTop,listWidth,listBottom-listTop,TRUE);

 if(gEmpty) {
  int emptyW=std::min(380,listWidth-40);
  MoveWindow(gEmpty,cardLeft+(listWidth-emptyW)/2,listTop+38,emptyW,28,TRUE);
 }
 if(gStartup) MoveWindow(gStartup,margin,footerY,156,28,TRUE);
 if(gStatus) MoveWindow(gStatus,198,footerY+4,std::max(210,width-198-120),20,TRUE);
 if(gPause) MoveWindow(gPause,width-106,footerY-2,86,32,TRUE);

 ListView_SetColumnWidth(gList,0,half);
 ListView_SetColumnWidth(gList,1,listWidth-half);
}

static void CreateUi(HWND h) {
 EnsureUiResources(h);
 BOOL dark=FALSE;
 DwmSetWindowAttribute(h,20,&dark,sizeof(dark));

 gIconControl=Make(h,L"STATIC",L"",SS_ICON|SS_REALSIZECONTROL,20,15,32,32,0);
 if(gIconLarge) SendMessageW(gIconControl,STM_SETICON,(WPARAM)gIconLarge,0);

 gTitle=Make(h,L"STATIC",L"Key Remapper",SS_LEFT,62,13,300,28,ID_TITLE);
 gSubtitle=Make(h,L"STATIC",L"Remap keyboard and mouse input.",SS_LEFT,62,39,420,20,ID_SUBTITLE);

 gSection=Make(h,L"STATIC",L"Create mapping",SS_LEFT,32,82,180,22,ID_SECTION);

 Make(h,L"STATIC",L"Source",SS_LEFT,32,122,56,20,0);
 gSource=Make(h,WC_COMBOBOXW,L"",CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS|WS_VSCROLL,96,112,210,320,ID_SOURCE);
 HWND cs=Make(h,L"BUTTON",L"Capture",BS_OWNERDRAW,316,112,80,32,ID_CAP_SRC);
 Make(h,L"STATIC",L"Modifiers",SS_LEFT,414,122,62,20,0);
 gSrcShift=Make(h,L"BUTTON",L"Shift",BS_OWNERDRAW,482,114,52,28,ID_SRC_SHIFT);
 gSrcCtrl=Make(h,L"BUTTON",L"Ctrl",BS_OWNERDRAW,540,114,48,28,ID_SRC_CTRL);
 gSrcAlt=Make(h,L"BUTTON",L"Alt",BS_OWNERDRAW,594,114,46,28,ID_SRC_ALT);
 gSrcWin=Make(h,L"BUTTON",L"Win",BS_OWNERDRAW,646,114,48,28,ID_SRC_WIN);

 Make(h,L"STATIC",L"Target",SS_LEFT,32,164,56,20,0);
 gTarget=Make(h,WC_COMBOBOXW,L"",CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS|WS_VSCROLL,96,154,210,320,ID_TARGET);
 HWND ct=Make(h,L"BUTTON",L"Capture",BS_OWNERDRAW,316,154,80,32,ID_CAP_DST);
 Make(h,L"STATIC",L"Modifiers",SS_LEFT,414,164,62,20,0);
 gDstShift=Make(h,L"BUTTON",L"Shift",BS_OWNERDRAW,482,156,52,28,ID_DST_SHIFT);
 gDstCtrl=Make(h,L"BUTTON",L"Ctrl",BS_OWNERDRAW,540,156,48,28,ID_DST_CTRL);
 gDstAlt=Make(h,L"BUTTON",L"Alt",BS_OWNERDRAW,594,156,46,28,ID_DST_ALT);
 gDstWin=Make(h,L"BUTTON",L"Win",BS_OWNERDRAW,646,156,48,28,ID_DST_WIN);

 gAdd=Make(h,L"BUTTON",L"Add mapping",BS_OWNERDRAW,32,204,108,34,ID_ADD);
 gModify=Make(h,L"BUTTON",L"Modify",BS_OWNERDRAW,150,204,88,34,ID_MODIFY);
 gRemove=Make(h,L"BUTTON",L"Remove",BS_OWNERDRAW,248,204,88,34,ID_REMOVE);
 gBlock=Make(h,L"BUTTON",L"Block source",BS_OWNERDRAW,366,207,122,28,ID_BLOCK);

 Make(h,L"STATIC",L"Mappings",SS_LEFT,32,276,100,22,0);
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
 gStartup=Make(h,L"BUTTON",L"Start with Windows",BS_OWNERDRAW,20,524,156,28,ID_STARTUP);
 gStatus=Make(h,L"STATIC",L"",SS_LEFT,198,528,420,20,ID_STATUS);
 gPause=Make(h,L"BUTTON",L"Pause",BS_OWNERDRAW,714,522,86,32,ID_PAUSE);

 ApplyFont(gTitle,gFontTitle);
 ApplyFont(gSubtitle,gFontSmall);
 ApplyFont(gSection,gFontLabel);
 ApplyFont(gListSource,gFontLabel);
 ApplyFont(gListTarget,gFontLabel);
 ApplyFont(gEmpty,gFontSmall);
 ApplyFont(gStatus,gFontSmall);

 HWND controls[]={gSource,cs,gTarget,ct,gBlock,gAdd,gModify,gRemove,gPause,gStartup,
  gSrcShift,gSrcCtrl,gSrcAlt,gSrcWin,gDstShift,gDstCtrl,gDstAlt,gDstWin};
 for(HWND z:controls) ApplyFont(z,gFontBody);
 ApplyFont(gList,gFontBody);

 SetWindowTheme(gSource,L"Explorer",nullptr);
 SetWindowTheme(gTarget,L"Explorer",nullptr);
 SetWindowTheme(gList,L"Explorer",nullptr);

 FillCombo(gSource,gSourceChoices);FillCombo(gTarget,gTargetChoices);
 SelectChoice(gSource,gSourceChoices,VK_CAPITAL,0);
 SelectChoice(gTarget,gTargetChoices,VK_ESCAPE,0);
 InstallStableComboWheel(gSource,gSourceWheel);
 InstallStableComboWheel(gTarget,gTargetWheel);
 Button_SetCheck(gStartup,StartupEnabled()?BST_CHECKED:BST_UNCHECKED);

 RECT rc{};GetClientRect(h,&rc);LayoutUi(rc.right,rc.bottom);
 Refresh();
}
static void AddMap() {
 Mapping m;
 m.source=ComboCode(gSource,gSourceChoices); m.target=ComboCode(gTarget,gTargetChoices);
 m.block=Button_GetCheck(gBlock)==BST_CHECKED;
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
 Save();
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
 Save();
 Refresh();
}
static LRESULT CALLBACK WndProc(HWND h,UINT m,WPARAM w,LPARAM l) {
 switch(m) {
  case WM_CREATE:
   CreateUi(h);
   InstallHooks();
   Refresh();
   return 0;

  case WM_SIZE:
   LayoutUi(LOWORD(l),HIWORD(l));
   InvalidateRect(h,nullptr,TRUE);
   return 0;

  case WM_GETMINMAXINFO: {
   auto* info=reinterpret_cast<MINMAXINFO*>(l);
   info->ptMinTrackSize.x=760;
   info->ptMinTrackSize.y=560;
   return 0;
  }

  case WM_PAINT: {
   PAINTSTRUCT ps{};
   HDC dc=BeginPaint(h,&ps);
   RECT client{};GetClientRect(h,&client);

   RECT editor{20,70,client.right-20,252};
   RECT mappings{20,264,client.right-20,client.bottom-58};
   DrawRoundedRect(dc,editor,C_SURFACE,C_BORDER,12);
   DrawRoundedRect(dc,mappings,C_SURFACE,C_BORDER,12);

   int footerY=client.bottom-44;
   COLORREF statusColor=(!gKeyboard||!gMouse)?RGB(198,67,67):(gPaused?RGB(214,147,35):C_ACTIVE);
   HBRUSH dotBrush=CreateSolidBrush(statusColor);
   HGDIOBJ old=SelectObject(dc,dotBrush);
   Ellipse(dc,184,footerY+8,192,footerY+16);
   SelectObject(dc,old);
   DeleteObject(dotBrush);

   EndPaint(h,&ps);
   return 0;
  }

  case WM_MEASUREITEM: {
   auto* mi=reinterpret_cast<MEASUREITEMSTRUCT*>(l);
   if(mi->CtlID==ID_SOURCE || mi->CtlID==ID_TARGET) {
    mi->itemHeight=28;
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
   SetBkMode(dc,TRANSPARENT);
   if(id==ID_TITLE || id==ID_SECTION) SetTextColor(dc,C_TEXT);
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
    HWND btn=GetDlgItem(h,id);
    Button_SetCheck(btn,Button_GetCheck(btn)==BST_CHECKED?BST_UNCHECKED:BST_CHECKED);
    InvalidateRect(btn,nullptr,TRUE);
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
     gPaused=!gPaused;
     Refresh();
     InvalidateRect(h,nullptr,TRUE);
     break;
    case ID_BLOCK:
     if(notification==BN_CLICKED) {
      Button_SetCheck(gBlock,Button_GetCheck(gBlock)==BST_CHECKED?BST_UNCHECKED:BST_CHECKED);
      BOOL enabled=Button_GetCheck(gBlock)!=BST_CHECKED;
      EnableWindow(gTarget,enabled);
      EnableWindow(gDstShift,enabled);
      EnableWindow(gDstCtrl,enabled);
      EnableWindow(gDstAlt,enabled);
      EnableWindow(gDstWin,enabled);
      InvalidateRect(gBlock,nullptr,TRUE);
     }
     break;
    case ID_STARTUP:
     if(notification==BN_CLICKED) {
      bool oldState=Button_GetCheck(gStartup)==BST_CHECKED;
      bool enabled=!oldState;
      Button_SetCheck(gStartup,enabled?BST_CHECKED:BST_UNCHECKED);
      if(!SetStartupEnabled(enabled)) {
       Button_SetCheck(gStartup,oldState?BST_CHECKED:BST_UNCHECKED);
       SetWindowTextW(gStatus,L"Could not update the Windows startup setting");
      } else {
       SetWindowTextW(gStatus,enabled?L"Starts with Windows":L"Start with Windows disabled");
      }
      InvalidateRect(gStartup,nullptr,TRUE);
     }
     break;
   }
   return 0;
  }

  case WM_KR_CAPTURE: {
   int code=(int)w;
   bool src=(l&1)==0;
   unsigned mods=(unsigned)((ULONG_PTR)l>>1);
   if(gSelfTestMode) { gSelfCapturedCode=code; gSelfCapturedMods=mods; return 0; }
   if(code==VK_ESCAPE){Refresh();return 0;}
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

  case WM_CLOSE:
   DestroyWindow(h);
   return 0;

  case WM_DESTROY:
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

 // UI-selection test: choose @ directly in the Target combo, then Add/Update.
 CreateUi(gWnd);
 SelectChoice(gSource,gSourceChoices,VK_OEM_3,0);
 SelectChoice(gTarget,gTargetChoices,'2',KRM_SHIFT);
 SetModChecks(0,gSrcShift,gSrcCtrl,gSrcAlt,gSrcWin);
 SetModChecks(0,gDstShift,gDstCtrl,gDstAlt,gDstWin);
 Button_SetCheck(gBlock,BST_UNCHECKED);
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
 Button_SetCheck(gBlock,BST_UNCHECKED);
 AddMap();
 wchar_t upsertTarget[64]{};
 if(ListView_GetItemCount(gList)>0) ListView_GetItemText(gList,0,1,upsertTarget,64);
 bool upsertOk=(gMaps.size()==1 && gMaps[0].target=='2' &&
                (gMaps[0].dstMods&KRM_SHIFT)!=0 && wcscmp(upsertTarget,L"@")==0);
 if(!upsertOk) { DestroyWindow(gWnd); gWnd=nullptr; gSelfTestMode=false; return 28; }

 // Persistence regression: save, reload, then recover from permanent backup.
 Save();
 gMaps.clear();
 Load();
 bool reloadOk=(gMaps.size()==1 && gMaps[0].target=='2' && (gMaps[0].dstMods&KRM_SHIFT)!=0);
 if(!reloadOk) { DestroyWindow(gWnd); gWnd=nullptr; gSelfTestMode=false; return 29; }

 CopyFileW(ConfigPath().c_str(),ConfigBackupPath().c_str(),FALSE);
 DeleteFileW(ConfigPath().c_str());
 gMaps.clear();
 Load();
 bool recoveryOk=(gMaps.size()==1 && gMaps[0].target=='2' &&
                  (gMaps[0].dstMods&KRM_SHIFT)!=0 &&
                  GetFileAttributesW(ConfigPath().c_str())!=INVALID_FILE_ATTRIBUTES);
 if(!recoveryOk) { DestroyWindow(gWnd); gWnd=nullptr; gSelfTestMode=false; return 30; }
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
 bool startupOff=SetStartupEnabled(false) && !StartupEnabled();
 bool startupRestore=SetStartupEnabled(startupInitial);
 if(!(startupOn&&startupOff&&startupRestore)) {
  DestroyWindow(gWnd); gWnd=nullptr; gSelfTestMode=false; return 27;
 }
 Button_SetCheck(gStartup,startupInitial?BST_CHECKED:BST_UNCHECKED);

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
 bool captureOk=(gSelfCapturedCode=='2' && (gSelfCapturedMods&KRM_SHIFT)!=0);
 if(!captureOk) {
  Unhook(); DestroyWindow(gWnd); gWnd=nullptr; gSelfTestMode=false; return 22;
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
 auto isShiftVk=[](int vk){return vk==VK_SHIFT||vk==VK_LSHIFT||vk==VK_RSHIFT;};
 bool outputOk=false;
 for(size_t i=0;i+3<gSelfOutputEvents.size();++i) {
  if(isShiftVk(gSelfOutputEvents[i].vk) && gSelfOutputEvents[i].down &&
     gSelfOutputEvents[i+1].vk=='2' && gSelfOutputEvents[i+1].down &&
     gSelfOutputEvents[i+2].vk=='2' && !gSelfOutputEvents[i+2].down &&
     isShiftVk(gSelfOutputEvents[i+3].vk) && !gSelfOutputEvents[i+3].down) {
    outputOk=true; break;
  }
 }
 Unhook();
 DestroyWindow(gWnd); gWnd=nullptr; gSelfTestMode=false;
 return outputOk?0:23;
}
int WINAPI wWinMain(HINSTANCE hi,HINSTANCE,PWSTR cmd,int show) {
 gInst=hi;if(wcsstr(cmd,L"--self-test"))return SelfTest();

 gInstanceMutex=CreateMutexW(nullptr,TRUE,L"Local\\KeyRemapper.SingleInstance");
 if(!gInstanceMutex) return 3;
 if(GetLastError()==ERROR_ALREADY_EXISTS) {
  HWND existing=FindWindowW(L"KRNative",nullptr);
  if(existing) {
   if(IsIconic(existing)) ShowWindow(existing,SW_RESTORE);
   SetForegroundWindow(existing);
  }
  CloseHandle(gInstanceMutex);gInstanceMutex=nullptr;
  return 0;
 }

 INITCOMMONCONTROLSEX ic{sizeof(ic),ICC_LISTVIEW_CLASSES};InitCommonControlsEx(&ic);SetProcessDPIAware();Load();

 gIconLarge=(HICON)LoadImageW(hi,MAKEINTRESOURCEW(101),IMAGE_ICON,32,32,LR_DEFAULTCOLOR);
 gIconSmall=(HICON)LoadImageW(hi,MAKEINTRESOURCEW(101),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);

 WNDCLASSEXW wc{sizeof(wc)};
 wc.hInstance=hi;wc.lpfnWndProc=WndProc;wc.lpszClassName=L"KRNative";wc.hCursor=LoadCursorW(nullptr,IDC_ARROW);
 wc.hIcon=gIconLarge?gIconLarge:LoadIconW(nullptr,IDI_APPLICATION);
 wc.hIconSm=gIconSmall?gIconSmall:LoadIconW(nullptr,IDI_APPLICATION);
 wc.hbrBackground=CreateSolidBrush(C_BG);
 RegisterClassExW(&wc);

 gWnd=CreateWindowExW(0,wc.lpszClassName,L"Key Remapper",WS_OVERLAPPEDWINDOW,
  CW_USEDEFAULT,CW_USEDEFAULT,840,600,nullptr,nullptr,hi,nullptr);
 if(!gWnd)return 2;
 ShowWindow(gWnd,show);UpdateWindow(gWnd);
 MSG msg;while(GetMessageW(&msg,nullptr,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}
 return (int)msg.wParam;
}