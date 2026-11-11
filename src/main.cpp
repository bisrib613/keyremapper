#define UNICODE
#define _UNICODE
#include <windows.h>
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

enum { ID_SOURCE=100, ID_TARGET, ID_CAP_SRC, ID_CAP_DST, ID_BLOCK, ID_ADD, ID_REMOVE, ID_LIST, ID_PAUSE, ID_STATUS };

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
 {MOUSE_LEFT,L"Mouse Left"},{MOUSE_RIGHT,L"Mouse Right"},{MOUSE_MIDDLE,L"Mouse Middle"},
 {MOUSE_X1,L"Mouse X1"},{MOUSE_X2,L"Mouse X2"},{WHEEL_UP,L"Wheel Up"},{WHEEL_DOWN,L"Wheel Down"}
};

struct Mapping { int source; int target; bool block; };
static HINSTANCE gInst;
static HWND gWnd, gSource, gTarget, gBlock, gList, gPause, gStatus;
static HHOOK gKeyboard, gMouse;
static bool gPaused=false, gCaptureSource=false, gCaptureTarget=false;
static int gCaptureSwallow=0;
static std::vector<Mapping> gMaps;
static std::vector<int> gActive(0x1200,0);

static std::wstring NameOf(int code) {
 for (auto &i:ITEMS) if(i.code==code) return i.name;
 wchar_t b[32]; wsprintfW(b,L"0x%X",code); return b;
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
static Mapping* FindMap(int src) {
 if(gPaused) return nullptr;
 for(auto &m:gMaps) if(m.source==src) return &m;
 return nullptr;
}
static bool Handle(int src,bool down,bool pulse=false) {
 if(gCaptureSource||gCaptureTarget) {
  if(down||pulse) {
   bool s=gCaptureSource; gCaptureSource=gCaptureTarget=false; gCaptureSwallow=pulse?0:src;
   PostMessageW(gWnd,WM_KR_CAPTURE,src,s?0:1); return true;
  }
  return false;
 }
 if(gCaptureSwallow==src) { if(!down)gCaptureSwallow=0; return true; }
 Mapping* m=FindMap(src); if(!m)return false; if(m->block)return true;
 if(pulse) { SendCode(m->target,true); SendCode(m->target,false); return true; }
 if(src>=0 && src<(int)gActive.size()) {
  if(down) {
   if(!gActive[src]) { gActive[src]=m->target; SendCode(m->target,true); }
   else if(m->target<0x1000) SendCode(m->target,true);
  } else {
   int target=gActive[src]?gActive[src]:m->target; SendCode(target,false); gActive[src]=0;
  }
 } else { SendCode(m->target,down); }
 return true;
}
static LRESULT CALLBACK KeyHook(int n,WPARAM w,LPARAM l) {
 if(n<0)return CallNextHookEx(gKeyboard,n,w,l);
 auto*k=(KBDLLHOOKSTRUCT*)l;
 if((k->flags&LLKHF_INJECTED)||k->dwExtraInfo==KR_MAGIC)return CallNextHookEx(gKeyboard,n,w,l);
 bool d=w==WM_KEYDOWN||w==WM_SYSKEYDOWN,u=w==WM_KEYUP||w==WM_SYSKEYUP;
 if((d||u)&&Handle((int)k->vkCode,d))return 1;
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
 for(size_t i=0;i<gActive.size();++i)if(gActive[i])SendCode(gActive[i],false);
 if(gKeyboard)UnhookWindowsHookEx(gKeyboard);if(gMouse)UnhookWindowsHookEx(gMouse);
 gKeyboard=gMouse=nullptr;
}
static std::wstring ConfigPath() {
 wchar_t p[MAX_PATH];SHGetFolderPathW(nullptr,CSIDL_APPDATA,nullptr,SHGFP_TYPE_CURRENT,p);
 std::wstring d=std::wstring(p)+L"\\KeyRemapperClean";CreateDirectoryW(d.c_str(),nullptr);return d+L"\\mappings.txt";
}
static void Save() {
 std::wofstream f(ConfigPath(),std::ios::trunc);
 if(!f)return;for(auto&m:gMaps)f<<m.source<<L' '<<m.target<<L' '<<(m.block?1:0)<<L'\n';
}
static void Load() {
 std::wifstream f(ConfigPath());int s,t,b;while(f>>s>>t>>b)gMaps.push_back({s,t,b!=0});
}
static int ComboCode(HWND h) {int i=ComboBox_GetCurSel(h);return i==CB_ERR?0:(int)ComboBox_GetItemData(h,i);}
static void SelectCode(HWND h,int code){for(int i=0;i<ComboBox_GetCount(h);++i)if((int)ComboBox_GetItemData(h,i)==code){ComboBox_SetCurSel(h,i);break;}}
static void FillCombo(HWND h){for(auto&i:ITEMS){int n=ComboBox_AddString(h,i.name);ComboBox_SetItemData(h,n,i.code);}}
static void Refresh() {
 ListView_DeleteAllItems(gList);int row=0;
 for(auto&m:gMaps) {
  std::wstring a=NameOf(m.source),b=m.block?L"[Blocked]":NameOf(m.target);
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
 label(L"Target",18,68,55);gTarget=Make(h,WC_COMBOBOXW,L"",CBS_DROPDOWNLIST|WS_VSCROLL,78,62,230,300,ID_TARGET);
 HWND ct=Make(h,L"BUTTON",L"Capture",BS_PUSHBUTTON,318,62,85,28,ID_CAP_DST);
 gBlock=Make(h,L"BUTTON",L"Block source",BS_AUTOCHECKBOX,430,63,115,26,ID_BLOCK);
 HWND add=Make(h,L"BUTTON",L"Add / Update",BS_DEFPUSHBUTTON,18,110,130,32,ID_ADD);
 HWND rem=Make(h,L"BUTTON",L"Remove selected",BS_PUSHBUTTON,158,110,140,32,ID_REMOVE);
 gPause=Make(h,L"BUTTON",L"Pause",BS_PUSHBUTTON,430,110,115,32,ID_PAUSE);
 gList=Make(h,WC_LISTVIEWW,L"",LVS_REPORT|LVS_SINGLESEL|LVS_SHOWSELALWAYS,18,160,530,270,ID_LIST,WS_EX_CLIENTEDGE);
 ListView_SetExtendedListViewStyle(gList,LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_DOUBLEBUFFER);
 LVCOLUMNW col{};col.mask=LVCF_TEXT|LVCF_WIDTH;col.pszText=(LPWSTR)L"Source";col.cx=250;ListView_InsertColumn(gList,0,&col);
 col.pszText=(LPWSTR)L"Target";col.cx=250;ListView_InsertColumn(gList,1,&col);
 gStatus=Make(h,L"STATIC",L"",SS_LEFT,18,445,530,24,ID_STATUS);
 HWND controls[]={gSource,cs,gTarget,ct,gBlock,add,rem,gPause,gList,gStatus};
 for(HWND z:controls)SendMessageW(z,WM_SETFONT,(WPARAM)font,TRUE);
 FillCombo(gSource);FillCombo(gTarget);SelectCode(gSource,VK_CAPITAL);SelectCode(gTarget,VK_ESCAPE);Refresh();
}
static void AddMap() {
 Mapping m{ComboCode(gSource),ComboCode(gTarget),Button_GetCheck(gBlock)==BST_CHECKED};
 if(!m.source||(!m.block&&!m.target))return;
 for(auto&x:gMaps)if(x.source==m.source){x=m;Save();Refresh();return;}
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
   int code=(int)w;bool src=l==0;if(code==VK_ESCAPE){Refresh();return 0;}SelectCode(src?gSource:gTarget,code);Refresh();return 0;}
  case WM_CLOSE:DestroyWindow(h);return 0;
  case WM_DESTROY:Save();Unhook();PostQuitMessage(0);return 0;
 }
 return DefWindowProcW(h,m,w,l);
}
static int SelfTest() {
 HHOOK k=SetWindowsHookExW(WH_KEYBOARD_LL,KeyHook,gInst,0),m=SetWindowsHookExW(WH_MOUSE_LL,MouseHook,gInst,0);
 int ok=(k&&m)?0:10;if(k)UnhookWindowsHookEx(k);if(m)UnhookWindowsHookEx(m);return ok;
}
int WINAPI wWinMain(HINSTANCE hi,HINSTANCE,PWSTR cmd,int show) {
 gInst=hi;if(wcsstr(cmd,L"--self-test"))return SelfTest();
 INITCOMMONCONTROLSEX ic{sizeof(ic),ICC_LISTVIEW_CLASSES};InitCommonControlsEx(&ic);SetProcessDPIAware();Load();
 WNDCLASSEXW wc{sizeof(wc)};wc.hInstance=hi;wc.lpfnWndProc=WndProc;wc.lpszClassName=L"KRNative";wc.hCursor=LoadCursorW(nullptr,IDC_ARROW);
 wc.hIcon=LoadIconW(nullptr,IDI_APPLICATION);wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);RegisterClassExW(&wc);
 gWnd=CreateWindowExW(0,wc.lpszClassName,L"Key Remapper - Native Rebuild",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,
  CW_USEDEFAULT,CW_USEDEFAULT,585,525,nullptr,nullptr,hi,nullptr);if(!gWnd)return 2;
 ShowWindow(gWnd,show);UpdateWindow(gWnd);MSG msg;while(GetMessageW(&msg,nullptr,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}return (int)msg.wParam;
}