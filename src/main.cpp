#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shellapi.h>
#include <shlobj.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")

constexpr ULONG_PTR KR_MAGIC = 0x4B524D50;
constexpr UINT WM_KR_CAPTURE = WM_APP + 1;
constexpr int MOUSE_LEFT = 0x1001;
constexpr int MOUSE_RIGHT = 0x1002;
constexpr int MOUSE_MIDDLE = 0x1003;
constexpr int MOUSE_X1 = 0x1004;
constexpr int MOUSE_X2 = 0x1005;
constexpr int WHEEL_UP = 0x1101;
constexpr int WHEEL_DOWN = 0x1102;

enum { ID_SOURCE=100, ID_TARGET, ID_CAP_SRC, ID_CAP_DST, ID_BLOCK, ID_ADD, ID_REMOVE, ID_LIST, ID_PAUSE, ID_STATUS,
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
static LPARAM EncodeChoice(int code,unsigned mods) {
 return (LPARAM)((code & 0xFFFF) | ((mods & 0xFF) << 16));
}
static int DecodeChoiceCode(LPARAM data) { return (int)(data & 0xFFFF); }
static unsigned DecodeChoiceMods(LPARAM data) { return (unsigned)((data >> 16) & 0xFF); }

struct Mapping { int source; int target; bool block; unsigned srcMods=0; unsigned dstMods=0; };
static HINSTANCE gInst;
static HWND gWnd, gSource, gTarget, gBlock, gList, gPause, gStatus;
static HWND gSrcShift,gSrcCtrl,gSrcAlt,gSrcWin,gDstShift,gDstCtrl,gDstAlt,gDstWin;
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
static LPARAM ComboChoice(HWND h) {
 int i=ComboBox_GetCurSel(h);
 return i==CB_ERR?0:ComboBox_GetItemData(h,i);
}
static int ComboCode(HWND h) { return DecodeChoiceCode(ComboChoice(h)); }
static unsigned ComboImpliedMods(HWND h) { return DecodeChoiceMods(ComboChoice(h)); }
static void SelectChoice(HWND h,int code,unsigned mods) {
 for(int i=0;i<ComboBox_GetCount(h);++i) {
  LPARAM d=ComboBox_GetItemData(h,i);
  if(DecodeChoiceCode(d)==code && DecodeChoiceMods(d)==mods) { ComboBox_SetCurSel(h,i); return; }
 }
 for(int i=0;i<ComboBox_GetCount(h);++i) {
  LPARAM d=ComboBox_GetItemData(h,i);
  if(DecodeChoiceCode(d)==code && DecodeChoiceMods(d)==0) { ComboBox_SetCurSel(h,i); return; }
 }
}
static void FillCombo(HWND h) {
 for(auto&i:ITEMS) {
  int n=ComboBox_AddString(h,i.name);
  ComboBox_SetItemData(h,n,EncodeChoice(i.code,0));
 }
 for(auto&i:SYMBOL_ITEMS) {
  int n=ComboBox_AddString(h,i.name);
  ComboBox_SetItemData(h,n,EncodeChoice(i.code,KRM_SHIFT));
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
static void Refresh() {
 ListView_DeleteAllItems(gList);int row=0;
 for(auto&m:gMaps) {
  std::wstring a=Describe(m.source,m.srcMods),b=m.block?L"[Blocked]":Describe(m.target,m.dstMods);
  LVITEMW it{};it.mask=LVIF_TEXT|LVIF_PARAM;it.iItem=row;it.pszText=a.data();it.lParam=(LPARAM)&m;
  ListView_InsertItem(gList,&it);ListView_SetItemText(gList,row,1,b.data());++row;
 }
 wchar_t st[128];wsprintfW(st,L"%s | %u mapping(s) | keyboard hook: %s | mouse hook: %s",
  gPaused?L"PAUSED":L"ACTIVE",(unsigned)gMaps.size(),gKeyboard?L"OK":L"FAILED",gMouse?L"OK":L"FAILED");
 SetWindowTextW(gStatus,st);SetWindowTextW(gPause,gPaused?L"Resume":L"Pause");
}
static HWND Make(HWND p,const wchar_t*c,const wchar_t*t,DWORD s,int x,int y,int w,int h,int id,DWORD ex=0) {
 return CreateWindowExW(ex,c,t,s|WS_CHILD|WS_VISIBLE,x,y,w,h,p,(HMENU)(INT_PTR)id,gInst,nullptr);
}
static void CreateUi(HWND h) {
 HFONT font=(HFONT)GetStockObject(DEFAULT_GUI_FONT);
 auto label=[&](const wchar_t*t,int x,int y,int w){HWND z=Make(h,L"STATIC",t,SS_LEFT,x,y,w,22,0);SendMessageW(z,WM_SETFONT,(WPARAM)font,TRUE);};
 label(L"Source",18,24,55);gSource=Make(h,WC_COMBOBOXW,L"",CBS_DROPDOWNLIST|WS_VSCROLL,78,18,230,300,ID_SOURCE);
 HWND cs=Make(h,L"BUTTON",L"Capture",BS_PUSHBUTTON,318,18,85,28,ID_CAP_SRC);
 label(L"Mods",420,24,38);
 gSrcShift=Make(h,L"BUTTON",L"Shift",BS_AUTOCHECKBOX,462,19,60,26,ID_SRC_SHIFT);
 gSrcCtrl=Make(h,L"BUTTON",L"Ctrl",BS_AUTOCHECKBOX,522,19,52,26,ID_SRC_CTRL);
 gSrcAlt=Make(h,L"BUTTON",L"Alt",BS_AUTOCHECKBOX,574,19,48,26,ID_SRC_ALT);
 gSrcWin=Make(h,L"BUTTON",L"Win",BS_AUTOCHECKBOX,622,19,52,26,ID_SRC_WIN);
 label(L"Target",18,68,55);gTarget=Make(h,WC_COMBOBOXW,L"",CBS_DROPDOWNLIST|WS_VSCROLL,78,62,230,300,ID_TARGET);
 HWND ct=Make(h,L"BUTTON",L"Capture",BS_PUSHBUTTON,318,62,85,28,ID_CAP_DST);
 label(L"Mods",420,68,38);
 gDstShift=Make(h,L"BUTTON",L"Shift",BS_AUTOCHECKBOX,462,63,60,26,ID_DST_SHIFT);
 gDstCtrl=Make(h,L"BUTTON",L"Ctrl",BS_AUTOCHECKBOX,522,63,52,26,ID_DST_CTRL);
 gDstAlt=Make(h,L"BUTTON",L"Alt",BS_AUTOCHECKBOX,574,63,48,26,ID_DST_ALT);
 gDstWin=Make(h,L"BUTTON",L"Win",BS_AUTOCHECKBOX,622,63,52,26,ID_DST_WIN);
 gBlock=Make(h,L"BUTTON",L"Block source",BS_AUTOCHECKBOX,430,110,115,26,ID_BLOCK);
 HWND add=Make(h,L"BUTTON",L"Add / Update",BS_DEFPUSHBUTTON,18,110,130,32,ID_ADD);
 HWND rem=Make(h,L"BUTTON",L"Remove selected",BS_PUSHBUTTON,158,110,140,32,ID_REMOVE);
 gPause=Make(h,L"BUTTON",L"Pause",BS_PUSHBUTTON,555,110,115,32,ID_PAUSE);
 gList=Make(h,WC_LISTVIEWW,L"",LVS_REPORT|LVS_SINGLESEL|LVS_SHOWSELALWAYS,18,160,656,270,ID_LIST,WS_EX_CLIENTEDGE);
 ListView_SetExtendedListViewStyle(gList,LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_DOUBLEBUFFER);
 LVCOLUMNW col{};col.mask=LVCF_TEXT|LVCF_WIDTH;col.pszText=(LPWSTR)L"Source";col.cx=250;ListView_InsertColumn(gList,0,&col);
 col.pszText=(LPWSTR)L"Target";col.cx=380;ListView_InsertColumn(gList,1,&col);
 gStatus=Make(h,L"STATIC",L"",SS_LEFT,18,445,530,24,ID_STATUS);
 HWND controls[]={gSource,cs,gTarget,ct,gBlock,add,rem,gPause,gList,gStatus,
  gSrcShift,gSrcCtrl,gSrcAlt,gSrcWin,gDstShift,gDstCtrl,gDstAlt,gDstWin};
 for(HWND z:controls)SendMessageW(z,WM_SETFONT,(WPARAM)font,TRUE);
 FillCombo(gSource);FillCombo(gTarget);SelectChoice(gSource,VK_CAPITAL,0);SelectChoice(gTarget,VK_ESCAPE,0);Refresh();
}
static void AddMap() {
 Mapping m;
 m.source=ComboCode(gSource); m.target=ComboCode(gTarget);
 m.block=Button_GetCheck(gBlock)==BST_CHECKED;
 m.srcMods=CheckedMods(gSrcShift,gSrcCtrl,gSrcAlt,gSrcWin) | ComboImpliedMods(gSource);
 m.dstMods=CheckedMods(gDstShift,gDstCtrl,gDstAlt,gDstWin) | ComboImpliedMods(gTarget);
 if(!m.source||(!m.block&&!m.target))return;
 for(auto&x:gMaps)if(x.source==m.source && x.srcMods==m.srcMods){x=m;Save();Refresh();return;}
 gMaps.push_back(m);Save();Refresh();
}
static void RemoveMap() {
 int r=ListView_GetNextItem(gList,-1,LVNI_SELECTED);if(r<0)return;
 LVITEMW it{};it.mask=LVIF_PARAM;it.iItem=r;if(!ListView_GetItem(gList,&it))return;
 Mapping*p=(Mapping*)it.lParam;auto x=std::find_if(gMaps.begin(),gMaps.end(),[&](Mapping&m){return &m==p;});
 if(x!=gMaps.end())gMaps.erase(x);Save();Refresh();
}
static LRESULT CALLBACK WndProc(HWND h,UINT m,WPARAM w,LPARAM l) {
 switch(m) {
  case WM_CREATE:CreateUi(h);InstallHooks();Refresh();return 0;
  case WM_COMMAND:
   switch(LOWORD(w)) {
    case ID_CAP_SRC:gCaptureSource=true;gCaptureTarget=false;SetWindowTextW(gStatus,L"Press source key/mouse button/wheel...");break;
    case ID_CAP_DST:gCaptureTarget=true;gCaptureSource=false;SetWindowTextW(gStatus,L"Press target key/mouse button/wheel...");break;
    case ID_ADD:AddMap();break;case ID_REMOVE:RemoveMap();break;
    case ID_PAUSE:gPaused=!gPaused;Refresh();break;
    case ID_BLOCK:{BOOL e=Button_GetCheck(gBlock)!=BST_CHECKED;EnableWindow(gTarget,e);break;}
   }return 0;
  case WM_KR_CAPTURE:{
   int code=(int)w;
   bool src=(l&1)==0;
   unsigned mods=(unsigned)((ULONG_PTR)l>>1);
   if(gSelfTestMode) { gSelfCapturedCode=code; gSelfCapturedMods=mods; return 0; }
   if(code==VK_ESCAPE){Refresh();return 0;}
   SelectChoice(src?gSource:gTarget,code,mods&KRM_SHIFT);
   if(src) SetModChecks(mods & ~ComboImpliedMods(gSource),gSrcShift,gSrcCtrl,gSrcAlt,gSrcWin);
   else SetModChecks(mods & ~ComboImpliedMods(gTarget),gDstShift,gDstCtrl,gDstAlt,gDstWin);
   Refresh();return 0;}
  case WM_CLOSE:DestroyWindow(h);return 0;
  case WM_DESTROY:Save();Unhook();PostQuitMessage(0);return 0;
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
 gWnd=CreateWindowExW(0,wc.lpszClassName,L"",0,0,0,0,0,HWND_MESSAGE,nullptr,gInst,nullptr);
 if(!gWnd) return 20;
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
 wc.hIcon=LoadIconW(nullptr,IDI_APPLICATION);wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);RegisterClassExW(&wc);
 gWnd=CreateWindowExW(0,wc.lpszClassName,L"Key Remapper - Native Rebuild",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,
  CW_USEDEFAULT,CW_USEDEFAULT,715,525,nullptr,nullptr,hi,nullptr);if(!gWnd)return 2;
 ShowWindow(gWnd,show);UpdateWindow(gWnd);MSG msg;while(GetMessageW(&msg,nullptr,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}return (int)msg.wParam;
}