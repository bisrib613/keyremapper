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

static constexpr COLORREF C_BG=RGB(24,24,24);
static constexpr COLORREF C_SURFACE=RGB(31,31,31);
static constexpr COLORREF C_CONTROL=RGB(39,39,39);
static constexpr COLORREF C_HOVER=RGB(49,49,49);
static constexpr COLORREF C_TEXT=RGB(255,255,255);
static constexpr COLORREF C_MUTED=RGB(155,155,155);
static constexpr COLORREF C_BORDER=RGB(49,49,49);
static HBRUSH gBrushBg=nullptr, gBrushSurface=nullptr, gBrushControl=nullptr;
static HFONT gFontBody=nullptr, gFontSmall=nullptr, gFontTitle=nullptr, gFontLabel=nullptr;
static HICON gIconLarge=nullptr, gIconSmall=nullptr;

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
static void Unhook() {
 for(size_t i=0;i<gActive.size();++i)if(gActive[i].target)EndTarget(gActive[i]);
 if(gKeyboard)UnhookWindowsHookEx(gKeyboard);if(gMouse)UnhookWindowsHookEx(gMouse);
 gKeyboard=gMouse=nullptr;
}
static std::wstring ConfigPath() {
 wchar_t p[MAX_PATH];SHGetFolderPathW(nullptr,CSIDL_APPDATA,nullptr,SHGFP_TYPE_CURRENT,p);
 std::wstring d=std::wstring(p)+L"\\KeyRemapperClean";CreateDirectoryW(d.c_str(),nullptr);return d+L"\\mappings.txt";
}
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
  std::wstring command=L"\""+std::wstring(exe)+L"\"";
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
static void Save() {
 std::wofstream f(ConfigPath(),std::ios::trunc);
 if(!f)return;
 for(auto&m:gMaps) f<<m.source<<L' '<<m.target<<L' '<<(m.block?1:0)<<L' '<<m.srcMods<<L' '<<m.dstMods<<L'\n';
}
static void Load() {
 std::wifstream f(ConfigPath());
 if(!f)return;
 std::wstring line;
 while(std::getline(f,line)) {
  if(line.empty())continue;
  std::wistringstream in(line);
  int src=0,dst=0,block=0; unsigned sm=0,dm=0;
  if(!(in>>src>>dst>>block))continue;
  in>>sm>>dm;
  gMaps.push_back({src,dst,block!=0,sm,dm});
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
 if(!gFontBody) gFontBody=MakeUiFont(h,15,FW_NORMAL);
 if(!gFontSmall) gFontSmall=MakeUiFont(h,13,FW_NORMAL);
 if(!gFontLabel) gFontLabel=MakeUiFont(h,13,FW_SEMIBOLD);
 if(!gFontTitle) gFontTitle=MakeUiFont(h,26,FW_SEMIBOLD);
}

static void ApplyFont(HWND h,HFONT font) {
 if(h && font) SendMessageW(h,WM_SETFONT,(WPARAM)font,TRUE);
}

static void DrawOwnerButton(const DRAWITEMSTRUCT* d) {
 RECT r=d->rcItem;
 bool primary=d->CtlID==ID_ADD;
 bool pressed=(d->itemState&ODS_SELECTED)!=0;
 bool disabled=(d->itemState&ODS_DISABLED)!=0;
 COLORREF fill=primary?C_TEXT:C_CONTROL;
 COLORREF text=primary?C_BG:C_TEXT;
 if(pressed) fill=primary?C_MUTED:C_HOVER;
 if(disabled) { fill=C_SURFACE; text=C_MUTED; }

 HBRUSH brush=CreateSolidBrush(fill);
 HPEN pen=CreatePen(PS_SOLID,1,primary?fill:C_BORDER);
 HGDIOBJ oldBrush=SelectObject(d->hDC,brush);
 HGDIOBJ oldPen=SelectObject(d->hDC,pen);
 RoundRect(d->hDC,r.left,r.top,r.right,r.bottom,8,8);
 SelectObject(d->hDC,oldBrush);
 SelectObject(d->hDC,oldPen);
 DeleteObject(brush);DeleteObject(pen);

 wchar_t textBuf[128]{};
 GetWindowTextW(d->hwndItem,textBuf,128);
 SetBkMode(d->hDC,TRANSPARENT);
 SetTextColor(d->hDC,text);
 HFONT oldFont=(HFONT)SelectObject(d->hDC,gFontLabel?gFontLabel:GetStockObject(DEFAULT_GUI_FONT));
 DrawTextW(d->hDC,textBuf,-1,&r,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
 SelectObject(d->hDC,oldFont);

 if(d->itemState&ODS_FOCUS) {
  RECT focus=r;InflateRect(&focus,-4,-4);
  DrawFocusRect(d->hDC,&focus);
 }
}

static void DrawOwnerCombo(const DRAWITEMSTRUCT* d) {
 FillRect(d->hDC,&d->rcItem,gBrushControl);
 if(d->itemID==(UINT)-1) return;
 wchar_t buf[128]{};
 SendMessageW(d->hwndItem,CB_GETLBTEXT,d->itemID,(LPARAM)buf);
 RECT r=d->rcItem;
 if(d->itemState&ODS_SELECTED) {
  HBRUSH selected=CreateSolidBrush(C_HOVER);
  FillRect(d->hDC,&r,selected);
  DeleteObject(selected);
 }
 r.left+=12;r.right-=8;
 SetBkMode(d->hDC,TRANSPARENT);
 SetTextColor(d->hDC,C_TEXT);
 HFONT old=(HFONT)SelectObject(d->hDC,gFontBody?gFontBody:GetStockObject(DEFAULT_GUI_FONT));
 DrawTextW(d->hDC,buf,-1,&r,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
 SelectObject(d->hDC,old);
}

static void LayoutUi(int width,int height) {
 if(!gList) return;
 int listTop=304;
 int footerHeight=56;
 int listBottom=std::max(listTop+160,height-footerHeight-16);
 int listWidth=std::max(520,width-48);
 MoveWindow(gList,24,listTop,listWidth,listBottom-listTop,TRUE);
 int footerY=height-48;
 if(gStartup) MoveWindow(gStartup,24,footerY,155,28,TRUE);
 if(gStatus) MoveWindow(gStatus,196,footerY+4,std::max(220,width-196-150-40),24,TRUE);
 if(gPause) MoveWindow(gPause,width-132,footerY-2,108,34,TRUE);
 ListView_SetColumnWidth(gList,0,std::max(220,(listWidth-4)/2));
 ListView_SetColumnWidth(gList,1,std::max(220,listWidth-4-ListView_GetColumnWidth(gList,0)));
}

static void CreateUi(HWND h) {
 EnsureUiResources(h);
 BOOL dark=TRUE;
 DwmSetWindowAttribute(h,20,&dark,sizeof(dark));

 gTitle=Make(h,L"STATIC",L"Key Remapper",SS_LEFT,24,18,420,34,ID_TITLE);
 gSubtitle=Make(h,L"STATIC",L"Map keyboard and mouse input without changing hardware drivers.",SS_LEFT,24,50,620,24,ID_SUBTITLE);
 gSection=Make(h,L"STATIC",L"Mapping editor",SS_LEFT,24,88,180,24,ID_SECTION);

 Make(h,L"STATIC",L"Source",SS_LEFT,24,126,64,24,0);
 gSource=Make(h,WC_COMBOBOXW,L"",CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS|WS_VSCROLL,96,118,250,360,ID_SOURCE);
 HWND cs=Make(h,L"BUTTON",L"Capture",BS_OWNERDRAW,358,118,88,34,ID_CAP_SRC);
 Make(h,L"STATIC",L"Modifiers",SS_LEFT,466,126,70,24,0);
 gSrcShift=Make(h,L"BUTTON",L"Shift",BS_AUTOCHECKBOX,540,120,62,28,ID_SRC_SHIFT);
 gSrcCtrl=Make(h,L"BUTTON",L"Ctrl",BS_AUTOCHECKBOX,606,120,54,28,ID_SRC_CTRL);
 gSrcAlt=Make(h,L"BUTTON",L"Alt",BS_AUTOCHECKBOX,664,120,50,28,ID_SRC_ALT);
 gSrcWin=Make(h,L"BUTTON",L"Win",BS_AUTOCHECKBOX,718,120,54,28,ID_SRC_WIN);

 Make(h,L"STATIC",L"Target",SS_LEFT,24,174,64,24,0);
 gTarget=Make(h,WC_COMBOBOXW,L"",CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS|WS_VSCROLL,96,166,250,360,ID_TARGET);
 HWND ct=Make(h,L"BUTTON",L"Capture",BS_OWNERDRAW,358,166,88,34,ID_CAP_DST);
 Make(h,L"STATIC",L"Modifiers",SS_LEFT,466,174,70,24,0);
 gDstShift=Make(h,L"BUTTON",L"Shift",BS_AUTOCHECKBOX,540,168,62,28,ID_DST_SHIFT);
 gDstCtrl=Make(h,L"BUTTON",L"Ctrl",BS_AUTOCHECKBOX,606,168,54,28,ID_DST_CTRL);
 gDstAlt=Make(h,L"BUTTON",L"Alt",BS_AUTOCHECKBOX,664,168,50,28,ID_DST_ALT);
 gDstWin=Make(h,L"BUTTON",L"Win",BS_AUTOCHECKBOX,718,168,54,28,ID_DST_WIN);

 gAdd=Make(h,L"BUTTON",L"Add mapping",BS_OWNERDRAW,24,220,124,36,ID_ADD);
 gModify=Make(h,L"BUTTON",L"Modify selected",BS_OWNERDRAW,160,220,140,36,ID_MODIFY);
 gRemove=Make(h,L"BUTTON",L"Remove selected",BS_OWNERDRAW,312,220,140,36,ID_REMOVE);
 gBlock=Make(h,L"BUTTON",L"Block source input",BS_AUTOCHECKBOX,480,224,150,28,ID_BLOCK);

 Make(h,L"STATIC",L"Mappings",SS_LEFT,24,272,120,24,0);
 gList=Make(h,WC_LISTVIEWW,L"",LVS_REPORT|LVS_SINGLESEL|LVS_SHOWSELALWAYS,24,304,752,260,ID_LIST,0);
 ListView_SetExtendedListViewStyle(gList,LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER);
 ListView_SetBkColor(gList,C_SURFACE);
 ListView_SetTextBkColor(gList,C_SURFACE);
 ListView_SetTextColor(gList,C_TEXT);
 LVCOLUMNW col{};col.mask=LVCF_TEXT|LVCF_WIDTH;
 col.pszText=(LPWSTR)L"Source";col.cx=360;ListView_InsertColumn(gList,0,&col);
 col.pszText=(LPWSTR)L"Target";col.cx=360;ListView_InsertColumn(gList,1,&col);

 gStartup=Make(h,L"BUTTON",L"Start with Windows",BS_AUTOCHECKBOX,24,576,155,28,ID_STARTUP);
 gStatus=Make(h,L"STATIC",L"",SS_LEFT,196,580,430,24,ID_STATUS);
 gPause=Make(h,L"BUTTON",L"Pause",BS_OWNERDRAW,684,574,108,34,ID_PAUSE);

 HWND labels[]={gTitle,gSubtitle,gSection,gStatus};
 ApplyFont(gTitle,gFontTitle);
 ApplyFont(gSubtitle,gFontSmall);
 ApplyFont(gSection,gFontLabel);
 ApplyFont(gStatus,gFontSmall);

 HWND controls[]={gSource,cs,gTarget,ct,gBlock,gAdd,gModify,gRemove,gPause,gStartup,
  gSrcShift,gSrcCtrl,gSrcAlt,gSrcWin,gDstShift,gDstCtrl,gDstAlt,gDstWin};
 for(HWND z:controls) ApplyFont(z,gFontBody);
 ApplyFont(gList,gFontBody);

 for(HWND z:{gSrcShift,gSrcCtrl,gSrcAlt,gSrcWin,gDstShift,gDstCtrl,gDstAlt,gDstWin,gBlock,gStartup})
  SetWindowTheme(z,L"Explorer",nullptr);
 SetWindowTheme(gList,L"DarkMode_Explorer",nullptr);
 SetWindowTheme(ListView_GetHeader(gList),L"DarkMode_ItemsView",nullptr);

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

 for(size_t i=0;i<gMaps.size();++i) {
  if((int)i==gEditingIndex) continue;
  if(gMaps[i].source==m.source && gMaps[i].srcMods==m.srcMods) {
   if(gStatus) SetWindowTextW(gStatus,L"A mapping with this source already exists. Select it and use Modify.");
   return;
  }
 }

 int selectIndex=-1;
 if(gEditingIndex>=0 && gEditingIndex<(int)gMaps.size()) {
  gMaps[(size_t)gEditingIndex]=m;
  selectIndex=gEditingIndex;
 } else {
  gMaps.push_back(m);
  selectIndex=(int)gMaps.size()-1;
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
   return 0;

  case WM_GETMINMAXINFO: {
   auto* info=reinterpret_cast<MINMAXINFO*>(l);
   info->ptMinTrackSize.x=800;
   info->ptMinTrackSize.y=620;
   return 0;
  }

  case WM_MEASUREITEM: {
   auto* mi=reinterpret_cast<MEASUREITEMSTRUCT*>(l);
   if(mi->CtlID==ID_SOURCE || mi->CtlID==ID_TARGET) {
    mi->itemHeight=32;
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
   SetTextColor(dc,id==ID_TITLE?C_TEXT:C_MUTED);
   return (LRESULT)gBrushBg;
  }

  case WM_CTLCOLORBTN: {
   HDC dc=(HDC)w;
   SetBkMode(dc,TRANSPARENT);
   SetTextColor(dc,C_TEXT);
   SetBkColor(dc,C_BG);
   return (LRESULT)gBrushBg;
  }

  case WM_CTLCOLORLISTBOX: {
   HDC dc=(HDC)w;
   SetBkColor(dc,C_CONTROL);
   SetTextColor(dc,C_TEXT);
   return (LRESULT)gBrushControl;
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
      cd->clrTextBk=selected?C_HOVER:C_SURFACE;
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
     break;
    case ID_BLOCK: {
     BOOL enabled=Button_GetCheck(gBlock)!=BST_CHECKED;
     EnableWindow(gTarget,enabled);
     EnableWindow(gDstShift,enabled);
     EnableWindow(gDstCtrl,enabled);
     EnableWindow(gDstAlt,enabled);
     EnableWindow(gDstWin,enabled);
     break;
    }
    case ID_STARTUP:
     if(notification==BN_CLICKED) {
      bool enabled=Button_GetCheck(gStartup)==BST_CHECKED;
      if(!SetStartupEnabled(enabled)) {
       Button_SetCheck(gStartup,enabled?BST_UNCHECKED:BST_CHECKED);
       SetWindowTextW(gStatus,L"Could not update the Windows startup setting");
      } else {
       SetWindowTextW(gStatus,enabled?L"Key Remapper will start with Windows":L"Start with Windows disabled");
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
   return 0;
  }

  case WM_CLOSE:
   DestroyWindow(h);
   return 0;

  case WM_DESTROY:
   Save();
   Unhook();
   if(gFontBody) DeleteObject(gFontBody);
   if(gFontSmall) DeleteObject(gFontSmall);
   if(gFontTitle) DeleteObject(gFontTitle);
   if(gFontLabel) DeleteObject(gFontLabel);
   if(gBrushBg) DeleteObject(gBrushBg);
   if(gBrushSurface) DeleteObject(gBrushSurface);
   if(gBrushControl) DeleteObject(gBrushControl);
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
 INITCOMMONCONTROLSEX ic{sizeof(ic),ICC_LISTVIEW_CLASSES};InitCommonControlsEx(&ic);SetProcessDPIAware();Load();
 WNDCLASSEXW wc{sizeof(wc)};wc.hInstance=hi;wc.lpfnWndProc=WndProc;wc.lpszClassName=L"KRNative";wc.hCursor=LoadCursorW(nullptr,IDC_ARROW);
 wc.hIcon=LoadIconW(nullptr,IDI_APPLICATION);wc.hbrBackground=CreateSolidBrush(C_BG);RegisterClassExW(&wc);
 gWnd=CreateWindowExW(0,wc.lpszClassName,L"Key Remapper",WS_OVERLAPPEDWINDOW,
  CW_USEDEFAULT,CW_USEDEFAULT,840,660,nullptr,nullptr,hi,nullptr);if(!gWnd)return 2;
 ShowWindow(gWnd,show);UpdateWindow(gWnd);MSG msg;while(GetMessageW(&msg,nullptr,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}return (int)msg.wParam;
}